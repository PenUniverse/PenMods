// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

enum vs_hw_cfg_t : int32_t {
    ADV = 0x0,
    PRO = 0x1,
    STD = 0x2,
    X3  = 0x3,
};

enum vs_ip_type_t : int32_t {
    NORMAL  = 0x0,
    MINIONS = 0x1,
    FROZEN  = 0x2,
    SPIDER  = 0x3,
};

enum vs_region_t : int32_t {
    SKU_CHN  = 0x0,
    SKU_TWN  = 0x1,
    SKU_KOR  = 0x2,
    SKU_JPN  = 0x3,
    SKU_USA  = 0x4,
    SKU_GBR  = 0x5,
    SKU_HLK  = 0x6,
    SKU_PEP  = 0x7,
    SKU_MG   = 0x8,
    SKU_EXA  = 0x9,
    SKU_CLA  = 0xA,
    SKU_CMCC = 0xB,
    SKU_CTC  = 0xC,
};

enum vs_serial_t : int32_t {
    S_D2  = 0x0,
    S_D3  = 0x1,
    S_G3  = 0x2,
    S_K3  = 0x3,
    S_P3  = 0x4,
    S_X3S = 0x5,
    S_D3Q = 0x6,
    S_G3Q = 0x7,
};

struct sku_t {
    vs_serial_t  serial;
    vs_region_t  region;
    vs_hw_cfg_t  hw_cfg;
    vs_ip_type_t ip_type;
};
