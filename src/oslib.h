/* DUKTO - A simple, fast and multi-platform file transfer tool for LAN users
 * Copyright (C) 2009 Emanuele Colombo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef OSLIB_H
#define OSLIB_H

#include <QString>

#if defined(Q_WS_WIN)
#define PLATFORM "Windows"
#elif defined(Q_WS_MAC)
#define PLATFORM "Macintosh"
#elif defined(Q_WS_X11)
#define PLATFORM "Linux"
#endif

#if defined(Q_WS_WIN)
#include <windows.h>
/*#elif defined(Q_WS_X11)
#include <stdlib.h>*/
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
/*
    static inline void openFile(QString path) {
    #if defined(Q_WS_WIN)
        ::ShellExecute(NULL, NULL, path.toStdWString().c_str(), NULL, NULL, SW_SHOWNORMAL);
    #elif defined(Q_WS_X11)
        system(("xdg-open \"" + path + "\"").toStdString().c_str());
    #elif defined(Q_WS_MAC)
        system(("open \"" + path + "\"").toStdString().c_str());
    #endif
    }

    static inline void openFolder(QString path) {
    #if defined(Q_WS_WIN)
        ::ShellExecute(NULL, NULL, path.toStdWString().c_str(), NULL, NULL, SW_SHOWNORMAL);
    #elif defined(Q_WS_X11)
        system(("xdg-open \"" + path + "\"").toStdString().c_str());
    #elif defined(Q_WS_MAC)
        system(("open \"" + path + "\"").toStdString().c_str());
    #endif
    }
*/
private:
    OsLib() { }

};

#endif // OSLIB_H
