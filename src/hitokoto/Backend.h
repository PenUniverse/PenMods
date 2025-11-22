// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include "QList"

namespace mod::hitokoto {

class Hitokoto : public QObject, public Singleton<Hitokoto>, private Logger {
    Q_OBJECT

public:
    Q_INVOKABLE QString getSentence(); // 可供 qml 前端调用的句子获取函数
    Q_INVOKABLE bool isAvailable(); // 检查功能是否可用，供 qml 前端调用

    // 句子结构体：句子内容、分类、出处
    struct item {
        QString sentence;
        QString from;
        QString fromWho;
    };

private:
    friend Singleton<Hitokoto>;
    explicit Hitokoto(); // 构造函数

    const QString         sentences_dir = "/userdata/sentences/"; // 句子的来源目录
    QList<Hitokoto::item> sentences; // 句子的列表

    QList<Hitokoto::item> loadHitokotoList(); // 读取文件内容，解析 JSON，返回所有一言句子列表
};

} // namespace mod::hitokoto