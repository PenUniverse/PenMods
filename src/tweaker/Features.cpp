// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "mod/Mod.h"

#include "base/YEnum.h"

#define FEATURE_ENABLE(fea)  (supportFeatures |= 1UL << ((fea) & 0x3F))
#define FEATURE_DISABLE(fea) (supportFeatures |= 0UL << ((fea) & 0x3F))
#define FEATURE_HAS(fea)     (((1UL << (fea & 0x3F)) & supportFeatures.to_ulong()) != 0)

PEN_HOOK(void, _ZN2FT11InitFeatureEv) {
    origin();

    if (!mod::Mod::getInstance().isTrustedDevice()) {
        return;
    }

    auto& supportFeatures = *static_cast<std::bitset<60>*>(PEN_SYM("_ZN2FT17s_supportFeaturesE"));

    FEATURE_ENABLE(DictPenFeature::OXFORD);
    FEATURE_ENABLE(DictPenFeature::WEBSTER);

    FEATURE_ENABLE(DictPenFeature::LANG_JPN);
    FEATURE_ENABLE(DictPenFeature::LANG_KOR);
    FEATURE_ENABLE(DictPenFeature::KOJN); // enable wordbook filter.
}
