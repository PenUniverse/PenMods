// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "mod/Config.h"

#include "common/service/Logger.h"

#include <QAbstractListModel>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>

namespace mod::filemanager {

class FileManager : public QAbstractListModel, public Singleton<FileManager>, private Logger {
    Q_OBJECT

    Q_PROPERTY(QString currentTitle READ getCurrentTitle NOTIFY currentTitleChanged);
    Q_PROPERTY(int order READ getOrder WRITE setOrder NOTIFY orderChanged);
    Q_PROPERTY(bool orderReversed READ getOrderReversed WRITE setOrderReversed NOTIFY orderReversedChanged);
    Q_PROPERTY(bool hasMore READ isHasMore NOTIFY hasMoreChanged);

    // MusicPlayer
    Q_PROPERTY(bool hidePairedLyrics READ getHidePairedLyrics WRITE setHidePairedLyrics NOTIFY hidePairedLyricsChanged);

public:
    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    // The listener of QFileSystemWatcher,
    // Used to watch currentDir and currentPlayingDir.
    void onDirectoryChanged(const QString& path);

    [[nodiscard]] QDir const& getCurrentPath() const;

    Q_INVOKABLE bool changeDir(const QString& dir);

    Q_INVOKABLE [[nodiscard]] bool canCdUp() const;

    Q_INVOKABLE void loadMore();

    void loadMore(int amount);

    // reset and load to current position.
    Q_INVOKABLE void reload();

    Q_INVOKABLE void reset();

    Q_INVOKABLE void remove(const QString& fileName);

    Q_INVOKABLE void rename(const QString& fileName, const QString& newFileName);

    [[nodiscard]] bool shouldHiddenAll() const;

    void negateHiddenAll();

    void markSuspendDirChangedNotifier();

    Q_INVOKABLE bool shouldNotifyDirChanged() { return mShouldNotifyDirChanged; };

    void setMtpOnoff(bool);

    [[nodiscard]] int getOrder() const;

    void setOrder(int);

    [[nodiscard]] bool getOrderReversed() const;

    void setOrderReversed(bool);

    [[nodiscard]] QString getCurrentTitle() const;

    [[nodiscard]] bool isHasMore() const;

    void forEachLoadedEntities(const std::function<void(std::shared_ptr<QFileInfo>)>& callback);

    // MusicPlayer

    [[nodiscard]] bool getHidePairedLyrics() const;

    void setHidePairedLyrics(bool);

    Q_INVOKABLE void playFromView(const QString& fileName);

signals:

    void currentTitleChanged();

    void orderChanged();

    void orderReversedChanged();

    void directoryChanged();

    void hasMoreChanged();

    void exception(const QString& msg);

    // MusicPlayer

    void hidePairedLyricsChanged();

private:
    friend Singleton<FileManager>;
    explicit FileManager();

    // FileManager

    enum class UserRoles { FileName = Qt::UserRole + 1, IsDirectory, SizeString, ExtensionName, ExtensionIcon };

    std::string mClassName{"fm"};
    json        mCfg;

    const QString mRoot{"/userdisk/Music"};

    QFileSystemWatcher mFileSystemWatcher;
    std::atomic<bool>  mShouldNotifyDirChanged{true};

    int  mOrder;
    bool mOrderReversed;

    QDir                                    mCurrentPath;
    std::vector<std::shared_ptr<QFileInfo>> mEntities;
    int                                     mProxyCount{};

    void _initCurrentDir();

    // MusicPlayer

    bool mHidePairedLyrics;

    QDir mCurrentPlayingPath;

    void refreshPlayList();
}; // namespace mod::filemanager
} // namespace mod::filemanager
