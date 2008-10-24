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

 /**
 erhem... just noticed and in case you should wonder:
 this is NOT derived from the Oxygen style, but rather kinda vice versa.
 so i'm not forgetting credits. period.
 */

#include <QPainter>
#include <cmath>
#include "oxrender.h"
#include "tileset.h"

using namespace Tile;

// some static elements (benders)
static QPixmap nullPix;
static PosFlags _shape = 0;
static const QPixmap *_texPix = 0;
static const QColor *_texColor = 0;
static const QColor *_bgColor = 0;
static const QPoint *_offset = 0;
static bool _preferClip = true;

// static functions
PosFlags Tile::shape() { return _shape; }
void Tile::setPreferClip(bool b) { _preferClip = b; }
void Tile::setSolidBackground(const QColor &c) { _bgColor = &c; }
void Tile::setShape(PosFlags pf) { _shape = pf; }
void Tile::reset()
{
    _shape = 0;
    _bgColor = 0;
    _preferClip = true;
}

static bool isEmpty(const QPixmap &pix)
{
    if (!pix.hasAlpha())
        return false;
    QImage img =  pix.toImage();
    uint *data = ( uint * ) img.bits();
    int total = img.width() * img.height();
    for ( int current = 0 ; current < total ; ++current )
        if (qAlpha(data[ current ]))
            return false;
    return true;
}
#if 0
static QPixmap invertAlpha(const QPixmap & pix)
{
   if (pix.isNull()) return pix;
   QImage img =  pix.toImage();
   QImage *dst = new QImage(img);
   uint *data = ( uint * ) img.bits();
   uint *ddata = ( uint * ) dst->bits();
   int total = img.width() * img.height();
   for ( int c = 0 ; c < total ; ++c )
      ddata[c] = qRgba( qRed(data[c]), qGreen(data[c]), qBlue(data[c]), 255-qAlpha(data[c]) );
   QPixmap ret = QPixmap::fromImage(*dst, 0);
   delete dst;
   return ret;
}
#endif

Set::Set(const QPixmap &pix, int xOff, int yOff, int width, int height, int round)
{
    if (pix.isNull())
    {
        _isBitmap = false;
        return;
    }
    _isBitmap = pix.isQBitmap();
    int w = qMax(1, width),
        h = qMax(1, height);

    int i = xOff*2*round/100;
    rndRect = QRect(i, i, i, i);

    int rOff = pix.width() - xOff - w;
    int bOff = pix.height() - yOff - h;
    int tileWidth = qMax(32, width);
    int tileHeight = qMax(32, height);

    QPainter p;
    QPixmap dump;
    QPixmap transSrc(qMax(32, pix.width()), qMax(32, pix.height()));
    transSrc.fill(Qt::transparent);

#define DUMP(_SECTION_, _WIDTH_, _HEIGHT_, _X_, _Y_, _W_, _H_)\
dump = pix.copy(_X_, _Y_, _W_, _H_);\
if (isEmpty(dump))\
    pixmap[_SECTION_] = QPixmap();\
else\
{\
    pixmap[_SECTION_] = transSrc.copy(0,0,_WIDTH_, _HEIGHT_);\
    p.begin(&pixmap[_SECTION_]);\
    p.drawTiledPixmap(pixmap[_SECTION_].rect(), dump);\
    p.end();\
}//

#define VALIDATE(_SECTION_)\
if (isEmpty(pixmap[_SECTION_]))\
    pixmap[_SECTION_] = QPixmap()

    pixmap[TopLeft] = pix.copy(0, 0, xOff, yOff);
    VALIDATE(TopLeft);

    DUMP(TopMid,   tileWidth, yOff,   xOff, 0, w, yOff);

    pixmap[TopRight] = pix.copy(xOff+w, 0, rOff, yOff);
    VALIDATE(TopRight);

    //----------------------------------
    DUMP(MidLeft,   xOff, tileHeight,   0, yOff, xOff, h);
    DUMP(MidMid,   tileWidth, tileHeight,   xOff, yOff, w, h);
    DUMP(MidRight,   rOff, tileHeight,   xOff+w, yOff, rOff, h);
    //----------------------------------

    pixmap[BtmLeft] = pix.copy(0, yOff+h, xOff, bOff);
    VALIDATE(BtmLeft);

    DUMP(BtmMid,   tileWidth, bOff,   xOff, yOff+h, w, bOff);

    pixmap[BtmRight] = pix.copy(xOff+w, yOff+h, rOff, bOff);
    VALIDATE(BtmRight);

    _clipOffset[0] = _clipOffset[2] = _clipOffset[1] = _clipOffset[3] = 0;
    _hasCorners = !pix.isNull();
    _defShape = Full;
#undef initPixmap
#undef finishPixmap
}

QRect
Set::rect(const QRect &rect, PosFlags pf) const
{
    QRect ret = rect;
    switch (pf)
    {
    case Center:
        ret.adjust(width(MidLeft),height(TopMid),-width(TopMid),-height(BtmMid)); break;
    case Left:
        ret.setRight(ret.left()+width(MidLeft)); break;
    case Top:
        ret.setBottom(ret.top()+height(TopMid)); break;
    case Right:
        ret.setLeft(ret.right()-width(MidRight)); break;
    case Bottom:
        ret.setTop(ret.bottom()-height(BtmMid)); break;
    default: break;
    }
    return ret;
}

void
Set::render(const QRect &r, QPainter *p) const
{

#define MAKE_FILL(_OFF_)\
if (!tile->isNull())\
{\
    if (_texPix || _texColor)\
    {\
        if (filledPix.size() != tile->size())\
            filledPix = QPixmap(tile->size());\
        if (_texPix)\
        {\
            filledPix.fill(Qt::transparent); \
            pixPainter.begin(&filledPix);\
            pixPainter.drawTiledPixmap(filledPix.rect(), *_texPix, _OFF_-off);\
            pixPainter.end();\
            filledPix = FX::applyAlpha(filledPix, *tile);\
        }\
        else\
            filledPix = FX::tint(*tile, *_texColor);\
        tile = &filledPix;\
    }\
    if (solidBg)\
    {\
        if (solidPix.size() != tile->size())\
            solidPix = QPixmap(tile->size());\
        solidPix.fill(*solidBg);\
        pixPainter.begin(&solidPix);\
        pixPainter.drawPixmap(0,0, *tile); pixPainter.end();\
        tile = &solidPix;\
    }\
} // skip semicolon

    PosFlags pf = _shape ? _shape : _defShape;
   
    if (_preferClip && (_texPix || _texColor) && (pf & Center))
    {   // first the inner region
        //NOTE: using full alphablend can become enourmously slow due to VRAM size -
        // even on HW that has Render acceleration!
        p->save();
        p->setClipRegion(clipRegion(r, pf), Qt::IntersectClip);
        if (_texPix)
            p->drawTiledPixmap(r, *_texPix, _offset ? *_offset : QPoint());
        else // if (_texColor)
            p->fillRect(r, *_texColor);
//       else // this is nonsense... just i don't forget :)
//          p->drawTiledPixmap(r, pixmap[MidMid]);
        p->restore();

        if (!_hasCorners)
            return;

        pf &= ~Center;
    }

    QPixmap filledPix, solidPix; QPainter pixPainter;
    const QColor *solidBg = 0;

    QPoint off = r.topLeft();
    if (_offset)
        off -= *_offset;
    int rOff = 0, xOff, yOff, w, h;

    r.getRect(&xOff, &yOff, &w, &h);
    int tlh = height(TopLeft), blh = height(BtmLeft),
        trh = height(TopRight), brh = height(BtmLeft),
        tlw = width(TopLeft), blw = width(BtmLeft),
        trw = width(TopRight), brw = width(BtmRight);

    // vertical overlap geometry adjustment (horizontal is handled during painting)
    if (pf & Left)
    {
        w -= width(TopLeft);
        xOff += width(TopLeft);
        if (pf & (Top | Bottom) && tlh + blh > r.height())
        {   // vertical edge overlap
            tlh = (tlh*r.height())/(tlh+blh);
            blh = r.height() - tlh;
        }
    }
    if (pf & Right)
    {
        w -= width(TopRight);
        if (matches(Top | Bottom, pf) && trh + brh > r.height())
        {   // vertical edge overlap
            trh = (trh*r.height())/(trh+brh);
            brh = r.height() - trh;
        }
    }

   // painting
    const QPixmap *tile;
    QRect checkRect;
    const bool unclipped = !p->hasClipping() || p->clipRegion().isEmpty();
#define UNCLIPPED (unclipped || p->clipRegion().intersects(checkRect))
   
    if (pf & Top)
    {
        if (matches(Left | Right, pf) && w < 0)
        {   // horizontal edge overlap
            tlw = tlw*r.width()/(tlw+trw);
            trw = r.width() - tlw;
        }

        rOff = r.right()-trw+1;
        yOff += tlh;
        h -= tlh;

        checkRect.setRect(r.x(),r.y(), tlw, tlh);
        if ((pf & Left) && UNCLIPPED)
        {
            tile = &pixmap[TopLeft];
            MAKE_FILL(r.topLeft());
            p->drawPixmap(r.x(),r.y(), *tile, 0, 0, tlw, tlh);
        }

        checkRect.setRect(rOff, r.y(), trw, trh);
        if ((pf & Right) && UNCLIPPED)
        {
            tile = &pixmap[TopRight];
            MAKE_FILL(r.topRight()-tile->rect().topRight());
            p->drawPixmap(rOff, r.y(), *tile, width(TopRight)-trw, 0, trw, trh);
        }

        checkRect.setRect(xOff, r.y(), w, tlh);
        if (w > 0 && !pixmap[TopMid].isNull() && UNCLIPPED)
        {   // upper line
            solidBg = _bgColor;
            tile = &pixmap[TopMid];
            MAKE_FILL(QPoint(xOff, r.y()));
            p->drawTiledPixmap(checkRect, *tile);
            solidBg = 0;
        }
    }
    
    if (pf & Bottom)
    {
        if (matches(Left | Right, pf) && w < 0)
        {   // horizontal edge overlap
            blw = (blw*r.width())/(blw+brw);
            brw = r.width() - blw;
        }

        int bOff = r.bottom()-blh+1;
        rOff = r.right()-brw+1;
        h -= blh;

        checkRect.setRect(r.x(), bOff, blw, blh);
        if ((pf & Left) && UNCLIPPED)
        {
            tile = &pixmap[BtmLeft];
            MAKE_FILL(r.bottomLeft()-tile->rect().bottomLeft());
            p->drawPixmap(r.x(), bOff, *tile, 0, height(BtmLeft)-blh, blw, blh);
        }

        checkRect.setRect(rOff, bOff, brw, brh);
        if ((pf & Right) && UNCLIPPED)
        {
            tile = &pixmap[BtmRight];
            MAKE_FILL(r.bottomRight()-tile->rect().bottomRight());
            p->drawPixmap(rOff, bOff, *tile, width(BtmRight)-brw, height(BtmRight)-brh, brw, brh);
        }

        checkRect.setRect(xOff, bOff, w, blh);
        if (w > 0 && !pixmap[BtmMid].isNull() && UNCLIPPED)
        {   // lower line
            solidBg = _bgColor;
            tile = &pixmap[BtmMid];
            MAKE_FILL(QPoint(xOff, bOff));
            p->drawTiledPixmap(checkRect, *tile, QPoint(0, height(BtmMid) - blh));
            solidBg = 0;
        }
    }
   
    if (h > 0)
    {
        checkRect.setRect(xOff, yOff, w, h);
        if ((pf & Center) && (w > 0) && UNCLIPPED)
        {   // center part
            tile = &pixmap[MidMid];
            MAKE_FILL(QPoint(xOff, yOff));
            p->drawTiledPixmap(checkRect, *tile);
        }
        checkRect.setRect(r.x(), yOff, width(MidLeft), h);
        if (pf & Left && !pixmap[MidLeft].isNull() && UNCLIPPED)
        {
            solidBg = _bgColor;
            tile = &pixmap[MidLeft];
            MAKE_FILL(QPoint(r.x(), yOff));
            p->drawTiledPixmap(checkRect, *tile);
            solidBg = 0;
        }
        checkRect.setRect(rOff, yOff, width(MidRight), h);
        if (pf & Right && !pixmap[MidRight].isNull() && UNCLIPPED)
        {
            solidBg = _bgColor;
            tile = &pixmap[MidRight];
            rOff = r.right()-width(MidRight)+1;
            MAKE_FILL(QPoint(rOff, yOff));
            p->drawTiledPixmap(checkRect, *tile);
            solidBg = 0;
        }
    }

#undef MAKE_FILL
}

void
Set::outline(const QRect &r, QPainter *p, QColor c, int size) const
{
    PosFlags pf = _shape ? _shape : _defShape;
    const int d = (size+1)/2-1;
//    const int o = size%2;
    QRect rect = r.adjusted(d,d,-d,-d);
    if (rect.isNull())
        return;

    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
//    p->setClipRect(r);
    QPen pen = p->pen();
    pen.setColor(c); pen.setWidth(size);
    p->setPen(pen); p->setBrush(Qt::NoBrush);

    QList<QPainterPath> paths;
    paths << QPainterPath();
    QPoint end = rect.topLeft();
    Set *that = const_cast<Set*>(this);

    if (pf & Top)
    {
        if (pf & Right)
        {
            that->rndRect.moveTopRight(rect.topRight());
            paths.last().arcMoveTo(rndRect, 0);
            paths.last().arcTo(rndRect, 0, 90);
        }
        else
            paths.last().moveTo(rect.topRight());
        if (pf & Left)
        {
            that->rndRect.moveTopLeft(rect.topLeft());
            paths.last().arcTo(rndRect, 90, 90);
        }
        else
            paths.last().lineTo(rect.topLeft());
    }
    else
        paths.last().moveTo(rect.topLeft());

    if (pf & Left)
    {
        if (pf & Bottom)
        {
            that->rndRect.moveBottomLeft(rect.bottomLeft());
            paths.last().arcTo(rndRect, 180, 90);
        }
        else
            paths.last().lineTo(rect.bottomLeft());
    }
    else
    {
        if (!paths.last().isEmpty())
            paths << QPainterPath();
        paths.last().moveTo(rect.bottomLeft());
    }

    if (pf & Bottom)
    {
        if (pf & Right)
        {
            that->rndRect.moveBottomRight(rect.bottomRight());
            paths.last().arcTo(rndRect, 270, 90);
        }
        else
            paths.last().lineTo(rect.bottomRight());
    }
    else
    {
        if (!paths.last().isEmpty())
            paths << QPainterPath();
        paths.last().moveTo(rect.bottomRight());
    }
    
    if (pf & Right)
    {
        if (pf & Top)
            paths.last().connectPath(paths.first());
        else
            paths.last().lineTo(rect.topRight());
    }

    for (int i = 0; i < paths.count(); ++i)
        p->drawPath(paths.at(i));
    p->restore();
}

void Set::setClipOffsets(uint left, uint top, uint right, uint bottom) {
   _clipOffset[0] = left;
   _clipOffset[2] = -right;
   _clipOffset[1] = top;
   _clipOffset[3] = -bottom;
   
   if (!left) pixmap[MidLeft] = QPixmap();
   if (!right) pixmap[MidRight] = QPixmap();
   if (!top) pixmap[TopMid] = QPixmap();
   if (!bottom) pixmap[BtmMid] = QPixmap();
}

QRect Set::bounds(const QRect &rect, PosFlags pf) const
{
   QRect ret = rect;
   if (pf & Left)
      ret.setLeft(ret.left()+_clipOffset[0]);
   if (pf & Top)
      ret.setTop(ret.top()+_clipOffset[1]);
   if (pf & Right)
      ret.setRight(ret.right()+_clipOffset[2]);
   if (pf & Bottom)
      ret.setBottom(ret.bottom()+_clipOffset[3]);
   return ret;
}

const QPixmap &Set::corner(PosFlags pf) const
{
   if (pf == (Top | Left))
      return pixmap[TopLeft];
   if (pf == (Top | Right))
      return pixmap[TopRight];
   if (pf == (Bottom | Right))
      return pixmap[BtmRight];
   if (pf == (Bottom | Left))
      return pixmap[BtmLeft];

   qWarning("requested impossible corner %d",pf);
   return nullPix;
}

QRegion Set::clipRegion(const QRect &rect, PosFlags pf) const
{
   QRegion ret(rect.adjusted(_clipOffset[0], _clipOffset[1],
                             _clipOffset[2], _clipOffset[3]));
   int w,h;
   if (matches(Top | Left, pf)) {
      ret -= QRect(rect.x(), rect.y(), width(TopLeft), height(TopLeft));
   }
   if (matches(Top | Right, pf)) {
      w = width(TopRight);
      ret -= QRect(rect.right()-w+1, rect.y(), w, height(TopRight));
   }
   if (matches(Bottom | Left, pf)) {
      h = height(BtmLeft);
      ret -= QRect(rect.x(), rect.bottom()-h+1, width(BtmLeft), h);
   }
   if (matches(Bottom | Right, pf)) {
      w = width(BtmRight); h = height(BtmRight);
      ret -= QRect(rect.right()-w+1, rect.bottom()-h+1, w, h);
   }
   if (!matches(Center, pf))
      ret &=
      QRegion(rect).subtracted(rect.adjusted(_clipOffset[0],
                                             _clipOffset[1],
                                             _clipOffset[2],
                                             _clipOffset[3]));
   return ret;
}

void
Set::render(const QRect &rect, QPainter *p, const QColor &c) const
{
    _texColor = &c; render(rect, p); _texColor = 0L;
}

void
Set::render(const QRect &rect, QPainter *p, const QPixmap &pix, const QPoint &offset) const
{
    _texPix = &pix; _offset = &offset;
    render(rect, p);
    _texPix = 0L; _offset = 0L;
}

Line::Line(const QPixmap &pix, Qt::Orientation o, int d1, int d2)
{
    _o = o;
    QPainter p;
    if (o == Qt::Horizontal)
    {
        _thickness = pix.height();
        pixmap[0] = pix.copy(0, 0, d1, pix.height());

        int d = qMax(1, pix.width()-d1+d2);
        QPixmap dump = pix.copy(d1, 0, d, pix.height());
        pixmap[1] = QPixmap(qMax(32 , d), pix.height());
        pixmap[1].fill(Qt::transparent);
        p.begin(&pixmap[1]);
        p.drawTiledPixmap(pixmap[1].rect(), dump);
        p.end();

        pixmap[2] = pix.copy(pix.width()+d2, 0, -d2, pix.height());
    }
    else
    {
        _thickness = pix.width();
        pixmap[0] = pix.copy(0, 0, pix.width(), d1);

        int d = qMax(1, pix.height()-d1+d2);
        QPixmap dump = pix.copy(0, d1, pix.width(), d);
        pixmap[1] = QPixmap(pix.width(), qMax(32, d));
        pixmap[1].fill(Qt::transparent);
        p.begin(&pixmap[1]);
        p.drawTiledPixmap(pixmap[1].rect(), dump);
        p.end();

        pixmap[2] = pix.copy(0, pix.height()+d2, pix.width(), -d2);
    }
}

void
Line::render(const QRect &rect, QPainter *p, PosFlags pf, bool btmRight) const
{
    int d0,d2;
    if (_o == Qt::Horizontal)
    {
        const int y = btmRight ? (rect.bottom() - _thickness) : rect.y();
        d0 = (pf & Left) ? width(0) : 0;
        d2 = (pf & Right) ? width(2) : 0;
        if ((pf & Center) && rect.width() >= d0+d2)
            p->drawTiledPixmap(rect.x() + d0, y, rect.width() - (d0 + d2), height(1), pixmap[1]);
        else if (d0 || d2)
        {
            d0 = qMin(d0, d0*rect.width()/(d0+d2));
            d2 = qMin(d2, rect.width() - d0);
        }
        if (pf & Left)
            p->drawPixmap(rect.x(), y, pixmap[0], 0, 0, d0, height(0));
        if (pf & Right)
            p->drawPixmap(rect.right() + 1 - d2, y, pixmap[2], width(2) - d2, 0, d2, height(2));
    }
    else
    {
        const int x = btmRight ? (rect.right() - _thickness) : rect.x();
        d0 = (pf & Top) ? height(0) : 0;
        d2 = (pf & Bottom) ? height(2) : 0;
        if ((pf & Center) && rect.height() >= d0+d2)
            p->drawTiledPixmap(x, rect.y() + d0, width(1), rect.height() - (d0 + d2), pixmap[1]);
        else if (d0 || d2)
        {
            d0 = qMin(d0, d0*rect.height()/(d0 + d2));
            d2 = qMin(d2, rect.height() - d0);
        }
        if (pf & Top)
            p->drawPixmap(x, rect.y(), pixmap[0], 0, 0, width(0), d0);
        if (pf & Bottom)
            p->drawPixmap(x, rect.bottom() + 1 - d2, pixmap[2], 0, height(2) - d2, width(2), d2);
    }
}
