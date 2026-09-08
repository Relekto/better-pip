#pragma once
#include <QObject>
#include <QTimer>
#include <QVariantMap>

namespace pip {
class PortalRequest final : public QObject {
    Q_OBJECT
public:
    static PortalRequest* call(const QString& interface, const QString& method,
                               QVariantList arguments, QVariantMap options, QObject* parent);
    ~PortalRequest() override;
    void cancel();
signals:
    void finished(uint response, const QVariantMap& results, const QString& error);
private slots:
    void response(uint code, const QVariantMap& results);
private:
    explicit PortalRequest(QObject* parent);
    void complete(uint code, const QVariantMap& results, const QString& error = {});
    QString path_;
    bool done_{false};
    QTimer timeout_;
};
}
