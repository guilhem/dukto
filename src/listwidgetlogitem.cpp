#include "listwidgetpeeritem.h"

ListWidgetPeerItem::ListWidgetPeerItem(QListWidget * parent, QString text, QString peerKey)
        : QListWidgetItem(parent, QListWidgetItem::Type)
{
    setText(text);
    mPeerKey = peerKey;
    setIcon(QIcon(":/duktoIcon"));
}
