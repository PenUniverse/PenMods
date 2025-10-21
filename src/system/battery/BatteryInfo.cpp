// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "system/battery/BatteryInfo.h"
#include "system/input/InputDaemon.h"

#include "common/Event.h"

#include <QQmlContext>

#if PL_BUILD_YDP02X
constexpr auto DESIGN_BATTERY_FULL = 1000.0;
#endif

namespace mod {

BatteryInfo::BatteryInfo() {

    mCfg = Config::getInstance().read(mClassName);

    mAutoSuspendDuration = mCfg["suspend_duration"];

    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("batteryInfo", this);
    });
}

void BatteryInfo::update() {

    // Status
    switch (H(readFileNoLast("/sys/class/power_supply/battery/status").c_str())) {
    case H("Charging"):
        setStatus("正在充电");
        break;
    case H("Discharging"):
        setStatus("正在放电");
        break;
    case H("Not charging"):
        setStatus("未在充电");
        break;
    case H("Full"):
        setStatus("已充满");
        break;
    case H("Unknown"):
    default:
        setStatus("未知");
        break;
    }

    // Voltage
    auto u = strtod(readFileNoLast("/sys/class/power_supply/battery/voltage_now").c_str(), nullptr);
    setVoltage(QString::number(u / 1000000.0, 'f', 2));

    // Temperature
    auto t = strtol(readFileNoLast("/sys/class/power_supply/battery/temp").c_str(), nullptr, 10) / 10;
    setTemperature(QString::number(t));

    // Health
    QString healthStatus;
    switch (H(readFileNoLast("/sys/class/power_supply/battery/health").c_str())) {
    case H("Good"):
        healthStatus = "良好";
        break;
    case H("Dead"):
        healthStatus = "已损坏";
        break;
    case H("Cold"):
    case H("Cool"):
        healthStatus = "过冷";
        break;
    case H("Warm"):
    case H("Hot"):
    case H("Overheat"):
        healthStatus = "过热";
        break;
    case H("Safety timer expire"):
        healthStatus = "安全期已过";
        break;
    case H("Over voltage"):
        healthStatus = "过电压";
        break;
    case H("Over current"):
        healthStatus = "过电流";
        break;
    case H("Calibration required"):
        healthStatus = "需要校准";
        break;
    case H("Unspecified failure"):
    case H("Watchdog timer expire"):
    case H("No battery"):
    case H("Unknown"):
    default:
        healthStatus = "错误";
        break;
    }

    auto fullCapacity = strtod(readFileNoLast("/sys/class/power_supply/battery/charge_full").c_str(), nullptr) / 1000.0;
    auto validRate    = fullCapacity / DESIGN_BATTERY_FULL;
    setHealth(healthStatus, QString::number(validRate * 100.0, 'f', 1));

    // Current
    auto mA = strtol(readFileNoLast("/sys/class/power_supply/battery/current_now").c_str(), nullptr, 10) / 1000;
    setCurrent(QString::fromStdString((mA >= 0 ? "+" : "") + std::to_string(mA) + "mA"));

    // Prediction
    auto    remaining = strtod(readFileNoLast("/sys/class/power_supply/battery/capacity").c_str(), nullptr) / 100.0;
    bool    charging  = readFileNoLast("/sys/class/power_supply/ac/status") == "Charging" && remaining < 1;
    QString type      = charging ? "距离充满" : "预计可用";
    QString duration;

    if (mA == 0 || (mA < 0 && charging) || (mA > 0 && !charging)) {
        duration = QString("∞ ");
    } else {
        if (charging) { // predict distance to full.
            duration = QString::number((fullCapacity * (1.0 - remaining) / (double)mA) * 60.0, 'f', 1);
        } else { // predict usage.
            duration = QString::number((fullCapacity * remaining / (double)abs(mA)) * 60.0, 'f', 1);
        }
    }
    setPrediction(type, duration);
}

uint32 BatteryInfo::getAutoSuspendDuration() const { return mAutoSuspendDuration; }

QString BatteryInfo::getAutoSuspendDurationStr() const {
    if (mAutoSuspendDuration == 0) {
        return "永不";
    }
    return QString::fromStdString(std::to_string(mAutoSuspendDuration / 60) + "分钟");
}

void BatteryInfo::setAutoSuspendDurationStr(const QString& str) {
    uint32 val = 600;
    if (str == "永不") {
        val = 0;
    } else if (str.contains("分")) {
        val = strtol(str.mid(0, str.indexOf("分")).toStdString().c_str(), nullptr, 10) * 60;
    }
    if (mAutoSuspendDuration != val) {
        mCfg["suspend_duration"] = val;
        mAutoSuspendDuration     = val;
        InputDaemon::getInstance().setSystemSuspend(val);
        WRITE_CFG;
        emit autoSuspendDurationChanged();
    }
}

QString BatteryInfo::getVoltage() const { return mVoltage + "V"; }

void BatteryInfo::setVoltage(const QString& mVoltage_) {
    if (mVoltage != mVoltage_) {
        mVoltage = mVoltage_;
        emit voltageChanged();
    }
}

QString BatteryInfo::getPrediction() const { return QString("%1: %2分钟").arg(mPrediction.type, mPrediction.duration); }

void BatteryInfo::setPrediction(const QString& type, const QString& duration) {
    if (type != mPrediction.type || duration != mPrediction.duration) {
        mPrediction.type     = type;
        mPrediction.duration = duration;
        emit predictionChanged();
    }
}

QString BatteryInfo::getHealth() const { return QString("%1 (%2%)").arg(mHealth.status, mHealth.valid); }

void BatteryInfo::setHealth(const QString& status, const QString& valid) {
    if (mHealth.status != status || mHealth.valid != valid) {
        mHealth.status = status;
        mHealth.valid  = valid;
        emit healthChanged();
    }
}

QString BatteryInfo::getCurrent() const { return mCurrent; }

void BatteryInfo::setCurrent(const QString& mCurrent_) {
    if (mCurrent != mCurrent_) {
        mCurrent = mCurrent_;
        emit currentChanged();
    }
}

QString BatteryInfo::getTemperature() const { return mTemperature + "°C"; }

void BatteryInfo::setTemperature(const QString& mTemperature_) {
    if (mTemperature != mTemperature_) {
        mTemperature = mTemperature_;
        emit temperatureChanged();
    }
}

QString BatteryInfo::getStatus() const { return mStatus; }

void BatteryInfo::setStatus(const QString& mStatus_) {
    if (mStatus != mStatus_) {
        mStatus = mStatus_;
        emit statusChanged();
    }
}

} // namespace mod
