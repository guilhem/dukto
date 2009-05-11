#include "duktoprotocol.h"

#include "OsLib.h"
#include <QStringList>
#include <QFileInfo>

#define UDP_PORT 4644
#define TCP_PORT 4644

#if defined(Q_WS_WIN)
#define PLATFORM "Windows"
#elif defined(Q_WS_MAC)
#define PLATFORM "Apple Mac"
#elif defined(Q_WS_X11)
#define PLATFORM "Linux"
#endif

DuktoProtocol::DuktoProtocol()
    : mSocket(NULL), mTcpServer(NULL), mCurrentFile(NULL), mCurrentSocket(NULL)
{
    mSocket = new QUdpSocket(this);
    mSocket->bind(UDP_PORT);
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(newUdpData()));

    mTcpServer = new QTcpServer(this);
    mTcpServer->listen(QHostAddress::Any, TCP_PORT);
    connect(mTcpServer, SIGNAL(newConnection()), this, SLOT(newIncomingFile()));

}

DuktoProtocol::~DuktoProtocol()
{
    if (mCurrentSocket) delete mCurrentSocket;
    if (mSocket) delete mSocket;
    if (mTcpServer) delete mTcpServer;
    if (mCurrentFile) delete mCurrentFile;
}

QString DuktoProtocol::retrieveSystemName()
{
    static QString systemName = "";
    if (systemName != "") return systemName;

    QString uname(getenv("USERNAME"));
    if (uname == "") uname = getenv("USER");
    if (uname == "") uname = "Unknown";
    systemName = uname + " at " + QHostInfo::localHostName() + " (" + PLATFORM + ")";
    return systemName;
}

void DuktoProtocol::sayHello(QHostAddress dest)
{
    QByteArray *packet = new QByteArray();
    if (dest == QHostAddress::Broadcast)
        packet->append(0x01);           // 0x01 -> HELLO MESSAGE (broadcast)
    else
        packet->append(0x02);           // 0x03 -> HELLO MESSAGE (unicast)
    packet->append(retrieveSystemName());
    mSocket->writeDatagram(packet->data(), packet->length(), dest, UDP_PORT);
    delete packet;
}

void DuktoProtocol::sayGoodbye()
{
    QByteArray *packet = new QByteArray();
    packet->append(0x03);               // 0x02 -> GOODBYE MESSAGE
    packet->append("Bye Bye");
    mSocket->writeDatagram(packet->data(), packet->length(), QHostAddress::Broadcast, UDP_PORT);
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
            if (data != retrieveSystemName()) {
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

void DuktoProtocol::newIncomingFile()
{
    while (mTcpServer->hasPendingConnections()) {
        QTcpSocket *s = mTcpServer->nextPendingConnection();
        if (mCurrentFile)
        {
            s->close();
            return;
        }
        receiveFileStart();
        if (mCurrentSocket) {
            delete mCurrentSocket;
            mCurrentSocket = NULL;
        }
        mCurrentSocket = s;
        if (!s->waitForReadyRead())
        {
            mCurrentSocket->close();
            delete mCurrentSocket;
            mCurrentSocket = NULL;
            return;
        }

        // Modalità ricezione
        mSending = false;
        // Numero di entità (hardcoded a 1)
        mCurrentSocket->read(sizeof(qint64));
        // Dimensione totale
        mReceivedData = 0;
        mCurrentSocket->read((char*)&mDataSize, sizeof(qint64));
        // Nome
        QString name;
        char c;
        while (true)
        {
            mCurrentSocket->getChar(&c);
            if (c == '\0') break;
            name += c;
        }
        // Size primo file (ignorato in quanto uguale alla dimensione totale)
        mCurrentSocket->read(sizeof(qint64));

        // TODO: se il file di output esiste decidere come procedere
        // per ora si sovrascrive il vecchio file
        mCurrentFile = new QFile(name);
        mCurrentFile->open(QIODevice::WriteOnly);
        connect(mCurrentSocket, SIGNAL(readyRead()), this, SLOT(readNewData()), Qt::DirectConnection);
        connect(mCurrentSocket, SIGNAL(disconnected()), this, SLOT(closedConnection()), Qt::DirectConnection);
        readNewData();
     }
}

void DuktoProtocol::readNewData()
{
    QByteArray d = mCurrentSocket->readAll();
    mReceivedData += d.size();
    updateStatus();
    if (d.size() > 0) mCurrentFile->write(d);
}

void DuktoProtocol::closedConnection()
{
    readNewData();

    QString name;
    if (mCurrentFile) {
        name = mCurrentFile->fileName();
        mCurrentFile->close();
        delete mCurrentFile;
        mCurrentFile = NULL;
    }

    if (mCurrentSocket) mCurrentSocket->close();

    receiveFileComplete(name);
}

void DuktoProtocol::sendFile(QString ipDest, QString path)
{
    if (mCurrentFile) return;

    // Modalità invio
    mSending = true;

    mCurrentFile = new QFile(path);
    mCurrentFile->open(QIODevice::ReadOnly);

    mCurrentSocket = new QTcpSocket(this);
    connect(mCurrentSocket, SIGNAL(connected()), this, SLOT(sendMetaData()), Qt::DirectConnection);
    connect(mCurrentSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(sendData(qint64)), Qt::DirectConnection);
    mCurrentSocket->connectToHost(ipDest, TCP_PORT);
}

void DuktoProtocol::sendMetaData()
{
    QFileInfo fi(*mCurrentFile);
    QByteArray d;
    QByteArray name = fi.fileName().toAscii();
    qint64 size = fi.size();

    // Meta-data
    //  - N° entità (file, cartelle, ecc...) hardcoded 1
    //  - Dimensione totale
    //  - Nome primo (e unico) file
    //  - Dimensione primo (e unico) file (-1 per una cartella)

    qint64 n = 1;
    d.append((char*) &n, sizeof(n));
    d.append((char*) &size, sizeof(size));
    d.append(name + '\0');
    d.append((char*) &size, sizeof(size));

    mCurrentSocket->write(d);
    mSentData = 0;
    mSentDataBuff = 0;
    updateStatus();
}

void DuktoProtocol::sendData(qint64 b)
{
    mSentDataBuff -= b;
    mSentData += b;     // non considera i byte del nome, ma sono trascurabili
    updateStatus();

    if (mSentDataBuff <= 0) {

        QByteArray d = mCurrentFile->read(10000);

        if (d.size() == 0) {
            mCurrentSocket->close();
            mCurrentFile->close();
            delete mCurrentSocket;
            delete mCurrentFile;
            mCurrentSocket = NULL;
            mCurrentFile = NULL;
            sendFileComplete();
            return;
        }

        mCurrentSocket->write(d);
        mSentDataBuff = d.size();
    }
}

void DuktoProtocol::updateStatus()
{
    if (mSending)
        transferStatusUpdate((int)((float)(mSentData) / (float)(mCurrentFile->size()) * 100));
    else
        transferStatusUpdate((int)((float)(mReceivedData) / (float)(mDataSize) * 100));
}
