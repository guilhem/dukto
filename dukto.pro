# Add more folders to ship with the application, here
qml_folder.source = qml/dukto
qml_folder.target = qml
DEPLOYMENTFOLDERS = qml_folder

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

QT += network

win32:RC_FILE = dukto.rc
win32:LIBS += libWs2_32 libole32
win32:VERSION = 5.0.0

# Smart Installer package's UID
# This UID is from the protected range and therefore the package will
# fail to install if self-signed. By default qmake uses the unprotected
# range value if unprotected UID is defined for the application and
# 0x2002CCCF value if protected UID is given to the application
#symbian:DEPLOYMENT.installer_header = 0x2002CCCF

# Allow network access on Symbian
symbian:TARGET.CAPABILITY += NetworkServices


unix {
	TARGET = dukto
	target.path = /usr/bin
	INSTALLS += target
	
	icon.path = /usr/share/pixmaps
	icon.files = dukto.png
	INSTALLS += icon
	
	desktop.path = /usr/share/applications/
	desktop.files = dukto.desktop
	INSTALLS += desktop
}

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    guibehind.cpp \
    platform.cpp \
    buddylistitemmodel.cpp \
    duktoprotocol.cpp \
    miniwebserver.cpp \
    ipaddressitemmodel.cpp \
    recentlistitemmodel.cpp \
    settings.cpp \
    destinationbuddy.cpp \
    duktowindow.cpp \
    ecwin7.cpp \
    theme.cpp \
    updateschecker.cpp

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

HEADERS += \
    guibehind.h \
    platform.h \
    buddylistitemmodel.h \
    duktoprotocol.h \
    peer.h \
    miniwebserver.h \
    ipaddressitemmodel.h \
    recentlistitemmodel.h \
    settings.h \
    destinationbuddy.h \
    duktowindow.h \
    ecwin7.h \
    theme.h \
    updateschecker.h

RESOURCES += \
    qml.qrc

include(qtsingleapplication/qtsingleapplication.pri)

OTHER_FILES +=










