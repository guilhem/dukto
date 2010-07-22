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

#include "duktoprotocol.h"

#include "oslib.h"
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QNetworkInterface>

#define UDP_PORT 4644
#define TCP_PORT 4644

DuktoProtocol::DuktoProtocol()
    : mSocket(NULL), mTcpServer(NULL), mCurrentSocket(NULL),
        mCurrentFile(NULL), mFilesToSend(NULL)
{
    mSocket = new QUdpSocket(this);
    mSocket->bind(QHostAddress::Any, UDP_PORT);
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(newUdpData()));

    mTcpServer = new QTcpServer(this);
    mTcpServer->listen(QHostAddress::Any, TCP_PORT);
    connect(mTcpServer, SIGNAL(newConnection()), this, SLOT(newIncomingConnection()));

    mIsSending = false;
    mIsReceiving = false;

}

DuktoProtocol::~DuktoProtocol()
{
    if (mCurrentSocket) delete mCurrentSocket;
    if (mSocket) delete mSocket;
    if (mTcpServer) delete mTcpServer;
    if (mCurrentFile) delete mCurrentFile;
}

void DuktoProtocol::sayHello(QHostAddress dest)
{
    // Preparazione pacchetto
    QByteArray *packet = new QByteArray();
    if (dest == QHostAddress::Broadcast)
        packet->append(0x01);           // 0x01 -> HELLO MESSAGE (broadcast)
    else
        packet->append(0x02);           // 0x02 -> HELLO MESSAGE (unicast)
    packet->append(OsLib::retrieveSystemName());

    // Invio pacchetto
    if (dest == QHostAddress::Broadcast)
        sendToAllBroadcast(packet);
    else
        mSocket->writeDatagram(packet->data(), packet->length(), dest, UDP_PORT);

    delete packet;
}

void DuktoProtocol::sayGoodbye()
{
    QByteArray *packet = new QByteArray();
    packet->append(0x03);               // 0x03 -> GOODBYE MESSAGE
    packet->append("Bye Bye");
    sendToAllBroadcast(packet);
    delete packet;
}

void DuktoProtocol::newUdpData()
{
    while (mSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(mSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        mSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        handleMessage(datagram, sender);
     }
}

void DuktoProtocol::handleMessage(QByteArray &data, QHostAddress &sender)
{
    char msgtype = data.at(0);

    switch(msgtype)
    {
        case 0x01:  // HELLO (broadcast)
        case 0x02:  // HELLO (unicast)
            data.remove(0, 1);
            if (data != OsLib::retrieveSystemName()) {
                mPeers[sender.toString()] = Peer(sender, data);
                if (msgtype == 0x01) sayHello(sender);
                peerListChanged();
            }
            break;

        case 0x03:  // GOODBYE
            mPeers.remove(sender.toString());
            peerListChanged();
            break;
    }

}

// Richiesta connessione TCP in ingresso
void DuktoProtocol::newIncomingConnection()
{

    // Verifica presenza connessione pendente
    if (!mTcpServer->hasPendingConnections()) return;

    // Recupero connessione
    QTcpSocket *s = mTcpServer->nextPendingConnection();

    // Se sto già ricevendo o inviando, rifiuto la connessione
    if (mIsReceiving || mIsSending)
    {
        s->close();
        return;
    }

    // Aggiornamento interfaccia grafica
    receiveFileStart();

    // Impostazione socket TCP corrente
    mCurrentSocket = s;

    // Attesa header della connessione (timeout 10 sec)
    if (!s->waitForReadyRead(10000))
    {
        // Non ho ricevuto l'header della connessione, chiudo
        mCurrentSocket->close();
        delete mCurrentSocket;
        mCurrentSocket = NULL;
        return;
    }

    // Registrazione gestori eventi socket
    connect(mCurrentSocket, SIGNAL(readyRead()), this, SLOT(readNewData()), Qt::DirectConnection);
    connect(mCurrentSocket, SIGNAL(disconnected()), this, SLOT(closedConnection()), Qt::DirectConnection);

    // Inizializzazione variabili
    mIsReceiving = true;
    mTotalReceivedData = 0;
    mElementSize = -1;
    mReceivedFiles = new QStringList();

    // -- Lettura header generale --
    // Numero entità da ricevere
    mCurrentSocket->read((char*) &mElementsToReceiveCount, sizeof(qint64));
    // Dimensione totale
    mCurrentSocket->read((char*) &mTotalSize, sizeof(qint64));

    // Inizio lettura dati sui file
    readNewData();

}

// Processo di lettura principale
void DuktoProtocol::readNewData()
{
    // Fino a che ci sono dati da leggere
    while (mCurrentSocket->bytesAvailable() > 0)
    {

        // Verifico se devo leggere un header
        if (mElementSize == -1)
        {
            // Lettura nome
            QString name;
            char c;

            // Check if part of the name has already been fetched
            if (mPartialName.length() > 0) {
                name = mPartialName;
                mPartialName = "";
            }
            while (1) {
                bool ret = mCurrentSocket->getChar(&c);
                if (!ret) {
                    mPartialName = name;
                    return;
                }
                if (c == '\0') break;
                name += c;
            }
            mReceivedFiles->append(name);

            // Lettura dimensioni
            mCurrentSocket->read((char*) &mElementSize, sizeof(qint64));
            mElementReceivedData = 0;

            // Se l'elemento corrente è una cartella, la creo e passo all'elemento successivo
            if (mElementSize == -1)
            {
                // Verifico il nome della cartella "root"
                QString rootName = name.section("/", 0, 0);

                // Se non ho ancora trattato questa root, lo faccio ora
                if (mRootFolderName != rootName) {

                    // Verifico se ho già una cartella con questo nome
                    // nel caso trovo un nome alternativo
                    int i = 2;
                    QString originalName = name;
                    while (QFile::exists(name))
                        name = originalName + " (" + QString::number(i++) + ")";
                    mRootFolderName = originalName;
                    mRootFolderRenamed = name;

                }

                // Se invece l'ho già trattata, allora rinomino questo percorso
                else if (mRootFolderName != mRootFolderRenamed)
                    name = name.replace(0, name.indexOf('/'), mRootFolderRenamed);

                // Creo la cartella
                QDir dir(".");
                dir.mkpath(name);
                continue;
            }

            // Potrebbe essere un invio di testo
            else if (name == "___DUKTO___TEXT___")
            {
                mReceivingText = true;
                mTextToReceive = "";
                mCurrentFile = NULL;
            }

            // Altrimenti creo il nuovo file
            else
            {
                // Se il file è in una cartella rinominata, devo provvedere di conseguenza
                if ((name.indexOf('/') != -1) && (name.section("/", 0, 0) == mRootFolderName))
                    name = name.replace(0, name.indexOf('/'), mRootFolderRenamed);

                // Se il file esiste già cambio il nome di quello nuovo
                int i = 2;
                QString originalName = name;
                while (QFile::exists(name)) {
                    QFileInfo fi(originalName);
                    name = fi.baseName() + " (" + QString::number(i) + ")." + fi.completeSuffix();
                    i++;
                }
                mCurrentFile = new QFile(name);
                mCurrentFile->open(QIODevice::WriteOnly);
                mReceivingText = false;
            }

        }

        // Provo a leggere quanto mi serve per finire il file corrente
        // (o per svuotare il buffer dei dati ricevuti)
        qint64 s = (mCurrentSocket->bytesAvailable() > (mElementSize - mElementReceivedData))
                    ? (mElementSize - mElementReceivedData)
                    : mCurrentSocket->bytesAvailable();
        QByteArray d = mCurrentSocket->read(s);
        mElementReceivedData += d.size();
        mTotalReceivedData += d.size();
        updateStatus();

        // Salvo i dati letti
        if (!mReceivingText)
            mCurrentFile->write(d);
        else
            mTextToReceive += d;

        // Verifico se ho completato l'elemento corrente
        if (mElementReceivedData == mElementSize)
        {
            // Completato, chiudo il file e mi preparo per il prossimo elemento
            mElementSize = -1;
            if (!mReceivingText)
            {
                mCurrentFile->close();
                delete mCurrentFile;
                mCurrentFile = NULL;
            }
        }
    }

}

// Chiusura della connessione TCP in ricezione
void DuktoProtocol::closedConnection()
{
    // Svuoto il buffer in ricezione
    readNewData();

    // Chiusura eventuale file corrente
    if (mCurrentFile)
    {
        QString name;
        name = mCurrentFile->fileName();
        mCurrentFile->close();
        delete mCurrentFile;
        mCurrentFile = NULL;
        QFile::remove(name);
        receiveFileCancelled();
    }

    // Ricezione file conclusa
    else if (!mReceivingText)
        receiveFileComplete(mReceivedFiles);

    // Ricezione testo conclusa
    else
        receiveTextComplete(&mTextToReceive);

    // Chiusura socket
    if (mCurrentSocket)
    {
        mCurrentSocket->disconnect();
        mCurrentSocket->disconnectFromHost();
        mCurrentSocket->close();
        mCurrentSocket->deleteLater();
        mCurrentSocket = NULL;
    }

    // Rilascio memoria
    delete mReceivedFiles;
    mReceivedFiles = NULL;

    // Impostazione stato
    mIsReceiving = false;

}

void DuktoProtocol::sendFile(QString ipDest, QStringList files)
{
    // Verifica altre attività in corso
    if (mIsReceiving || mIsSending) return;
    mIsSending = true;

    // File da inviare
    mFilesToSend = expandTree(files);
    mFileCounter = 0;

    // Connessione al destinatario
    mCurrentSocket = new QTcpSocket(this);
    connect(mCurrentSocket, SIGNAL(connected()), this, SLOT(sendMetaData()), Qt::DirectConnection);
    connect(mCurrentSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(sendConnectError(QAbstractSocket::SocketError)), Qt::DirectConnection);
    connect(mCurrentSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(sendData(qint64)), Qt::DirectConnection);
    mCurrentSocket->connectToHost(ipDest, TCP_PORT);
}

void DuktoProtocol::sendText(QString ipDest, QString text)
{
    // Verifica altre attività in corso
    if (mIsReceiving || mIsSending) return;
    mIsSending = true;

    // Testo da inviare
    mFilesToSend = new QStringList();
    mFilesToSend->append("___DUKTO___TEXT___");
    mFileCounter = 0;
    mTextToSend = text;

    // Connessione al destinatario
    mCurrentSocket = new QTcpSocket(this);
    connect(mCurrentSocket, SIGNAL(connected()), this, SLOT(sendMetaData()), Qt::DirectConnection);
    connect(mCurrentSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(sendConnectError(QAbstractSocket::SocketError)), Qt::DirectConnection);
    connect(mCurrentSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(sendData(qint64)), Qt::DirectConnection);
    mCurrentSocket->connectToHost(ipDest, TCP_PORT);
}

void DuktoProtocol::sendMetaData()
{
    // Header
    //  - N. entità (file, cartelle, ecc...)
    //  - Dimensione totale
    //  - Nome primo file
    //  - Dimensione primo (e unico) file (-1 per una cartella)

    QByteArray header;
    qint64 tmp;

    // N. entità
    tmp = mFilesToSend->count();
    header.append((char*) &tmp, sizeof(tmp));
    // Dimensione totale
    mTotalSize = computeTotalSize(mFilesToSend);
    header.append((char*) &mTotalSize, sizeof(mTotalSize));

    // Primo elemento
    header.append(nextElementHeader());

    // Invio header
    mCurrentSocket->write(header);

    // Inizializzazione variabili
    mTotalSize += header.size();
    mSentData = 0;
    mSentBuffer = 0;

    // Aggiornamento interfaccia utente
    updateStatus();

}

void DuktoProtocol::sendData(qint64 b)
{
    QByteArray d;

    // Aggiornamento statistiche
    mSentData += b;
    updateStatus();

    // Verifica se tutti i dati messi nel buffer
    // sono stati inviati
    mSentBuffer -= b;

    // Se ci sono altri dati da inviare, attendo
    // che vengano inviati
    if (mSentBuffer > 0) return;

    // Se si tratta di un invio testuale, butto dentro
    // tutto il testo
    if ((!mTextToSend.isEmpty()) && (mFilesToSend->at(mFileCounter - 1) == "___DUKTO___TEXT___"))
    {
        d.append(mTextToSend.toUtf8().data());
        mCurrentSocket->write(d);
        mSentBuffer = d.size();
        mTextToSend.clear();
        return;
    }

    // Se il file corrente non è ancora terminato
    // invio una nuova parte del file
    if (mCurrentFile)
        d = mCurrentFile->read(10000);
    if (d.size() > 0)
    {
        mCurrentSocket->write(d);
        mSentBuffer = d.size();
        return;
    }

    // Altrimenti chiudo il file e passo al prossimo
    d.append(nextElementHeader());

    // Non ci sono altri file da inviare?
    if (d.size() == 0)
    {
        // Chiudo la connessione
        mCurrentSocket->disconnect();
        mCurrentSocket->disconnectFromHost();
        mCurrentSocket->waitForDisconnected(1000);
        mCurrentSocket->close();
        mCurrentSocket->deleteLater();
        mCurrentSocket = NULL;
        if (mCurrentFile)
        {
            mCurrentFile->close();
            delete mCurrentFile;
            mCurrentFile = NULL;
        }
        mIsSending = false;
        sendFileComplete(mFilesToSend);
        delete mFilesToSend;
        mFilesToSend = NULL;
        return;
    }

    // Invio l'header insime al primo chunk di file
    mTotalSize += d.size();
    if (mCurrentFile)
        d.append(mCurrentFile->read(10000));
    mCurrentSocket->write(d);
    mSentBuffer += d.size();
    return;

}

// Aggiornamento delle statistiche di invio
void DuktoProtocol::updateStatus()
{
    if (mIsSending)
        transferStatusUpdate((int)((float)(mSentData) / (float)(mTotalSize) * 100));
    else if (mIsReceiving)
        transferStatusUpdate((int)((float)(mTotalReceivedData) / (float)(mTotalSize) * 100));
}

// In caso di errore di connessione
void DuktoProtocol::sendConnectError(QAbstractSocket::SocketError e)
{
    if (mCurrentSocket)
    {
        mCurrentSocket->close();
        delete mCurrentSocket;
        mCurrentSocket = NULL;
    }
    if (mCurrentFile)
    {
        mCurrentFile->close();
        delete mCurrentFile;
        mCurrentFile = NULL;
    }
    mIsSending = false;
    sendFileError(e);
}

// Dato un elenco di file e cartelle, viene espanso in modo da
// contenere tutti i file e le cartelle contenuti
QStringList* DuktoProtocol::expandTree(QStringList files)
{
    // Percorso base
    QString bp = files.at(0);
    if (bp.right(1) == "/") bp.chop(1);
    mBasePath = QFileInfo(bp).absolutePath();
    if (mBasePath.right(1) == "/") mBasePath.chop(1);

    // Iterazione sugli elementi
    QStringList* expanded = new QStringList();
    for (int i = 0; i < files.count(); i++)
        addRecursive(expanded, files.at(i));

    return expanded;
}

// Aggiunge ricorsivamente tutte le cartelle e file contenuti in una cartella
void DuktoProtocol::addRecursive(QStringList *e, QString path)
{

    path.replace("//", "/");
    if (path.right(1) == "/") path.chop(1);
    e->append(path);

    QFileInfo fi(path);
    if (fi.isDir())
    {
        QStringList entries = QDir(path).entryList();
        for (int i = 0; i < entries.count(); i++)
        {
            QString entry = entries.at(i);
            if ((entry != ".") && (entry != ".."))
                addRecursive(e, path + "/" + entry);
        }
    }
}

// Restituisce l'header da inviare per il prossimo elemento
// da tramsettere
QByteArray DuktoProtocol::nextElementHeader()
{
    QByteArray header;

    // Ricava il nome del file (se non è l'ultimo)
    if (mFilesToSend->size() == mFileCounter) return header;
    QString fullname = mFilesToSend->at(mFileCounter++);

    // Chiusura file precedente, se non è già stato chiuso
    if (mCurrentFile) {
        mCurrentFile->close();
        delete mCurrentFile;
        mCurrentFile = NULL;
    }

    // Verifico se si tratta di un invio testo
    if (fullname == "___DUKTO___TEXT___") {
        header.append(fullname.toAscii() + '\0');
        qint64 size = mTextToSend.toUtf8().length();
        header.append((char*) &size, sizeof(size));
        return header;
    }

    // Nome elemento
    QString name = fullname;
    name.replace(mBasePath + "/", "");
    header.append(name.toAscii() + '\0');

    // Dimensione elemento
    qint64 size = -1;
    QFileInfo fi2(fullname);
    if (fi2.isFile()) size = fi2.size();
    header.append((char*) &size, sizeof(size));

    // Apertura file
    if (size > -1) {
        mCurrentFile = new QFile(fullname);
        mCurrentFile->open(QIODevice::ReadOnly);
    }

    return header;

}

// Calcola l'occupazione totale di tutti i file da trasferire
qint64 DuktoProtocol::computeTotalSize(QStringList *e)
{
    // Se è un invio testuale
    if ((e->length() == 1) && (e->at(0) == "___DUKTO___TEXT___"))
        return mTextToSend.toUtf8().length();

    // Se è un invio normale
    qint64 size = 0;
    for (int i = 0; i < e->count(); i++)
    {
        QFileInfo fi(e->at(i));
        if (!fi.isDir()) size += fi.size();
    }
    return size;
}

// Invia un pacchetto a tutti gli indirizzi broadcast del PC
void DuktoProtocol::sendToAllBroadcast(QByteArray *packet)
{
    // Elenco interfacce disponibili
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

    // Iterazione sulle interfacce
    for (int i = 0; i < ifaces.size(); i++)
    {
        // Iterazione per tutti gli IP dell'interfaccia
        QList<QNetworkAddressEntry> addrs = ifaces[i].addressEntries();

        // Invio pacchetto per ogni IP di broadcast
        for (int j = 0; j < addrs.size(); j++)
            if ((addrs[j].ip().protocol() == QAbstractSocket::IPv4Protocol) && (addrs[j].broadcast().toString() != ""))
                mSocket->writeDatagram(packet->data(), packet->length(), addrs[j].broadcast(), UDP_PORT);

    }

}
