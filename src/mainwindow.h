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
    void receiveTextComplete(QString *text);
    void receiveFileCancelled();
    void transferStatusUpdate(int p);
    void contextMenu_sendFiles();
    void contextMenu_sendText();

private:
    void startFileTransfer(QStringList files);
    void startTextTransfer(QString text);
    void log(QString text, QString filename);
    QString getAllIPString();

    Ui::MainWindowClass *ui;
    DuktoProtocol *mProtocol;


private slots:
    void on_listPeers_itemSelectionChanged();
    void on_buttonOpenDir_clicked();
    void on_buttonSendTextToIp_clicked();
    void on_buttonChangeDir_clicked();
    void on_listLog_itemDoubleClicked(QListWidgetItem* item);
    void on_listPeers_itemDoubleClicked(QListWidgetItem* item);
    void on_textDestination_textChanged(QString );
    void on_buttonSendToIp_clicked();

};

#endif // MAINWINDOW_H
