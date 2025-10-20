// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "common/Utils.h"

#include "mod/Config.h"

namespace mod {

class NetworkSettings : public QObject, public Singleton<NetworkSettings> {
    Q_OBJECT
public:
    enum Type { Socks5, HTTP };
    Q_ENUM(Type)

private:
    Q_PROPERTY(bool proxyEnabled READ getProxyEnabled WRITE setProxyEnabled NOTIFY proxyEnabledChanged);
    Q_PROPERTY(Type proxyType READ getProxyType WRITE setProxyType NOTIFY proxyTypeChanged);
    Q_PROPERTY(QString proxyHostName READ getProxyHostName WRITE setProxyHostName NOTIFY proxyHostNameChanged);
    Q_PROPERTY(int proxyPort READ getProxyPort WRITE setProxyPort NOTIFY proxyPortChanged);
    Q_PROPERTY(QString proxyUserName READ getProxyUserName WRITE setProxyUserName NOTIFY proxyUserNameChanged);
    Q_PROPERTY(QString proxyPassword READ getProxyPassword WRITE setProxyPassword NOTIFY proxyPasswordChanged);

    Q_PROPERTY(QString localIpAddress READ getLocalIpAddress NOTIFY networkChanged);
    Q_PROPERTY(QString netGateway READ getNetGateway NOTIFY networkChanged);
    Q_PROPERTY(QString dns READ getDNS NOTIFY networkChanged);

public:
    [[nodiscard]] bool getProxyEnabled() const;
    void               setProxyEnabled(bool enabled);

    [[nodiscard]] Type getProxyType() const;
    void               setProxyType(Type type);

    [[nodiscard]] QString getProxyHostName() const;
    void                  setProxyHostName(const QString& hostName);

    [[nodiscard]] int getProxyPort() const;
    void              setProxyPort(int port);

    [[nodiscard]] QString getProxyUserName() const;
    void                  setProxyUserName(const QString& userName);

    [[nodiscard]] QString getProxyPassword() const;
    void                  setProxyPassword(const QString& password);

    [[nodiscard]] QString getLocalIpAddress() const;
    [[nodiscard]] QString getNetGateway() const;
    [[nodiscard]] QString getDNS() const;

signals:

    void proxyEnabledChanged();
    void proxyTypeChanged();
    void proxyHostNameChanged();
    void proxyPortChanged();
    void proxyUserNameChanged();
    void proxyPasswordChanged();

    void networkChanged();

private:
    friend Singleton<NetworkSettings>;
    explicit NetworkSettings();

    std::string mClassName = "net";
    json        mCfg;

    bool    mProxyEnabled;
    Type    mProxyType;
    QString mProxyHostName;
    int     mProxyPort;
    QString mProxyUserName;
    QString mProxyPassword;

    void _refreshApplicationProxy();
};

} // namespace mod
