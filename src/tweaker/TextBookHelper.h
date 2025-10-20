// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "common/service/Logger.h"

namespace mod {

class TextBookHelper : public QObject, public Singleton<TextBookHelper>, private Logger {
    Q_OBJECT

public:
    void onUiCompleted();

    bool isInstalled(const QString&);

    QString getRootPath();

    void remove(const QString&);

private:
    friend Singleton<TextBookHelper>;
    explicit TextBookHelper();

    std::vector<QString> mBookIds;
};

} // namespace mod
