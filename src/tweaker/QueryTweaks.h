// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "mod/Config.h"

namespace mod {

class QueryTweaks : public QObject, public Singleton<QueryTweaks> {
    Q_OBJECT

    Q_PROPERTY(bool lowerScan READ getLowerScan WRITE setLowerScan NOTIFY lowerScanChanged);
    Q_PROPERTY(bool typeByHand READ getTypeByHand WRITE setTypeByHand NOTIFY typeByHandChanged);

public:
    bool getLowerScan();

    bool getTypeByHand();

    void setLowerScan(bool);

    void setTypeByHand(bool);

signals:

    void lowerScanChanged();

    void typeByHandChanged();

private:
    friend Singleton<QueryTweaks>;
    explicit QueryTweaks();

    std::string mClassName = "query";
    json        mCfg;

    bool mLowerResult;
    bool mTypeByHand;
};

} // namespace mod
