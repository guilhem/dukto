#ifndef LISTWIDGETLOGITEM_H
#define LISTWIDGETLOGITEM_H

#include <QtGui/QListWidgetItem>

class ListWidgetLogItem : public QListWidgetItem
{
public:
    ListWidgetLogItem(QListWidget * parent, QString text, QString filename);
    inline QString getFilename() { return mFilename; }

private:
    QString mFilename;

};

#endif // LISTWIDGETLOGITEM_H
