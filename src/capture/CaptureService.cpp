#include "CaptureService.h"
#include <QVideoFrame>
#ifdef Q_OS_LINUX
#include "platform/linux/PortalCapture.h"
#endif

namespace pip {
CaptureService::CaptureService(QObject *parent) : QObject(parent) {
    connect(&windowCapture_, &QWindowCapture::errorOccurred, this,
            [this](QWindowCapture::Error, const QString &message) { fail(message); });
    sourceMonitor_.setInterval(1000);
    connect(&sourceMonitor_, &QTimer::timeout, this, [this] {
        if (active_ && !portal_ && !source_.isValid()) {
            fail(QStringLiteral("The source window closed. Choose another window."));
        }
    });
    firstFrameTimeout_.setSingleShot(true);
    firstFrameTimeout_.setInterval(15000);
    connect(&firstFrameTimeout_, &QTimer::timeout, this, [this] {
        if (!hasFrame_ && active_) {
            fail(QStringLiteral("No picture arrived. Restore the source window and try again."));
        }
    });
}
CaptureService::~CaptureService() {
    stop();
}
void CaptureService::setVideoSink(QVideoSink *sink) {
    if (sink_ == sink) {
        return;
    }
    QObject::disconnect(frameConnection_);
    sink_ = sink;
    session_.setVideoSink(sink);
    if (sink) {
        frameConnection_ =
            connect(sink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame &frame) {
                if (!active_ || !frame.isValid()) {
                    return;
                }
                if (frameSize_ != frame.size()) {
                    frameSize_ = frame.size();
                    emit frameSizeChanged();
                }
                if (!hasFrame_) {
                    hasFrame_ = true;
                    firstFrameTimeout_.stop();
                    emit changed();
                }
            });
    }
}
void CaptureService::start(const QCapturableWindow &window) {
    stop();
    if (!window.isValid()) {
        emit failed(
            QStringLiteral("This window is no longer available. Refresh and choose again."));
        return;
    }
    source_ = window;
    title_ = window.description();
    session_.setWindowCapture(&windowCapture_);
    windowCapture_.setWindow(window);
    active_ = true;
    emit changed();
    windowCapture_.start();
    if (active_) {
        sourceMonitor_.start();
        firstFrameTimeout_.start();
    }
}
void CaptureService::startPortal() {
    stop();
#ifdef Q_OS_LINUX
    if (!portalCapture_) {
        portalCapture_ = std::make_unique<PortalCapture>();
        connect(portalCapture_.get(), &PortalCapture::failed, this, &CaptureService::fail);
        connect(portalCapture_.get(), &PortalCapture::frameReady, this,
                [this](const QVideoFrame &frame) {
                    if (active_ && portal_ && sink_) {
                        sink_->setVideoFrame(frame);
                    }
                });
    }
    portal_ = true;
    title_ = QStringLiteral("Shared window");
    active_ = true;
    emit changed();
    portalCapture_->start();
#else
    emit failed(QStringLiteral("Use the window list to choose a source."));
#endif
}
void CaptureService::stop() {
    active_ = false;
    hasFrame_ = false;
    sourceMonitor_.stop();
#ifdef Q_OS_LINUX
    if (portalCapture_) {
        portalCapture_->stop();
    }
#endif
    firstFrameTimeout_.stop();
    windowCapture_.stop();
    session_.setWindowCapture(nullptr);
    source_ = {};
    portal_ = false;
    title_.clear();
    frameSize_ = {};
    if (sink_) {
        sink_->setVideoFrame({});
    }
    emit changed();
}
void CaptureService::fail(const QString &message) {
    stop();
    emit failed(message.isEmpty() ? QStringLiteral("Capture is unavailable for this window.")
                                  : message);
}
bool CaptureService::active() const {
    return active_;
}
bool CaptureService::hasFrame() const {
    return hasFrame_;
}
QSize CaptureService::frameSize() const {
    return frameSize_;
}
QString CaptureService::title() const {
    return title_;
}
}
