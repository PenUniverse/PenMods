#include "filemanager/viewer/ImageViewer.h"

#include "filemanager/FileManager.h"

#include "common/Event.h"
#include "common/service/Logger.h"

#include <QQmlContext>
#include <QImage>
#include <QBuffer>
#include <QMimeDatabase>
#include <QQuickView>

#include <webp/decode.h>

namespace mod::filemanager {

WebPImageProvider::WebPImageProvider() : QQuickImageProvider(QQuickImageProvider::Image), Logger("WebPImageProvider") {
}

QImage WebPImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
    Q_UNUSED(id);
    Q_UNUSED(requestedSize);
    
    if (m_currentWebPPath.isEmpty()) {
        return QImage();
    }
    
    QString fullPath = FileManager::getInstance().getCurrentPath().filePath(m_currentWebPPath);
    
    QFile webpFile(fullPath);
    if (!webpFile.open(QIODevice::ReadOnly)) {
        error("Could not open WebP file: {}", fullPath.toStdString());
        return QImage();
    }
    
    QByteArray webpData = webpFile.readAll();
    webpFile.close();
    
    // 使用 libwebp 手动解码
    int width, height;
    const uint8_t* data = reinterpret_cast<const uint8_t*>(webpData.constData());
    
    // 先获取图片尺寸
    if (!WebPGetInfo(data, webpData.size(), &width, &height)) {
        error("Failed to get WebP info: {}", fullPath.toStdString());
        return QImage();
    }
    
    // 解码为 RGBA
    uint8_t* decoded = WebPDecodeRGBA(data, webpData.size(), &width, &height);
    if (!decoded) {
        error("Failed to decode WebP image: {}", fullPath.toStdString());
        return QImage();
    }
    
    // 创建 QImage（深拷贝数据）
    QImage image(decoded, width, height, width * 4, QImage::Format_RGBA8888);
    QImage result = image.copy();
    
    // 释放 libwebp 分配的内存
    WebPFree(decoded);
    
    if (size) {
        *size = result.size();
    }
    
    info("Successfully decoded WebP image: {}x{}", width, height);
    return result;
}

void WebPImageProvider::setCurrentWebPPath(const QString &path) {
    m_currentWebPPath = path;
}

ImageViewer::ImageViewer(QObject *parent) : QObject(parent), Logger("ImageViewer") {
    m_webpProvider = new WebPImageProvider();
    
    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView &view, QQmlContext *context) {
        context->setContextProperty("imageViewer", this);
        // 注册 image provider
        view.engine()->addImageProvider("webp", m_webpProvider);
    });
}

QString ImageViewer::source() const {
    if (m_openingFileName.isEmpty())
        return "";
    
    // 检查是否是 WebP 文件
    if (m_openingFileName.toLower().endsWith(".webp")) {
        // 使用自定义 image provider
        const_cast<WebPImageProvider*>(m_webpProvider)->setCurrentWebPPath(m_openingFileName);
        return "image://webp/current";
    }
    
    return QString("file://%1").arg(FileManager::getInstance().getCurrentPath().filePath(m_openingFileName));
}

void ImageViewer::open(const QString &path) {
    m_openingFileName = path;
    emit sourceChanged();
}

} // namespace mod::filemanager