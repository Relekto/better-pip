#include <QApplication>
#include <QPainter>
#include <QTimer>
#include <QWidget>

class SourceWindow final : public QWidget {
  public:
    SourceWindow() {
        setAttribute(Qt::WA_ShowWithoutActivating);
        setWindowTitle(
            QCoreApplication::arguments().value(1, QStringLiteral("Better PiP test source")));
        setGeometry(150, 160, 320, 180);
        timer_.setInterval(50);
        QObject::connect(&timer_, &QTimer::timeout, this, [this] {
            phase_ = (phase_ + 8) % width();
            update();
        });
        timer_.start();
    }

  protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.fillRect(rect(), QColor(30, 160, 90));
        painter.fillRect(QRect(phase_, 0, 20, height()), QColor(240, 220, 70));
    }

  private:
    QTimer timer_;
    int phase_{0};
};
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    SourceWindow source;
    source.show();
    QTimer::singleShot(30000, &app, &QCoreApplication::quit);
    return app.exec();
}
