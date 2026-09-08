#pragma once
#include <QElapsedTimer>
#include <QMediaCaptureSession>
#include <QPointer>
#include <QScreenCapture>
#include <QSize>
#include <QTimer>
#include <QVideoSink>
#include <QWindowCapture>

namespace pip {
class CaptureService final : public QObject {
    Q_OBJECT
public:
    explicit CaptureService(QObject* parent = nullptr);
    ~CaptureService() override;
    void setVideoSink(QVideoSink* sink);
    void start(const QCapturableWindow& window);
    void startPortal();
    void stop();
    [[nodiscard]] bool active() const;
    [[nodiscard]] bool hasFrame() const;
    [[nodiscard]] QSize frameSize() const;
    [[nodiscard]] QString title() const;
signals:
    void changed();
    void frameSizeChanged();
    void failed(const QString& message);
private:
    void fail(const QString& message);
    QWindowCapture windowCapture_;
    QScreenCapture screenCapture_;
    QMediaCaptureSession session_;
    QPointer<QVideoSink> sink_;
    QMetaObject::Connection frameConnection_;
    QTimer sourceMonitor_;
    QTimer firstFrameTimeout_;
    QCapturableWindow source_;
    QString title_;
    QSize frameSize_;
    bool active_{false};
    bool hasFrame_{false};
    bool portal_{false};
};
}
