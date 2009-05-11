#include "oslib.h"

#include <QHostInfo>

#if defined(Q_WS_WIN)
#define PLATFORM "Windows"
#elif defined(Q_WS_MAC)
#define PLATFORM "Apple Mac"
#elif defined(Q_WS_X11)
#define PLATFORM "Linux"
#endif

QString OsLib::removeUrlPrefix(QString url)
{
#if defined(Q_WS_WIN)
    url.replace("file:///", "");
#else
    url.replace("file://", "");
#endif
    return url;
}

QString OsLib::retrieveSystemName()
{
    static QString systemName = "";
    if (systemName != "") return systemName;

    QString uname(getenv("USERNAME"));
    if (uname == "") uname = getenv("USER");
    if (uname == "") uname = "Unknown";
    systemName = uname + " at " + QHostInfo::localHostName() + " (" + PLATFORM + ")";
    return systemName;
}
