#pragma once
#include <QLocalServer>

namespace pip {
class SingleInstance final : public QObject {
    Q_OBJECT
public:
    explicit SingleInstance(QObject* parent = nullptr);
    [[nodiscard]] bool start();
signals:
    void activationRequested();
private:
    QLocalServer server_;
};
}
