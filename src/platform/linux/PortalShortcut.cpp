#include "PortalShortcut.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusMetaType>
#include <QUuid>
#include <utility>

namespace pip {
namespace {
const QString service = QStringLiteral("org.freedesktop.portal.Desktop");
const QString path = QStringLiteral("/org/freedesktop/portal/desktop");
const QString interface = QStringLiteral("org.freedesktop.portal.GlobalShortcuts");
}
QDBusArgument &operator<<(QDBusArgument &argument, const PortalBinding &binding) {
    argument.beginStructure();
    argument << binding.id << binding.options;
    argument.endStructure();
    return argument;
}
const QDBusArgument &operator>>(const QDBusArgument &argument, PortalBinding &binding) {
    argument.beginStructure();
    argument >> binding.id >> binding.options;
    argument.endStructure();
    return argument;
}
PortalShortcut::PortalShortcut(GlobalShortcut &owner) : owner_(owner) {
    qDBusRegisterMetaType<PortalBinding>();
    qDBusRegisterMetaType<PortalBindings>();
    QDBusConnection::sessionBus().connect(
        service, path, interface, QStringLiteral("Activated"), this,
        SLOT(activated(QDBusObjectPath, QString, qulonglong, QVariantMap)));
}
PortalShortcut::~PortalShortcut() {
    cancelCandidate();
    closeSession(session_);
}
bool PortalShortcut::asynchronous() const {
    return true;
}
bool PortalShortcut::bind(const QKeySequence &sequence, QString &error) {
    cancelCandidate();
    const auto combination = sequence[0];
    QStringList parts;
    const auto modifiers = combination.keyboardModifiers();
    if (modifiers.testFlag(Qt::ControlModifier)) {
        parts << QStringLiteral("CTRL");
    }
    if (modifiers.testFlag(Qt::AltModifier)) {
        parts << QStringLiteral("ALT");
    }
    if (modifiers.testFlag(Qt::ShiftModifier)) {
        parts << QStringLiteral("SHIFT");
    }
    if (modifiers.testFlag(Qt::MetaModifier)) {
        parts << QStringLiteral("LOGO");
    }
    const auto key = combination.key();
    if (key >= Qt::Key_A && key <= Qt::Key_Z) {
        parts << QString(QChar(static_cast<char16_t>(key))).toLower();
    } else if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        parts << QString(QChar(static_cast<char16_t>(key)));
    } else if (key >= Qt::Key_F1 && key <= Qt::Key_F24) {
        parts << QStringLiteral("F%1").arg(key - Qt::Key_F1 + 1);
    } else if (key == Qt::Key_Space) {
        parts << QStringLiteral("space");
    } else {
        error = QStringLiteral("Choose a letter, number, function key, or Space.");
        return false;
    }
    const auto trigger = parts.join('+');
    const QVariantMap options{
        {QStringLiteral("session_handle_token"),
         QStringLiteral("betterpipkeys_") + QUuid::createUuid().toString(QUuid::Id128)}};
    request_ = PortalRequest::call(interface, QStringLiteral("CreateSession"), {}, options, this);
    connect(request_, &PortalRequest::finished, this,
            [this, trigger](uint code, const QVariantMap &results, const QString &message) {
                request_.clear();
                if (code != 0) {
                    reject(message);
                    return;
                }
                const auto value = results.value(QStringLiteral("session_handle"));
                candidate_ = value.canConvert<QDBusObjectPath>()
                                 ? value.value<QDBusObjectPath>().path()
                                 : value.toString();
                if (candidate_.isEmpty()) {
                    reject(QStringLiteral("The desktop did not create a shortcut session."));
                    return;
                }
                bindCandidate(trigger);
            });
    return true;
}
void PortalShortcut::bindCandidate(const QString &trigger) {
    const PortalBindings bindings{
        {QStringLiteral("toggle-lock"),
         {{QStringLiteral("description"), QStringLiteral("Lock or unlock Better PiP")},
          {QStringLiteral("preferred_trigger"), trigger}}}};
    request_ = PortalRequest::call(interface, QStringLiteral("BindShortcuts"),
                                   {QVariant::fromValue(QDBusObjectPath(candidate_)),
                                    QVariant::fromValue(bindings), QString{}},
                                   {}, this);
    connect(request_, &PortalRequest::finished, this,
            [this](uint code, const QVariantMap &results, const QString &message) {
                request_.clear();
                if (code != 0) {
                    reject(message);
                    return;
                }
                const auto bindings =
                    qdbus_cast<PortalBindings>(results.value(QStringLiteral("shortcuts")));
                QString description;
                bool found = false;
                for (const auto &binding : bindings) {
                    if (binding.id == QStringLiteral("toggle-lock")) {
                        found = true;
                        description =
                            binding.options.value(QStringLiteral("trigger_description")).toString();
                    }
                }
                if (!found) {
                    reject(QStringLiteral("The desktop did not bind the shortcut."));
                    return;
                }
                closeSession(session_);
                session_ = std::exchange(candidate_, {});
                QDBusConnection::sessionBus().connect(
                    service, session_, QStringLiteral("org.freedesktop.portal.Session"),
                    QStringLiteral("Closed"), this, SLOT(sessionClosed()));
                owner_.completeRegistration(true, {}, description);
            });
}
void PortalShortcut::activated(const QDBusObjectPath &session, const QString &id, qulonglong,
                               const QVariantMap &) {
    if (session.path() == session_ && id == QStringLiteral("toggle-lock")) {
        emit owner_.activated();
    }
}
void PortalShortcut::sessionClosed() {
    session_.clear();
    owner_.invalidate(QStringLiteral(
        "The desktop ended the shortcut session. Use the control window to unlock."));
}
void PortalShortcut::reject(const QString &message) {
    cancelCandidate();
    owner_.completeRegistration(
        false,
        message.isEmpty()
            ? QStringLiteral("Global shortcuts are unavailable. Use the control window to unlock.")
            : message);
}
void PortalShortcut::cancelCandidate() {
    if (request_) {
        request_->cancel();
        request_->deleteLater();
        request_.clear();
    }
    closeSession(candidate_);
    candidate_.clear();
}
void PortalShortcut::closeSession(const QString &session) {
    if (session.isEmpty()) {
        return;
    }
    QDBusConnection::sessionBus().disconnect(service, session,
                                             QStringLiteral("org.freedesktop.portal.Session"),
                                             QStringLiteral("Closed"), this, SLOT(sessionClosed()));
    const auto message = QDBusMessage::createMethodCall(
        service, session, QStringLiteral("org.freedesktop.portal.Session"),
        QStringLiteral("Close"));
    QDBusConnection::sessionBus().asyncCall(message);
}
}
