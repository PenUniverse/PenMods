// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "common/Utils.h"

#include "mod/Config.h"

#include <QMap>

namespace mod {

class BatteryInfo : public QObject, public Singleton<BatteryInfo> {
    Q_OBJECT

    Q_PROPERTY(QString status READ getStatus NOTIFY statusChanged);
    Q_PROPERTY(QString voltage READ getVoltage NOTIFY voltageChanged);
    Q_PROPERTY(QString temperature READ getTemperature NOTIFY temperatureChanged);
    Q_PROPERTY(QString health READ getHealth NOTIFY healthChanged);
    Q_PROPERTY(QString prediction READ getPrediction NOTIFY predictionChanged);
    Q_PROPERTY(QString current READ getCurrent NOTIFY currentChanged);

    Q_PROPERTY(QString autoSuspendDuration READ getAutoSuspendDurationStr WRITE setAutoSuspendDurationStr NOTIFY
                   autoSuspendDurationChanged);

public:
    Q_INVOKABLE void update();

    uint32 getAutoSuspendDuration() const;

    QString getStatus() const;

    QString getVoltage() const;

    QString getTemperature() const;

    QString getHealth() const;

    QString getPrediction() const;

    QString getCurrent() const;

    QString getAutoSuspendDurationStr() const;

    void setStatus(const QString&);

    void setVoltage(const QString&);

    void setTemperature(const QString&);

    void setHealth(const QString&, const QString&);

    void setPrediction(const QString&, const QString&);

    void setCurrent(const QString&);

    // auto str -> uint32 typed.
    void setAutoSuspendDurationStr(const QString&);

signals:

    void statusChanged();

    void voltageChanged();

    void temperatureChanged();

    void healthChanged();

    void predictionChanged();

    void validChanged();

    void currentChanged();

    void autoSuspendDurationChanged();

private:
    friend Singleton<BatteryInfo>;
    explicit BatteryInfo();

    std::string mClassName = "battery";
    json        mCfg;

    QString mStatus;
    QString mVoltage;
    QString mTemperature;
    QString mCurrent;

    struct Health {
        QString status;
        QString valid;
    } mHealth;

    struct Prediction {
        QString type;
        QString duration;
    } mPrediction;

    uint32 mAutoSuspendDuration;
};

} // namespace mod
