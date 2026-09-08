#include "PortalRequest.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QUuid>

namespace pip {
PortalRequest::PortalRequest(QObject* parent) : QObject(parent) {
    timeout_.setSingleShot(true);
    timeout_.setInterval(120000);
    connect(&timeout_, &QTimer::timeout, this, [this] {
        complete(2, {}, QStringLiteral("The desktop did not respond. Check your portal service."));
    });
}
PortalRequest::~PortalRequest() { cancel(); }
PortalRequest* PortalRequest::call(const QString& interface, const QString& method,
                                  QVariantList arguments, QVariantMap options, QObject* parent) {
    auto* request = new PortalRequest(parent);
    auto bus = QDBusConnection::sessionBus();
    auto sender = bus.baseService().mid(1);
    sender.replace('.', '_');
    const auto token = QStringLiteral("betterpip_") + QUuid::createUuid().toString(QUuid::Id128);
    request->path_ = QStringLiteral("/org/freedesktop/portal/desktop/request/") + sender + '/' + token;
    options.insert(QStringLiteral("handle_token"), token);
    arguments.append(options);
    bus.connect(QStringLiteral("org.freedesktop.portal.Desktop"), request->path_,
                QStringLiteral("org.freedesktop.portal.Request"), QStringLiteral("Response"),
                request, SLOT(response(uint,QVariantMap)));
    auto message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.portal.Desktop"),
                    QStringLiteral("/org/freedesktop/portal/desktop"), interface, method);
    message.setArguments(arguments);
    auto* watcher = new QDBusPendingCallWatcher(bus.asyncCall(message), request);
    connect(watcher, &QDBusPendingCallWatcher::finished, request,
            [request](QDBusPendingCallWatcher* completed) {
        QDBusPendingReply<QDBusObjectPath> reply = *completed;
        completed->deleteLater();
        if (request->done_) return;
        if (reply.isError()) {
            request->complete(2, {}, reply.error().message());
        } else if (reply.value().path() != request->path_) {
            request->complete(2, {}, QStringLiteral("The desktop returned an unexpected request handle."));
        }
    });
    request->timeout_.start();
    return request;
}
void PortalRequest::response(uint code, const QVariantMap& results) {
    complete(code, results, code == 1 ? QStringLiteral("Sharing was cancelled.") : QString{});
}
void PortalRequest::complete(uint code, const QVariantMap& results, const QString& error) {
    if (done_) return;
    done_ = true;
    timeout_.stop();
    emit finished(code, results, error);
    deleteLater();
}
void PortalRequest::cancel() {
    if (done_) return;
    done_ = true;
    timeout_.stop();
    if (!path_.isEmpty()) {
        const auto message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.portal.Desktop"),
            path_, QStringLiteral("org.freedesktop.portal.Request"), QStringLiteral("Close"));
        QDBusConnection::sessionBus().asyncCall(message);
    }
}
}
