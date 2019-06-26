#include "launcheritem.h"
#include "ui_launcheritem.h"

#include <QProcess>
#include <QDir>

#include <QtDebug>

LauncherItem::LauncherItem(const QString &icon, const QString &text, const QString &path, const QString &workdir, QWidget *parent)
    : QWidget{parent},
      ui{new Ui::LauncherItem},
      manager{new QProcess(this)},
      execPath{path}
{
    ui->setupUi(this);
    ui->iconButton->setIcon(QIcon(icon));
    ui->textLabel->setText(text);

    if (!workdir.isEmpty())
        manager->setWorkingDirectory(workdir);
    connect(ui->iconButton, &QToolButton::clicked, this, &LauncherItem::startStop);
    connect(manager, &QProcess::stateChanged, [this](QProcess::ProcessState state) {
        bool isStarted = state == QProcess::Running;
        ui->iconButton->setChecked(isStarted);
        emit stateChange(isStarted);
    });
}

LauncherItem::~LauncherItem()
{
    delete ui;
}

void LauncherItem::startStop()
{
    switch (manager->state()) {
    case QProcess::NotRunning:
        manager->start(execPath);
        break;
    case QProcess::Starting:
    case QProcess::Running:
        manager->terminate();
        break;
    }
}
