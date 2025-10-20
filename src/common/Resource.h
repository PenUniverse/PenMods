// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

namespace mod {

class Resource : public QObject, public Singleton<Resource> {
    Q_OBJECT
public:
    Q_INVOKABLE QString get(const QString& name);

    Q_INVOKABLE QString getDisk(const QString& name);

private:
    friend Singleton<Resource>;
    explicit Resource();
};

} // namespace mod
