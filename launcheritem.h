#ifndef LAUNCHERITEM_H
#define LAUNCHERITEM_H

#include <QWidget>

namespace Ui {
class LauncherItem;
}

class QProcess;

class LauncherItem : public QWidget
{
    Q_OBJECT

public:
    explicit LauncherItem(const QString& icon,
                          const QString& text,
                          const QString& path,
                          const QString& workdir,
                          QWidget *parent = nullptr);
    ~LauncherItem();

public slots:
    void startStop();

signals:
    void stateChange(bool started);

private:
    Ui::LauncherItem *ui;
    QProcess *manager;
    QString execPath;
};

#endif // LAUNCHERITEM_H
