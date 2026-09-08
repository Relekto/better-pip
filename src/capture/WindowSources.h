#pragma once
#include <QAbstractListModel>
#include <QCapturableWindow>
#include <QTimer>

namespace pip {
class WindowSources final : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
  public:
    enum Role { TitleRole = Qt::UserRole + 1, TokenRole };
    explicit WindowSources(QObject *parent = nullptr);
    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] QString filter() const;
    void setFilter(const QString &filter);
    [[nodiscard]] int count() const;
    [[nodiscard]] QCapturableWindow window(const QString &token) const;
    Q_INVOKABLE void refresh();
  signals:
    void filterChanged();
    void countChanged();

  private:
    struct Source {
        QCapturableWindow window;
        QString token;
    };
    void rebuild();
    QList<Source> sources_;
    QList<Source> filtered_;
    QString filter_;
    QTimer refreshTimer_;
};
}
