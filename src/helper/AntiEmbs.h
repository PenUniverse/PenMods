// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "mod/Config.h"

#include <QDateTime>

namespace mod {

class AntiEmbs : public QObject, public Singleton<AntiEmbs> {
    Q_OBJECT

    Q_PROPERTY(bool autoMute READ getAutoMute WRITE setAutoMute NOTIFY autoMuteChanged);
    Q_PROPERTY(bool autoPronLocked READ getAutoPronLocked WRITE setAutoPronLocked NOTIFY autoPronLockChanged);
    Q_PROPERTY(bool fastHide READ getFastHide WRITE setFastHide NOTIFY fastHideChanged);
    Q_PROPERTY(bool lowVoiceMode READ getLowVoiceMode WRITE setLowVoiceMode NOTIFY lowVoiceModeChanged);
    Q_PROPERTY(bool fastMute READ getFastMute WRITE setFastMute NOTIFY fastMuteChanged);

public:
    void onUiCompleted();

    void onHomeButtonPress();

    // =================================

    [[nodiscard]] bool getAutoMute() const;

    [[nodiscard]] bool getAutoPronLocked() const;

    [[nodiscard]] bool getFastHide() const;

    [[nodiscard]] bool getLowVoiceMode() const;

    [[nodiscard]] bool getFastMute() const;

    // =================================

    void setAutoMute(bool);

    void setAutoPronLocked(bool);

    void setFastHide(bool);

    void setLowVoiceMode(bool);

    void setFastMute(bool);

signals:

    void autoMuteChanged();

    void autoPronLockChanged();

    void fastHideChanged();

    void lowVoiceModeChanged();

    void fastMuteChanged();

private:
    friend Singleton<AntiEmbs>;
    explicit AntiEmbs();

    std::string mClassName = "antiembs";
    json        mCfg;

    std::vector<qint64> mHomePressTimes;

    bool mAutoMute;
    bool mAutoPronLocked;
    bool mFastHide;
    bool mLowVoiceMode;
    bool mFastMute;
};

} // namespace mod
