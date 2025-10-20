// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "common/Event.h"

#include <QQmlContext>

namespace mod {

Event::Event() {
    connect(this, &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("builtinEvents", this);
    });
}

} // namespace mod

PEN_HOOK(void*, _ZN11YSystemBase17headSetInitStatusEv, void* self) {
    static bool called = false;
    if (!called) {
        called = true;
        emit mod::Event::getInstance().beforeUiCompleted();
        auto result = origin(self);
        emit mod::Event::getInstance().uiCompleted();
        return result;
    }
    return origin(self);
}

PEN_HOOK(
    uint64,
    _ZN14YButtonMonitor16_do_button_pressE11button_id_tiii,
    uint64 self,
    uint32 buttonId,
    int    unk_a3,
    int    unk_a4,
    int    unk_a5
) {
    switch (buttonId) {
    case 3: {
        emit mod::Event::getInstance().homeButtonPressed();
        break;
    }
    case 6:
    default:
        break;
    }
    return origin(self, buttonId, unk_a3, unk_a4, unk_a5);
}

PEN_HOOK(void*, _ZN7YGlobal23currentPageIndexChangedEv, void* self, void* a2, void* a3, void* a4, void* a5) {
    emit mod::Event ::getInstance().currentPageIndexChanged(
        PEN_CALL(int, "_ZNK7YGlobal16currentPageIndexEv", void*)(self)
    );
    return origin(self, a2, a3, a4, a5);
}

PEN_HOOK(uint64, _ZN8Database10AsyncQuery7prepareERK7QString, void* self, QString& a2) {
    emit mod::Event::getInstance().beforeDatabasePrepareAsyncQuery(a2);
    return origin(self, a2);
}

// EARLY INITIALIZED.
