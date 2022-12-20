#include "launcheritem.h"
#include "ui_launcheritem.h"

#include <QProcess>
#include <QDir>
#include <QTextBrowser>
#include <QScrollBar>

#include <QtDebug>

static void insertText(QTextBrowser *b, const QString& t, const QColor& color)
{
    auto c = b->textCursor();
    c.movePosition(QTextCursor::End);
    c.beginEditBlock();
    QTextCharFormat fmt;
    fmt.setForeground(color);
    c.setCharFormat(fmt);
    c.insertText(t);
    c.endEditBlock();
    b->setTextCursor(c);
    b->ensureCursorVisible();
}

LauncherItem::LauncherItem(const QIcon &icon,
                           const QString &text,
                           const QString &path,
                           const QString &workdir,
                           const QProcessEnvironment &env,
                           QTextBrowser *log,
                           QWidget *parent)
    : QWidget{parent},
      ui{new Ui::LauncherItem},
      manager{new QProcess(this)},
      execPath{path}
{
    ui->setupUi(this);
    ui->iconButton->setIcon(icon.isNull()? QIcon(":/resources/applauncher.png") : icon);
    ui->iconButton->setText(text);
    ui->iconButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    // ui->textLabel->setText(text);

    if (!workdir.isEmpty())
        manager->setWorkingDirectory(workdir);
    qDebug() << "Process" << text << "env:\n\t" << env.toStringList();
    manager->setProcessEnvironment(env);
    connect(ui->iconButton, &QToolButton::clicked, this, &LauncherItem::startStop);
    connect(manager, &QProcess::stateChanged, this, [this](QProcess::ProcessState state) {
        bool isStarted = state == QProcess::Running;
        ui->iconButton->setChecked(isStarted);
        emit stateChange(isStarted);
    });
    connect(manager, &QProcess::readyReadStandardError, this, [this, log] () {
        insertText(log, manager->readAllStandardError(), Qt::darkRed);
    });
    connect(manager, &QProcess::readyReadStandardOutput, this, [this, log] () {
        insertText(log, manager->readAllStandardOutput(), Qt::darkBlue);
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
        start();
        break;
    case QProcess::Starting:
    case QProcess::Running:
        stop();
        break;
    }
}

void LauncherItem::start()
{
    auto args = QProcess::splitCommand(execPath);
    if (args.isEmpty())
        return;
    auto cmd = args.first();
    auto argv = args.mid(1);
    manager->start(cmd, argv);
}

void LauncherItem::stop()
{
    manager->terminate();
}
