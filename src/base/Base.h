// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include <algorithm>
#include <bitset>
#include <cmath>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <bits/stdint-intn.h>
#include <unistd.h>

#include <QObject>
#include <QString>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "base/Hook.h"
#include "base/StdInt.h"

#if PL_BUILD_YDP02X
constexpr uint APP_VERSION = 212; // 2.1.2
#endif

#define QML_PACKAGE_NAME          "com.github.penuniverse"
#define QML_PACKAGE_VERSION_MAJOR 1
#define QML_PACKAGE_VERSION_MINOR 0

// for C++17 nodiscard support.
#define nodiscard gnu::warn_unused_result