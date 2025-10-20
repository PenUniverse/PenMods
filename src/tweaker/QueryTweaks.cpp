// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "tweaker/QueryTweaks.h"

#include "common/Event.h"

#include <QQmlContext>

namespace mod {

QueryTweaks::QueryTweaks() {

    mCfg = Config::getInstance().read(mClassName);

    mLowerResult = mCfg["lower_scan"];
    mTypeByHand  = mCfg["type_by_hand"];

    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("queryTweaks", this);
    });
}

bool QueryTweaks::getLowerScan() { return mCfg["lower_scan"]; }

bool QueryTweaks::getTypeByHand() { return mCfg["type_by_hand"]; }

void QueryTweaks::setLowerScan(bool val) {
    if (mLowerResult != val) {
        mLowerResult       = val;
        mCfg["lower_scan"] = val;
        WRITE_CFG;
        emit lowerScanChanged();
    }
}

void QueryTweaks::setTypeByHand(bool val) {
    if (mTypeByHand != val) {
        mTypeByHand          = val;
        mCfg["type_by_hand"] = val;
        WRITE_CFG;
        emit typeByHandChanged();
    }
}

} // namespace mod

#if PL_BUILD_YDP02X
PEN_HOOK(
    uint64,
    _ZN14YResultManager11entryResultERK7QStringS2_S2_N12YEnumWrapper9PageIndexEib,
    uint64  self,
    QString what,
    uint64  a3,
    uint64  a4,
    uint64  a5,
    int     a6,
    bool    a7
) {
    if (mod::QueryTweaks::getInstance().getLowerScan()) {
        what = what.toLower();
    }
    return origin(self, what, a3, a4, a5, a6, a7);
}
#endif
