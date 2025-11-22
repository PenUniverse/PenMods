// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "hitokoto/Backend.h"

#include "mod/Mod.h"

#include "common/Event.h"
#include "common/service/Logger.h"

#include <QQmlContext>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

#include <QRandomGenerator>

namespace mod::hitokoto {

// 构造函数：初始化日志器，载入句子列表，注册 qml 绑定供前端使用
Hitokoto::Hitokoto() : Logger("Hitokoto"), sentences() {

    sentences = loadHitokotoList();

    if (sentences.isEmpty())
        debug("载入的句子列表为空");
    else {
        debug("句子列表长度：{}",sentences.length());
    }

    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("hitokoto", this);
    });
}

// 检查功能是否可用：目录存在且句子数量大于 10
bool Hitokoto::isAvailable() {
    QDir dir(sentences_dir);
    return dir.exists() && sentences.size() > 10;
}

// 读取文件内容，解析 JSON，返回所有一言句子列表
QList<Hitokoto::item> Hitokoto::loadHitokotoList() {
    QList<Hitokoto::item> list;
    
    QDir dir(sentences_dir);
    if (!dir.exists()) {
        debug("句子目录不存在: {}", sentences_dir.toStdString());
        return list;
    }
    
    // 获取目录下所有 JSON 文件
    QFileInfoList fileInfoList = dir.entryInfoList(QStringList() << "*.json", QDir::Files | QDir::Readable);
    
    if (fileInfoList.isEmpty()) {
        debug("句子目录中没有找到 JSON 文件: {}", sentences_dir.toStdString());
        return list;
    }
    
    for (const QFileInfo& fileInfo : fileInfoList) {
        QFile file(fileInfo.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            debug("无法打开文件: {}", fileInfo.absoluteFilePath().toStdString());
            continue;
        }

        QByteArray data = file.readAll();
        file.close();

        QJsonParseError parseError;
        QJsonDocument   doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
            debug("JSON 解析错误，跳过文件: {}", fileInfo.absoluteFilePath().toStdString());
            continue;
        }

        QJsonArray array = doc.array();
        for (const QJsonValue& val : array) {
            if (!val.isObject()) continue;
            QJsonObject obj = val.toObject();
            if (obj.contains("hitokoto") && obj["hitokoto"].isString()) {
                Hitokoto::item h;
                h.sentence = obj["hitokoto"].toString();
                h.from     = obj.contains("from") ? obj["from"].toString() : QString();
                h.fromWho  = obj.contains("from_who") ? obj["from_who"].toString() : QString();
                list.append(h);
            }
        }
    }
    return list;
}

// 从加载的列表中随机获取一个带出处的句子
QString Hitokoto::getSentence() {
    if (sentences.isEmpty()) return QString();
    int         index = QRandomGenerator::global()->bounded(sentences.size());
    const auto& item  = sentences.at(index);

    QString fromText;
    if (!item.from.isEmpty() && !item.fromWho.isEmpty()) fromText = QString("（%1·%2）").arg(item.from, item.fromWho);
    else if (!item.from.isEmpty()) fromText = QString("（%1）").arg(item.from);
    else if (!item.fromWho.isEmpty()) fromText = QString("（%1）").arg(item.fromWho);

    QString fullText = item.sentence + fromText;

    debug("获取的句子：{}", fullText.toStdString());

    return fullText;
}

} // namespace mod::hitokoto