#include "MotionWindow.h"
#include <QApplication>
#include <QPainter>
#include <QTimer>
#include <QWidget>
#include <cmath>

namespace {
class MotionWindow final : public QWidget {
  public:
    MotionWindow() {
        setWindowTitle(QStringLiteral("Motion study - Better PiP studio"));
        setWindowFlag(Qt::FramelessWindowHint);
        resize(640, 400);
        move(20, 40);
        timer_.setInterval(33);
        connect(&timer_, &QTimer::timeout, this, [this] {
            phase_ += 0.006;
            update();
        });
        timer_.start();
    }

  protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor("#101b32"));
        painter.setPen(QPen(QColor("#203353"), 1));
        for (int x = 0; x < width(); x += 32) {
            painter.drawLine(x, 0, x, height());
        }
        for (int y = 0; y < height(); y += 32) {
            painter.drawLine(0, y, width(), y);
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#386bba"));
        painter.drawEllipse(QPointF(320, 215), 125, 125);
        painter.setBrush(QColor("#a4c4ff"));
        painter.drawEllipse(QPointF(320 + std::cos(phase_) * 95, 215 + std::sin(phase_) * 95), 42,
                            42);
        painter.setBrush(QColor("#101b32"));
        painter.drawEllipse(QPointF(320, 215), 65, 65);
        painter.setPen(QColor("#edf2fc"));
        QFont font(QStringLiteral("Segoe UI"), 18, QFont::DemiBold);
        painter.setFont(font);
        painter.drawText(32, 49, QStringLiteral("Motion study"));
        font.setPointSize(10);
        painter.setFont(font);
        painter.setPen(QColor("#a4b3cf"));
        painter.drawText(32, 374, QStringLiteral("LIVE WINDOW   /   GENERATED DEMO CONTENT"));
    }

  private:
    QTimer timer_;
    double phase_{};
};
}

int runMotionWindow() {
    MotionWindow window;
    window.show();
    QTimer deadline;
    deadline.setSingleShot(true);
    QObject::connect(&deadline, &QTimer::timeout, qApp, &QApplication::quit);
    deadline.start(600000);
    return QApplication::exec();
}
