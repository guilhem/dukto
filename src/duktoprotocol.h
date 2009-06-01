#ifndef DUKTOPROTOCOL_H
#define DUKTOPROTOCOL_H

#include <QObject>

#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QHostInfo>
#include <QHash>
#include <QFile>

#include "peer.h"

class DuktoProtocol : public QObject
{
    Q_OBJECT

public:
    DuktoProtocol();
    virtual ~DuktoProtocol();
    void sayHello(QHostAddress dest);
    void sayGoodbye();
    inline QHash<QString, Peer>& getPeers() { return mPeers; }
    void sendFile(QString ipDest, QString path);
    
public slots:
    void newUdpData();
    void newIncomingFile();
    void readNewData();
    void closedConnection();
    void sendMetaData();
    void sendData(qint64 b);
    void sendConnectError(QAbstractSocket::SocketError);

signals:
     void peerListChanged();
     void sendFileComplete(QString name);
     void sendFileError(int code);
     void receiveFileStart();
     void receiveFileComplete(QString name);
     void receiveFileCancelled(QString name);
     void transferStatusUpdate(int p);

private:
    void handleMessage(QByteArray &data, QHostAddress &sender);
    void updateStatus();

    QUdpSocket *mSocket;
    QTcpServer *mTcpServer;
    QHash<QString, Peer> mPeers;
    QFile *mCurrentFile;
    QTcpSocket *mCurrentSocket;
    qint64 mSentData;
    qint64 mSentDataBuff;
    qint64 mReceivedData;
    qint64 mDataSize;
    bool mSending;

};

#endif // DUKTOPROTOCOL_H
