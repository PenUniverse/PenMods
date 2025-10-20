// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

namespace mod {

class Torch : public QObject, public Singleton<Torch> {
    Q_OBJECT

    Q_PROPERTY(bool switch READ getStatus WRITE setStatus NOTIFY statusChanged);

public:
    // explicit Torch();

    bool getStatus();

    void setStatus(bool);

signals:

    void statusChanged();

private:
    friend Singleton<Torch>;
    explicit Torch();

    std::string mClassName = "torch";
};


} // namespace mod
