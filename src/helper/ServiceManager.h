// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "mod/Config.h"

namespace mod {

class ServiceManager : public QObject, public Singleton<ServiceManager> {
    Q_OBJECT

    Q_PROPERTY(bool adbStatus READ getAdbStatus NOTIFY adbStatusChanged);
    Q_PROPERTY(bool sshStatus READ getSshStatus NOTIFY sshStatusChanged);
    Q_PROPERTY(bool adbAutoRun READ getAdbAutoRun WRITE setAdbAutoRun NOTIFY adbAutoRunChanged);
    Q_PROPERTY(bool skipAdbVerification READ getSkipAdbVerification WRITE setSkipAdbVerification NOTIFY
                   skipAdbVerificationChanged);
    Q_PROPERTY(bool sshAutoRun READ getSshAutoRun WRITE setSshAutoRun NOTIFY sshAutoRunChanged);

public:
    Q_INVOKABLE bool startAdb(bool dontShowToast = false);

    Q_INVOKABLE bool startSsh(bool dontShowToast = false);

    Q_INVOKABLE bool stopAdb(bool dontShowToast = false);

    Q_INVOKABLE bool stopSsh(bool dontShowToast = false);

    Q_INVOKABLE bool setSshRootPasswd(const QString&);

    void onUiCompleted();

    bool getAdbStatus() const;

    bool getSshStatus() const;

    bool getSkipAdbVerification() const;

    bool getAdbAutoRun() const;

    bool getSshAutoRun() const;

    void setSkipAdbVerification(bool);

    void setAdbAutoRun(bool);

    void setSshAutoRun(bool);

signals:

    void adbStatusChanged();

    void sshStatusChanged();

    void skipAdbVerificationChanged();

    void adbAutoRunChanged();

    void sshAutoRunChanged();

private:
    friend Singleton<ServiceManager>;
    explicit ServiceManager();

    std::string mClassName = "serv";
    json        mCfg;

    bool mAdbAutoRun;
    bool mSkipAdbVerification;
    bool mSshAutoRun;

    void _passAdbVerification();

    std::string _getRandomString(uint32 length);
};

} // namespace mod
