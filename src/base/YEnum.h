// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

enum class ASRState : int { AsrIdle, AsrBegin, AsrEnd, QueryBeign, QueryEnd, InternetError };

enum class AudioSequence {
    ORDER,      // 顺序播放
    RANDOM,     // 随机播放
    SINGLE,     // 单篇循环
    SINGLE_SHOT // 单篇播放
};

enum class PlayState { PLAYING, PAUSED, STOPPED };

enum UpdateStatus : int32_t {
    ERROR_LOW_BATTERY      = 0,
    ERROR_NO_CONNECTION    = 1,
    ERROR_NO_ENOUGH_MEMORY = 2,
    CHECKING               = 3,
    HAS_NEW_VERSION        = 4,
    LATEST_VERSION         = 5,
    // DOWNLOAD_PAUSE = 6, do not use me.
    // DOWNLOAD_ABORT = 7,
    DOWNLOADING            = 8,
    DOWNLOAD_FINISHED      = 9,
    DOWNLOAD_FAILED        = 10,
    MD5_CHECKING           = 11,
    MD5_CHECK_SUCCESSFULLY = 12,
    ERROR_MD5_CHECK        = 13,
    INSTALLING             = 14,
    INSTALL_SUCCESSFULLY   = 15,
    INSTALL_FAILED         = 16,
};

struct WifiStatus {
    bool mEnabled;
    bool mIsConnected;
    bool mIsNetworkAvailable;
    char mSSID[512];
    int  mSignal;
};

static_assert(sizeof(WifiStatus) == 0x208, "wrong size");

#if PL_BUILD_YDP02X

enum DictPenFeature : int32_t {
    VERSION_PRO     = 0x0,
    VERSION_X3      = 0x1,
    SKU_EN          = 0x2,
    SKU_TW          = 0x3,
    SKU_JN          = 0x4,
    SKU_KO          = 0x5,
    SKU_ES          = 0x6,
    OLD_DEV         = 0x7,
    LANG_CHS        = 0x8,
    LANG_CHT        = 0x9,
    LANG_ENG        = 0xA,
    LANG_KOR        = 0xB,
    LANG_JPN        = 0xC,
    LANG_ES         = 0xD,
    MICROSOFT_TTS   = 0xE,
    WORD_SYNC       = 0xF,
    SPEAK_SPEED_SET = 0x10,
    WEBSTER         = 0x11,
    OXFORD          = 0x12,
    DICT_SENIOR     = 0x13,
    DICT_KO         = 0x14,
    DICT_JN         = 0x15,
    DICT_SIMPLE     = 0x16,
    DICT_ENCHKID    = 0x17,
    DICT_SSAT       = 0x18,
    DICT_SAT        = 0x19,
    DICT_GRE        = 0x1A,
    DICT_TOEFL      = 0x1B,
    DICT_IELTS      = 0x1C,
    DICT_CHCHINESE  = 0x1D,
    DICT_CHENGLISH  = 0x1E,
    DICT_CHLARGE    = 0x1F,
    DICT_ChENKID    = 0x20,
    DICT_CHPINYIN   = 0x21,
    DICT_CHIDIOM    = 0x22,
    HILINK          = 0x23,
    YOUDAO_TTS      = 0x24,
    AUDIO           = 0x25,
    FOLLOWSPEECH    = 0x26,
    OID             = 0x27,
    KOJN            = 0x28,
    OCR_LANGMODEL   = 0x29,
    ASSISTANT       = 0x2A,
    CN_CULTURE      = 0x2B,
    WORDBOOK        = 0x2C,
    MTP             = 0x2D,
    ANCIENTPOEM     = 0x2E,
    MANGO           = 0x2F,
    MANGO_MINIONS   = 0x30,
    MANGO_FROZEN    = 0x31,
    MANGO_SPIDER    = 0x32,
    TEXTBOOK        = 0x33,
    MATH_CALCULATE  = 0x34,
    MATH_EXERCISE   = 0x35,
    SERIAL_X3S      = 0x36,
    SERIAL_P3       = 0x37,
    POINT_SCAN      = 0x38,
    SERIAL_D2       = 0x39,
    SKU_EXA         = 0x3A,
    SKU_CLA         = 0x3B,
    COUNT           = 0x3C,
};

#endif