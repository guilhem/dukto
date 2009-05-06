#ifndef LISTWIDGETPEERITEM_H
#define LISTWIDGETPEERITEM_H

#include <QtGui/QListWidgetItem>

class ListWidgetPeerItem : public QListWidgetItem
{
public:
    ListWidgetPeerItem(QListWidget * parent, QString text, QString peerKey);
    inline QString getPeerKey() { return mPeerKey; }

private:
    QString mPeerKey;

};

#endif // LISTWIDGETPEERITEM_H
