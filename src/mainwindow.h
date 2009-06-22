#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QListWidgetItem>
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
    void sendFileComplete(QStringList *files);
    void sendFileError(int e);
    void receiveFileStart();
    void receiveFileComplete(QStringList *files);
    void receiveFileCancelled();
    void transferStatusUpdate(int p);

private:
    void startFileTransfer(QStringList files);
    void log(QString text, QString filename);

    Ui::MainWindowClass *ui;
    DuktoProtocol *mProtocol;


private slots:
    void on_buttonChangeDir_clicked();
    void on_listLog_itemDoubleClicked(QListWidgetItem* item);
    void on_listPeers_itemDoubleClicked(QListWidgetItem* item);
    void on_textDestination_textChanged(QString );
    void on_buttonSendToIp_clicked();
};

#endif // MAINWINDOW_H
