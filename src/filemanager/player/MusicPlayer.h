// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "base/YEnum.h"

#include "common/service/Logger.h"

#include <QDir>

namespace mod::filemanager {

using PlayFile = std::shared_ptr<QFileInfo>;
using PlayList = std::vector<PlayFile>;

class MusicPlayer : public QObject, public Singleton<MusicPlayer>, private Logger {
    Q_OBJECT

public:
    void play(size_t idx);

    void clickNext();

    void clickPrev();

    void clickRand();

    void onSoundEnd();

    PlayList& getPlayListRef() { return mPlayList; };

    static AudioSequence getCurrentAudioSequence();

    static bool mIsTakeOver;

private:
    friend Singleton<MusicPlayer>;
    explicit MusicPlayer();

    PlayList mPlayList;

    struct {
        PlayFile mFile;
        size_t   mIndex{0};
        bool     mIsEnd{true};

        void setPlaying(size_t idx) {
            mIsEnd = false;
            mIndex = idx;
        }

    } mCurrentPlaying;

    void _play(const PlayFile& file);
};
} // namespace mod::filemanager
