// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

class QString;

#include <QColor>

namespace mod {

std::string exec(const char* cmd);
std::string exec(const QString& cmd);

double dec(double d, uint16 n);

constexpr uint32 H(const char* str, int h = 0) { return !str[h] ? 5381 : (H(str, h + 1) * 33) ^ str[h]; }

inline uint32 do_hash_runtime(const char* str, int h = 0) {
    return !str[h] ? 5381 : (do_hash_runtime(str, h + 1) * 33) ^ str[h];
}

std::string readFile(const char*);

std::string readFileNoLast(const char*);

void showToast(const std::string& content, const QColor& theme = "#1A1B1F");

bool judgeIsLegalFileName(const QString& filename);

QString generateUUID();

} // namespace mod
