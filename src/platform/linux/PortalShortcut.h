#pragma once
#include "PortalRequest.h"
#include "platform/GlobalShortcut.h"
#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QPointer>

namespace pip {
struct PortalBinding {
    QString id;
    QVariantMap options;
};
using PortalBindings = QList<PortalBinding>;
QDBusArgument &operator<<(QDBusArgument &argument, const PortalBinding &binding);
const QDBusArgument &operator>>(const QDBusArgument &argument, PortalBinding &binding);
class PortalShortcut final : public QObject, public ShortcutBackend {
    Q_OBJECT
  public:
    explicit PortalShortcut(GlobalShortcut &owner);
    ~PortalShortcut() override;
    bool bind(const QKeySequence &sequence, QString &error) override;
    [[nodiscard]] bool asynchronous() const override;
  private slots:
    void activated(const QDBusObjectPath &session, const QString &id, qulonglong timestamp,
                   const QVariantMap &options);
    void sessionClosed();

  private:
    void bindCandidate(const QString &trigger);
    void reject(const QString &message);
    void cancelCandidate();
    void closeSession(const QString &path);
    GlobalShortcut &owner_;
    QPointer<PortalRequest> request_;
    QString session_;
    QString candidate_;
};
}
Q_DECLARE_METATYPE(pip::PortalBinding)
Q_DECLARE_METATYPE(pip::PortalBindings)
