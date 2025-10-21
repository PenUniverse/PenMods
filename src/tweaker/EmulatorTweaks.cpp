// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

// This file contains some necessary tweaks for running under QEMU environment.
// Should not be included in a release that runs in a real environment.

#if PL_QEMU

#include "common/Utils.h"
#include "common/util/System.h"

#include "base/YEnum.h"

using namespace mod;

// from: librkdev.so
// return value:
//   OK    0
//   ERROR -1
PEN_HOOK(int64_t, _console_run, const char* cmd, char* result) {
    switch (H(cmd)) {
    case H(
        R"(update_engine --misc=display | grep "update.info" | awk -F "status=" '{print $2}' | awk -F "." '{print $1}')"
    ):
        strcpy(result, "0");
        break;
    case H("vendor_storage -r VENDOR_CUSTOM_ID_0E -t string | awk '{print $NF}'"):
        strcpy(result, "OVERHEAD_D2_SKU_EXA_ADV");
        break;
    case H("load_sys_cfg sku"):
        strcpy(result, "OVERHEAD_D2_SKU_EXA_ADV");
        break;
    case H("load_sys_cfg brightness"):
        strcpy(result, "99");
        break;
    case H("playback_dev_ctrl show"):
        strcpy(result, "spk");
        break;
    case H("load_sys_cfg spk-volume"):
        strcpy(result, "30");
        break;
    case H("cat /Version | grep Version | awk '{print $2}'"):
        strcpy(result, "2.1.2");
        break;
    case H("/bin/uname -r"):
        strcpy(result, "4.4.159");
        break;
    case H("cat /proc/cpuinfo | grep Features"):
        strcpy(result, "Features        : fp asimd evtstrm aes pmull sha1 sha2 crc32");
        break;
    case H("hciconfig | grep hci0"):
        strcpy(result, "");
        break;
    default:
        spdlog::warn("BANNED executing: \"{}\"", cmd);
        return 0;
    }
    spdlog::warn("INTERRUPTED executing: \"{}\"", cmd);
    return 0;
}

// from: librkdev.so
// result value:
//   OK 0
PEN_HOOK(int64_t, get_battery_info, int& result_capacity, bool& result_charging) {
    result_capacity = 100;
    result_charging = true;
    return 0;
}

// network

PEN_HOOK(int64_t, get_wifi_status, WifiStatus& result) {
    result.mEnabled            = true;
    result.mIsConnected        = true;
    result.mIsNetworkAvailable = true;
    strcpy(result.mSSID, "Emulator Environment");
    result.mSignal = 100;
    return 0;
}

PEN_HOOK(int64_t, wifi_scan, char* deviceList, uint32& deviceCount) {
    strcpy(deviceList, "Emulator Environment");
    deviceCount = 1;
    return 0;
}

PEN_HOOK(int64_t, set_wifi_onoff, bool onoff) { return 0; }

PEN_HOOK(int64_t, wifi_connect, const char* a1, const char* a2) { return 0; }

PEN_HOOK(int64_t, wifi_disconnect, const char* onoff) { return 0; }

PEN_HOOK(int64_t, wifi_remove, const char* onoff) { return 0; }


// block sound play

PEN_HOOK(void*, _ZN13YRecordCenter18startRecordProcessEv, void* self) {
    spdlog::warn("BANNED start record process.");
    return nullptr;
}

PEN_HOOK(void*, _ZN12YSoundCenter17startSoundProcessEv, void* self) {
    spdlog::warn("BANNED start sound process");
    return nullptr;
}

PEN_HOOK(uint32, _ZN12YSoundCenter4playERK7QStringS2_S2_i, void* a1, void* a2, void* a3, void* a4, void* a5) {
    return 0;
}

PEN_HOOK(uint32, _ZN12YSoundCenter8playFileERK7QString, void* a1, void* a2) { return 0; }

PEN_HOOK(uint32, _ZN12YSoundCenter9playMusicERK7QStringxd, void* a1, void* a2, void* a3, void* a4) { return 0; }

PEN_HOOK(uint32, _ZN12YSoundCenter12playFileDataERK7QString, void* self, QString* a2) { return 0; }

// relocation database

PEN_HOOK(void*, _ZN8Database17ConnectionManager15setDatabaseNameERK7QString, void* self, QString const& path) {
    QString newPath = path;
    if (path.startsWith("/userdisk/database/")) {
        newPath = newPath.replace("/userdisk/database/", util::getModuleFileInfo().absolutePath());
    }
    spdlog::warn("{} database path: {}", newPath == path ? "setting" : "INTERRUPTED", newPath.toStdString());
    return origin(self, newPath);
}

// device

PEN_HOOK(int64_t, get_sn, char* result) {
    strcpy(result, "2BC0000011451400000");
    return 0;
}

PEN_HOOK(int64_t, get_mac, char* result) {
    strcpy(result, "43:0b:6f:e2:71:e0");
    return 0;
}

#endif