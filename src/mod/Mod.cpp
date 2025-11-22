// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "mod/Mod.h"

#include "base/YPointer.h"

#include "common/Event.h"
#include "common/Utils.h"
#include "common/util/System.h"

#include "Version.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QProcessEnvironment>
#include <QQmlContext>
#include <QQuickView>

namespace mod {

Mod::Mod() {

    connect(&Event::getInstance(), &Event::uiCompleted, this, &Mod::onUiCompleted);
    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("mod", this);
        qmlRegisterUncreatableType<PageIndex>(
            QML_PACKAGE_NAME,
            QML_PACKAGE_VERSION_MAJOR,
            QML_PACKAGE_VERSION_MINOR,
            "PageIndex",
            "Not creatable as it is an enum type."
        );
    });
}

bool Mod::isTrustedDevice() const { return true; }

QString Mod::getVersionStr() const { return VERSION_STRING; }

size_t Mod::getCachedSymCount() const { return SymDB::getInstance().count(); }

QString Mod::getBuildInfoStr() const { return BUILD_INFO_STRING; }

QString Mod::getOtherSlot() const {
    return exec("update_engine --misc=display").find("[0]->priority = 15") != std::string::npos ? "System B"
                                                                                                : "System A";
}

void Mod::changeSlot() { exec("update_engine --misc=other --reboot"); }

void Mod::uninstall() {
    try {
        QFile modLibrary(util::getModuleFileInfo().absoluteFilePath());
        QFile mainProcess(util::getApplicationFileInfo().absoluteFilePath());
        QFile mainProcessBak(util::getApplicationFileInfo().absolutePath() + "/YoudaoDictPen.original_bak");
        if (!mainProcessBak.exists()) throw std::runtime_error("无法还原主程序, 因为备份已丢失");
        if (!mainProcess.remove() || !mainProcessBak.rename("YoudaoDictPen"))
            throw std::runtime_error("无法还原主程序");
        if (!modLibrary.remove()) throw std::runtime_error("无法删除 Mod 动态库，但主程序已还原");
        softReboot();
    } catch (const std::exception& e) {
        showToast(e.what(), "#E9900C");
    }
}

void Mod::softReboot() { std::terminate(); }

void Mod::reboot() { exec("sync && reboot"); }

void Mod::onUiCompleted() const {

    // AutoFix vendor_storage.

    struct StoragedItem {
        QString mName;
        QString mType;
        QString mDefaultValue;
    };

    std::vector<StoragedItem> list = {
#if PL_BUILD_YDP02X
        {"VENDOR_COMPANY_ID",   "string", "COMPANY_HZ"             },
        // YDP021/022 满分版 16G
        {"VENDOR_CUSTOM_ID_0E", "string", "OVERHEAD_D2_SKU_EXA_ADV"}
    // YDP022 经典版 16G:   OVERHEAD_D2_SKU_CLA_ADV
    // YDP032 X3S 16G:     OVERHEAD_X3S_SKU_CHN_STD
    // YDP035 HLK STD:     OVERHEAD_D3_SKU_HILINK_STD
#endif
    };

    for (auto i : list) {
        if (exec(QString("vendor_storage -r %1 -t %2").arg(i.mName, i.mType)).find("vendor read error -1")
            != std::string::npos) {
            spdlog::warn("Automatically repairing vendor_storage: {}", i.mName.toStdString());
            exec(QString("vendor_storage -w %1 -t %2 -i %3").arg(i.mName, i.mType, i.mDefaultValue));
        }
    }

    // Set default read-write file system.

    exec("mount -o remount,rw /");
}

} // namespace mod

// Bypass the verification.

PEN_HOOK(bool, _ZNK15YSettingManager10isVerifiedEv, void* self) { return true; }

PEN_HOOK(bool, license_verify) { return true; }

// Setup mods.

#include "base/SymDB.h"
#include "base/YPointer.h"

#include "common/Downloader.h"
#include "common/Event.h"
#include "common/Resource.h"

#include "filemanager/FileManager.h"
#include "filemanager/player/MusicPlayer.h"
#include "filemanager/player/VideoPlayer.h"
#include "filemanager/reader/TextReader.h"
#include "filemanager/viewer/ImageViewer.h"

#include "helper/AntiEmbs.h"
#include "helper/DeveloperSettings.h"
#include "helper/NetworkSettings.h"
#include "helper/ServiceManager.h"

#include "locker/Locker.h"

#include "mod/Config.h"
#include "mod/Mod.h"
#include "mod/Updater.h"

#include "recorder/AudioRecorder.h"

#include "system/battery/BatteryInfo.h"
#include "system/input/InputDaemon.h"
#include "system/input/ScreenManager.h"
#include "system/sound/ASound.h"

#include "torch/Torch.h"

#include "tweaker/ColumnDBLimiter.h"
#include "tweaker/KeyBoard.h"
#include "tweaker/LoggerMonitor.h"
#include "tweaker/QueryTweaks.h"
#include "tweaker/TextBookHelper.h"
#include "tweaker/WordBookTweaks.h"

using namespace mod;

__attribute__((constructor)) static void BeforeMain() {

    // Setup global logger.

    auto global = spdlog::stdout_color_mt("Global");
#ifdef PL_DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif
    spdlog::set_pattern("[%H:%M:%S.%e] [%n] [%l] %v");
    spdlog::set_default_logger(global);

    // Setup mod instances.

#define INSTANCE(x) x::createInstance();

    // base
    INSTANCE(SymDB);
    INSTANCE(YPointerInitializer);

    // mod
    INSTANCE(Config);
    INSTANCE(Mod);
    INSTANCE(Updater);

    // common
    INSTANCE(Downloader);
    INSTANCE(Event);
    INSTANCE(Resource);

    // filemanager
    INSTANCE(filemanager::MusicPlayer);
    INSTANCE(filemanager::VideoPlayer);
    INSTANCE(filemanager::TextReader);
    INSTANCE(filemanager::FileManager);
    INSTANCE(filemanager::ImageViewer);

    // helper
    INSTANCE(AntiEmbs);
    INSTANCE(DeveloperSettings);
    INSTANCE(NetworkSettings);
    INSTANCE(ServiceManager);

    // locker
    INSTANCE(Locker);

    // recorder
    INSTANCE(AudioRecorder);

    // system
    INSTANCE(BatteryInfo);
    INSTANCE(InputDaemon);
    INSTANCE(ScreenManager);
    INSTANCE(ASound);

    // torch
    INSTANCE(Torch);

    // tweaker
    INSTANCE(ColumnDBLimiter);
    INSTANCE(KeyBoard);
    INSTANCE(LoggerMonitor);
    INSTANCE(QueryTweaks);
    INSTANCE(TextBookHelper);
    INSTANCE(WordBookTweaks);

#undef INSTANCE
}
