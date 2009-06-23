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

#include "3rd/qtsingleapplication/qtsingleapplication.h"
#include "mainwindow.h"
#include "duktoprotocol.h"

#include <QDir>
#include <QFile>

int main(int argc, char *argv[])
{
    if (argc == 2) {
        QDir c(".");
        if (c.exists(argv[1])) QDir::setCurrent(argv[1]);
    }
    
    QtSingleApplication a(argc, argv);
    if (a.isRunning()) {
        a.sendMessage("FOREGROUND");
        return 0;
    }

    MainWindow w;
    DuktoProtocol p;
    w.setProtocolReference(&p);

    p.sayHello(QHostAddress::Broadcast);

    w.show();
    a.setActivationWindow(&w, true);
    int ret = a.exec();
    p.sayGoodbye();
    return ret;
}
