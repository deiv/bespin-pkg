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

#ifndef GRADIENTS_H
#define GRADIENTS_H

#include <QBrush>
#include <QColor>
#include <QPixmap>

#ifndef Q_WS_X11
#define QT_NO_XRENDER #
#endif

namespace Bespin {

#ifndef BESPIN_DECO
class BgSet {
public:
   BgSet(){}
   QPixmap topTile, btmTile;
   QPixmap cornerTile, lCorner, rCorner;
};
#endif

namespace Gradients {

enum Type {
   None = 0, Simple, Button, Sunken, Gloss, Glass, Metal, Cloudy, RadialGloss,
   TypeAmount
};

enum BgMode {
   BevelV = 2, BevelH, LightV, LightH
};

enum Position { Top = 0, Bottom, Left, Right };


/** use only if sure you're not requesting Type::None */
const QPixmap& pix(const QColor &c,
                   int size,
                   Qt::Orientation o,
                   Type type = Simple);

/** wrapper to support Type::None */
inline const QBrush
brush(const QColor &c, int size, Qt::Orientation o, Type type  = Simple) {
   if (type == None)
      return QBrush(c);
   return QBrush(pix(c, size, o, type));
}

inline const bool isReflective(Type type = Simple) {
   return type > Button;
}

#ifndef BESPIN_DECO
/** a diagonal NW -> SE light */
const QPixmap &shadow(int height, bool bottom = false);

/** a diagonal 16:9 SE -> NW light */
const QPixmap &ambient(int height);

/** a horizontal black bevel from low alpha to transparent */
const QPixmap &bevel(bool ltr = true);

/** a vertical N -> S light */
const QPixmap &light(int height);

const QPixmap &structure(const QColor &c, bool light = false);

const BgSet &bgSet(const QColor &c);
// const QPixmap &bgCorner(const QColor &c, bool other = false);

void init(BgMode mode, int structure = 0, int bgBevelIntesity = 110, int btnBevelSize = 16, bool force = false);
#else
void init();
#endif

const QPixmap &borderline(const QColor &c, Position pos);
void wipe();

Type fromInfo(int info);
int toInfo(Type type);

}
}

#endif //GRADIENTS_H
