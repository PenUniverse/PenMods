// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "tweaker/WordBookTweaks.h"

#include "base/YEnum.h"
#include "base/YPointer.h"

#include "common/Event.h"
#include "common/Utils.h"

#include "filemanager/FileManager.h"

#include <QFile>
#include <QQmlContext>
#include <QVariant>


#define WB_EXPORT_FILE "/userdisk/Favorite/WordBook.json"

bool pickUpQuery = false;

namespace mod {

WordBookTweaks::WordBookTweaks() {

    mCfg = Config::getInstance().read(mClassName);

    mPhraseTab       = mCfg["phrase_tab"];
    mNoCaseSensitive = mCfg["nocase_sensitive"];

    // No Case Sensitive
    connect(&Event::getInstance(), &Event::beforeDatabasePrepareAsyncQuery, [this](QString& query) {
        // for no-case-sensitive;
        if (getNoCaseSensitive()) {
            query.replace("word = (:word)", "word = (:word) COLLATE NOCASE");
        }

        // for tab-type phrase, fix wordCount;
        if (query.startsWith("SELECT count(*) FROM table_wordbook") && getPhraseTab()) {
            auto tabType =
                PEN_CALL(uint32, "_ZNK16YWordBookManager7tabTypeEv", void*)(YPointer<YWordBookManager>::getInstance());
            if (tabType == 2) {
                query += " AND word NOT LIKE '% %'";
            } else if (tabType == 3) {
                query += " AND word LIKE '% %'";
            }
        }
    });

    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("workBookTweaks", this);
    });
}

void WordBookTweaks::doExport() {
    showToast("正在导出单词本");
    // see xrefs for: YWordbookDB::loadAllWords()
    // line 41~45
    // set queryId.
    *(byte*)(YPointer<YWordBookManager>::getInstance() + 128) = 1;
    *(uint64*)(YPointer<YWordBookManager>::getInstance() + 136) =
        PEN_CALL(uint64, "_ZNK11YWordbookDB12loadAllWordsEN12YEnumWrapper9ItemStateE", void*, uint32)(
            YPointer<YWordbookDB>::getInstance(),
            1
        );
    pickUpQuery = true;
}

bool WordBookTweaks::exportFromQueryResult(uint64 queryResult) {
    QFile file(WB_EXPORT_FILE);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        showToast("单词本导出失败，文件打开失败", "#E9900C");
        return false;
    }
    json j;
    j["version"]    = 100;
    int index       = -1;
    mExportProgress = 0;
    auto count      = PEN_CALL(int, "_ZNK8Database16AsyncQueryResult5countEv", uint64)(queryResult);
    while (count > index) {
        index++;
        mExportProgress += (index / (double)count) * 100.0;
        emit exportProgressChanged();
        auto qv = PEN_CALL(QVariant, "_ZNK8Database16AsyncQueryResult5valueEiRK7QString", uint64, int, QString)(
            queryResult,
            index,
            "word"
        );
        if (!qv.canConvert<QString>()) {
            continue;
        }
        auto word       = qv.toString();
        auto HasFeature = PEN_CALL(bool, "_ZN2FTL10HasFeatureE14DictPenFeature", DictPenFeature);
        if (HasFeature(DICT_SENIOR)) {
            QString querySenior =
                PEN_CALL(QString, "_ZN16YDictQueryEngine11querySeniorERK7QString", void*, QString, QString)(
                    YPointer<YDictQueryEngine>::getInstance(),
                    word,
                    QString()
                ); // 高中生词典
            try {
                if (!querySenior.isEmpty()) {
                    j["data"]["Senior"][word.toStdString()] = json::parse(querySenior.toStdString());
                }
            } catch (...) {}
        }
        if (HasFeature(DICT_SIMPLE)) {
            QString queryPureEnglishAndExample =
                PEN_CALL(QString, "_ZN16YDictQueryEngine26queryPureEnglishAndExampleERK7QString", void*, QString, QString)(
                    YPointer<YDictQueryEngine>::getInstance(),
                    word,
                    QString()
                ); // 有道简明释义
            try {
                if (!queryPureEnglishAndExample.isEmpty()) {
                    j["data"]["PureEnglishAndExample"][word.toStdString()] =
                        json::parse(queryPureEnglishAndExample.toStdString());
                }
            } catch (...) {}
        }
        // if (HasFeature(OXFORD)) {
        //     QString queryOxford = PEN_CALL(QString,"_ZN16YDictQueryEngine11queryOxfordERK7QString",
        //                     uint64,QString,QString)(engine,word,QString()); // 牛津词典
        //     try {
        //         if (!queryOxford.isEmpty()) {
        //             j["data"]["Oxford"][word.toStdString()] = json::parse(queryOxford.toStdString());
        //         }
        //     } catch (...) {}
        // }
    }
    file.write(QString::fromStdString(j.dump()).toUtf8());
    file.close();
    showToast("单词本导出成功");
    filemanager::FileManager::getInstance().setMtpOnoff(true);
    return true;
}

bool WordBookTweaks::getExporting() const { return mExporting; }

bool WordBookTweaks::getPhraseTab() const { return mPhraseTab; }

bool WordBookTweaks::getNoCaseSensitive() const { return mNoCaseSensitive; }

void WordBookTweaks::setExporting(bool val) {
    if (mExporting != val) {
        mExporting = val;
        emit exportingChanged();
    }
}

void WordBookTweaks::setPhraseTab(bool val) {
    if (mPhraseTab != val) {
        mPhraseTab         = val;
        mCfg["phrase_tab"] = val;
        WRITE_CFG;
        emit phraseTabChanged();
    }
}

void WordBookTweaks::setNoCaseSensitive(bool val) {
    if (mNoCaseSensitive != val) {
        mNoCaseSensitive         = val;
        mCfg["nocase_sensitive"] = val;
        WRITE_CFG;
        emit noCaseSensitiveChanged();
    }
}

} // namespace mod

// Bugfix: Missing "wbLanguageFitler" cfg item.
PEN_HOOK(uint64, _ZN16YWordBookManager8wipeDataEb, uint64 self, bool a2) {
    a2 = false;
    return origin(self, a2);
}

// Phrase Tab

PEN_HOOK(
    QString,
    _ZNK18YWordbookDBPrivate15getLoadItemsCmdExN12YEnumWrapper13WordGroupTypeEiNS0_12LanguageTypeENS0_9ItemStateENS0_9SyncStateE,
    uint64 a1,
    uint64 a2,
    int    tabType,
    uint32 limit,
    uint32 a5,
    uint32 a6,
    uint32 a7,
    uint64 rtn
) {
    auto cmd = origin(a1, a2, tabType, limit, a5, a6, a7, rtn);
    // tabType
    // 0 = (def) sentence
    // 1 = (def) word
    // 2 = (mod) word
    // 3 = (mod) phrase
    if (mod::WordBookTweaks::getInstance().getPhraseTab()) {
        if (tabType == 2) {
            cmd.replace("ORDER BY", "AND word NOT LIKE '% %' ORDER BY");
        } else if (tabType == 3) {
            cmd.replace("ORDER BY", "AND word LIKE '% %' ORDER BY");
        }
    }
    return cmd;
}

// Mod Exporter

PEN_HOOK(void, _ZN16YWordBookManager8doExportERKN8Database16AsyncQueryResultE, uint64 self, uint64 result) {
    mod::WordBookTweaks::getInstance().setExporting(true);
    if (!pickUpQuery) {
        QFile(WB_EXPORT_FILE).remove();
        origin(self, result);
        mod::WordBookTweaks::getInstance().setExporting(false);
    } else {
        pickUpQuery  = false;
        auto isValid = PEN_CALL(bool, "_ZNK8Database16AsyncQueryResult7isValidEv", uint64)(result);
        if (isValid) {
            mod::WordBookTweaks::getInstance().exportFromQueryResult(result);
        } else {
            mod::showToast("导出失败，无效的查询", "#E9900C");
        }
        *(byte*)(*((uint64*)self + 4) + 128) = 0;
        *(byte*)(*((uint64*)self + 4) + 136) = 0;
        mod::WordBookTweaks::getInstance().setExporting(false);
    }
}
