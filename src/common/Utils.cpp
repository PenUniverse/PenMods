// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "common/Utils.h"

#include "base/YPointer.h"

#include <QUuid>

namespace mod {

std::string exec(const QString& cmd) { return exec(cmd.toUtf8().constData()); }

std::string exec(const char* cmd) {
    char        buffer[128];
    std::string result;
    FILE*       pipe = popen(cmd, "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    try {
        while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result.substr(0, result.length() - 1);
}

// 12.333 -> 12.3, if n=1
double dec(double d, uint16 n) { return round(d * pow(10, n)) / pow(10, n); }

std::string readFileNoLast(const char* path) {
    auto str = readFile(path);
    return str.substr(0, str.length() - 1);
}

std::string readFile(const char* path) {
    std::ifstream ifile;
    ifile.open(path, std::ios::in);
    if (!ifile.good()) {
        return "";
    }
    std::string str((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
    return str;
}

void showToast(const std::string& content, const QColor& theme) {
    PEN_CALL(uint64, "_ZN7YGlobal9showToastERK7QStringRK6QColor", YGlobal*, const QString&, const QColor&)
    (YPointer<YGlobal>::getInstance(), QString::fromStdString(content), theme);
}

bool judgeIsLegalFileName(const QString& filename) {
    return !(
        filename.contains(QStringLiteral("/")) || filename.contains(QStringLiteral("\\"))
        || filename.contains(QStringLiteral("\"")) || filename.contains(QStringLiteral("\n"))
        || filename.contains(QStringLiteral(":")) || filename.contains(QStringLiteral("?"))
        || filename.contains(QStringLiteral("*")) || filename.contains(QStringLiteral("|"))
        || filename.contains(QStringLiteral("<")) || filename.contains(QStringLiteral(">"))
        || filename.contains(QStringLiteral("$"))
    );
}

QString generateUUID() { return QUuid::createUuid().toString().remove("{").remove("}"); }

} // namespace mod
