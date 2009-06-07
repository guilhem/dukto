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
    void sendFile(QString ipDest, QStringList files);
    
public slots:
    void newUdpData();
    void newIncomingConnection();
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
    QStringList* expandTree(QStringList files);
    void addRecursive(QStringList *e, QString path);
    qint64 computeTotalSize(QStringList *e);
    qint64 computeTotalSizeRecursive(QString path);
    QByteArray nextElementHeader();

    void handleMessage(QByteArray &data, QHostAddress &sender);
    void updateStatus();

    QUdpSocket *mSocket;            // Socket UDP segnalazione
    QTcpServer *mTcpServer;         // Socket TCP attesa dati
    QTcpSocket *mCurrentSocket;     // Socket TCP dell'attuale trasferimento file

    QHash<QString, Peer> mPeers;    // Elenco peer individuati

    // Send and receive members
    bool mIsSending;
    bool mIsReceiving;
    QFile *mCurrentFile;            // Puntatore al file aperto corrente
    qint64 mTotalSize;              // Quantità totale di dati da inviare o ricevere
    int mFileCounter;              // Puntatore all'elemento correntemente da trasmettere o ricevere

    // Sending members
    QStringList *mFilesToSend;      // Elenco degli elementi da trasmettere
    qint64 mSentData;               // Quantità di dati totale trasmessi
    qint64 mSentBuffer;             // Quantità di dati rimanenti nel buffer di trasmissione

    // Receive members
    qint64 mElementsToReceiveCount;    // Numero di elementi da ricevere
    qint64 mTotalReceivedData;         // Quantità di dati ricevuti totale
    qint64 mElementReceivedData;       // Quantità di dati ricevuti per l'elemento corrente
    qint64 mElementSize;               // Dimensione dell'elemento corrente

};

#endif // DUKTOPROTOCOL_H
