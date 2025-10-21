// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "torch/Torch.h"

#include "common/Event.h"
#include "common/Utils.h"

#include <QQmlContext>

#if PL_BUILD_YDP02X
constexpr auto LED_DEFAULT_GPIO_ID = 15;
#endif

namespace mod {

Torch::Torch() {
    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("torch", this);
    });
}

bool Torch::getStatus() { return exec(QString("cat /sys/class/gpio/gpio%1/value").arg(LED_DEFAULT_GPIO_ID)) == "1"; }

void Torch::setStatus(bool stat) {
    if (getStatus() != stat) {
        if (stat) {
#if PL_BUILD_YDP02X
            PEN_CALL(void*, "led_on", uint32)(LED_DEFAULT_GPIO_ID);
#endif
        } else {
#if PL_BUILD_YDP02X
            PEN_CALL(void*, "led_off", uint32)(LED_DEFAULT_GPIO_ID);
#endif
        }
        emit statusChanged();
    }
}

} // namespace mod
