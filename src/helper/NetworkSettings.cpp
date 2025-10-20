// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "helper/NetworkSettings.h"

#include "common/Event.h"

#include <QNetworkProxy>
#include <QQmlContext>

namespace mod {

NetworkSettings::NetworkSettings() {

    mCfg = Config::getInstance().read(mClassName);

    mProxyEnabled  = mCfg["proxy_enabled"];
    mProxyType     = mCfg["proxy_type"];
    mProxyHostName = QString::fromStdString(mCfg["proxy_hostname"]);
    mProxyPort     = mCfg["proxy_port"];
    mProxyUserName = QString::fromStdString(mCfg["proxy_username"]);
    mProxyPassword = QString::fromStdString(mCfg["proxy_password"]);

    connect(this, &NetworkSettings::proxyEnabledChanged, this, &NetworkSettings::_refreshApplicationProxy);
    connect(this, &NetworkSettings::proxyTypeChanged, this, &NetworkSettings::_refreshApplicationProxy);
    connect(this, &NetworkSettings::proxyHostNameChanged, this, &NetworkSettings::_refreshApplicationProxy);
    connect(this, &NetworkSettings::proxyPortChanged, this, &NetworkSettings::_refreshApplicationProxy);
    connect(this, &NetworkSettings::proxyUserNameChanged, this, &NetworkSettings::_refreshApplicationProxy);
    connect(this, &NetworkSettings::proxyPasswordChanged, this, &NetworkSettings::_refreshApplicationProxy);

    _refreshApplicationProxy();

    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("networkSettings", this);
    });
}

bool NetworkSettings::getProxyEnabled() const { return mProxyEnabled; }
void NetworkSettings::setProxyEnabled(bool enabled) {
    if (mProxyEnabled != enabled) {
        mProxyEnabled         = enabled;
        mCfg["proxy_enabled"] = enabled;
        WRITE_CFG;
        emit proxyEnabledChanged();
    }
}

NetworkSettings::Type NetworkSettings::getProxyType() const { return mProxyType; }
void                  NetworkSettings::setProxyType(Type type) {
    if (mProxyType != type) {
        mProxyType         = type;
        mCfg["proxy_type"] = type;
        WRITE_CFG;
        emit proxyTypeChanged();
    }
}

QString NetworkSettings::getProxyHostName() const { return mProxyHostName; }
void    NetworkSettings::setProxyHostName(const QString& hostName) {
    if (mProxyHostName != hostName) {
        mProxyHostName         = hostName;
        mCfg["proxy_hostname"] = hostName.toStdString();
        WRITE_CFG;
        emit proxyHostNameChanged();
    }
}

int  NetworkSettings::getProxyPort() const { return mProxyPort; }
void NetworkSettings::setProxyPort(int port) {
    if (mProxyPort != port) {
        mProxyPort         = port;
        mCfg["proxy_port"] = port;
        WRITE_CFG;
        emit proxyPortChanged();
    }
}

QString NetworkSettings::getProxyUserName() const { return mProxyUserName; }
void    NetworkSettings::setProxyUserName(const QString& userName) {
    if (mProxyUserName != userName) {
        mProxyUserName         = userName;
        mCfg["proxy_username"] = userName.toStdString();
        WRITE_CFG;
        emit proxyUserNameChanged();
    }
}

QString NetworkSettings::getProxyPassword() const { return mProxyPassword; }
void    NetworkSettings::setProxyPassword(const QString& password) {
    if (mProxyPassword != password) {
        mProxyPassword         = password;
        mCfg["proxy_password"] = password.toStdString();
        WRITE_CFG;
        emit proxyPasswordChanged();
    }
}

void NetworkSettings::_refreshApplicationProxy() {
    QNetworkProxy proxy;
    proxy.setType([this]() {
        if (!mProxyEnabled) {
            return QNetworkProxy::NoProxy;
        }
        switch (mProxyType) {
        case Socks5:
            return QNetworkProxy::Socks5Proxy;
        case HTTP:
            return QNetworkProxy::HttpProxy;
        default:
            return QNetworkProxy::NoProxy;
        }
    }());
    proxy.setHostName(mProxyHostName);
    proxy.setPort(mProxyPort);
    if (mProxyUserName.isEmpty()) {
        proxy.setUser(mProxyUserName);
        proxy.setPassword(mProxyPassword);
    }
    QNetworkProxy::setApplicationProxy(proxy);
}

QString NetworkSettings::getLocalIpAddress() const {
    auto ip = exec("/sbin/ifconfig -a|grep inet|grep -v 127.0.0.1|grep -v inet6|awk '{print $2}'|tr -d 'addr:'");
    return QString::fromStdString(ip.empty() ? "不可用" : ip);
}

QString NetworkSettings::getNetGateway() const {
    auto gateway = exec("ip route | grep default | awk '{print $3}'");
    return QString::fromStdString(gateway.empty() ? "不可用" : gateway);
}

QString NetworkSettings::getDNS() const {
    auto dns = exec("grep \"nameserver\" /etc/resolv.conf | awk '{print $2}'");
    return QString::fromStdString(dns.empty() ? "不可用" : dns);
}

} // namespace mod

PEN_HOOK(void*, _ZN12YWifiManager22internetConnectChangedEv, void* a1, void* a2, void* a3, void* a4, void* a5) {
    emit mod::NetworkSettings::getInstance().networkChanged();
    return origin(a1, a2, a3, a4, a5);
}
