// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "system/input/InputDaemon.h"
#include "system/input/ScreenManager.h"

#include "system/battery/BatteryInfo.h"

#include "common/Event.h"
#include "common/Utils.h"

namespace mod {

InputDaemon::InputDaemon() : Logger("InputDaemon") {
    connect(&Event::getInstance(), &Event::uiCompleted, this, &InputDaemon::onUiCompleted);
}

void InputDaemon::onUiCompleted() { reset(); }

bool InputDaemon::setScreenOff(uint32 sec) {
    mScreenOff = sec;
    if (sec > 10) {
        mBackLightDown = sec - 10;
    } else {
        mBackLightDown = 0;
    }
    _resetConfig();
    return true;
}

bool InputDaemon::setSystemSuspend(uint32 sec) {
    if (mSystemSuspend <= mBackLightDown && mSystemSuspend != 0) {
        return false;
    }
    mSystemSuspend = sec;
    _resetConfig();
    return true;
}

void InputDaemon::reset() {
    setScreenOff(ScreenManager::getInstance().getAutoSleepDuration());
    setSystemSuspend(BatteryInfo::getInstance().getAutoSuspendDuration());
}

void InputDaemon::pause() { PEN_CALL(void*, "stop_auto_screen_off")(); }

void InputDaemon::resume() { PEN_CALL(void*, "start_auto_screen_off")(); }

bool InputDaemon::_resetConfig() {
    auto          cfg = _getConfig();
    std::ofstream ofile(cfg.mPath);
    if (ofile.good()) {
        exec("killall input-event-daemon");
        ofile << QString::fromStdString(cfg.mContent)
                     .replace("{backlight_down}", mBackLightDown ? QString::number(mBackLightDown) : "#")
                     .replace("{screen_off}", mScreenOff ? QString::number(mScreenOff) : "#")
                     .replace("{system_suspend}", mSystemSuspend ? QString::number(mSystemSuspend) : "#")
                     .toStdString();
    } else {
        return false;
    }
    ofile.close();
    exec("input-event-daemon");
    return true;
}

std::string InputDaemon::_getRawConfigure(const char* model) {
    switch (H(model)) {
    case H("Cherry"):
        return R"(
#
# /etc/input-event-daemon.conf
#

[Global]
listen = /dev/input/by-path/platform-adc-keys-event
listen = /dev/input/by-path/platform-led_control-event
listen = /dev/input/by-path/platform-ff190000.i2c-event


[Keys]
#POWER        = system_sleep_wakeup auto
#POWER         = screen_onoff switch
#POWER        = system_sleep_wakeup resume
#MUTE         = amixer -q set Master mute
#FN+VOLUMEUP  = factory_reset_cfg
#CTRL+ALT+ESC = beep

[Switches]
RADIO:0 = ifconfig wlan0 down

[Idle]
{backlight_down}s = screen_onoff backlight_down
{screen_off}s = len_onoff.sh off; screen_onoff off
{system_suspend}s = killall -9 hilink-server; system_sleep_wakeup suspend
reset = sleep 0.1;screen_onoff on; sleep 1; killall -9 system_sleep_wakeup
)";
    case H("Cherry_3566"):
        return R"(#
# /etc/input-event-daemon.conf
#

[Global]
listen = /dev/input/by-path/platform-adc-keys-event
listen = /dev/input/by-path/platform-led_control-event
listen = /dev/input/by-path/platform-fe5a0000.i2c-event

[Keys]
#POWER        = system_sleep_wakeup auto
#POWER         = screen_onoff switch
#POWER        = system_sleep_wakeup resume
#MUTE         = amixer -q set Master mute
#FN+VOLUMEUP  = factory_reset_cfg
#CTRL+ALT+ESC = beep

[Switches]
RADIO:0 = ifconfig wlan0 down

[Idle]
{backlight_down}s = screen_onoff backlight_down
{screen_off}s = len_onoff.sh off; screen_onoff off
{system_suspend}s = killall -9 hilink-server; system_sleep_wakeup suspend
reset = sleep 0.1; screen_onoff on; system_post_wakeup; sleep 1; killall -9 system_sleep_wakeup
)";
    case H("V4"):
        return R"(
#
# /etc/input-event-daemon.conf
#

[Global]
listen = /dev/input/event2
listen = /dev/input/event3
listen = /dev/input/event4

[Keys]
#POWER        = system_sleep_wakeup auto
#POWER         = screen_onoff switch
#POWER        = system_sleep_wakeup resume
#MUTE         = amixer -q set Master mute
#FN+VOLUMEUP  = factory_reset_cfg
#CTRL+ALT+ESC = beep

[Switches]
RADIO:0 = ifconfig wlan0 down

[Idle]
{backlight_down}s = screen_onoff backlight_down
{screen_off}s = len_onoff.sh off; screen_onoff off
{system_suspend}s = killall -9 hilink-server; system_sleep_wakeup suspend
reset = screen_onoff on; sleep 1; killall -9 system_sleep_wakeup
)";
    case H("Exam"):
        return R"(
#
# /etc/input-event-daemon.conf
#

[Global]
listen = /dev/input/event2
listen = /dev/input/event3
listen = /dev/input/event4

[Keys]
#POWER        = system_sleep_wakeup auto
#POWER         = screen_onoff switch
#POWER        = system_sleep_wakeup resume
#MUTE         = amixer -q set Master mute
#FN+VOLUMEUP  = factory_reset_cfg
#CTRL+ALT+ESC = beep

[Switches]
RADIO:0 = ifconfig wlan0 down

[Idle]
{backlight_down}s = screen_onoff backlight_down
{screen_off}s = len_onoff.sh off; screen_onoff off
{system_suspend}s = killall -9 hilink-server; system_sleep_wakeup suspend
reset = sleep 0.1; screen_onoff on; system_post_wakeup; sleep 1; killall -9 system_sleep_wakeup
)";
    case H("V0"):
    default:
        return R"(#
# /etc/input-event-daemon.conf
#

[Global]
listen = /dev/input/event1
listen = /dev/input/event2
listen = /dev/input/event3

[Keys]
#POWER        = system_sleep_wakeup auto
#POWER         = screen_onoff switch
#POWER        = system_sleep_wakeup resume
#MUTE         = amixer -q set Master mute
#FN+VOLUMEUP  = factory_reset_cfg
#CTRL+ALT+ESC = beep

[Switches]
RADIO:0 = ifconfig wlan0 down

[Idle]
{backlight_down}s = screen_onoff backlight_down
{screen_off}s = len_onoff.sh off; screen_onoff off
{system_suspend}s = system_sleep_wakeup suspend
reset = screen_onoff on; sleep 1; killall -9 system_sleep_wakeup
)";
    }
}

InputDaemon::Config InputDaemon::_getConfig() {
    auto pcba = exec("get_pcba_version");
    if (pcba == "Dictpen2.0_V4") {
        return {"/etc/input-event-daemon_V4.conf", _getRawConfigure("V4")};
    }
    if (pcba == "Dictpen2.0_V0") {
        return {"/etc/input-event-daemon_V0.conf", _getRawConfigure("V0")};
    }
    if (pcba == "Exam_V0") {
        return {"/etc/input-event-daemon_Exam.conf", _getRawConfigure("Exam")};
    }
    if (pcba.find("Cherry_V0") != std::string::npos) {
        return {"/etc/input-event-daemon_Cherry.conf", _getRawConfigure("Cherry")};
    }
    if (pcba.find("Cherry-3566") != std::string::npos || pcba == "Kiwi-3566_V0") {
        return {"/etc/input-event-daemon_Cherry-3566.conf", _getRawConfigure("Cherry_3566")};
    }
    warn("Unable to find a matching input-event-daemon configuration file for this pcba({}).", pcba);
    return {"/etc/input-event-daemon_V0.conf", _getRawConfigure("V0")};
}

} // namespace mod
