// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

namespace mod::filemanager {

class ExternalPlayer : public QObject, public Singleton<ExternalPlayer> {
    Q_OBJECT

    Q_PROPERTY(QString path READ getOpeningPath);

public:
    Q_INVOKABLE void open(const QString &path);

    QString getOpeningPath();

private:
    friend Singleton<ExternalPlayer>;
    explicit ExternalPlayer();

    QString mOpeningFileName;
};

}