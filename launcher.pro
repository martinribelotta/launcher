#-------------------------------------------------
#
# Project created by QtCreator 2019-06-20T20:48:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = applauncher
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        aboutdialog.cpp \
        launcheritem.cpp \
        main.cpp \
        MainWidget.cpp

HEADERS += \
        MainWidget.h \
        aboutdialog.h \
        launcheritem.h

FORMS += \
        MainWidget.ui \
        aboutdialog.ui \
        launcheritem.ui

# Single Application implementation
#include(SingleApplication/singleapplication.pri)
#DEFINES += QAPPLICATION_CLASS=QApplication

unix {
    QMAKE_LFLAGS_RELEASE += -static-libstdc++ -static-libgcc
    QMAKE_LFLAGS_DEBUG += -static-libstdc++ -static-libgcc
    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    target.path = $$PREFIX/bin

    desktopfile.files = applauncher.desktop
    desktopfile.path = $$PREFIX/share/applications

    appimageintegration.file = desktop-integration.sh
    appimageintegration.path = $$PREFIX/bin

    iconfiles.files = resources/applauncher.svg resources/applauncher.png
    iconfiles.path = $$PREFIX/share/icons/default/256x256/apps/

    INSTALLS += desktopfile
    INSTALLS += iconfiles
    INSTALLS += target
    INSTALLS += appimageintegration
}

DISTFILES += \
    applauncher.desktop \
    desktop-integration.sh

RESOURCES += \
    images.qrc
