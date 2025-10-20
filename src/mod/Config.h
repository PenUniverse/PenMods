// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "common/service/Logger.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mod {

class Config : public Singleton<Config>, private Logger {
public:
    json read(const std::string& name);

    bool write(const std::string& name, json content, bool saveImmediately = true);

private:
    friend Singleton<Config>;
    explicit Config();

    json mData;

    bool _save();

    bool _load();

    bool _update(json&);
};

} // namespace mod

#define WRITE_CFG Config::getInstance().write(mClassName, mCfg)

#define UPDATE_CFG(item, value)                                                                                        \
    mCfg[item] = value;                                                                                                \
    WRITE_CFG
