// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

class QQuickView;
class QQmlContext;

namespace mod {

class Event : public QObject, public Singleton<Event> {
    Q_OBJECT
signals:

    void beforeUiInitialization(QQuickView& view, QQmlContext* context);

    void beforeUiCompleted();

    void beforeDatabasePrepareAsyncQuery(QString& query);

    void uiCompleted();

    void homeButtonPressed();

    void currentPageIndexChanged(int pageIndex);


private:
    friend Singleton<Event>;
    explicit Event();
};

} // namespace mod
