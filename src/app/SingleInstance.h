#pragma once
#include <QLocalServer>

namespace pip {
class SingleInstance final : public QObject {
    Q_OBJECT
  public:
    explicit SingleInstance(QObject *parent = nullptr);
    enum class StartResult { Primary, Forwarded, Failed };
    [[nodiscard]] StartResult start();
    [[nodiscard]] QString error() const;
  signals:
    void activationRequested();

  private:
    QLocalServer server_;
};
}
