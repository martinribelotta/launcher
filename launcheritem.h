#ifndef LAUNCHERITEM_H
#define LAUNCHERITEM_H

#include <QWidget>

namespace Ui {
class LauncherItem;
}

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

private:
    Ui::LauncherItem *ui;
};

#endif // LAUNCHERITEM_H
