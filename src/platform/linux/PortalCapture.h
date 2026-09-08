#pragma once
#include <QObject>
#include <QVideoFrame>
#include <memory>

namespace pip {
class PortalCapture final : public QObject {
    Q_OBJECT
public:
    explicit PortalCapture(QObject* parent = nullptr);
    ~PortalCapture() override;
    void start();
    void stop();
signals:
    void frameReady(const QVideoFrame& frame);
    void failed(const QString& message);
private slots:
    void sessionClosed();
private:
    struct Private;
    std::unique_ptr<Private> d_;
};
}
