#pragma once

#include "common/service/Logger.h"

#include <QQuickImageProvider>
#include <QImage>

namespace mod::filemanager {

class WebPImageProvider : public QQuickImageProvider, private Logger {
public:
    WebPImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    
    void setCurrentWebPPath(const QString &path);
    
private:
    QString m_currentWebPPath;
};

class ImageViewer : public QObject, public Singleton<ImageViewer>, private Logger {
    Q_OBJECT
    Q_PROPERTY(QString source READ source NOTIFY sourceChanged)

public:
    QString source() const;

    Q_INVOKABLE void open(const QString &path);

signals:
    void sourceChanged();

private:
    friend class Singleton<ImageViewer>;
    explicit ImageViewer(QObject *parent = nullptr);

    QString m_openingFileName;
    WebPImageProvider* m_webpProvider;
};

} // namespace mod::filemanager
