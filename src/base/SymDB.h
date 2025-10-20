// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "common/service/Logger.h"
#include "common/service/Singleton.h"

#include <QHash>

namespace mod {

class SymDB : public Singleton<SymDB>, private Logger {
public:
    size_t count();

    void* query(const std::string& name);

private:
    friend Singleton<SymDB>;
    explicit SymDB();

    uint64 _getImageBase(const std::string& module);

    QHash<uint32, uint64> mDatabase;
};

} // namespace mod
