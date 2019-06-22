#include "launcheritem.h"
#include "ui_launcheritem.h"

#include <QProcess>
#include <QDir>

#include <QtDebug>

LauncherItem::LauncherItem(const QString &icon, const QString &text, const QString &path, const QString &workdir, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::LauncherItem)
{
    ui->setupUi(this);
    ui->iconButton->setIcon(QIcon(icon));
    ui->textLabel->setText(text);
    auto manager{new QProcess(this)};
    if (!workdir.isEmpty())
        manager->setWorkingDirectory(workdir);
    connect(ui->iconButton, &QToolButton::clicked, [manager, path]() {
        switch (manager->state()) {
        case QProcess::NotRunning:
            manager->start(path);
            break;
        case QProcess::Starting:
        case QProcess::Running:
            manager->terminate();
            break;
        }
    });
    connect(manager, &QProcess::stateChanged, [this](QProcess::ProcessState state) {
        ui->iconButton->setChecked(state == QProcess::Running);
    });
}

LauncherItem::~LauncherItem()
{
    delete ui;
}
