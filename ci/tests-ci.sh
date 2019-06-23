#!/bin/bash

set -x

source /opt/qt*/bin/qt*-env.sh

set -e

qmake --version

VERSION=$(git rev-parse --short HEAD)

INSTALL_DIR=/tmp/applauncher
DEPLOY_OPT="-no-translations -verbose=2 -executable=$INSTALL_DIR/usr/bin/applauncher"
DESKTOP_FILE=$INSTALL_DIR/usr/share/applications/applauncher.desktop

echo ************** LINUX BUILD ***********************

qmake CONFIG+=release CONFIG+=force_debug_info
make -j4
make install INSTALL_ROOT=${INSTALL_DIR}
linuxdeployqt $DESKTOP_FILE $DEPLOY_OPT -appimage
echo ************** WINDOWS BUILD ***********************

make distclean
export MXE_PREFIX=i686-w64-mingw32.static
export MXE=/usr/lib/mxe/usr
export MXEQT=${MXE}/${MXE_PREFIX}/qt5
export PATH=${MXE}/bin:${PATH}
${MXEQT}/bin/qmake CONFIG+=release CONFIG+=force_debug_info
make -j4
(cd release && zip ../Application_Launcher-${VERSION}-win32.zip applauncher.exe)
