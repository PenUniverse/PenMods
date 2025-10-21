// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

namespace mod {

struct PageIndex {
    enum Enum { _ModPage = 100, AudioRecorder, VideoPlayer };
    Q_ENUM(Enum)
    Q_GADGET
};

class Mod : public QObject, public Singleton<Mod> {
    Q_OBJECT

    Q_PROPERTY(bool trustedDevice READ isTrustedDevice);
    Q_PROPERTY(QString version READ getVersionStr);
    Q_PROPERTY(uint32 cachedSymCount READ getCachedSymCount);
    Q_PROPERTY(QString buildInfo READ getBuildInfoStr)

public:
    void onUiCompleted() const;

    [[nodiscard]] bool isTrustedDevice() const;

    [[nodiscard]] QString getVersionStr() const;

    [[nodiscard]] size_t getCachedSymCount() const;

    [[nodiscard]] QString getBuildInfoStr() const;

    Q_INVOKABLE [[nodiscard]] QString getOtherSlot() const;

    Q_INVOKABLE void changeSlot();

    Q_INVOKABLE void uninstall();

    Q_INVOKABLE void softReboot();

    Q_INVOKABLE void reboot();

private:
    friend Singleton<Mod>;
    explicit Mod();

    std::string mClassName = "mod";
};

} // namespace mod
