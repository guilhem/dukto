/* DUKTO - A simple, fast and multi-platform file transfer tool for LAN users
 * Copyright (C) 2011 Emanuele Colombo
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

#if defined(Q_WS_WIN)
    #include <windows.h>
#endif

#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QNetworkInterface>
#include <QTimer>

#include "platform.h"

#define DEFAULT_UDP_PORT 4644
#define DEFAULT_TCP_PORT 4644

DuktoProtocol::DuktoProtocol()
    : mSocket(NULL), mTcpServer(NULL), mCurrentSocket(NULL),
        mCurrentFile(NULL), mFilesToSend(NULL)
{
    mLocalUdpPort = DEFAULT_UDP_PORT;
    mLocalTcpPort = DEFAULT_TCP_PORT;

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

void DuktoProtocol::initialize()
{
    mSocket = new QUdpSocket(this);
    mSocket->bind(QHostAddress::Any, mLocalUdpPort);
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(newUdpData()));

    mTcpServer = new QTcpServer(this);
    mTcpServer->listen(QHostAddress::Any, mLocalTcpPort);
    connect(mTcpServer, SIGNAL(newConnection()), this, SLOT(newIncomingConnection()));
}

void DuktoProtocol::setPorts(qint16 udp, qint16 tcp)
{
    mLocalUdpPort = udp;
    mLocalTcpPort = tcp;
}

QString DuktoProtocol::getSystemSignature()
{
    static QString signature = "";
    if (signature != "") return signature;

    signature = Platform::getSystemUsername()
              + " at " + Platform::getHostname()
              + " (" + Platform::getPlatformName() + ")";
    return signature;
}

void DuktoProtocol::sayHello(QHostAddress dest)
{
    sayHello(dest, mLocalUdpPort);
}

void DuktoProtocol::sayHello(QHostAddress dest, qint16 port)
{
    // Preparazione pacchetto
    QByteArray *packet = new QByteArray();
    if ((port == DEFAULT_UDP_PORT) && (mLocalUdpPort == DEFAULT_UDP_PORT))
    {
        if (dest == QHostAddress::Broadcast)
            packet->append(0x01);           // 0x01 -> HELLO MESSAGE (broadcast)
        else
            packet->append(0x02);           // 0x02 -> HELLO MESSAGE (unicast)
    }
    else
    {
        if (dest == QHostAddress::Broadcast)
            packet->append(0x04);           // 0x04 -> HELLO MESSAGE (broadcast) with PORT
        else
            packet->append(0x05);           // 0x05 -> HELLO MESSAGE (unicast) with PORT
        packet->append((char*)&mLocalUdpPort, sizeof(qint16));
    }
    packet->append(getSystemSignature());

    // Invio pacchetto
    if (dest == QHostAddress::Broadcast) {
        sendToAllBroadcast(packet, port);
        if (port != DEFAULT_UDP_PORT) sendToAllBroadcast(packet, DEFAULT_UDP_PORT);
    }
    else
        mSocket->writeDatagram(packet->data(), packet->length(), dest, port);

    delete packet;
}

void DuktoProtocol::sayGoodbye()
{
    // Create packet
    QByteArray *packet = new QByteArray();
    packet->append(0x03);               // 0x03 -> GOODBYE MESSAGE
    packet->append("Bye Bye");

    // Look for all the discovered ports
    QList<qint16> ports;
    ports.append(mLocalUdpPort);
    if (mLocalUdpPort != DEFAULT_UDP_PORT) ports.append(DEFAULT_UDP_PORT);
    foreach (const Peer &p, mPeers.values())
        if (!ports.contains(p.port))
            ports.append(p.port);

    // Send broadcast message to all discovered ports
    foreach (const qint16 &port, ports)
        sendToAllBroadcast(packet, port);

    // Free memory
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
            if (data != getSystemSignature()) {
                mPeers[sender.toString()] = Peer(sender, data, DEFAULT_UDP_PORT);
                if (msgtype == 0x01) sayHello(sender, DEFAULT_UDP_PORT);
                emit peerListAdded(mPeers[sender.toString()]);
            }
            break;

        case 0x03:  // GOODBYE
            emit peerListRemoved(mPeers[sender.toString()]);
            mPeers.remove(sender.toString());
            break;

        case 0x04:  // HELLO (broadcast) with PORT
        case 0x05:  // HELLO (unicast) with PORT
            data.remove(0, 1);
            qint16 port = *((qint16*) data.constData());
            data.remove(0, 2);
            if (data != getSystemSignature()) {
                mPeers[sender.toString()] = Peer(sender, data, port);
                if (msgtype == 0x04) sayHello(sender, port);
                emit peerListAdded(mPeers[sender.toString()]);
            }
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
    receiveFileStart(s->peerAddress().toString());

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
    connect(mCurrentSocket, SIGNAL(disconnected()), this, SLOT(closedConnectionTmp()), Qt::QueuedConnection);

    // Inizializzazione variabili
    mIsReceiving = true;
    mTotalReceivedData = 0;
    mElementSize = -1;
    mReceivedFiles = new QStringList();
    mRootFolderName = "";
    mRootFolderRenamed = "";
    mReceivingText = false;

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
    // Temporanea disconnessione per evitare di
    // ricevere altri segnali mentre ne sto già gestendo uno
    disconnect(mCurrentSocket, SIGNAL(readyRead()), this, SLOT(readNewData()));

    // Fino a che ci sono dati da leggere
    while (mCurrentSocket->bytesAvailable() > 0)
    {

        // Verifico se devo leggere un header
        if (mElementSize == -1)
        {
            // Lettura nome
            char c;
            while (1) {
                bool ret = mCurrentSocket->getChar(&c);
                if (!ret) return;
                if (c == '\0') break;
                mPartialName.append(c);
            }
            QString name = QString::fromUtf8(mPartialName);
            mPartialName.clear();

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
                    mReceivedFiles->append(name);

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
                mReceivedFiles->append(name);
                mReceivingText = true;
                mTextToReceive.clear();
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
                mReceivedFiles->append(name);
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
            mTextToReceive.append(d);

        // Verifico se ho completato l'elemento corrente
        if (mElementReceivedData == mElementSize)
        {
            // Completato, chiudo il file e mi preparo per il prossimo elemento
            mElementSize = -1;
            if (!mReceivingText)
            {
                mCurrentFile->deleteLater();
                mCurrentFile = NULL;
            }
        }
    }

    // Riabilitazione segnale
    connect(mCurrentSocket, SIGNAL(readyRead()), this, SLOT(readNewData()), Qt::DirectConnection);
}

void DuktoProtocol::closedConnectionTmp()
{
    QTimer::singleShot(100, this, SLOT(closedConnection()));
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
        receiveFileComplete(mReceivedFiles, mTotalSize);

    // Ricezione testo conclusa
    else
    {
        QString rec = QString::fromUtf8(mTextToReceive);
        receiveTextComplete(&rec, mTotalSize);
    }

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

void DuktoProtocol::sendFile(QString ipDest, qint16 port, QStringList files)
{
    // Check for default port
    if (port == 0) port = DEFAULT_TCP_PORT;

    // Verifica altre attività in corso
    if (mIsReceiving || mIsSending) return;
    mIsSending = true;

    // File da inviare
    mFilesToSend = expandTree(files);
    mFileCounter = 0;

    // Connessione al destinatario
    mCurrentSocket = new QTcpSocket(this);

    // Gestione segnali
    connect(mCurrentSocket, SIGNAL(connected()), this, SLOT(sendMetaData()), Qt::DirectConnection);
    connect(mCurrentSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(sendConnectError(QAbstractSocket::SocketError)), Qt::DirectConnection);
    connect(mCurrentSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(sendData(qint64)), Qt::DirectConnection);

    // Connessione
    mCurrentSocket->connectToHost(ipDest, port);
}

void DuktoProtocol::sendText(QString ipDest, qint16 port, QString text)
{
    // Check for default port
    if (port == 0) port = DEFAULT_TCP_PORT;

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
    mCurrentSocket->connectToHost(ipDest, port);
}

void DuktoProtocol::sendMetaData()
{
    // Impostazione buffer di invio
#if defined(Q_WS_WIN)
    int v = 49152;
    ::setsockopt(mCurrentSocket->socketDescriptor(), SOL_SOCKET, SO_SNDBUF, (char*)&v, sizeof(v));
#endif

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
        closeCurrentTransfer();
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

// Chiusura trasferimento dati
void DuktoProtocol::closeCurrentTransfer(bool aborted)
{
    mCurrentSocket->disconnect();
    mCurrentSocket->disconnectFromHost();
    if (mCurrentSocket->state() != QTcpSocket::UnconnectedState)
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
    if (!aborted)
        emit sendFileComplete(mFilesToSend);
    delete mFilesToSend;
    mFilesToSend = NULL;

    return;
}

// Aggiornamento delle statistiche di invio
void DuktoProtocol::updateStatus()
{
    if (mIsSending)
        emit transferStatusUpdate(mTotalSize, mSentData);
    else if (mIsReceiving)
        emit transferStatusUpdate(mTotalSize, mTotalReceivedData);
}

// In caso di errore di connessione
void DuktoProtocol::sendConnectError(QAbstractSocket::SocketError e)
{
    if (mCurrentSocket)
    {
        mCurrentSocket->close();
        mCurrentSocket->deleteLater();
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
    path.replace("\\", "/");
    if (path.right(1) == "/") path.chop(1);
    e->append(path);

    QFileInfo fi(path);
    if (fi.isDir())
    {
        QStringList entries = QDir(path).entryList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
        for (int i = 0; i < entries.count(); i++)
            addRecursive(e, path + "/" + entries.at(i));
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
    header.append(name.toUtf8() + '\0');

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
void DuktoProtocol::sendToAllBroadcast(QByteArray *packet, qint16 port)
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
            {
                mSocket->writeDatagram(packet->data(), packet->length(), addrs[j].broadcast(), port);
                mSocket->flush();
            }
    }
}

// Interrompe un trasferimento in corso (utilizzabile solo lato invio)
void DuktoProtocol::abortCurrentTransfer()
{
    // Check if it's sending data
    if (!mIsSending) return;

    // Abort current connection
    closeCurrentTransfer(true);
    emit sendFileAborted();
}
