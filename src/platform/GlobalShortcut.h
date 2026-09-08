#pragma once
#include <QKeySequence>
#include <QObject>
#include <memory>

namespace pip {
class ShortcutBackend {
public:
    virtual ~ShortcutBackend() = default;
    virtual bool bind(const QKeySequence& sequence, QString& error) = 0;
};
class GlobalShortcut final : public QObject {
    Q_OBJECT
public:
    explicit GlobalShortcut(QObject* parent = nullptr);
    ~GlobalShortcut() override;
    [[nodiscard]] bool setSequence(const QString& text);
    [[nodiscard]] QString sequence() const;
    [[nodiscard]] QString error() const;
    [[nodiscard]] bool registered() const;
signals:
    void activated();
private:
    std::unique_ptr<ShortcutBackend> backend_;
    QString sequence_;
    QString error_;
    bool registered_{false};
};
[[nodiscard]] std::unique_ptr<ShortcutBackend> makeShortcutBackend(GlobalShortcut& owner);
}
