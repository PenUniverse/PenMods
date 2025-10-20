// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

namespace mod {

template <typename T>
class YPointer {
public:
    inline static T* getInstance() { return ptr_; }

    inline static void setInstance(T* ptr) { ptr_ = ptr; }

    YPointer(const YPointer&)            = delete;
    YPointer& operator=(const YPointer&) = delete;

private:
    static T* ptr_;

    YPointer() = default;
};

class YPointerInitializer : public QObject, public Singleton<YPointerInitializer> {
    Q_OBJECT
public:
    void onBeforeUiCompleted();

private:
    friend Singleton<YPointerInitializer>;
    explicit YPointerInitializer();
};

template <typename T>
T* YPointer<T>::ptr_ = nullptr;

} // namespace mod

// ----------- Youdao Classes -----------

class YGlobal {
    char filler[0x20];
};

class YMediaManager {
    char filler[0x30];
};

class YMediaPlayerManager {
    char filler[0x28];
};

class YDownloader {
    char filler[0x40];
};

class YSettingManager {
    char filler[0x28];
};

class YBatteryManager {
    char filler[0x18];
};

class YUpdateManager {
    char filler[0x18];
};

class YSoundCenter {
    char filler[0xA8];
};

class YTextBookDb {
    char filler[0x18];
};

class YWordbookDB {
    char filler[0x18];
};

class YWordBookManager {
    char filler[0x28];
};

class YDictQueryEngine {
    char filler[0x8];
};
