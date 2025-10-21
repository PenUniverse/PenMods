// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "common/service/Logger.h"

#include "Version.h"

#include <nlohmann/json.hpp>

#include "base/YEnum.h"

using json = nlohmann::json;

namespace mod {

struct Version {
    bool                  operator==(Version) const;
    bool                  operator>(Version) const;
    bool                  operator<(Version) const;
    [[nodiscard]] QString toString() const;
    [[nodiscard]] uint32  toNumber() const;
    uint32                mMajor;
    uint32                mMinor;
    uint32                mRevision;
};

class Updater : public QObject, public Singleton<Updater>, private Logger {
    Q_OBJECT

    struct UpdateInfo;

public:
    Q_INVOKABLE void check();

    Q_INVOKABLE void download();

    Q_INVOKABLE void install();

    void onUiCompleted();

    UpdateInfo& getInfo();

private:
    friend Singleton<Updater>;
    explicit Updater();

    std::string mClassName = "updater";

    // latest.
    json mLatestObject;

    const Version mSelfVersion = {VERSION_MAJOR, VERSION_MINOR, VERSION_ALTER};

    struct UpdateInfo {
        QString      mCurrentStr       = "";
        QString      mLatestStr        = "";
        UpdateStatus mOtaStatus        = UpdateStatus::LATEST_VERSION;
        QString      mUpdateNote       = "";
        double       mPackageSize      = 0.0;
        uint32       mDownloadProgress = 0;
        uint32       mInstallProgress  = 0;
    } mInfo;

    std::string _calcFileMd5(const QString&);

    void _unzip(QString, QString);

    void _cleanupTemp();

    void _setCurrentVersion(QString);

    void _setLatestVersion(QString);

    void _setUpdateNote(QString);

    void _setUpdatePackSize(double);

    void _setDownloadProgress(int);

    void _setOtaStatus(UpdateStatus);

    void _setInstallProgress(int);
};

} // namespace mod
