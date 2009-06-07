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
