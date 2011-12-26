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

#include "destinationbuddy.h"

#include "buddylistitemmodel.h"

#include <QStandardItem>

DestinationBuddy::DestinationBuddy(QObject *parent) :
    QObject(parent)
{
}

void DestinationBuddy::fillFromItem(QStandardItem *item)
{
    mIp = item->data(BuddyListItemModel::Ip).toString();
    mPort = item->data(BuddyListItemModel::Port).toInt();
    mUsername = item->data(BuddyListItemModel::Username).toString();
    mSystem = item->data(BuddyListItemModel::System).toString();
    mPlatform = item->data(BuddyListItemModel::Platform).toString();
    mGenericAvatar = item->data(BuddyListItemModel::GenericAvatar).toString();
    mAvatar = item->data(BuddyListItemModel::Avatar).toString();
    mOsLogo = item->data(BuddyListItemModel::OsLogo).toString();
    mShowBack = item->data(BuddyListItemModel::ShowBack).toString();
    emit ipChanged();
    emit portChanged();
    emit usernameChanged();
    emit systemChanged();
    emit platformChanged();
    emit genericAvatarChanged();
    emit avatarChanged();
    emit osLogoChanged();
    emit showBackChanged();
}

