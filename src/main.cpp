#include <QtGui/QApplication>
#include "mainwindow.h"
#include "duktoprotocol.h"

int main(int argc, char *argv[])
{
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
