/* Bespin widget style for Qt4
   Copyright (C) 2007 Thomas Luebking <thomas.luebking@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef COLORS_H
#define COLORS_H

class QWidget;
#include <QColor>
#include <QPalette>

namespace Bespin {

namespace Colors {

const QColor &bg(const QPalette &pal, const QWidget *w);
QColor btnBg(const QPalette &pal, bool isEnabled, int hasFocus = false, int step = 0);
QColor btnFg(const QPalette &pal, bool isEnabled, int hover, int step = 0);
int contrast(const QColor &a, const QColor &b);
QPalette::ColorRole counterRole(QPalette::ColorRole role);
bool counterRole(QPalette::ColorRole &from, QPalette::ColorRole &to,
                 QPalette::ColorRole defFrom = QPalette::WindowText,
                 QPalette::ColorRole defTo = QPalette::Window);
QColor emphasize(const QColor &c, int value = 10);
bool haveContrast(const QColor &a, const QColor &b);
QColor light(const QColor &c, int value);
QColor mid(const QColor &oc1, const QColor &c2, int w1 = 1, int w2 = 1);
int value(const QColor &c);

void setButtonRoles(QPalette::ColorRole bg, QPalette::ColorRole fg,
                    QPalette::ColorRole bgActive, QPalette::ColorRole fgActive);

};
};

#endif //COLORS_H
