// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "tweaker/KeyBoard.h"

#include "common/Event.h"

#include "base/YPointer.h"

#include <QQmlContext>

namespace mod {

KeyBoard::KeyBoard() {
    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("keyBoard", this);
    });
}

} // namespace mod

PEN_HOOK(bool, _ZN11YSystemBase12onScanFinishERK7QStringi, uint64 self, QString const& content, ScanType scanType) {
    if (PEN_CALL(bool, "_ZNK7YGlobal16inputPageShowingEv", void*)(mod::YPointer<YGlobal>::getInstance())) {
        emit mod::KeyBoard::getInstance().scanFinished(content);
        return false;
    }
    return origin(self, content, scanType);
}

PEN_HOOK(uint64, _ZN11YSystemBase8ocrStartEv, uint64 self, uint64 a2, uint64 a3, uint64 a4, uint64 a5) {
    if (PEN_CALL(bool, "_ZNK7YGlobal16inputPageShowingEv", void*)(mod::YPointer<YGlobal>::getInstance())) {
        return false;
    }
    return origin(self, a2, a3, a4, a5);
}

PEN_HOOK(uint64, _ZN11YSystemBase7ocrStopEi, uint64 self, int a2, uint64 a3, uint64 a4, uint64 a5) {
    if (PEN_CALL(bool, "_ZNK7YGlobal16inputPageShowingEv", void*)(mod::YPointer<YGlobal>::getInstance())) {
        return false;
    }
    return origin(self, a2, a3, a4, a5);
}

PEN_HOOK(
    uint64,
    _ZN11YSystemBase25ocrCompletedResultChangedEv,
    uint64 self,
    uint64 a2,
    uint64 a3,
    uint64 a4,
    uint64 a5
) {
    if (PEN_CALL(bool, "_ZNK7YGlobal16inputPageShowingEv", void*)(mod::YPointer<YGlobal>::getInstance())) {
        return false;
    }
    return origin(self, a5, a2, a3, a4);
}
