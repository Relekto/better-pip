#pragma once
#include <QKeySequence>
#include <QObject>
#include <memory>

namespace pip {
class ShortcutBackend {
  public:
    virtual ~ShortcutBackend() = default;
    virtual bool bind(const QKeySequence &sequence, QString &error) = 0;
    [[nodiscard]] virtual bool asynchronous() const {
        return false;
    }
};
class GlobalShortcut final : public QObject {
    Q_OBJECT
  public:
    explicit GlobalShortcut(QObject *parent = nullptr);
    ~GlobalShortcut() override;
    [[nodiscard]] bool setSequence(const QString &text);
    [[nodiscard]] QString sequence() const;
    [[nodiscard]] QString error() const;
    [[nodiscard]] bool registered() const;
    [[nodiscard]] QString description() const;
    [[nodiscard]] bool pending() const;
    void completeRegistration(bool success, const QString &error, const QString &description = {});
    void invalidate(const QString &error);
  signals:
    void activated();
    void registrationChanged();

  private:
    std::unique_ptr<ShortcutBackend> backend_;
    QString sequence_;
    QString error_;
    QString pendingSequence_;
    QString description_;
    bool registered_{false};
};
[[nodiscard]] std::unique_ptr<ShortcutBackend> makeShortcutBackend(GlobalShortcut &owner);
}
