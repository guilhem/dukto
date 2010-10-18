#-------------------------------------------------
#
# Project created by QtCreator 2010-06-30T19:14:12
#
#-------------------------------------------------

QT       += core gui network

TARGET = Dukto
TEMPLATE = app

CONFIG += static

LIBS += libole32

SOURCES += \ 
    peer.cpp \
    oslib.cpp \
    mainwindow.cpp \
    main.cpp \
    duktoprotocol.cpp \
    textdialog.cpp \
    listwidgetpeeritem.cpp \
    listwidgetlogitem.cpp \
    ecwin7.cpp

HEADERS  += \ 
    peer.h \
    oslib.h \
    mainwindow.h \
    duktoprotocol.h \
    textdialog.h \
    listwidgetpeeritem.h \
    listwidgetlogitem.h \
    ecwin7.h

FORMS    += \ 
    mainwindow.ui \
    textdialog.ui

RESOURCES += \
    D:/Home/Progetti/Dukto/SVN/trunk/src/resources.qrc

RC_FILE = dukto.rc

win32 {
QMAKE_LFLAGS += -static-libgcc
}

include(3rd/qtsingleapplication/qtsingleapplication.pri)
