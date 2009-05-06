#ifndef PEER_H
#define PEER_H

#include <QtNetwork/QHostInfo>

class Peer
{
public:
    Peer();
    Peer(QHostAddress a, QString n);
    QHostAddress address;
    QString name;
};

#endif // PEER_H
