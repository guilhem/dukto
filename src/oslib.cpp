#include "oslib.h"

QString OsLib::removeUrlPrefix(QString url)
{
#if defined(Q_WS_WIN)
    url.replace("file:///", "");
#else
    url.replace("file://", "");
#endif
    return url;
}
