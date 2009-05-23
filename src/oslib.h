#ifndef OSLIB_H
#define OSLIB_H

#include <QString>

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


private:
    OsLib() { }

};

#endif // OSLIB_H
