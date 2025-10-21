// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "Updater.h"

#include "base/YPointer.h"

#include "common/Downloader.h"
#include "common/Event.h"
#include "common/Utils.h"
#include "common/util/System.h"

#include <QDir>
#include <QQmlContext>
#include <QStorageInfo>

#define UH_TEMP_PATH    "/tmp/modupdate/"
#define UH_TEMP_DIRNAME "modupdate"

#if PL_BUILD_YDP02X
#define UH_MAIN_URL "https://dictpen-backend.0xffffffffffffffff.xyz/"
#endif

namespace mod {

Updater::Updater() : Logger("Updater") {
    connect(&Event::getInstance(), &Event::uiCompleted, this, &Updater::onUiCompleted);
    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("updater", this);
    });
}

QString Version::toString() const {
    spdlog::warn("1.1");
    return QString("%1.%2.%3").arg(QString::number(mMajor), QString::number(mMinor), QString::number(mRevision));
}

uint32 Version::toNumber() const { return mMajor * 100 + mMinor * 10 + mRevision; }

bool Version::operator==(const Version b) const {
    return mMajor == b.mMajor && mMinor == b.mMinor && mRevision == b.mRevision;
}

bool Version::operator>(const Version b) const { return toNumber() > b.toNumber(); }

bool Version::operator<(const Version b) const { return toNumber() < b.toNumber(); }

void Updater::check() {

    info("Starting to check update...");
    auto power = PEN_CALL(uint32, "_ZN15YBatteryManager5powerEv", void*)(YPointer<YBatteryManager>::getInstance());
    if (power < 10) {
        error("Checking for updates stalled: Low battery.");
        _setOtaStatus(ERROR_LOW_BATTERY);
        return;
    }
    _setOtaStatus(CHECKING);
    _cleanupTemp();
    Downloader::getInstance().createTask(
        UH_MAIN_URL,
        UH_TEMP_PATH "version_list.temp",
        nullptr,
        [&](Downloader::TaskId, const QString& savedPath, Downloader::ResultStatus stat) {
            switch (stat) {
            case Downloader::ResultStatus::ERROR_FAIL_TO_START:
                error("Checking for updates stalled: Fail to start.");
            case Downloader::ResultStatus::ERROR_STOPPED:
                error("Checking for updates stalled: Stopped by user.");
            case Downloader::ResultStatus::ERROR_UNKNOWN:
                error("Checking for updates stalled: Unknwon error.");
                _setOtaStatus(ERROR_NO_CONNECTION);
                return;
            case Downloader::ResultStatus::ERROR_FAIL_OPEN_FILE:
                error("Checking for updates stalled: Fail to open file.");
            case Downloader::ResultStatus::ERROR_FAIL_STORAGE_INVALID:
                error("Checking for updates stalled: No enough space(1).");
                _setOtaStatus(ERROR_NO_ENOUGH_MEMORY);
                return;
            case Downloader::ResultStatus::SUCCEED:
                break;
            }
            QFile file(savedPath);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                error("Checking for updates stalled: No network connection");
                _setOtaStatus(ERROR_NO_CONNECTION);
                return;
            }
            auto result = file.readAll().toStdString();
            try {
                auto j = json::parse(result);
                if (j["status"] != 200) {
                    throw std::runtime_error("status != 200");
                }
                // matchSelfVersion -> needGoNextVersion? -> latestDirectly / nextVersion
                bool selfVersionDetected = false;
                auto topVer              = mSelfVersion;
                for (auto i : j["versions"]) {
                    auto that = Version{i["version"][0], i["version"][1], i["version"][2]};
                    if (that > topVer) {
                        topVer = that;
                    } else if (that == mSelfVersion) {
                        selfVersionDetected = true;
                        if (i.find("next") != i.end()) {
                            topVer = Version{i["next"][0], i["next"][1], i["next"][2]};
                            break;
                        }
                    }
                }
                if (!selfVersionDetected) {
                    throw std::runtime_error("Cannot get self version from mod_versions.");
                }
                for (auto i : j["versions"]) {
                    auto that = Version{i["version"][0], i["version"][1], i["version"][2]};
                    if (that == topVer) {
                        mLatestObject = i;
                        break;
                    }
                }
                _setLatestVersion(topVer.toString());
                if (topVer > mSelfVersion) {
                    auto   avail = QStorageInfo(UH_TEMP_PATH).bytesAvailable();
                    qint64 bytes = mLatestObject["size"];
                    if (avail == -1 || avail < bytes) {
                        error("Checking for updates stalled: No enough space(2).");
                        _setOtaStatus(ERROR_NO_ENOUGH_MEMORY);
                        return;
                    }
                    _setUpdateNote(QString::fromStdString(mLatestObject["note"]));
                    _setUpdatePackSize((double)bytes / 1024.0 / 1024.0);
                    info("New version({}) founded!", topVer.toString().toStdString());
                    _setOtaStatus(HAS_NEW_VERSION);
                } else {
                    info("Already the latest version.");
                    _setOtaStatus(LATEST_VERSION);
                }
            } catch (const std::exception& e) {
                error("An error occurred while parsing the version information.");
                error(e.what());
                _setOtaStatus(ERROR_NO_CONNECTION);
            }
        }
    );
}

Updater::UpdateInfo& Updater::getInfo() { return mInfo; }

void Updater::download() {

    _setOtaStatus(DOWNLOADING);
    Downloader::getInstance().createTask(
        QString::fromStdString(mLatestObject["download"]),
        UH_TEMP_PATH "download.temp",
        [&](Downloader::TaskId, int prog) { _setDownloadProgress(prog); },
        [&](Downloader::TaskId, const QString& savedPath, Downloader::ResultStatus stat) {
            if (stat == Downloader::ResultStatus::SUCCEED) {
                _setOtaStatus(DOWNLOAD_FINISHED);
            } else {
                _setOtaStatus(DOWNLOAD_FAILED);
            }
        }
    );
}

void Updater::install() {

    _setOtaStatus(MD5_CHECKING);
    if (_calcFileMd5(UH_TEMP_PATH "download.temp")
        != QString::fromStdString(mLatestObject["md5"]).toLower().toStdString()) {
        _setOtaStatus(ERROR_MD5_CHECK);
    } else {
        _unzip(UH_TEMP_PATH "download.temp", UH_TEMP_PATH);
        if (!QFile(UH_TEMP_PATH "_do_update.sh").exists()) {
            _setOtaStatus(ERROR_MD5_CHECK);
        } else {
            _setOtaStatus(MD5_CHECK_SUCCESSFULLY);
            _setOtaStatus(INSTALLING);
            exec(QString("chmod +x \"%1\"").arg(UH_TEMP_PATH "_do_update.sh"));
            exec(QString("cd \"%1\" && bash _do_update.sh").arg(UH_TEMP_PATH));
            _setInstallProgress(100);
            if (QFile(UH_TEMP_PATH "INSTALL_SUCCESSFULLY").exists()) {
                _setOtaStatus(INSTALL_SUCCESSFULLY);
            } else {
                _setOtaStatus(INSTALL_FAILED);
            }
        }
    }
}

void Updater::onUiCompleted() {

    _cleanupTemp();

    _setCurrentVersion(mSelfVersion.toString());
}

std::string Updater::_calcFileMd5(const QString& path) {
    return exec(QString(R"(md5sum -b "%1" | cut -d" " -f1)").arg(path));
}

void Updater::_unzip(QString zipPath, QString toWhere) {
    exec(QString(R"(unzip -q -o "%1" -d "%2")").arg(zipPath, toWhere));
}

void Updater::_cleanupTemp() {
    util::getModuleFileInfo().absoluteDir().rmdir(UH_TEMP_DIRNAME);
    QDir(UH_TEMP_PATH).mkpath(".");
}

void Updater::_setCurrentVersion(QString verstr) {
    mInfo.mCurrentStr = std::move(verstr);
    PEN_CALL(void*, "_ZN14YUpdateManager23onCurrentVersionChangedEv", void*)(YPointer<YUpdateManager>::getInstance());
}

void Updater::_setLatestVersion(QString latest) {
    mInfo.mLatestStr = std::move(latest);
    PEN_CALL(void*, "_ZN14YUpdateManager22onTargetVersionChangedEv", void*)(YPointer<YUpdateManager>::getInstance());
}

void Updater::_setUpdateNote(QString note) {
    mInfo.mUpdateNote = std::move(note);
    PEN_CALL(void*, "_ZN14YUpdateManager19onUpdateNoteChangedEv", void*)(YPointer<YUpdateManager>::getInstance());
}

void Updater::_setUpdatePackSize(double size) {
    mInfo.mPackageSize                                                         = size;
    *(double*)(*((uint64*)YPointer<YUpdateManager>::getInstance() + 2) + 64LL) = dec(size, 1);
    PEN_CALL(void*, "_ZN14YUpdateManager20updateImgSizeChangedEv", void*)(YPointer<YUpdateManager>::getInstance());
}

void Updater::_setOtaStatus(UpdateStatus stat) {
    mInfo.mOtaStatus = stat;
    PEN_CALL(void*, "_ZN14YUpdateManager18onOtaStatusChangedEv", void*)(YPointer<YUpdateManager>::getInstance());
}

void Updater::_setDownloadProgress(int prog) { mInfo.mDownloadProgress = prog; }

void Updater::_setInstallProgress(int prog) { mInfo.mInstallProgress = prog; }

} // namespace mod

PEN_HOOK(QString, _ZN14YUpdateManager14currentVersionEv, uint64 self, void* a2) {
    return mod::Updater::getInstance().getInfo().mCurrentStr;
}

PEN_HOOK(QString, _ZN14YUpdateManager13targetVersionEv, uint64 self, void* a2) {
    return mod::Updater::getInstance().getInfo().mLatestStr;
}

PEN_HOOK(UpdateStatus, _ZN14YUpdateManager9otaStatusEv, uint64) {
    return mod::Updater::getInstance().getInfo().mOtaStatus;
}

PEN_HOOK(QString, _ZN14YUpdateManager10updateNoteEv, uint64 self, void* a2, void* a3) {
    return mod::Updater::getInstance().getInfo().mUpdateNote;
}

// Disabled Functions;

PEN_HOOK(void, _ZN14YUpdateManager7installEv, uint64 self) {}

PEN_HOOK(void, _ZN14YUpdateManager13downloadStartEv, uint64 self) {}

PEN_HOOK(void, _ZN14YUpdateManager13downloadPauseEv, uint64 self) {}

PEN_HOOK(void, _ZN14YUpdateManager14downloadResumeEv, uint64 self) {}

PEN_HOOK(bool, _ZN21YUpdateManagerPrivate25checkOtaOperateThreadFuncEv, uint64 self) { return false; }

PEN_HOOK(bool, _ZN21YUpdateManagerPrivate32checkInstallOtaOperateThreadFuncEv, uint64 self) { return false; }

PEN_HOOK(uint64, _ZN21YUpdateManagerPrivate15checkOtaInstallEv, uint64 self) { return 0; }

PEN_HOOK(void, _ZN21YUpdateManagerPrivate14checkOtaUpdateEv, uint64 self) {}

PEN_HOOK(void, _ZN21YUpdateManagerPrivate19onPowerStatusChangeEiRK7QString, void* self, void* a2, void* a3) {}
