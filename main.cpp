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

#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"

#include "guibehind.h"
#include "duktowindow.h"

#ifndef Q_WS_S60
#include "qtsingleapplication.h"
#endif

int main(int argc, char *argv[])
{
#if defined(Q_WS_X11)
    QApplication::setGraphicsSystem("raster");
#elif defined (Q_WS_WIN)
    qputenv("QML_ENABLE_TEXT_IMAGE_CACHE", "true");
#endif

#if defined(Q_WS_S60)
    QApplication app(argc, argv);
#else
    // Check for single running instance    
    QtSingleApplication app(argc, argv);
    if (app.isRunning()) {
        app.sendMessage("FOREGROUND");
        return 0;
    }
#endif

    DuktoWindow viewer;
#ifndef Q_WS_S60
    app.setActivationWindow(&viewer, true);
#endif
    GuiBehind gb(&viewer);
    viewer.showExpanded();
    app.installEventFilter(&gb);

    return app.exec();
}
