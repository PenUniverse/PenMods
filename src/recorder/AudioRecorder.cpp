// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "recorder/AudioRecorder.h"

#include "base/YPointer.h"

#include "common/Event.h"
#include "common/Utils.h"

#include "system/input/InputDaemon.h"

#include <QAudio>
#include <QAudioDeviceInfo>
#include <QDir>
#include <QQmlContext>
#include <QQuickView>
#include <QUrl>

#include <lame/lame.h>

namespace mod {

constexpr auto SAVE_PATH = "/userdisk/Music/录音文件/";
constexpr auto LENGTH    = 1024 * 1024;

AudioRecorder::AudioRecorder() : Logger("AudioRecorder") {
    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("audioRecorder", this);
        qmlRegisterUncreatableType<AudioRecorder>(
            QML_PACKAGE_NAME,
            QML_PACKAGE_VERSION_MAJOR,
            QML_PACKAGE_VERSION_MINOR,
            "AudioRecorder",
            "Not creatable as it is an enum type."
        );
    });
}

int AudioRecorder::getState() const { return mState; }

void AudioRecorder::setState(QAudio::State state) {
    if (mState != state) {
        mState = state;
        emit stateChanged();
    }
}

QString AudioRecorder::getFileName() { return QFileInfo(mSavePath).fileName(); }

AudioRecorder::SetPathResult AudioRecorder::setFileName(QString name) {
    if (!judgeIsLegalFileName(name)) {
        return SetPathResult::IllegalSymbolDetected;
    }
    if (!name.endsWith(".mp3")) {
        name += ".mp3";
    }
    mSavePath = SAVE_PATH + name;
    emit fileNameChanged();
    return SetPathResult::Ok;
}

bool AudioRecorder::start() {
    if (mInputAudio) {
        return false;
    }

    // Init saving path.
    mSavePath = _getSavePath();
    mOutputFile.setFileName(mSavePath);
    mOutputFile.open(QIODevice::ReadWrite | QIODevice::Truncate);

    // Init audio format.
    QAudioFormat format;
    format.setSampleRate(16000);
#if PL_BUILD_YDP02X
    format.setChannelCount(2);
#endif
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");

    // Init input device.
    PEN_CALL(void*, "_ZN12YSoundCenter9forceStopEv", void*)(YPointer<YSoundCenter>::getInstance());
    exec("amixer cset numid=2 1");
    auto info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(format)) {
        warn("Default format not supported, trying to use the nearest.");
        info.nearestFormat(format);
    }

    // Create input audio instance.
    mInputAudio = new QAudioInput(info, format, this);
    mInputAudio->setBufferSize(LENGTH);
    mInputAudio->setVolume(1.0);
    mInputAudio->setNotifyInterval(1000);

    // Start recording.
    mInputDevice = mInputAudio->start();

    // Create lame instance.
    mLame = lame_init();
    lame_set_num_channels(mLame, format.channelCount());
    lame_set_in_samplerate(mLame, format.sampleRate());
    lame_set_brate(mLame, 320);
    lame_set_mode(mLame, format.channelCount() >= 2 ? STEREO : MONO);
    lame_set_quality(mLame, 2);
    lame_init_params(mLame);
    mMp3Data = new char[LENGTH];

    // Connect to signals.
    connect(mInputDevice, &QIODevice::readyRead, [&]() {
        auto bytes   = mInputDevice->readAll();
        int  samples = bytes.size() >> 1;
        if (lame_get_num_channels(mLame) > 1) {
            samples >>= 1;
        }
        int out_bytes = 0;
        if (lame_get_num_channels(mLame) >= 2) {
            out_bytes = lame_encode_buffer_interleaved(mLame, (short*)bytes.data(), samples, (uchar*)mMp3Data, LENGTH);
        } else {
            out_bytes = lame_encode_buffer(mLame, (short*)bytes.data(), nullptr, samples, (uchar*)mMp3Data, LENGTH);
        }
        if (out_bytes) {
            mOutputFile.write(mMp3Data, out_bytes);
        }
    });

    connect(mInputAudio, &QAudioInput::notify, [&]() { emit notify(); });

    connect(mInputAudio, &QAudioInput::stateChanged, [&](QAudio::State state) { setState(state); });

    // Others...
    InputDaemon::getInstance().setSystemSuspend(0);

    emit fileNameChanged();

    return true;
}

bool AudioRecorder::stop() {
    if (!mInputAudio) {
        return false;
    }
    auto error = mInputAudio->error();

    disconnect(this, nullptr, nullptr, nullptr);

    // Reset audio device.
    exec("amixer cset numid=2 0");
    InputDaemon::getInstance().reset();
    mInputAudio->stop();
    lame_close(mLame);
    mOutputFile.rename(mSavePath);
    mOutputFile.close();

    // Free vars.
    delete mInputAudio;
    delete[] mMp3Data;
    mLame       = nullptr;
    mInputAudio = nullptr;

    switch (error) {
    case QAudio::OpenError:
        showToast("打开输入设备时出现错误", "#E9900C");
        return false;
    case QAudio::IOError:
        showToast("从输入设备读取数据时出现错误", "#E9900C");
        return false;
    // case QAudio::UnderrunError:
    //    showToast("输入流采样率异常", "#E9900C");
    //    return false;
    case QAudio::FatalError:
        showToast("输入设备引发无法恢复的错误", "#E9900C");
        return false;
    case QAudio::NoError:
    case QAudio::UnderrunError: // temp...
    default:
        showToast("录音保存成功");
        return true;
    }
}

QString AudioRecorder::_getSavePath() {
    auto savedir = QDir(SAVE_PATH);
    if (!savedir.exists()) {
        savedir.mkpath(".");
    }
    int index = 0;
    while (true) {
        index++;
        auto file = QString("新录音%1.mp3").arg(index, 2, 10, QLatin1Char('0'));
        if (!savedir.exists(file)) {
            return SAVE_PATH + file;
        }
    }
}

bool AudioRecorder::isWorking() { return mInputAudio != nullptr; }

} // namespace mod

#if !PL_QEMU

PEN_HOOK(uint, _ZN12YSoundCenter4playERK7QStringS2_S2_i, void* self, void* a2, void* a3, void* a4, void* a5) {
    if (mod::AudioRecorder::getInstance().isWorking()) {
        return (*(uint32*)PEN_SYM("g_playSeq"))++;
    }
    return origin(self, a2, a3, a4, a5);
}

PEN_HOOK(uint, _ZN12YSoundCenter8playFileERK7QString, void* self, void* a2) {
    if (mod::AudioRecorder::getInstance().isWorking()) {
        return (*(uint32*)PEN_SYM("g_playSeq"))++;
    }
    return origin(self, a2);
}

PEN_HOOK(uint, _ZN12YSoundCenter12playFileDataERK7QString, void* self, QString* a2) {
    if (mod::AudioRecorder::getInstance().isWorking()) {
        return (*(uint32*)PEN_SYM("g_playSeq"))++;
    }
    return origin(self, a2);
}

PEN_HOOK(uint, _ZN12YSoundCenter9playMusicERK7QStringxd, void* self, void* a2, void* a3, void* a4) {
    if (mod::AudioRecorder::getInstance().isWorking()) {
        return (*(uint32*)PEN_SYM("g_playSeq"))++;
    }
    return origin(self, a2, a3, a4);
}

#endif

PEN_HOOK(uint64, _ZN13YRecordCenter8startAsrEv, uint64 self, void* a2, void* a3, void* a4, void* a5) {
    if (mod::AudioRecorder::getInstance().isWorking()) {
        return ++*((uint64*)self + 14);
    }
    return origin(self, a2, a3, a4, a5);
}

PEN_HOOK(
    uint64,
    _ZN13YRecordCenter17startFollowSpeechERK7QStringS2_,
    uint64 self,
    void*  a2,
    void*  a3,
    void*  a4,
    void*  a5
) {
    if (mod::AudioRecorder::getInstance().isWorking()) {
        return ++*((uint64*)self + 14);
    }
    return origin(self, a2, a3, a4, a5);
}

PEN_HOOK(uint64, capture_path_onoff, void* a1) {
    if (mod::AudioRecorder::getInstance().isWorking()) {
        return 0;
    }
    return origin(a1);
}
