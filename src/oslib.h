#ifndef OSLIB_H
#define OSLIB_H

#include <QString>

class OsLib
{
public:
    static QString removeUrlPrefix(QString url);
    static QString retrieveSystemName();

private:
    OsLib() { }

};

#endif // OSLIB_H
