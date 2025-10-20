// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "Locker.h"

#include "common/Event.h"

#include <QQmlContext>

namespace mod {

Locker::Locker() {

    mCfg = Config::getInstance().read(mClassName);

    mEnabled  = mCfg["enabled"];
    mPassword = QString::fromStdString(mCfg["password"]);

    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("locker", this);
    });
}

bool Locker::getEnabled() const { return mEnabled; }

QString Locker::getPassword() const { return mPassword; }

bool Locker::getScene(const QString& val) const { return getEnabled() && mCfg["scene"][val.toStdString()]; }

void Locker::setEnabled(bool val) {
    if (mEnabled != val) {
        mEnabled        = val;
        mCfg["enabled"] = val;
        WRITE_CFG;
        emit enabledChanged();
    }
}

void Locker::setPassword(const QString& val) {
    if (mPassword != val) {
        mPassword        = val;
        mCfg["password"] = val.toStdString();
        WRITE_CFG;
        emit passwordChanged();
    }
}

void Locker::setScene(const QString& scene, bool val) {
    mCfg["scene"][scene.toStdString()] = val;
    WRITE_CFG;
}

} // namespace mod
