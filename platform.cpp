/* DUKTO - A simple, fast and multi-platform file transfer tool for LAN users
 * Copyright (C) 2011 Emanuele Colombo
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

#include "platform.h"

#include <QString>
#include <QHostInfo>
#include <QFile>
#include <QDir>

// Returns the system username
QString Platform::getSystemUsername()
{
    // Save in a static variable so that It's always ready
    static QString username = "";
    if (username != "") return username;

    // Looking for the username
    QString uname(getenv("USERNAME"));
    if (uname == "") uname = getenv("USER");
    if (uname == "") uname = "Unknown";
    uname = uname.left(1).toUpper() + uname.mid(1);
    username = uname;

    return uname;
}

// Returns the hostname
QString Platform::getHostname()
{
    // Save in a static variable so that It's always ready
    static QString hostname = "";
    if (hostname != "") return hostname;

    // Get the hostname
    // (replace ".local" for MacOSX)
    hostname = QHostInfo::localHostName().replace(".local", "");

    return hostname;
}

// Returns the platform name
QString Platform::getPlatformName()
{
#if defined(Q_WS_WIN)
    return "Windows";
#elif defined(Q_WS_MAC)
    return "Macintosh";
#elif defined(Q_WS_X11)
    return "Linux";
#elif defined(Q_WS_S60)
    return "Symbian";
#else
    return "Unknown";
#endif
}

// Returns the platform avatar path
QString Platform::getAvatarPath()
{
#if defined(Q_WS_WIN)
    QString username = getSystemUsername().replace("\\", "+");
    QString path = QString(getenv("LOCALAPPDATA")) + "\\Temp\\" + username + ".bmp";
    if (!QFile::exists(path))
        path = QString(getenv("PROGRAMDATA")) + "\\Microsoft\\User Account Pictures\\Guest.bmp";
    if (!QFile::exists(path))
        path = QString(getenv("ALLUSERSPROFILE")) + "\\" + QDir(getenv("APPDATA")).dirName() + "\\Microsoft\\User Account Pictures\\" + username + ".bmp";
    if (!QFile::exists(path))
        path = QString(getenv("ALLUSERSPROFILE")) + "\\" + QDir(getenv("APPDATA")).dirName() + "\\Microsoft\\User Account Pictures\\Guest.bmp";
    return path;
#elif defined(Q_WS_MAC)
    return "";
#elif defined(Q_WS_X11)
    return getLinuxAvatarPath();
#elif defined(Q_WS_S60)
    return "";
#else
    return "";
#endif
}

// Returns the platform default output path
QString Platform::getDefaultPath()
{
    // For Windows it's the Desktop folder
#if defined(Q_WS_WIN)
    return QString(getenv("USERPROFILE")) + "\\Desktop";
#elif defined(Q_WS_MAC)
    return QString(getenv("HOME")) + "/Desktop";
#elif defined(Q_WS_X11)
    return QString(getenv("HOME"));
#elif defined(Q_WS_S60)
    return "E:\\Dukto";
#else
    #error "Unknown default path for this platform"
#endif

}

// Special function for Linux
QString Platform::getLinuxAvatarPath()
{
    QString path;

    // Gnome2 check
    path = QString(getenv("HOME")) + "/.face";
    if (QFile::exists(path)) return path;

    // Gnome3 check
    QFile f("/var/lib/AccountsService/users/" + QString(getenv("USER")));
    if (!f.open(QFile::ReadOnly)) return "";
    QTextStream ts(&f);
    QString line;
    bool found = false;
    while (true) {
        line = ts.readLine();
        if (line.isNull()) break;
        if (line.startsWith("Icon=")) {
            QRegExp re("^Icon=(.*)$");
            if (re.indexIn(line) == -1) continue;
            QStringList pathlist = re.capturedTexts();
            path = pathlist[1];
            found = true;
            break;
        }
    }
    f.close();
    if (found && QFile::exists(path)) return path;

    // Not found
    return "";
}
