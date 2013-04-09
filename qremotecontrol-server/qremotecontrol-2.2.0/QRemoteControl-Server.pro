# -------------------------------------------------
# Project created by QtCreator 2009-07-27T19:26:51
# -------------------------------------------------
TARGET = qremotecontrol-server
VERSION = 2.2.0
DEFINES += VERSION=\"\\\"$$VERSION\\\"\"

include(./qtsingleapplication/qtsingleapplication.pri)

QT += core \
      gui \
      network
TEMPLATE = app
SOURCES += main.cpp \
    qremotecontrolserver.cpp \
    qglobalfakekey.cpp \
    usercommanddialog.cpp \
    helpdialog.cpp \
    icondialog.cpp

HEADERS += qremotecontrolserver.h \
    qglobalfakekey.h \
    usercommanddialog.h \
    helpdialog.h \
    icondialog.h

FORMS += qremotecontrolserver.ui \
    usercommanddialog.ui \
    helpdialog.ui \
    icondialog.ui

linux-g++ | linux-g++-64 | linux-g++-32 {
#    QT += dbus
    LIBS += -lX11 \
            -lXtst
    SOURCES += qglobalfakekey_x11.cpp
# playwolf/playercontroller.cpp
    HEADERS += qglobalfakekey_x11.h
# playwolf/playercontroller.h
}
windows {
    LIBS += -lUser32
    SOURCES += qglobalfakekey_win.cpp
    HEADERS += qglobalfakekey_win.h
    RC_FILE = icon.rc
}

RESOURCES += icons/icons.qrc \
    images/images.qrc

target.path = /usr/bin

desktop.path = /usr/share/applications
desktop.files = misc/qremotecontrol.desktop

icon.path = /usr/share/pixmaps
icon.files = icons/qremotecontrol.png

INSTALLS += target desktop icon
