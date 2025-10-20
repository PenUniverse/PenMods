// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

enum ScanType : int { POINT, SCAN };

namespace mod {

class KeyBoard : public QObject, public Singleton<KeyBoard> {
    Q_OBJECT
public:
signals:

    void scanFinished(QString result);

private:
    friend Singleton<KeyBoard>;
    explicit KeyBoard();
};

} // namespace mod