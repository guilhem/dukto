#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "listwidgetpeeritem.h"
#include "oslib.h"

#include <QtGui/QMessageBox>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass), mProtocol(NULL)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setProtocolReference(DuktoProtocol *p)
{
    mProtocol = p;
    connect(p, SIGNAL(peerListChanged()), this, SLOT(refreshPeerList()));
    connect(p, SIGNAL(sendFileComplete()), this, SLOT(sendFileComplete()));
    connect(p, SIGNAL(receiveFileStart()), this, SLOT(receiveFileStart()));
    connect(p, SIGNAL(receiveFileComplete(QString)), this, SLOT(receiveFileComplete(QString)));
    connect(p, SIGNAL(transferStatusUpdate(int)), this, SLOT(transferStatusUpdate(int)), Qt::DirectConnection);
}

void MainWindow::refreshPeerList()
{
    QHash<QString, Peer>& peers = mProtocol->getPeers();
    QStringList list;
    QHashIterator<QString, Peer> i(peers);
    ui->listPeers->clear();
    while (i.hasNext()) {
        i.next();
        new ListWidgetPeerItem(ui->listPeers, i.value().name, i.key());
    }
    if (ui->listPeers->count() > 0)
        ui->listPeers->setCurrentRow(0);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasFormat("text/uri-list"))
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
    if (ui->toolBox->currentIndex() == 0)
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
    else
    {
        dest = ui->textDestination->text();
        if (dest.length() == 0) {
            QMessageBox::critical(this, "Error", "No ip address or hostname specified.");
            return;
        }
    }

    ui->toolBox->setEnabled(false);
    ui->statusBar->showMessage("Sending file...");
    ui->progressBar->setValue(0);
    mProtocol->sendFile(dest, files.at(0));
}

void MainWindow::sendFileComplete()
{
    ui->statusBar->showMessage("File sent.");
    ui->toolBox->setEnabled(true);
}

void MainWindow::receiveFileStart()
{
    ui->progressBar->setValue(0);
    ui->toolBox->setEnabled(false);
    ui->statusBar->showMessage("Receiving file...");
}

void MainWindow::receiveFileComplete(QString name)
{
    ui->toolBox->setEnabled(true);
    ui->statusBar->showMessage("File '" + name +  "' received.");
}

void MainWindow::transferStatusUpdate(int p)
{
    ui->progressBar->setValue(p);
}
