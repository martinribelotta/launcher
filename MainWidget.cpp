#include "MainWidget.h"
#include "aboutdialog.h"
#include "launcheritem.h"
#include "ui_MainWidget.h"

#include <QScreen>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QMessageBox>
#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QDesktopWidget>
#include <QMenu>
#include <QStyle>

#include <flowlayout.h>

#include <QtDebug>

#ifdef Q_OS_WIN
constexpr auto PATH_SEPARATOR = ';';
#else
constexpr auto PATH_SEPARATOR = ':';
#endif
constexpr auto CONF_NAME = "launcher-conf.json";
constexpr auto SHARE_DIR = "applauncher";
constexpr auto RESOURCE_DIR = "resources";

static QByteArray readEntireFile(const QString& path)
{
    QFile f(path);
    if (f.open(QFile::ReadOnly))
        return QTextStream{&f}.readAll().toUtf8();
    qDebug() << "file error: " << path << "\n" << f.errorString();
    return {};
}

static bool isAppImage()
{
#ifdef Q_OS_WIN
    // In windows be linke as appImage
    return true;
#else
    return qEnvironmentVariableIsSet("APPDIR");
#endif
}

static QString pathJoin(const QStringList& parts)
{
    return parts.join(QDir::separator());
}

static QString appFile()
{
#ifdef Q_OS_LINUX
    if (isAppImage())
        return qgetenv("ARGV0");
#endif
    return QApplication::instance()->applicationFilePath();
}

static QString appPath()
{
    return QFileInfo(appFile()).absolutePath();
}

static QString sharePath()
{
    return pathJoin({ isAppImage()? appPath() : pathJoin({ appPath(), "..", "share" }), SHARE_DIR });
}

static QString resPath()
{
    return pathJoin({ sharePath(), RESOURCE_DIR });
}

static QString configurationFileName()
{
    return pathJoin({ sharePath(), CONF_NAME });
}

static QJsonObject loadConfig()
{
    QJsonParseError err;
    auto configFile = configurationFileName();
    if (configFile.isEmpty())
        return {};
    auto doc = QJsonDocument::fromJson(readEntireFile(configFile), &err).object();
    if (err.error != QJsonParseError::NoError) {
        qDebug() << "Error loading " << configFile << ": " << err.errorString();
        return {};
    }
    return doc;
}

static QSpacerItem *newSpacer()
{
    return new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
}

static void adjustInitialEnv()
{
    auto homePath = QDir::home().absolutePath();
    if (!qEnvironmentVariableIsSet("HOME")) {
        auto homePaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if (!homePaths.isEmpty())
            homePath = homePaths.first();
        qputenv("HOME", homePath.toLocal8Bit());
    }
    auto app = appFile().toLocal8Bit();
    auto appDir = appPath().toLocal8Bit();
    auto resDir = resPath().toLocal8Bit();

    qputenv("APPLICATION_FILE_PATH", app);
    qputenv("APPLICATION_DIR_PATH", appDir);
    qputenv("APPLICATION_RESOURCE_PATH", resDir);
    if (isAppImage())
        qputenv("APPLICATION_REAL_FILE_PATH",
                QApplication::instance()->applicationDirPath().toLocal8Bit());
}

static QString env(const QString& e)
{
    QString r;
    auto env = QProcessEnvironment::systemEnvironment();
    int i=0;
    r.reserve(e.size());
    while (i<e.size()) {
        if (e[i] == QChar{'$'} && e[i+1] == QChar{'{'}) {
            i+=2;
            int idxEnd = e.indexOf('}', i);
            QString var = e.mid(i, idxEnd - i);
            QString val = env.value(var, QString{"${%1}"}.arg(var));
            r.append(val);
            i = idxEnd + 1;
        } else {
            r.append(e[i]);
            i++;
        }
    }
    return r;
}

static QIcon loadIcon(const QString& name)
{
    QIcon icon;
    if (name.startsWith("theme:")){
        auto themedName = name.mid(6);
        qDebug() << "loading from theme" << themedName;
        icon = QIcon::fromTheme(themedName);
    } else
        icon = QIcon(name);
    if (icon.isNull())
        qDebug() << "icon:" << name << "is null";
    return icon;
}

Widget::Widget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Widget)
{
    adjustInitialEnv();
    ui->setupUi(this);
    ui->logView->setFont(QFont{"Monospace, Consolas, Courier"});
    ui->logView->setContextMenuPolicy(Qt::CustomContextMenu);
    auto logViewMenu = new QMenu(this);
    logViewMenu->addAction(tr("Select All"), ui->logView, &QTextEdit::selectAll);
    logViewMenu->addAction(tr("Copy"), ui->logView, &QTextEdit::copy);
    logViewMenu->addSeparator();
    logViewMenu->addAction(tr("Clear"), ui->logView, &QTextEdit::clear);
    connect(ui->logView, &QWidget::customContextMenuRequested, [this, logViewMenu](const QPoint& p) {
        logViewMenu->exec(ui->logView->mapToGlobal(p));
    });
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 0);
    ui->splitter->setSizes({ 120, 120 });
    connect(ui->buttonUpDown, &QToolButton::clicked, [this] () {
        auto t = ui->logView->isVisible();
        ui->logView->setVisible(!t);
        ui->buttonUpDown->setArrowType(t? Qt::UpArrow : Qt::DownArrow);
    });
    ui->logView->hide();
    ui->buttonUpDown->setArrowType(Qt::UpArrow);

    auto doc = loadConfig();

    if (doc.isEmpty()) {
        QJsonParseError err;
        doc = QJsonDocument::fromJson(readEntireFile(":/default-config.json"), &err).object();
        if (err.error != QJsonParseError::NoError)
            qDebug() << err.errorString();
    }

    QJsonObject initialSize = doc.value("initialSize").toObject();
    resize(initialSize.value("width").toInt(width()), initialSize.value("height").toInt(height()));

    for (const QJsonValueRef a: doc.value("res").toArray())
        QDir::addSearchPath("res", env(a.toString()));

    auto mainIcon = loadIcon(env(doc.value("mainIcon").toString()));
    ui->mainIcon->setPixmap(mainIcon.pixmap(ui->mainIcon->size()));
    auto mainLabel = env(doc.value("mainLabel").toString());
    ui->mainLabel->setText(mainLabel);
    setWindowTitle(mainLabel);

    auto trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(mainIcon);
    auto menu = new QMenu(this);
    toggleWindow = new QAction(this);
    toggleWindow->setText(tr("Show Launcher"));
    connect(toggleWindow, &QAction::triggered, [this]() { setVisible(!isVisible()); });
    menu->addAction(toggleWindow);
    menu->addSeparator();

    auto sysPath = QProcessEnvironment::systemEnvironment().value("PATH");
    auto newPath = QStringList{};
    for (const QJsonValueRef a: doc.value("path").toArray())
        newPath.append(env(a.toString()));
    newPath.append(sysPath);
    if (!newPath.isEmpty()) {
        auto pathStr = newPath.join(PATH_SEPARATOR);
        qDebug() << pathStr << newPath;
        qputenv("PATH", pathStr.toLocal8Bit());
    }
    auto envObj = doc.value("env").toObject();
    for (auto it = envObj.constBegin(); it != envObj.constEnd(); ++it)
        qputenv(it.key().toLocal8Bit().data(), env(it.value().toString()).toLocal8Bit());

    int row = 0;
    int col = 0;
    auto layout = new FlowLayout(ui->scrollAreaWidgetContents);
    for(const QJsonValueRef a: doc.value("applications").toArray()) {
        auto o = a.toObject();
        auto icon = loadIcon(env(o.value("icon").toString()));
        auto text = env(o.value("text").toString());
        auto exec = env(o.value("exec").toString());
        auto work = env(o.value("work").toString());
        auto launcher = new LauncherItem{icon, text, exec, work, ui->logView, ui->scrollAreaWidgetContents};
        auto action = menu->addAction(icon, text, launcher, &LauncherItem::startStop);
        action->setCheckable(true);
        connect(launcher, &LauncherItem::stateChange, action, &QAction::setChecked);
        layout->addWidget(launcher);
        if (++col == 3)
            { col = 0; row++; }
    }
    if (col==1 || col==2)
        layout->addItem(newSpacer());
    // layout->setRowStretch(row + 1, 1);
    connect(ui->buttonShutdown, &QToolButton::clicked,
            QApplication::instance(), &QApplication::quit);
    connect(ui->buttonHelp, &QToolButton::clicked, [this]() {
        AboutDialog(size() * 0.9, this).exec();
    });
    if (col + row > 0)
        menu->addSeparator();

    menu->addAction(tr("Terminate Launcher"), QApplication::instance(), &QApplication::quit);
    trayIcon->setContextMenu(menu);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &QWidget::show);
    trayIcon->show();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}

void Widget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    toggleWindow->setText(tr("Hide Launcher"));
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            size(),
            QApplication::desktop()->screenGeometry(this))
        );
}

void Widget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    toggleWindow->setText(tr("Show Launcher"));
}
