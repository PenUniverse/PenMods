// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "system/input/ScreenManager.h"
#include "system/input/InputDaemon.h"

#include "base/YEnum.h"

#include "common/Event.h"

#include <QQmlContext>

namespace mod {

ScreenManager::ScreenManager() {

    mCfg = Config::getInstance().read(mClassName);

    mAutoSleepDuration = mCfg["sleep_duration"];
    mIntelSleep        = mCfg["intel_sleep"];

    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("screenManager", this);
    });
}

void ScreenManager::onPlayStateChanged(PlayState state) {
    mPlayState = state;
    rtSetAutoScreenOff(!(mPlayState == PlayState::PLAYING && mLrcShowing && mInPlayerPage));
}

void ScreenManager::onLrcShowChanged(bool show) {
    mLrcShowing = show;
    rtSetAutoScreenOff(!(mPlayState == PlayState::PLAYING && mLrcShowing && mInPlayerPage));
}

void ScreenManager::onInPlayerPageChanged(bool in) {
    mInPlayerPage = in;
    rtSetAutoScreenOff(!(mPlayState == PlayState::PLAYING && mLrcShowing && mInPlayerPage));
}

// Dynamic Actions
void ScreenManager::reportAction(const QString& action) {
    if (!getIntelSleep()) {
        return;
    }
    switch (H(action.toLocal8Bit().data())) {
    case H("wordbook_cardview_enter"):
        rtSetAutoScreenOff(false);
        break;
    case H("wordbook_cardview_quit"):
        rtSetAutoScreenOff(true);
        break;
    case H("musicplayer_lrc_show"):
        onLrcShowChanged(true);
        break;
    case H("musicplayer_lrc_hide"):
        onLrcShowChanged(false);
        break;
    }
}

QString ScreenManager::getAutoSleepDurationStr() const {
    auto dur = getAutoSleepDuration();
    if (dur == 0) {
        return "永不";
    }
    if (dur == 30) {
        return "30秒";
    }
    return QString::fromStdString(std::to_string(dur / 60) + "分钟");
}

void ScreenManager::setAutoSleepDurationStr(const QString& str) {
    int dur = 30;
    if (str == "永不") {
        dur = 0;
    }
    if (str.contains("秒")) {
        dur = (int)strtol(str.mid(0, str.indexOf("秒")).toStdString().c_str(), nullptr, 10);
    }
    if (str.contains("分")) {
        dur = (int)strtol(str.mid(0, str.indexOf("分")).toStdString().c_str(), nullptr, 10) * 60;
    }
    setAutoSleepDuration(dur);
}

int ScreenManager::getAutoSleepDuration() const { return mAutoSleepDuration; }

bool ScreenManager::getIntelSleep() const { return mIntelSleep; }

void ScreenManager::setAutoSleepDuration(int val) {
    if (mAutoSleepDuration != val) {
        mAutoSleepDuration     = val;
        mCfg["sleep_duration"] = val;
        InputDaemon::getInstance().setScreenOff(val);
        WRITE_CFG;
        emit autoSleepDurationChanged();
    }
}

void ScreenManager::setIntelSleep(bool val) {
    if (mIntelSleep != val) {
        mIntelSleep         = val;
        mCfg["intel_sleep"] = val;
        WRITE_CFG;
        emit intelSleepChanged();
    }
}

void ScreenManager::rtSetAutoScreenOff(bool val) {
    val ? InputDaemon::getInstance().resume() : InputDaemon::getInstance().pause();
}

} // namespace mod

// MusicPlayer
PEN_HOOK(uint64, _ZN7YGlobal27isInPlayerCenterPageChangedEv, uint64 self, uint64 a2, uint64 a3, uint64 a4, uint64 a5) {
    mod::ScreenManager ::getInstance().onInPlayerPageChanged(
        PEN_CALL(bool, "_ZNK7YGlobal20isInPlayerCenterPageEv", uint64)(self)
    );
    return origin(self, a2, a3, a4, a5);
}

PEN_HOOK(
    uint64,
    _ZN19YMediaPlayerManager16playStateChangedEv,
    uint64 self,
    uint64 a2,
    uint64 a3,
    uint64 a4,
    uint64 a5
) {
    mod::ScreenManager ::getInstance().onPlayStateChanged(
        PEN_CALL(PlayState, "_ZNK19YMediaPlayerManager9playStateEv", uint64)(self)
    );
    return origin(self, a5, a2, a3, a4);
}
