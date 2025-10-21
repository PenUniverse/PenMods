// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

namespace mod::filemanager {

class VideoPlayer : public QObject, public Singleton<VideoPlayer> {
    Q_OBJECT

    Q_PROPERTY(QString path READ getOpeningPath);

public:
    Q_INVOKABLE void open(QString dir);

    QString getOpeningPath();

    Q_INVOKABLE void onStatusChanged(const QString& status);

private:
    friend Singleton<VideoPlayer>;
    explicit VideoPlayer();

    QString mOpeningFileName;
}; // namespace mod::filemanager
} // namespace mod::filemanager
