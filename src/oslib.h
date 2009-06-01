#ifndef OSLIB_H
#define OSLIB_H

#include <QString>

#if defined(Q_WS_WIN)
#include <windows.h>
#endif

class OsLib
{
public:
    static QString removeUrlPrefix(QString url);
    static QString retrieveSystemName();

    static inline QString adaptPath(QString path) {
        #if defined(Q_WS_WIN)
        path.replace('/', "\\");
        #endif
        return path;
    }

    static inline void openFile(QString path) {
        #if defined(Q_WS_WIN)
        ::ShellExecute(NULL, NULL, path.toStdWString().c_str(), NULL, NULL, SW_SHOWNORMAL);
        #endif
    }

private:
    OsLib() { }

};

#endif // OSLIB_H
