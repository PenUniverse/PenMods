// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "tweaker/TextBookHelper.h"

#include "base/YPointer.h"

#include "common/Event.h"

#include <QDirIterator>
#include <QFile>

#define TEXTBOOK_ROOT_DIR "/userdisk/resource/TextBook/"

namespace mod {

TextBookHelper::TextBookHelper() : Logger("TextBook") {

    // BCD58AB7A9004A8CBF41DABF7A1E0ABE, 必修 1
    // D91ACFBE248C419D8A8E3ED1B48C95D5, 必修 2
    // CE6918225BBE4DD58E610AE8FECC8F32, 必修 3
    // 581D37B91DDB45679AA5EDA30A74C5A9, 选择性必修 1
    // D34C358E3D55485F9B12497491D9EEE0, 选择性必修 2
    // FF8EEEC8B7A14EB18405A1A98DD12EA3, 选择性必修 3
    // 87E24D5136C64316BF2B8B3A6FE70E19, 选择性必修 4

    connect(&Event::getInstance(), &Event::beforeDatabasePrepareAsyncQuery, [this](QString& query) {
        if (query.startsWith("SELECT * FROM table_book ")) {
            QString inject;
            for (const auto& i : mBookIds) {
                inject += QString("OR id = \"%1\" ").arg(i);
            }
            query.replace("WHERE bought = 1", " WHERE bought = 1 " + inject);
        }
    });

    connect(&Event::getInstance(), &Event::uiCompleted, this, &TextBookHelper::onUiCompleted);
}

void TextBookHelper::onUiCompleted() {

    QDirIterator it(TEXTBOOK_ROOT_DIR);
    while (it.hasNext()) {
        auto path = it.next();
        auto id   = path.mid(it.path().length());
        if (id.length() < 5) {
            continue;
        }
        auto jsonPath = path + "/package/output.json";
        auto zipPath  = path + "/package/package.zip";
        if (QFile(jsonPath).exists()) {
            // installed.
            info("Checking offline textbook, Id ==> {}.", id.toStdString());
        } else if (QFile(zipPath).exists()) {
            // need to install.
            info("Try to install offline textbook, Id ==> {}.", id.toStdString());
            auto dbPriv = *((uint64*)YPointer<YTextBookDb>::getInstance() + 2);
            PEN_CALL(void, "_ZN18YTextBookDbPrivate12unzipPackageERK7QString", uint64, QString)(dbPriv, zipPath);
            PEN_CALL(void, "_ZN18YTextBookDbPrivate19addDownloadedBlocksERK7QString", uint64, QString)
            (dbPriv, zipPath);
        } else {
            // not uploaded.
            continue;
        }
        mBookIds.emplace_back(id);
    }
}

bool TextBookHelper::isInstalled(const QString& bookId) {
    return std::find(mBookIds.begin(), mBookIds.end(), bookId) != mBookIds.end();
}

void TextBookHelper::remove(const QString& book) {
    for (auto it = mBookIds.begin(); it != mBookIds.end(); it++) {
        if (*it == book) {
            mBookIds.erase(it);
            break; // because, bookId is unique.
        }
    }
}

} // namespace mod

inline QString getBookId(uint64 entity) { return *(QString*)((uint64*)entity + 3); }

PEN_HOOK(uint64, _ZN15YTextBookEntity16setDownloadStateEN12YEnumWrapper14Download_StateE, uint64 self, uint32 a2) {
    if (mod::TextBookHelper::getInstance().isInstalled(getBookId(self))) {
        a2 = 1;
    }
    return origin(self, a2);
}

PEN_HOOK(uint64, _ZN15YTextBookEntity11setIsBoughtEb, uint64 self, bool a2) {
    if (mod::TextBookHelper::getInstance().isInstalled(getBookId(self))) {
        a2 = true;
    }
    return origin(self, a2);
}

PEN_HOOK(uint64, _ZN15YTextBookEntity13setExpirationEx, uint64 self, int64 ts) {
    if (mod::TextBookHelper::getInstance().isInstalled(getBookId(self))) {
        ts = 4070939400000; // timestamp: 2099/01/01, 16:30.
    }
    return origin(self, ts);
}

PEN_HOOK(uint64, _ZN15YTextBookEntity12setLocalFileERK7QString, uint64 self, QString a2) {
    auto id = getBookId(self);
    if (mod::TextBookHelper::getInstance().isInstalled(id)) {
        a2 = TEXTBOOK_ROOT_DIR + id + "/package/package.zip";
    }
    return origin(self, a2);
}

PEN_HOOK(uint64, _ZN16YTextBookManager6removeERK7QString, uint64 self, const QString& book) {
    mod::TextBookHelper::getInstance().remove(book);
    return origin(self, book);
}
