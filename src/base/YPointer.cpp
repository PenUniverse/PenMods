// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "base/YPointer.h"

#include "common/Event.h"

namespace mod {

YPointerInitializer::YPointerInitializer() {

    connect(&Event::getInstance(), &Event::beforeUiCompleted, this, &YPointerInitializer::onBeforeUiCompleted);
}

template <typename T>
void instance() {
    const char* front  = "_ZN10YSingletonI";
    const char* mid    = typeid(T).name();
    const char* back   = "E8instanceEv";
    size_t      length = std::strlen(front) + std::strlen(mid) + std::strlen(back) + 1;
    char*       symbol = new char[length];
    std::strcpy(symbol, front);
    std::strcat(symbol, mid);
    std::strcat(symbol, back);
    YPointer<T>::setInstance(PEN_CALL(T*, symbol)());
    delete[] symbol;
}

void YPointerInitializer::onBeforeUiCompleted() {

    instance<YGlobal>();
    instance<YMediaManager>();
    // instance<YMediaPlayerManager>();
    instance<YDownloader>();
    instance<YSettingManager>();
    // instance<YBatteryManager>();
    instance<YUpdateManager>();
    instance<YSoundCenter>();
    instance<YTextBookDb>();
    instance<YWordbookDB>();
    instance<YWordBookManager>();
    instance<YDictQueryEngine>();

    YPointer<YMediaPlayerManager>::setInstance(
        reinterpret_cast<YMediaPlayerManager*>(*(uint64**)PEN_SYM("_ZN10YSingletonI19YMediaPlayerManagerE1tE"))
    );
    YPointer<YBatteryManager>::setInstance(
        reinterpret_cast<YBatteryManager*>(*(uint64**)PEN_SYM("_ZN10YSingletonI15YBatteryManagerE1tE"))
    );
}

} // namespace mod
