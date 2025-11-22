// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "filemanager/FileManager.h"
#include "filemanager/player/MusicPlayer.h"

#include "common/Event.h"
#include "common/Utils.h"
#include "common/service/Logger.h"

#include "mod/Mod.h"

#include "tweaker/ColumnDBLimiter.h"

#include <QFile>
#include <QHash>
#include <QQmlContext>
#include <QTimer>
#include <QUrl>

namespace mod::filemanager {

static const char* HIDDEN_FLAG = ".HIDDEN_DIR";
static size_t      MAX_FILES   = 65535;

FileManager::FileManager() : QAbstractListModel(), Logger("FileManager") {

    mCfg = Config::getInstance().read(mClassName);

    mOrder            = mCfg["order"]["basic"];
    mOrderReversed    = mCfg["order"]["reversed"];
    mHidePairedLyrics = mCfg["hide_paired_lyrics"];
    mShowHiddenFiles  = mCfg["show_hidden_files"];

    connect(&mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &FileManager::onDirectoryChanged);
    connect(&Event::getInstance(), &Event::uiCompleted, [this]() {
        if (shouldHiddenAll()) {
            QTimer::singleShot(15000, this, [&]() { setMtpOnoff(false); });
        }
    });
    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("fileManager", this);
    });
}

int FileManager::rowCount(const QModelIndex& parent) const { return mProxyCount; };

QVariant FileManager::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }

    const size_t row = index.row();
    if (row < 0 || row > (size_t)mProxyCount - 1) {
        return {};
    }

    auto& entity = mEntities.at(row);

    auto getSizeString = [&]() -> QString {
        auto size = entity->size();
        if (size <= 0) {
            return "0B";
        }
        QList<QString> units = {"B", "KB", "MB", "GB"};
        for (int i = 1; i < units.size() + 1; i++) {
            auto step = pow(1024, i);
            if ((double)size <= step) {
                return QString::number((double)size / step * 1024.0, 'f', 0) + units.at(i - 1);
            }
        } // dict pen's largest storage size is 32GB, lol.
        const_cast<FileManager*>(this)->error("Abnormal file size({}) detected!", size);
        return "-1B";
    };

    auto getExtIcon = [&]() -> QString {
        if (entity->isDir()) {
            return "qrc:/images/folder-empty.png";
        }
        auto    ext = entity->suffix().toLower();
        QString name;
        switch (H(ext.toUtf8())) {
        case H("mp3"):
            name = "mp3";
            break;
        case H("md"):
            name = "md";
            break;
        case H("txt"):
        case H("lrc"):
            name = "txt";
            break;
        case H("json"):
            name = "json";
            break;
        case H("yml"):
        case H("yaml"):
        case H("xml"):
            name = "xml";
            break;
        case H("avi"):
        case H("mp4"):
        case H("mov"):
        case H("flv"):
        case H("mkv"):
        case H("webm"):
            name = "mp4";
            break;
        default:
            return "qrc:/images/file-empty.png";
        }
        return QString("qrc:/images/format/suffix-%1.png").arg(name);
    };

    switch ((UserRoles)role) {
    case UserRoles::FileName:
        return entity->fileName();
    case UserRoles::IsDirectory:
        return entity->isDir();
    case UserRoles::SizeString:
        return getSizeString();
    case UserRoles::ExtensionName:
        return entity->suffix().toLower();
    case UserRoles::ExtensionIcon:
        return getExtIcon();
    default:
        return {};
    }
};

QHash<int, QByteArray> FileManager::roleNames() const {
    return QHash<int, QByteArray>{
        {(int)UserRoles::FileName,      "fileName"},
        {(int)UserRoles::IsDirectory,   "isDir"   },
        {(int)UserRoles::SizeString,    "sizeStr" },
        {(int)UserRoles::ExtensionName, "extName" },
        {(int)UserRoles::ExtensionIcon, "extIcon" }
    };
};

void FileManager::onDirectoryChanged(const QString& path) {
    debug("dir-changed: {}", path.toStdString());
    auto url = QUrl(path);
    debug("url.path() {}", url.path().toStdString());
    debug("cur.path() {}", mCurrentPath.path().toStdString());
    if (url.path() == mCurrentPath.path()) {
        emit directoryChanged();
    }
    if (path == mCurrentPlayingPath.path()) {
        refreshPlayList();
    }
}

QDir const& FileManager::getCurrentPath() const { return mCurrentPath; }

bool FileManager::changeDir(const QString& dir) {
    if (dir.isEmpty()) {
        return changeDir(mRoot);
    }
    debug("Move to dir -> {}", dir.toStdString());
    mFileSystemWatcher.removePath(mCurrentPath.absolutePath());
    if (!mCurrentPath.cd(dir)) {
        return false;
    }
    emit currentTitleChanged();
    reset();
    _initCurrentDir();
    loadMore();
    if (!mFileSystemWatcher.addPath(mCurrentPath.absolutePath())) {
        debug("failed to add path watcher");
    } else {
        debug("path watcher added");
    }
    return true;
}

bool FileManager::canCdUp() const {
    auto curLength  = mCurrentPath.path().length();
    auto rootLength = mRoot.length();
    return curLength != rootLength && curLength - 1 != rootLength;
}

void FileManager::loadMore() { loadMore(ColumnDBLimiter::getInstance().getLimit()); }

void FileManager::loadMore(int amount) {
    // Do not use 'isHasMore()' here!
    auto finalCount = std::min(mProxyCount + amount, (int)mEntities.size());
    beginInsertRows(QModelIndex(), mProxyCount, finalCount - 1);
    mProxyCount = finalCount; // safe: limited by MAX_FILES.
    endInsertRows();
    emit hasMoreChanged();
}

void FileManager::reload() { changeDir("."); }

void FileManager::reset() {
    beginResetModel();
    mEntities.clear();
    mProxyCount = 0;
    endResetModel();
    emit hasMoreChanged();
}

void FileManager::remove(const QString& fileName) {
    if (!mCurrentPath.exists(fileName)) {
        return;
    }
    int idx = 0;
    for (const auto& i : mEntities) {
        if (i->fileName() != fileName) {
            idx++;
            continue;
        }
        if (i->isDir()) {
            exec(QString("rm -rf \"%1\"").arg(mCurrentPath.absoluteFilePath(fileName)));
        } else {
            mCurrentPath.remove(fileName);
        }
        mEntities.erase(mEntities.begin() + idx);
        beginRemoveRows(QModelIndex(), idx, idx);
        mProxyCount--;
        endRemoveRows();
        markSuspendDirChangedNotifier();
        emit hasMoreChanged();
        break;
    }
}

void FileManager::rename(const QString& fileName, const QString& newFileName) {
    if (!judgeIsLegalFileName(newFileName)) {
        showToast("文件名不能包含特殊字符", "#E9900C");
        return;
    }
    auto newFilePath = mCurrentPath.absoluteFilePath(newFileName);
    markSuspendDirChangedNotifier();
    if (!mCurrentPath.rename(fileName, newFileName)) {
        showToast("修改失败", "#E9900C");
        return;
    }
    auto idx = 0;
    for (auto& i : mEntities) {
        if (i->fileName() == fileName) {
            i->setFile(newFilePath);
            auto midx = index(idx);
            emit dataChanged(midx, midx);
            break;
        }
        idx++;
    }
}

bool FileManager::shouldHiddenAll() const { return QFile(QString("%1/%2").arg(mRoot, HIDDEN_FLAG)).exists(); }

void FileManager::negateHiddenAll() {
    QFile file(QString("%1/%2").arg(mRoot, HIDDEN_FLAG));
    if (file.exists()) {
        file.remove();
        setMtpOnoff(true);
    } else {
        file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        file.write("This directory has been hidden.");
        file.close();
        setMtpOnoff(false);
    }
}

void FileManager::markSuspendDirChangedNotifier() {
    mShouldNotifyDirChanged = false;
    QTimer::singleShot(5000, this, [&]() { mShouldNotifyDirChanged = true; });
}

void FileManager::setMtpOnoff(bool onoff) {
    if (onoff && shouldHiddenAll()) {
        return;
    }
    if (onoff) {
        exec("grep usb_mtp_en /tmp/.usb_config || echo usb_mtp_en >> /tmp/.usb_config");
    } else {
        exec("sed -i '/usb_mtp_en/d' /tmp/.usb_config");
    }
    exec("/etc/init.d/S98usbdevice restart");
}

int FileManager::getOrder() const { return mOrder; }

bool FileManager::getOrderReversed() const { return mOrderReversed; }

void FileManager::setOrder(int order) {
    if (mOrder != order) {
        mOrder                 = order;
        mCfg["order"]["basic"] = order;
        WRITE_CFG;
        emit orderChanged();
    }
}

void FileManager::setOrderReversed(bool val) {
    if (mOrderReversed != val) {
        mOrderReversed            = val;
        mCfg["order"]["reversed"] = val;
        WRITE_CFG;
        emit orderReversedChanged();
    }
}

QString FileManager::getCurrentTitle() const {
    auto    path = mCurrentPath.path().remove(0, mRoot.size());
    QString ret  = "(根目录)";
    auto    list = path.split("/", Qt::SkipEmptyParts);
    for (auto i = 0; i < list.size(); i++) {
        auto part = " ﹥ " + list[i];
        if (i == list.size() - 1) { // final
            part = "<font color=\"white\">" + part + "</font>";
        }
        ret += part;
    }
    return ret;
}

bool FileManager::isHasMore() const {
    return !mEntities.empty() && (int)mEntities.size() > mProxyCount; // safe.
}

void FileManager::_initCurrentDir() {

    if (Mod::getInstance().isTrustedDevice() && shouldHiddenAll()) {
        emit error("空文件夹");
        reset();
        return;
    }

    auto order = getOrder();
    if (getOrderReversed()) {
        order |= QDir::Reversed;
    }
    order     = order | QDir::DirsFirst | QDir::IgnoreCase;
    auto flags = QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot;
    if (getShowHiddenFiles()) {
        flags |= QDir::Hidden;
    }
    auto list = mCurrentPath.entryInfoList(
        flags,
        static_cast<QDir::SortFlags>(order)
    );

    if (list.empty()) {
        emit error("空文件夹");
        reset();
        return;
    }

    // To prevent memory overuse.
    if ((size_t)list.size() > MAX_FILES) {
        emit error("该目录下文件太多");
        reset();
        return;
    }

    // For paired lyrics auto-hidden.
    std::vector<QString> pairedLyrics;
    if (getHidePairedLyrics()) {
        for (auto& i : list) {
            auto path = i.absoluteFilePath();
            if (path.endsWith(".mp3", Qt::CaseInsensitive)) { // Current only support mp3 format.
                auto lrcPath = path.mid(0, path.length() - 4) + ".lrc";
                if (QFile(lrcPath).exists()) {
                    pairedLyrics.emplace_back(lrcPath);
                }
            }
        }
    }

    for (auto& i : list) {
        if (std::find(pairedLyrics.begin(), pairedLyrics.end(), i.absoluteFilePath()) != pairedLyrics.end()) {
            continue;
        }
        if (!getShowHiddenFiles() && i.fileName().startsWith('.')) {
            continue;
        }
        mEntities.emplace_back(std::make_shared<QFileInfo>(i));
    }
}

void FileManager::forEachLoadedEntities(const std::function<void(std::shared_ptr<QFileInfo>)>& callback) {
    for (const auto& i : mEntities) {
        callback(i);
    }
}

// MusicPlayer

bool FileManager::getHidePairedLyrics() const { return mHidePairedLyrics; }

void FileManager::setHidePairedLyrics(bool val) {
    if (mHidePairedLyrics != val) {
        mHidePairedLyrics          = val;
        mCfg["hide_paired_lyrics"] = val;
        WRITE_CFG;
        emit hidePairedLyricsChanged();
    }
}

bool FileManager::getShowHiddenFiles() const { return mShowHiddenFiles; }

void FileManager::setShowHiddenFiles(bool val) {
    if (mShowHiddenFiles != val) {
        mShowHiddenFiles          = val;
        mCfg["show_hidden_files"] = val;
        WRITE_CFG;
        emit showHiddenFilesChanged();
    }
}

void FileManager::playFromView(const QString& fileName) {
    if (mCurrentPlayingPath != mCurrentPath) {
        refreshPlayList();
        mFileSystemWatcher.removePath(mCurrentPlayingPath.absolutePath());
        mCurrentPlayingPath = mCurrentPath;
        mFileSystemWatcher.addPath(mCurrentPlayingPath.absolutePath());
    }
    size_t idx   = 0;
    bool   valid = false;
    for (auto& file : MusicPlayer::getInstance().getPlayListRef()) {
        if (file->fileName() == fileName) {
            valid = true;
            break;
        }
        idx++;
    }
    if (valid) MusicPlayer::getInstance().play(idx);
}

void FileManager::refreshPlayList() {
    auto& list = MusicPlayer::getInstance().getPlayListRef();
    list.clear();
    forEachLoadedEntities([&](const std::shared_ptr<QFileInfo>& file) {
        if (file->suffix().toLower() == "mp3") {
            list.emplace_back(file);
        }
    });
}
} // namespace mod::filemanager

PEN_HOOK(void, _ZN13YMediaManager13entryMyImportEv, uint64) {}

PEN_HOOK(void, _ZN13YMediaManager16entryMyImportDirERK7QString, uint64, const QString& a2) {}

PEN_HOOK(void, _ZN13YMediaManager20launchMyImportMediasEv, uint64) {}
