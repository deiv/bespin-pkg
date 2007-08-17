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

#ifndef TILESET_H
#define TILESET_H

#include <QBitmap>
#include <QRect>
#include <QHash>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include "fixx11h.h"
#include "gradients.h"

namespace Tile
{

   enum Section
   { // DON'T CHANGE THE ORDER FOR NO REASON, i misuse them on the masks...
      TopLeft = 0, TopRight, BtmLeft, BtmRight,
      TopMid, BtmMid, MidLeft, MidMid, MidRight
   };
   enum Position
   {
      Top = 0x1, Left=0x2, Bottom=0x4, Right=0x8,
      Ring=0xf, Center=0x10, Full=0x1f
   };

   typedef uint PosFlags;

inline static bool matches(PosFlags This, PosFlags That){return (This & That) == This;}

class Set
{
public:
   Set(const QPixmap &pix, int xOff, int yOff, int width, int height, int rx = 0, int ry = 0);
   Set(){setDefaultShape(Ring);}
   void render(const QRect &rect, QPainter *p) const;
   void outline(const QRect &rect, QPainter *p, QColor c, bool strong = false, int size = 1) const;
   Picture render(int width, int height) const;
   Picture render(const QSize &size) const;
   QRect rect(const QRect &rect, PosFlags pf) const;
   inline int width(Section sect) const {return pixmap[sect].width();}
   inline int height(Section sect) const {return pixmap[sect].height();}
   inline bool isQBitmap() const {return _isBitmap;}
   inline void setDefaultShape(PosFlags pf) {_shape = _defShape = pf;}
   inline void setShape(PosFlags pf) const {
      Set *that = const_cast<Set*>(this);
      that->_shape = pf;
   }
   inline PosFlags shape() const { return _shape; }
   inline void reset() {
      _shape = _defShape;
   }
protected:
   QPixmap pixmap[9];
private:
   int rxf, ryf;
   bool _isBitmap;
   PosFlags _shape, _defShape;
};

class Mask : public Set
{
public:
   Mask(const QPixmap &pix, int xOff, int yOff, int width, int height, int rx = 0, int ry = 0);
   Mask() : Set() {
      _clipOffset[0] = _clipOffset[1] = _clipOffset[2] = _clipOffset[3] = 0;
      _hasCorners = false;
      _texPix = 0L; _texColor = 0L; _offset = 0L;
      setDefaultShape(Full);
   }
   
   QRect bounds(const QRect &rect, PosFlags pf = Full) const;
   
   QRegion clipRegion(const QRect &rect, PosFlags pf = Ring) const;
   
   const QPixmap &corner(PosFlags pf) const;
   
   inline bool hasCorners() const {return _hasCorners;}
   
   void render(const QRect &rect, QPainter *p) const;

   inline void render(const QRect &rect, QPainter *p,
                      const QColor &c) const {
      Mask *that = const_cast<Mask*>(this);
      that->_texColor = &c;
      render(rect, p);
      that->_texColor = 0L;
   }
   
   inline void render(const QRect &rect, QPainter *p,
                      const QPixmap &pix, const QPoint &offset = QPoint()) const {
      Mask *that = const_cast<Mask*>(this);
      that->_texPix = &pix; that->_offset = &offset;
      render(rect, p);
      that->_texPix = 0L; that->_offset = 0L;
   
   }
   
   inline void render(const QRect &rect, QPainter *p,
                      Bespin::Gradients::Type type, Qt::Orientation o,
                      const QColor &c, int size = -1,
                      const QPoint &offset = QPoint()) const {

      if (type == Bespin::Gradients::None)
         render(rect, p, c);
      else {
         const int s = (size > 0) ? size :
               (o == Qt::Vertical) ? rect.height() :
               rect.width();
         render(rect, p, Bespin::Gradients::pix(c, s, o, type), offset);
      }
   }
   
   inline void render(const QRect &rect, QPainter *p,
                      const QBrush &brush, const QPoint &offset = QPoint()) const {

      if (brush.style() == Qt::TexturePattern)
         render(rect, p, brush.texture(), offset);
      else
         render(rect, p, brush.color());
   }
   
   inline void reset() const {
      Mask *that = const_cast<Mask*>(this);
      that->_justClip = false; that->Set::reset();
   }
   
   void setClipOffsets(uint left, uint top, uint right, uint bottom);
   
   inline void setClipOnly(bool b = true) { _justClip = b; }

   private:
   int _clipOffset[4];
   bool _hasCorners, _justClip;
   const QPixmap *_texPix; const QColor *_texColor; const QPoint *_offset;
};

class Line
{
public:
   Line(const QPixmap &pix, Qt::Orientation o, int d1, int d2);
   Line(){}
   void render(const QRect &rect, QPainter *p, PosFlags pf = Full, bool btmRight = false) const;
   inline int thickness() const { return _thickness; }
private:
   inline int width(int i) const {return pixmap[i].width();}
   inline int height(int i) const {return pixmap[i].height();}
   Qt::Orientation _o;
   QPixmap pixmap[3];
   int _thickness;
};

} // namespace Tile

#endif //TILESET_H
