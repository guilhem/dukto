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

#include "theme.h"

#include <QDebug>
#include <QColor>

const QString Theme::DEFAULT_THEME_COLOR = "#248b00";

Theme::Theme(QObject *parent) :
    QObject(parent),
    mColor1("#000000"), mColor2(DEFAULT_THEME_COLOR), mColor3("#4cb328"),
    mColor4("#555555"), mColor5("#888888"), mColor6("#ffffff"),
    mColor7("#cccccc"), mColor8("#eeeeee"), mColor9("#ccffffff")
{
}

void Theme::setThemeColor(QString color)
{
    QColor c;
    c.setNamedColor(color);
    c.setRed(c.red() + 40);
    c.setGreen(c.green() + 40);
    c.setBlue(c.blue() + 40);

    mColor2 = color;
    mColor3 = c.name();
    emit color2Changed();
    emit color3Changed();
}
