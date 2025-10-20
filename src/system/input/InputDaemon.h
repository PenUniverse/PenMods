// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "common/service/Logger.h"

namespace mod {

class InputDaemon : public QObject, public Singleton<InputDaemon>, private Logger {
    Q_OBJECT

public:
    void onUiCompleted();

    // Set to 0 will never execute;
    bool setScreenOff(uint sec);
    bool setSystemSuspend(uint sec);

    // Reset input daemon from cfg.
    void reset();

    // temporary settings.
    void pause();

    void resume();

private:
    friend Singleton<InputDaemon>;
    explicit InputDaemon();

    struct Config {
        std::string mPath;
        std::string mContent;
    };

    uint mBackLightDown = 30;
    uint mScreenOff     = 60;
    uint mSystemSuspend = 600;

    bool _resetConfig();

    Config _getConfig();

    std::string _getRawConfigure(const char* model);
};

} // namespace mod
