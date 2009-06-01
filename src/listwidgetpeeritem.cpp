#include "listwidgetlogitem.h"

ListWidgetLogItem::ListWidgetLogItem(QListWidget * parent, QString text, QString filename)
        : QListWidgetItem(parent, QListWidgetItem::UserType)
{
    setText(text);
    mFilename = filename;
}
