// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "base/StdInt.h"
#include "base/SymDB.h"

#include <dobby.h>

#define PEN_SYM(sym) (::mod::SymDB::getInstance().query(sym))

#define PEN_CALL(ret_t, sym, args_t...) ((ret_t(*)(args_t))(PEN_SYM(sym)))

#define PEN_HOOK(ret_t, sym, args_t...) PEN_HOOK_ADDR(ret_t, sym, PEN_SYM(#sym), args_t)

#define PEN_HOOK_ADDR(ret_t, name, addr, args_t...)                                                                    \
    class HookRegistrar_##name {                                                                                       \
    public:                                                                                                            \
        explicit HookRegistrar_##name() {                                                                              \
            if (DobbyHook(addr, (dobby_dummy_func_t)detour, (dobby_dummy_func_t*)&origin) != 0) {                      \
                spdlog::error("Fail to hook: {} ({:#x}).", #name, reinterpret_cast<uint64>(addr));                     \
            }                                                                                                          \
        }                                                                                                              \
        static ret_t (*origin)(args_t);                                                                                \
        static ret_t detour(args_t);                                                                                   \
    };                                                                                                                 \
    ret_t (*HookRegistrar_##name::origin)(args_t) = nullptr;                                                           \
    static HookRegistrar_##name hookRegistrar_##name;                                                                  \
    ret_t                       HookRegistrar_##name::detour(args_t)
