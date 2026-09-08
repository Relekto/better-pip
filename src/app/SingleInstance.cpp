#include "SingleInstance.h"
#include <QCryptographicHash>
#include <QLocalSocket>
#include <QStandardPaths>

namespace pip {
SingleInstance::SingleInstance(QObject* parent) : QObject(parent) {
    server_.setSocketOptions(QLocalServer::UserAccessOption);
    connect(&server_, &QLocalServer::newConnection, this, [this] {
        while (server_.hasPendingConnections()) {
            auto* socket = server_.nextPendingConnection();
            connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
            socket->disconnectFromServer();
            emit activationRequested();
        }
    });
}
bool SingleInstance::start() {
    const auto identity = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toUtf8();
    const auto name = QStringLiteral("better-pip-") +
                      QString::fromLatin1(QCryptographicHash::hash(identity, QCryptographicHash::Sha256)
                                              .toHex().left(20));
    if (server_.listen(name))
        return true;
    QLocalSocket existing;
    existing.connectToServer(name);
    if (existing.waitForConnected(1000))
        return false;
    if (existing.error() == QLocalSocket::ServerNotFoundError ||
        existing.error() == QLocalSocket::ConnectionRefusedError) {
        QLocalServer::removeServer(name);
        return server_.listen(name);
    }
    return false;
}
}
