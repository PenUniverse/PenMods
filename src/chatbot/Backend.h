// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QQueue>
#include <QTimer>

#include "common/service/Logger.h"

namespace mod::chatbot {

class ChatBot : public QObject, public Singleton<ChatBot>, private Logger {
    Q_OBJECT

    // QML 属性：API 设置
    Q_PROPERTY(QString apiKey READ getApiKey WRITE setApiKey NOTIFY apiKeyChanged)
    Q_PROPERTY(QString apiEndpoint READ getApiEndpoint WRITE setApiEndpoint NOTIFY apiEndpointChanged)
    Q_PROPERTY(QString model READ getModel WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(qreal temperature READ getTemperature WRITE setTemperature NOTIFY temperatureChanged)
    Q_PROPERTY(QString defaultPrompt READ getDefaultPrompt WRITE setDefaultPrompt NOTIFY defaultPromptChanged)
    Q_PROPERTY(bool isStreaming READ getIsStreaming WRITE setIsStreaming NOTIFY isStreamingChanged)
    Q_PROPERTY(bool isAvailable READ isAvailable CONSTANT)

public:
    // 可供 QML 调用的方法
    Q_INVOKABLE void sendMessage(const QString& message);
    Q_INVOKABLE void sendMessageWithHistory(const QString& message, const QJsonArray& history);
    Q_INVOKABLE bool isAvailable();  // 检查是否已配置必要设置
    Q_INVOKABLE void reloadConfig(); // 重载配置
    Q_INVOKABLE void clearHistory(); // 清除历史记录
    Q_INVOKABLE QString markdownToHtml(const QString& markdown); // 将 Markdown 文本转换为 HTML

    // 设置获取器
    QString getApiKey() const;
    QString getApiEndpoint() const;
    QString getModel() const;
    qreal   getTemperature() const;
    QString getDefaultPrompt() const;
    bool    getIsStreaming() const;

    // 设置器（带信号）
    void setApiKey(const QString& key);
    void setApiEndpoint(const QString& endpoint);
    void setModel(const QString& model);
    void setTemperature(qreal temp);
    void setDefaultPrompt(const QString& prompt);
    void setIsStreaming(bool streaming);

signals:
    // QML 通信信号
    void messageReceived(const QString& content, bool isComplete = true);
    void streamStart();
    void streamChunk(const QString& content);
    void streamEnd();
    void errorOccurred(const QString& error);
    void apiKeyChanged();
    void apiEndpointChanged();
    void modelChanged();
    void temperatureChanged();
    void defaultPromptChanged();
    void isStreamingChanged();

private:
    friend Singleton<ChatBot>;
    explicit ChatBot(); // 构造函数

    // 网络基础设施
    QNetworkAccessManager* m_networkManager;

    // 配置
    QString m_apiKey;
    QString m_apiEndpoint;
    QString m_model;
    qreal   m_temperature;
    QString m_defaultPrompt;
    bool    m_isStreaming;

    // 对话历史
    QQueue<QPair<QString, QString>> m_conversationHistory; // {role, content}
    static const int                MAX_HISTORY_SIZE = 10; // 保留最后 10 条消息对

    // 网络请求方法
    void makeApiRequest(const QJsonArray& messages);
    void handleNetworkReply(QNetworkReply* reply, bool isStream);

    // 流式传输支持
    QString m_currentStreamBuffer;
    QString m_responseBuffer; // 用于累积响应数据，处理不完整的 SSE 行
};

} // namespace mod::chatbot