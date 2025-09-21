#pragma once

#include "filemanager/Config.h"

FILEMANAGER_BEGIN

class VideoPlayer : public QObject, public Singleton<VideoPlayer> {
    Q_OBJECT

    Q_PROPERTY(QString path READ getOpeningPath);

public:
    Q_INVOKABLE void open(QString dir);

    QString getOpeningPath();

    Q_INVOKABLE void onStatusChanged(const QString& status);

private:
    friend Singleton<VideoPlayer>;
    explicit VideoPlayer();

    QString mOpeningFileName;
};

FILEMANAGER_END
