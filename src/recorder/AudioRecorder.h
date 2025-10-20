// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "common/service/Logger.h"

#include <QAudioInput>
#include <QFile>

struct lame_global_struct;
typedef struct lame_global_struct lame_global_flags;

namespace mod {

class AudioRecorder : public QObject, public Singleton<AudioRecorder>, private Logger {
    Q_OBJECT

    Q_PROPERTY(int state READ getState NOTIFY stateChanged);
    Q_PROPERTY(QString fileName READ getFileName NOTIFY fileNameChanged);

public:
    enum Error { NoError, OpenError, IOError, UnderrunError, FatalError };
    Q_ENUM(Error)

    enum State { ActiveState, SuspendedState, StoppedState, IdleState, InterruptedState };
    Q_ENUM(State)

    enum SetPathResult { Ok, IllegalSymbolDetected };
    Q_ENUM(SetPathResult)

    int getState() const;

    void setState(QAudio::State state);

    Q_INVOKABLE bool start();

    Q_INVOKABLE bool stop();

    // Set file "name", such as `a.mp3`, not path!
    // audioRecorder.fileName is read-only.
    Q_INVOKABLE SetPathResult setFileName(QString name);

    QString getFileName();

    bool suspend();

    bool resume();

    bool isWorking();

signals:

    void stateChanged();

    void notify();

    void fileNameChanged();

private:
    friend Singleton<AudioRecorder>;
    explicit AudioRecorder();

    std::string mClassName = "recorder";

    lame_global_flags* mLame{};
    QAudioInput*       mInputAudio{};
    QIODevice*         mInputDevice{};

    QFile   mOutputFile;
    QString mSavePath;

    char* mMp3Data{};

    int mState{-1};

    QString _getSavePath();
};

} // namespace mod
