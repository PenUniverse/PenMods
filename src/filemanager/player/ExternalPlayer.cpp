// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "filemanager/player/ExternalPlayer.h"

#include "filemanager/FileManager.h"

#include "common/Event.h"
#include "common/Utils.h"

#include <QQmlContext>
#include <QProcess>

namespace mod::filemanager {

ExternalPlayer::ExternalPlayer() {
    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("externalPlayer", this);
    });
} // namespace mod::filemanager

void ExternalPlayer::open(const QString &path) {
    mOpeningFileName = path;
    QString videoPlayer = "/userdisk/VideoPlayer";
    QStringList args;
    args << getOpeningPath();
    QProcess::startDetached(videoPlayer, args);
}

QString ExternalPlayer::getOpeningPath() {
    return "file://" + FileManager::getInstance().getCurrentPath().absoluteFilePath(mOpeningFileName);
}
} // namespace mod::filemanager
