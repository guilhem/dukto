#include <QtGui/QApplication>
#include "mainwindow.h"
#include "duktoprotocol.h"

#include <QDir>

int main(int argc, char *argv[])
{
    if (argc == 2) {
        QDir c(".");
        if (c.exists(argv[1])) QDir::setCurrent(argv[1]);
    }
    
    QApplication a(argc, argv);
    MainWindow w;
    DuktoProtocol p;
    w.setProtocolReference(&p);

    p.sayHello(QHostAddress::Broadcast);

    w.show();
    int ret = a.exec();
    p.sayGoodbye();
    return ret;
}
