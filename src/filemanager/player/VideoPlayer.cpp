#include "filemanager/player/VideoPlayer.h"

#include "filemanager/FileManager.h"

#include "common/Event.h"
#include "common/Utils.h"

#include <QQmlContext>

FILEMANAGER_BEGIN

VideoPlayer::VideoPlayer() {
    connect(&Event::getInstance(), &Event::beforeUiInitialization, [this](QQuickView& view, QQmlContext* context) {
        context->setContextProperty("videoPlayer", this);
    });
}

void VideoPlayer::open(QString dir) { mOpeningFileName = std::move(dir); }

QString VideoPlayer::getOpeningPath() {
    return "file://" + FileManager::getInstance().getCurrentPath().absoluteFilePath(mOpeningFileName);
}

void VideoPlayer::onStatusChanged(const QString& status) {
    switch (H(status.toLocal8Bit().data())) {
    case H("playing"):
        PEN_CALL(void*, "ydubus_sender_send_event", int, const char*)(0, R"({"action":"Open"})");
        // InputDaemon>::getInstance().pause();
        break;
    case H("paused"):
    case H("stopped"):
        PEN_CALL(void*, "ydubus_sender_send_event", int, const char*)(0, R"({"action":"Close"})");
        // InputDaemon>::getInstance().resume();
        break;
    default:
        break;
    }
}

FILEMANAGER_END
