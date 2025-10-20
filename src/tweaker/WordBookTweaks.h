// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "mod/Config.h"

namespace mod {

class WordBookTweaks : public QObject, public Singleton<WordBookTweaks> {
    Q_OBJECT

    Q_PROPERTY(bool phraseTab READ getPhraseTab WRITE setPhraseTab NOTIFY phraseTabChanged);
    Q_PROPERTY(bool noCaseSensitive READ getNoCaseSensitive WRITE setNoCaseSensitive NOTIFY noCaseSensitiveChanged);

    Q_PROPERTY(bool exporting READ getExporting NOTIFY exportingChanged);
    Q_PROPERTY(int exportProgress MEMBER mExportProgress NOTIFY exportProgressChanged);

public:
    bool getPhraseTab() const;

    bool getNoCaseSensitive() const;

    bool getExporting() const;

    void setPhraseTab(bool);

    void setNoCaseSensitive(bool);

    void setExporting(bool);

    // This function is asynchronous.
    Q_INVOKABLE void doExport();

    bool exportFromQueryResult(uint64 query);

signals:

    void phraseTabChanged();

    void noCaseSensitiveChanged();

    void exportingChanged();

    void exportProgressChanged();

private:
    friend Singleton<WordBookTweaks>;
    explicit WordBookTweaks();

    std::string mClassName = "wordbook";
    json        mCfg;

    bool mPhraseTab;
    bool mNoCaseSensitive;

    bool mExporting = false;

    int mExportProgress = 0;

    // QString _getMainTableName();
};

} // namespace mod
