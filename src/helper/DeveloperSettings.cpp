// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "helper/DeveloperSettings.h"

#include "common/Event.h"

#include <QQmlContext>

namespace mod {

DeveloperSettings::DeveloperSettings() {

    mCfg = Config::getInstance().read(mClassName);

    mOfflineRM = mCfg["offline_rm"];

    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("developerSettings", this);
    });
}

PEN_HOOK(void, _ZN23YOfflineResourceManager3runEv, uint64 self) {
    if (DeveloperSettings::getInstance().getOfflineRM()) {
        origin(self);
    }
}

bool DeveloperSettings::getOfflineRM() const { return mOfflineRM; }

void DeveloperSettings::setOfflineRM(bool val) {
    if (mOfflineRM != val) {
        mOfflineRM         = val;
        mCfg["offline_rm"] = val;
        WRITE_CFG;
        emit offlineRMChanged();
    }
}

} // namespace mod
