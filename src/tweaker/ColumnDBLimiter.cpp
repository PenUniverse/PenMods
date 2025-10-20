// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "tweaker/ColumnDBLimiter.h"

#include "common/Event.h"

#include <QQmlContext>

#define LIMIT (80)

namespace mod {

ColumnDBLimiter::ColumnDBLimiter() {

    mCfg = Config::getInstance().read(mClassName);

    mPatch = mCfg["patch"];

    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("columnDb", this);
    });
}

int ColumnDBLimiter::getLimit() const { return mPatch ? LIMIT : 10; }

bool ColumnDBLimiter::getPatch() const { return mPatch; }

void ColumnDBLimiter::setPatch(bool val) {
    if (mPatch != val) {
        mPatch        = val;
        mCfg["patch"] = val;
        WRITE_CFG;
        emit patchChanged();
    }
}

} // namespace mod

PEN_HOOK(
    uint64,
    _ZNK9YColumnDB11loadColumnsERK7QStringS2_iib,
    uint64 self,
    uint64 a2,
    uint64 a3,
    int    a4,
    int    limit,
    bool   a6
) {
    limit = LIMIT;
    return origin(self, a2, a3, a4, limit, a6);
}

PEN_HOOK(uint64, _ZNK10YHistoryDB9loadItemsExi, uint64 self, uint64 a2, uint32 limit) {
    limit = LIMIT;
    return origin(self, a2, limit);
}

PEN_HOOK(
    uint64,
    _ZNK9YColumnDB10loadMediasERK7QStringiiN12YEnumWrapper14Download_StateEb,
    uint64 a1,
    uint64 a2,
    uint32 a3,
    uint32 limit,
    uint32 a5,
    uint32 a6
) {
    limit = LIMIT;
    return origin(a1, a2, a3, limit, a5, a6);
}

PEN_HOOK(uint64, _ZNK10YReadingDB17loadReadingSeriesEiibb, uint64 self, int a2, int limit, bool a4, bool a5) {
    limit = LIMIT;
    return origin(self, a2, limit, a4, a5);
}

PEN_HOOK(
    uint64,
    _ZNK11YTextBookDb10loadBlocksERK7QStringS2_iibb,
    uint64 self,
    uint64 a2,
    uint64 a3,
    int    a4,
    int    limit,
    bool   a6,
    bool   a7
) {
    limit = LIMIT;
    return origin(self, a2, a3, a4, limit, a6, a7);
}

PEN_HOOK(uint64, _ZNK11YTextBookDb9loadBooksERK7QStringiib, uint64 self, uint64 a2, int a3, int limit, bool a5) {
    limit = LIMIT;
    return origin(self, a2, a3, limit, a5);
}

PEN_HOOK(uint64, _ZNK11YTextBookDb9loadTasksERK7QStringiib, uint64 self, uint64 a2, int a3, int limit, bool a5) {
    limit = LIMIT;
    return origin(self, a2, a3, limit, a5);
}


PEN_HOOK(
    uint64,
    _ZNK11YWordbookDB9loadItemsExN12YEnumWrapper13WordGroupTypeEiNS0_12LanguageTypeENS0_9ItemStateENS0_9SyncStateE,
    uint64 self,
    uint64 a2,
    uint32 a3,
    uint32 limit,
    uint32 a5,
    uint32 a6,
    uint32 a7
) {
    limit = LIMIT;
    return origin(self, a2, a3, limit, a5, a6, a7);
}

// YReadingBookManager::loadMore, ignored.
// YResultManager::loadMore, ignored
