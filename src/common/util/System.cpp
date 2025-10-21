// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "System.h"

#include <dlfcn.h>
#include <unistd.h>

namespace mod::util {

QFileInfo getModuleFileInfo() {
    Dl_info info;
    if (dladdr((void*)getModuleFileInfo, &info) == 0) return {};
    return QFileInfo(info.dli_fname);
}

QFileInfo getApplicationFileInfo() {
    char path[4096];
    if (readlink("/proc/self/exe", path, sizeof(path) - 1) == -1) return {};
    return QFileInfo(path);
}

} // namespace mod::util