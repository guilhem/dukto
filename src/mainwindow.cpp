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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "listwidgetpeeritem.h"
#include "listwidgetlogitem.h"
#include "oslib.h"
#include "textdialog.h"

#include <QtGui/QFileDialog>
#include <QUrl>
#include <QTime>
#include <QDir>
#include <QtGui/QMessageBox>
#include <QNetworkInterface>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass), mProtocol(NULL)
{

    // Main window initialization
    ui->setupUi(this);

    // Taskbar integration with Win7
    win7.init(this->winId());

    // Window data setup
    ui->statusBar->showMessage("Drag some files and folders in this window to send them.");
    ui->labelOutput->setText(OsLib::adaptPath(QDir::currentPath()));
    ui->labelIPs->setText(ui->labelIPs->text() + getAllIPString());

    // Context menu actions for buddy list
    QAction *action;
    action = new QAction("Send files...", this);
    connect(action, SIGNAL(triggered()), this, SLOT(contextMenu_sendFiles()));
    ui->listPeers->addAction(action);
    action = new QAction("Send text...", this);
    action->setPriority(QAction::HighPriority);
    connect(action, SIGNAL(triggered()), this, SLOT(contextMenu_sendText()));
    ui->listPeers->addAction(action);
    ui->listPeers->setContextMenuPolicy(Qt::PreventContextMenu);
}

MainWindow::~MainWindow()
{
    delete ui;
}

#ifdef Q_WS_WIN
bool MainWindow::winEvent(MSG * message, long * result)
{
    return win7.winEvent(message, result);
}
#endif

void MainWindow::setProtocolReference(DuktoProtocol *p)
{
    mProtocol = p;
    connect(p, SIGNAL(peerListChanged()), this, SLOT(refreshPeerList()));
    connect(p, SIGNAL(sendFileComplete(QStringList*)), this, SLOT(sendFileComplete(QStringList*)));
    connect(p, SIGNAL(sendFileError(int)), this, SLOT(sendFileError(int)));
    connect(p, SIGNAL(receiveFileStart()), this, SLOT(receiveFileStart()));
    connect(p, SIGNAL(receiveFileComplete(QStringList*)), this, SLOT(receiveFileComplete(QStringList*)));
    connect(p, SIGNAL(receiveTextComplete(QString*)), this, SLOT(receiveTextComplete(QString*)));
    connect(p, SIGNAL(receiveFileCancelled()), this, SLOT(receiveFileCancelled()));
    connect(p, SIGNAL(transferStatusUpdate(int)), this, SLOT(transferStatusUpdate(int)), Qt::DirectConnection);
}

// Aggiornamento lista dei client
void MainWindow::refreshPeerList()
{
    QHash<QString, Peer>& peers = mProtocol->getPeers();
    QHashIterator<QString, Peer> i(peers);

    // Salvataggio elemento corrente
    QString current;
    if (ui->listPeers->selectedItems().count() > 0)
    {
        ListWidgetPeerItem *i = static_cast<ListWidgetPeerItem*>(ui->listPeers->selectedItems().at(0));
        current = i->getPeerKey();
    }

    // Cancellazione lista corrente
    ui->listPeers->clear();

    // Riempimento lista
    while (i.hasNext())
    {
        i.next();
        new ListWidgetPeerItem(ui->listPeers, i.value().name, i.key());
        if (i.key() == current)
            ui->listPeers->setCurrentRow(ui->listPeers->count() - 1);
    }

    // Selezione primo elemento
    if ((ui->listPeers->count() > 0) && (ui->listPeers->selectedItems().count() == 0))
        ui->listPeers->setCurrentRow(0);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasFormat("text/uri-list") && (!mProtocol->isBusy()))
        e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    QList<QUrl> urlList;
    QStringList files;

    if (e->mimeData()->hasUrls())
    {
        urlList = e->mimeData()->urls();
        for (int i = 0; i < urlList.size(); i++)
            files.append(OsLib::removeUrlPrefix(urlList[i].toString()));
        startFileTransfer(files);
    }

    e->acceptProposedAction();
}

void MainWindow::startFileTransfer(QStringList files)
{

    QString dest;
    if (ui->tabWidget->currentIndex() == 0)
    {
        if (ui->listPeers->selectedItems().count() > 0) {
            ListWidgetPeerItem *i = static_cast<ListWidgetPeerItem*>(ui->listPeers->selectedItems().at(0));
            dest = i->getPeerKey();
        }
        else {
            QMessageBox::critical(this, "Error", "No peer available as target.");
            return;
        }

    }
    else if (ui->tabWidget->currentIndex() == 1)
    {
        dest = ui->textDestination->text();
        if (dest.length() == 0) {
            QMessageBox::critical(this, "Error", "No ip address or hostname specified.");
            return;
        }
    }
    else
    {
        QMessageBox::critical(this, "Error", "Before dropping a file here, switch to the 'available clients' or to the 'manual IP connection' modes.");
        return;
    }

    setBusy(true, "Sending files...");
    ui->progressBar->setValue(0);
    win7.setProgressState(win7.Normal);
    win7.setProgressValue(0, 100);
    mProtocol->sendFile(dest, files);
}

void MainWindow::startTextTransfer(QString text)
{

    QString dest;
    if (ui->tabWidget->currentIndex() == 0)
    {
        if (ui->listPeers->selectedItems().count() > 0) {
            ListWidgetPeerItem *i = static_cast<ListWidgetPeerItem*>(ui->listPeers->selectedItems().at(0));
            dest = i->getPeerKey();
        }
        else {
            QMessageBox::critical(this, "Error", "No peer available as target.");
            return;
        }

    }
    else if (ui->tabWidget->currentIndex() == 1)
    {
        dest = ui->textDestination->text();
        if (dest.length() == 0) {
            QMessageBox::critical(this, "Error", "No ip address or hostname specified.");
            return;
        }
    }

    setBusy(true, "Sending text...");
    ui->progressBar->setValue(0);
    win7.setProgressState(win7.Normal);
    win7.setProgressValue(0, 100);
    mProtocol->sendText(dest, text);
}

void MainWindow::sendFileComplete(QStringList *files)
{
    if (files->count() == 1)
    {
        QFileInfo fi(files->at(0));
        if (fi.fileName() == "___DUKTO___TEXT___") {
            ui->statusBar->showMessage("Text sent.");
            log("Text sent.", "");
        }
        else {
            ui->statusBar->showMessage("File '" + fi.fileName() + "' sent.");
            log("File '" + fi.fileName() + "' sent.", files->at(0));
        }
    }
    else
    {
        ui->statusBar->showMessage("Multiple files and folders sent.");
        log("Multiple files and folders sent.", "");
    }

    setBusy(false);
    win7.setProgressState(win7.NoProgress);
}

void MainWindow::receiveFileStart()
{
    setBusy(true, "Receiving...");
    ui->progressBar->setValue(0);
    win7.setProgressValue(0, 100);
    win7.setProgressState(win7.Normal);
}

void MainWindow::receiveFileComplete(QStringList *files)
{
    if (files->count() == 1)
    {
        ui->statusBar->showMessage("File '" + files->at(0) +  "' received.");
        QString path = QDir::currentPath() + "/" + files->at(0);
        path.replace("//", "/");
        log("File '" + files->at(0) +  "' received.", path);
    }
    else
    {
        ui->statusBar->showMessage("Multiple files and folders received.");
        log("Multiple files and folders received.", "");
    }
    win7.setProgressState(win7.NoProgress);
    QApplication::alert(this, 3000);
    setBusy(false);
}

void MainWindow::receiveFileCancelled()
{
    setBusy(false, "Transfer cancelled.");
    ui->progressBar->setValue(0);
    ui->statusBar->showMessage("Transfer cancelled.");
    log("Transfer cancelled.", "");
    win7.setProgressState(win7.Error);
}

void MainWindow::transferStatusUpdate(int p)
{
    ui->progressBar->setValue(p);
    win7.setProgressValue(p, 100);
}

void MainWindow::on_buttonSendToIp_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Select file to send", "", "Any file (*.*)");
    if (files.count() == 0) return;
    startFileTransfer(files);
}

void MainWindow::on_textDestination_textChanged(QString s)
{
    ui->buttonSendToIp->setEnabled((s != ""));
    ui->buttonSendTextToIp->setEnabled((s != ""));
}

void MainWindow::sendFileError(int e)
{
    ui->progressBar->setValue(0);
    setBusy(false, "Error while sending data.");
    win7.setProgressState(win7.Error);
    QMessageBox::critical(this, "Send file", "An error has occurred while sending data (" + QString::number(e, 10) + ").");
}

void MainWindow::on_listPeers_itemDoubleClicked(QListWidgetItem* item)
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Select file to send", "", "Any file (*.*)");
    if (files.count() == 0) return;
    startFileTransfer(files);
}

void MainWindow::log(QString text, QString filename)
{
    new ListWidgetLogItem(ui->listLog, QTime::currentTime().toString("HH.mm") + ": " + text, filename);
}

void MainWindow::on_listLog_itemDoubleClicked(QListWidgetItem* item)
{
    ListWidgetLogItem *i = static_cast<ListWidgetLogItem*>(ui->listLog->selectedItems().at(0));
    if (i->getFilename() != "")
    {
        //QString n = QDir::currentPath() + "/" + i->getFilename();
        OsLib::openFile(i->getFilename());
    }
}

void MainWindow::on_buttonChangeDir_clicked()
{
    QString dirname = QFileDialog::getExistingDirectory(this, "Select the folder where received files will be saved:", ".",
                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirname == "") return;
    QDir::setCurrent(dirname);
    ui->labelOutput->setText(OsLib::adaptPath(QDir::currentPath()));
}

void MainWindow::on_buttonSendTextToIp_clicked()
{
    // Visualizzazione finestra di input testo
    TextDialog td(this);
    td.setMode(true);
    int ret = td.exec();
    if (ret != QDialog::Accepted) return;

    // Invio dati
    QString text = td.contents();
    startTextTransfer(text);

}

void MainWindow::contextMenu_sendFiles()
{
    // Recupero elemento selezionato
    ListWidgetPeerItem *i = NULL;
    if (ui->listPeers->selectedItems().count() > 0) {
        i = static_cast<ListWidgetPeerItem*>(ui->listPeers->selectedItems().at(0));
    }
    else {
        QMessageBox::critical(this, "Error", "No peer available as target.");
        return;
    }

    // Gestisco l'evento come un doppio click
    on_listPeers_itemDoubleClicked(i);

}

void MainWindow::contextMenu_sendText()
{
    // Recupero l'elemento selezionato
    if (ui->listPeers->selectedItems().count() == 0) {
        QMessageBox::critical(this, "Error", "No peer available as target.");
        return;
    }

    // Visualizzazione finestra di input testo
    TextDialog td(this);
    td.setMode(true);
    int ret = td.exec();
    if (ret != QDialog::Accepted) return;

    // Invio dati
    QString text = td.contents();
    startTextTransfer(text);
}

QString MainWindow::getAllIPString()
{
    // Recupero tutti gli indirizzi IP
    QList<QHostAddress> addrs = QNetworkInterface::allAddresses();
    QString ret = "";
    for (int i = 0; i < addrs.length(); i++)
        if ((addrs[i].protocol() == QAbstractSocket::IPv4Protocol)
            && (addrs[i].toString() != "127.0.0.1"))
            ret += ", " + addrs[i].toString();
    if (ret.length() > 0) ret = ret.mid(2);
    return ret;
}

void MainWindow::receiveTextComplete(QString *text)
{
    // Ripristino finestra principale
    setBusy(false, "Text message received");
    log("Text message received.", "");

    // Apertura finestra con messaggio di testo
    TextDialog *td = new TextDialog(this);
    td->setModal(false);
    td->setMode(false);
    td->setContents(*text);
    td->show();

    // Alert applicazione
    win7.setProgressState(win7.NoProgress);
    QApplication::alert(this, 3000);
}

void MainWindow::on_buttonOpenDir_clicked()
{
    // Apertura finestra del sistema operativo
    // relativa alla cartella corrente
    OsLib::openFolder(OsLib::adaptPath(QDir::currentPath()));
}

void MainWindow::on_listPeers_itemSelectionChanged()
{
    if (ui->listPeers->selectedItems().count() > 0)
        ui->listPeers->setContextMenuPolicy(Qt::ActionsContextMenu);
    else
        ui->listPeers->setContextMenuPolicy(Qt::PreventContextMenu);
}

void MainWindow::setBusy(bool busy)
{
    ui->tabWidget->setEnabled(!busy);
    ui->buttonChangeDir->setEnabled(!busy);
    ui->buttonOpenDir->setEnabled(!busy);
    if (busy)
        win7.setOverlayIcon("IDI_OVERLAY", "Transfering data...");
    else
        win7.setOverlayIcon("", "");
}

void MainWindow::setBusy(bool busy, QString status)
{
    setBusy(busy);
    ui->statusBar->showMessage(status);
}
