// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "filemanager/reader/TextReader.h"
#include "filemanager/FileManager.h"

#include "common/Event.h"

#include <QFile>
#include <QQmlContext>

namespace mod::filemanager {

TextReader::TextReader() {
    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("textReader", this);
    });
} // namespace mod::filemanager

void TextReader::open(QString dir) { mOpeningFileName = std::move(dir); }

bool TextReader::getIsMarkdown() { return mOpeningFileName.endsWith(".md", Qt::CaseInsensitive); }

QString TextReader::getContent() {
    QFile file(FileManager::getInstance().getCurrentPath().filePath(mOpeningFileName));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "打开文件失败...";
    }
    std::string ret = file.readAll().toStdString();
    file.close();
    return QString::fromStdString(ret);
}

QString TextReader::getTitle() { return mOpeningFileName; }
} // namespace mod::filemanager
