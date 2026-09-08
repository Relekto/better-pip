#pragma once
#include <QRect>
#include <QString>

namespace pip {
enum class ScaleMode { Fit, Fill, Stretch };
enum class Theme { System, Light, Dark };

struct Preferences {
    ScaleMode scaleMode{ScaleMode::Fit};
    Theme theme{Theme::System};
    bool preserveAspect{false};
    int opacity{100};
    QString lockShortcut{QStringLiteral("Ctrl+Alt+L")};
    QRect geometry{80, 80, 640, 360};
};

[[nodiscard]] Preferences sanitized(Preferences value);
[[nodiscard]] QRect visibleGeometry(QRect requested, const QRect& available);
[[nodiscard]] QSize constrainedSize(QSize requested, double aspect);
}
