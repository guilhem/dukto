#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include "duktoprotocol.h"

namespace Ui
{
    class MainWindowClass;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setProtocolReference(DuktoProtocol *p);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

public slots:
    void refreshPeerList();
    void sendFileComplete();
    void receiveFileStart();
    void receiveFileComplete(QString name);
    void transferStatusUpdate(int p);

private:
    void startFileTransfer(QStringList files);

    Ui::MainWindowClass *ui;
    DuktoProtocol *mProtocol;

};

#endif // MAINWINDOW_H
