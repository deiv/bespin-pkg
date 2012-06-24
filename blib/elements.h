/*
 *   Bespin library for Qt style, KWin decoration and everythng else
 *   Copyright 2007-2012 by Thomas Lübking <thomas.luebking@gmail.com>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

class QPixmap;

namespace Bespin {
namespace Elements {

    BLIB_EXPORT void setShadowIntensity(float intensity);
    BLIB_EXPORT void setScale(float scale);
    BLIB_EXPORT void setRoundness(int roundness);
    BLIB_EXPORT QImage glow(int size, float width);
    BLIB_EXPORT QImage shadow(int size, bool opaque, bool sunken, float factor = 1.0);
    BLIB_EXPORT QImage roundMask(int size);
    BLIB_EXPORT QImage roundedMask(int size, int factor);
    BLIB_EXPORT QImage sunkenShadow(int size, bool enabled);
    BLIB_EXPORT QImage relief(int size, bool enabled);
    BLIB_EXPORT QImage groupShadow(int size);

#if 0
BLIB_EXPORT void renderButtonLight(Tile::Set &set);
BLIB_EXPORT void renderLightLine(Tile::Line &line);
#endif

}
}

#undef fillRect
