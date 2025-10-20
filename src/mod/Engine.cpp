// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#if PL_BUILD_YDP02X
#include "../resource/models/YDP02X/qrc_qml.h"
#endif

#include <QQmlContext>
#include <QQuickView>

#include "base/YPointer.h"

#include "common/Event.h"

PEN_HOOK(void, _ZN22YGuiApplicationPrivate6initUiEv, QWindow** self) {

    auto& view    = *(QQuickView*)*self;
    auto* context = view.rootContext();

    mod::YPointer<QQuickView>::setInstance(&view);

    emit mod::Event::getInstance().beforeUiInitialization(view, context);

    // Replace QResources
    PEN_CALL(void*, "_Z21qCleanupResources_qmlv")();
    auto res = PEN_CALL(bool, "_Z21qRegisterResourceDataiPKhS0_S0_", int, const uchar*, const uchar*, const uchar*)(
        0x3,
        qt_resource_struct,
        qt_resource_name,
        qt_resource_data
    );
    if (res) {
        spdlog::info("Resource files have been replaced!");
    } else {
        spdlog::error("The resource file replacement failed, reload the original resource file.");
        PEN_CALL(void*, "_Z18qInitResources_qmlv")();
    }

    origin(self);
}
