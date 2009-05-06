#include "listwidgetpeeritem.h"

ListWidgetPeerItem::ListWidgetPeerItem(QListWidget * parent, QString text, QString peerKey)
        : QListWidgetItem(parent, QListWidgetItem::UserType)
{
    setText(text);
    mPeerKey = peerKey;
}
