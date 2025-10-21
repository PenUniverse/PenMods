// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

namespace mod::filemanager {

class TextReader : public QObject, public Singleton<TextReader> {
    Q_OBJECT

    Q_PROPERTY(bool isMarkdown READ getIsMarkdown);
    Q_PROPERTY(QString content READ getContent);
    Q_PROPERTY(QString title READ getTitle)

public:
    Q_INVOKABLE void open(QString dir);

    bool getIsMarkdown();

    QString getContent();

    QString getTitle();

private:
    friend Singleton<TextReader>;
    explicit TextReader();

    QString mOpeningFileName;
    QString mContent;
}; // namespace mod::filemanager
} // namespace mod::filemanager
