#pragma once

#include "filemanager/Config.h"

FILEMANAGER_BEGIN

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
};

FILEMANAGER_END
