#include "peer.h"

Peer::Peer()
{
}

Peer::Peer(QHostAddress a, QString n)
{
    address = a;
    name = n;
}
