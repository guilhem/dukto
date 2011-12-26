/* DUKTO - A simple, fast and multi-platform file transfer tool for LAN users
 * Copyright (C) 2011 Emanuele Colombo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef RECENTLISTITEMMODEL_H
#define RECENTLISTITEMMODEL_H

#include <QStandardItemModel>

class RecentListItemModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit RecentListItemModel();
    void addRecent(QString name, QString value, QString type, QString sender, qint64 size);

    enum RecentRoles {
        Name = Qt::UserRole + 1,
        Value,
        Type,
        TypeIcon,
        DateTime,
        Sender,
        Size
    };

signals:

public slots:

};

#endif // RECENTLISTITEMMODEL_H
