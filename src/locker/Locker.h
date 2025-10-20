// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "mod/Config.h"

namespace mod {

class Locker : public QObject, public Singleton<Locker> {
    Q_OBJECT

    Q_PROPERTY(QString password READ getPassword WRITE setPassword NOTIFY passwordChanged);
    Q_PROPERTY(bool enabled READ getEnabled WRITE setEnabled NOTIFY enabledChanged);

public:
    bool getEnabled() const;

    QString getPassword() const;

    void setEnabled(bool);

    void setPassword(const QString&);

    Q_INVOKABLE bool getScene(const QString&) const;

    Q_INVOKABLE void setScene(const QString&, bool);

signals:

    void passwordChanged();

    void enabledChanged();

    void requestUnlock();

private:
    friend Singleton<Locker>;
    explicit Locker();

    std::string mClassName = "locker";
    json        mCfg;

    QString mPassword;
    bool    mEnabled;
};

} // namespace mod
