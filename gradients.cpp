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

#include <QCache>
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <cmath>

#include "gradients.h"

#ifndef QT_NO_XRENDER
#include "oxrender.h"
#endif

using namespace Bespin;

static QPixmap nullPix;
static Gradients::BgMode _mode;
// if you don't link the style, this will create a vtabel error
// define e.g. "Gradients::Type _progressBase = Glass;" instead
extern Gradients::Type _progressBase;


/* ========= MAGIC NUMBERS ARE COOL ;) =================
Ok, we want to cache the gradients, but unfortunately we have no idea about
what kind of gradients will be demanded in the future
Thus creating a 2 component map (uint for color and uint for size)
would be some overhead and cause a lot of unused space in the dictionaries -
while hashing by a string is stupid slow ;)

So we store all the gradients by a uint index
Therefore we substitute the alpha component (byte << 24) of the demanded color
with the demanded size
As this would limit the size to 255/256 pixels we'll be a bit sloppy,
depending on the users resolution (e.g. use 0 to store a gradient with 2px,
usefull for demand of 1px or 2px) and shift the index
(e.g. gradients from 0 to 6 px size will hardly be needed -
maybe add statistics on this)
So the handled size is actually demandedSize + (demandedSize % sizeSloppyness),
beeing at least demanded size and the next sloppy size above at max
====================================================== */
static inline uint
hash(int size, const QColor &c, int *sloppyAdd) {
   
   uint magicNumber = 0;
   int sizeSloppyness = 1, frameBase = 0, frameSize = 20;
   while ((frameBase += frameSize) < size) {
      ++sizeSloppyness;
      frameSize += 20;
   }
      
   frameBase -=frameSize; frameSize -= 20;
   
   *sloppyAdd = size % sizeSloppyness;
   if (!*sloppyAdd)
      *sloppyAdd = sizeSloppyness;

   // first 11 bits to store the size, remaining 21 bits for the color (7bpc)
   magicNumber =  (((frameSize + (size - frameBase)/sizeSloppyness) & 0xff) << 21) |
      (((c.red() >> 1) & 0x7f) << 14) |
      (((c.green() >> 1) & 0x7f) << 7 ) |
      ((c.blue() >> 1) & 0x7f);
   
   return magicNumber;
}

static QPixmap*
newPix(int size, Qt::Orientation o, QPoint *start, QPoint *stop, int other = 32) {
   QPixmap *pix;
   if (o == Qt::Horizontal) {
      pix = new QPixmap(size, other);
      *start = QPoint(0, other); *stop = QPoint(pix->width(), other);
   }
   else {
      pix = new QPixmap(other, size);
      *start = QPoint(other, 0); *stop = QPoint(other, pix->height());
   }
   return pix;
}


static inline QLinearGradient
simpleGradient(const QColor &c, const QPoint &start, const QPoint &stop) {
   QLinearGradient lg(start, stop);
   lg.setColorAt(0, c.light(112));
   lg.setColorAt(1, c.dark(110));
   return lg;
}

static inline QLinearGradient
metalGradient(const QColor &c, const QPoint &start, const QPoint &stop) {
   QLinearGradient lg(start, stop);
   QColor iC = c.light(106); lg.setColorAt(0, iC);
   iC = c.light(103); lg.setColorAt(0.45, iC);
   iC = c.dark(103); lg.setColorAt(0.45, iC);
   iC = c.dark(110); lg.setColorAt(1, iC);
   return lg;
}

static inline QLinearGradient
sunkenGradient(const QColor &c, const QPoint &start, const QPoint &stop) {
   QLinearGradient lg(start, stop);
   lg.setColorAt(0, c.dark(110));
   lg.setColorAt(1, c.light(112));
   return lg;
}

static inline QLinearGradient
buttonGradient(const QColor &c, const QPoint &start, const QPoint &stop) {
   int h,s,v, inc, dec;
   c.getHsv(&h,&s,&v);
   
   // calc difference
   inc = 15; dec = 6;
   if (v+inc > 255) {
      inc = 255-v; dec += (15-inc);
   }
   QLinearGradient lg(start, stop);
   QColor ic; ic.setHsv(h,s,v+inc);
   lg.setColorAt(0, ic);
   ic.setHsv(h,s,v-dec);
   lg.setColorAt(0.75, ic);
   return lg;
}

inline static void
gl_ssColors(const QColor &c, QColor *bb, QColor *dd, bool glass = false) {
   
   int h,s,v, ch,cs,cv, delta, add;
   
   c.getHsv(&h,&s,&v);

   // calculate the variation
   add = ((180-qGray(c.rgb()))>>1);
   if (add < 0) add = -add/2;
   if (glass)
      add = add>>4;

   // the brightest color (top)
   cv = v+27+add;
   if (cv > 255) {
      delta = cv-255; cv = 255;
      cs = s - delta; if (cs < 0) cs = 0;
      ch = h - delta/6; if (ch < 0) ch = 360+ch;
   }
   else {
      ch = h; cs = s;
   }
   bb->setHsv(ch,cs,cv);
   
   // the darkest color (lower center)
   cv = v - 14-add; if (cv < 0) cv = 0;
   cs = s*13/7; if (cs > 255) cs = 255;
   dd->setHsv(h,cs,cv);
}

static inline QLinearGradient
gl_ssGradient(const QColor &c, const QPoint &start, const QPoint &stop, bool glass = false) {
   QColor bb,dd; // b = d = c;
   gl_ssColors(c, &bb, &dd, glass);
   QLinearGradient lg(start, stop);
   lg.setColorAt(0,bb); lg.setColorAt(0.5,c);
   lg.setColorAt(0.5, dd); lg.setColorAt(glass ? 1 : .90, bb);
   return lg;
}

static inline QPixmap *
rGlossGradient(const QColor &c, int size) {
   QColor bb,dd; // b = d = c;
   gl_ssColors(c, &bb, &dd);
   QPixmap *pix = new QPixmap(size, size);
   QRadialGradient rg(2*pix->width()/3, pix->height(), pix->height());
   rg.setColorAt(0,c); rg.setColorAt(0.8,dd);
   rg.setColorAt(0.8, c); rg.setColorAt(1, bb);
   QPainter p(pix); p.fillRect(pix->rect(), rg); p.end();
   return pix;
}

static inline QPixmap *
progressGradient(const QColor &c, int size, Qt::Orientation o) {
#define GLASS true
#define GLOSS false
// in addition, glosses should have the last stop at 0.9
   
   // some psychovisual stuff, we search a dark & bright surrounding and
   // slightly shift hue as well (e.g. for green the dark color will slide to
   // blue and the bright one to yellow - A LITTLE! ;)
   int h,s,v;
   c.getHsv(&h,&s,&v);
   QColor dkC = c, ltC = c;
   int dv = 4*(v-80)/35; // v == 80 -> dv = 0, v = 255 -> dv = 20
//    int th = h + 400;
   int dh = qAbs((h % 120)-60)/6;
   dkC.setHsv(h+dh, s, v - dv);
   h -=dh; if (h < 0) h = 400 + h;
   dv = 20 - dv;
   ltC.setHsv(h-5,s, v + dv);
   
//    int dc = Colors::value(c)/5; // how much darken/lighten we will
//    QColor dkC = c.dark(100+sqrt(2*dc));
//    QColor ltC = c.light(150-dc);
   
   QPoint start, stop;
   QPixmap *dark = newPix(size, o, &start, &stop, 3*size);
   QGradient lg1 = simpleGradient(ltC, start, stop),
      lg2 = gl_ssGradient(dkC, start, stop, true);
//    switch (_progressBase) {
//    case Gradients::Button:
//       lg1 = buttonGradient(dkC, start, stop);
//       lg2 = buttonGradient(ltC, start, stop); break;
//    case Gradients::Glass:
//    default:
//       lg1 = gl_ssGradient(dkC, start, stop, true);
//       lg2 = gl_ssGradient(ltC, start, stop, true); break;
//    case Gradients::Simple:
//       lg1 = simpleGradient(dkC, start, stop);
//       lg2 = simpleGradient(ltC, start, stop); break;
//    case Gradients::Sunken:
//       lg1 = sunkenGradient(dkC, start, stop);
//       lg2 = sunkenGradient(ltC, start, stop); break;
//    case Gradients::Gloss:
//       lg1 = gl_ssGradient(dkC, start, stop);
//       lg2 = gl_ssGradient(ltC, start, stop); break;
//    }
   QPainter p(dark); p.fillRect(dark->rect(), lg1); p.end();
   
   QPixmap alpha = QPixmap(dark->size());
   QRadialGradient rg(alpha.rect().center(), 3*size/2);
   rg.setColorAt(0, Qt::white);
#ifndef QT_NO_XRENDER
   rg.setColorAt(0.9, Qt::transparent);
   alpha.fill(Qt::transparent);
#else
   rg.setColorAt(0.9, Qt::black);
#endif
   p.begin(&alpha); p.fillRect(alpha.rect(), rg); p.end();
#ifndef QT_NO_XRENDER
   alpha = OXRender::applyAlpha(*dark, alpha);
#else
   dark->setAlphaChannel(alpha);
#endif
   
   QPixmap *pix = new QPixmap(dark->size());
   p.begin(pix);
   p.fillRect(pix->rect(), lg2);
#ifndef QT_NO_XRENDER
   p.drawPixmap(0,0, alpha);
#else
   p.drawPixmap(0,0, *dark);
#endif
   p.end();
   
   delete dark;
   return pix;
#undef GLASS
#undef GLOSS
}

static inline uint costs(QPixmap *pix) {
   return ((pix->width()*pix->height()*pix->depth())>>3);
}

static inline uint costs(BgSet *set) {
   return (set->topTile.width()*set->topTile.height() +
           set->btmTile.width()*set->btmTile.height() +
           set->cornerTile.width()*set->cornerTile.height() +
           set->lCorner.width()*set->lCorner.height() +
           set->rCorner.width()*set->rCorner.height())*set->topTile.depth()/8;
}

typedef QCache<uint, QPixmap> PixmapCache;
static PixmapCache gradients[2][Gradients::TypeAmount];
static PixmapCache _btnAmbient, _tabShadow, _groupLight;
typedef QCache<uint, BgSet> BgSetCache;
static BgSetCache _bgSet;

const QPixmap&
Gradients::pix(const QColor &c, int size, Qt::Orientation o, Gradients::Type type) {
   // validity check
   if (size <= 0) {
      qWarning("NULL Pixmap requested, size was %d",size);
      return nullPix;
   }
   else if (size > 105883) { // this is where our dictionary reaches - should be enough for the moment ;)
      qWarning("gradient with more than 105883 steps requested, returning NULL pixmap");
      return nullPix;
   }
   
   // very dark colors won't make nice buttons =)
   QColor iC = c;
   int v = Colors::value(c);
   if (v < 80) {
      int h,s;
      c.getHsv(&h,&s,&v);
      iC.setHsv(h,s,80);
   }

   // hash 
   int sloppyAdd = 1;
   uint magicNumber = hash(size, iC, &sloppyAdd);

   PixmapCache *cache = &gradients[o == Qt::Horizontal][type];
   QPixmap *pix = cache->object(magicNumber);
   if (pix)
      return *pix;
   
   QPoint start, stop;
   
   if (type == Gradients::Progress)
      pix = progressGradient(iC, size, o);
   else if (type == Gradients::RadialGloss)
      pix = rGlossGradient(iC, size);
   else {
      pix = newPix(size, o, &start, &stop);
   
      QGradient grad;
      // no cache entry found, so let's create one
      size += sloppyAdd; // rather to big than to small ;)
      switch (type) {
      case Gradients::Button:
         grad = buttonGradient(iC, start, stop);
         break;
      case Gradients::Glass:
         grad = gl_ssGradient(iC, start, stop, true);
         break;
      case Gradients::Simple:
      default:
         grad = simpleGradient(iC, start, stop);
         break;
      case Gradients::Sunken:
         grad = sunkenGradient(iC, start, stop);
         break;
      case Gradients::Gloss:
         grad = gl_ssGradient(iC, start, stop);
         break;
      case Gradients::Metal:
         grad = metalGradient(iC, start, stop);
         break;
      }
      QPainter p(pix); p.fillRect(pix->rect(), grad); p.end();
   }
   
   // cache for later
   if (cache)
      cache->insert(magicNumber, pix, costs(pix));
   return *pix;
}

const QPixmap &Gradients::light(int height) {
   if (height <= 0) {
      qWarning("NULL Pixmap requested, height was %d",height);
      return nullPix;
   }
   QPixmap *pix = _groupLight.object(height);
   if (pix)
      return *pix;
      
   pix = new QPixmap(32, height); //golden mean relations
   pix->fill(Qt::transparent);
   QPoint start(0,0), stop(0,height);
   QLinearGradient lg(start, stop);
   lg.setColorAt(0, QColor(255,255,255,116));
   lg.setColorAt(1, QColor(255,255,255,0));
   QPainter p(pix); p.fillRect(pix->rect(), lg); p.end();
   
   // cache for later ;)
   _groupLight.insert(height, pix, costs(pix));
   return *pix;
}

const QPixmap &Gradients::ambient(int height) {
   if (height <= 0) {
      qWarning("NULL Pixmap requested, height was %d",height);
      return nullPix;
   }

   QPixmap *pix = _btnAmbient.object(height);
   if (pix)
      return *pix;

   pix = new QPixmap(16*height/9,height); //golden mean relations
   pix->fill(Qt::transparent);
   QLinearGradient lg(QPoint(pix->width(), pix->height()),
                      QPoint(pix->width()/2,pix->height()/2));
   lg.setColorAt(0, QColor(255,255,255,0));
   lg.setColorAt(0.2, QColor(255,255,255,100));
   lg.setColorAt(1, QColor(255,255,255,0));
   QPainter p(pix); p.fillRect(pix->rect(), lg); p.end();

   // cache for later ;)
   _btnAmbient.insert(height, pix, costs(pix));
   return *pix;
}

const QPixmap &Gradients::shadow(int height, bool bottom) {
   if (height <= 0) {
      qWarning("NULL Pixmap requested, height was %d",height);
      return nullPix;
   }
   uint val = height + bottom*0x80000000;
   QPixmap *pix = _tabShadow.object(val);
   if (pix)
      return *pix;
      
   pix = new QPixmap(height/3,height);
   pix->fill(Qt::transparent);
   
   float hypo = sqrt(pow(pix->width(),2)+pow(pix->height(),2));
   float cosalpha = (float)(pix->height())/hypo;
   QPoint p1, p2;
   if (bottom) {
      p1 = QPoint(0, 0);
      p2 = QPoint((int)(pix->width()*pow(cosalpha, 2)),
                  (int)(pow(pix->width(), 2)*cosalpha/hypo));
   }
   else {
      p1 = QPoint(0, pix->height());
      p2 = QPoint((int)(pix->width()*pow(cosalpha, 2)),
                  (int)pix->height() - (int)(pow(pix->width(), 2)*cosalpha/hypo));
   }
   QLinearGradient lg(p1, p2);
   lg.setColorAt(0, QColor(0,0,0,75));
   lg.setColorAt(1, QColor(0,0,0,0));
   QPainter p(pix); p.fillRect(pix->rect(), lg); p.end();
   
   // cache for later ;)
   _tabShadow.insert(val, pix, costs(pix));
   return *pix;
}

static inline QPixmap *cornerMask(bool right = false) {
   QPixmap *alpha = new QPixmap(128,128);
   QRadialGradient rg(right ? alpha->rect().topLeft() :
                      alpha->rect().topRight(), 128);
#ifndef QT_NO_XRENDER
   alpha->fill(Qt::transparent);
   rg.setColorAt(0, Qt::transparent);
#else
   rg.setColorAt(0, Qt::black);
#endif
   rg.setColorAt(1, Qt::white);
   QPainter p(alpha); p.fillRect(alpha->rect(), rg); p.end();
   return alpha;
}

const BgSet &Gradients::bgSet(const QColor &c) {
   BgSet *set = _bgSet.object(c.rgb());
   if (set)
      return *set;
   set = new BgSet;
   QLinearGradient lg;
   QPainter p;
   switch (_mode) {
   case BevelV: {
      set->topTile = QPixmap(32, 256);
      set->btmTile = QPixmap(32, 256);
      set->cornerTile = QPixmap(32, 128);
      set->lCorner = QPixmap(128, 128);
      set->rCorner = QPixmap(128, 128);
      const QColor c1 = c.light(106);
      const QColor c2 = Colors::mid(c1, c);

      lg = QLinearGradient(QPoint(0,0), QPoint(0,256));
      QGradientStops stops;
      // Top Tile
      p.begin(&set->topTile);
      stops << QGradientStop(0, c1) << QGradientStop(1, c);
      lg.setStops(stops); p.fillRect(set->topTile.rect(), lg);
      stops.clear(); p.end();
      // Bottom Tile
      p.begin(&set->btmTile);
      stops << QGradientStop(0, c) << QGradientStop(1, c.dark(106));
      lg.setStops(stops); p.fillRect(set->btmTile.rect(), lg);
      stops.clear(); p.end();
      if (Colors::value(c) > 244)
         break; // would be mathematically nonsense, i.e. shoulders = 255...
      // Corner Tile
      lg = QLinearGradient(QPoint(0,0), QPoint(0,128));
      p.begin(&set->cornerTile);
      stops << QGradientStop(0, Colors::mid(c1, c2,1,4)) << QGradientStop(1, c2);
      lg.setStops(stops); p.fillRect(set->cornerTile.rect(), lg);
      stops.clear(); p.end();
      // Left Corner, right corner
      QPixmap *mask, *pix;
      for (int cnr = 0; cnr < 2; ++cnr) {
         pix = cnr ? &set->rCorner : &set->lCorner;
         p.begin(pix);
         p.drawTiledPixmap(pix->rect(), set->topTile);
         p.end();
         mask = cornerMask(cnr);
#ifndef QT_NO_XRENDER
         for (int i = 0; i < 128; i += 32)
            OXRender::composite(set->cornerTile, mask->x11PictureHandle(),
                                *pix, 0, 0, i, 0, i, 0, 32, 128, PictOpOver);
#else
         QPixmap fill(pix->size());
         p.begin(&fill);
         p.drawTiledPixmap(fill.rect(), set->cornerTile); p.end();
         fill.setAlphaChannel(*mask);
         p.begin(pix); p.drawPixmap(0,0, fill); p.end();
#endif
         delete mask;
      }
      break;
   }
   case BevelH: {
      set->topTile = QPixmap(256, 32);
      set->btmTile = QPixmap(256, 32);
      set->lCorner = QPixmap(256, 32);
      set->rCorner = QPixmap(256, 32);
      set->cornerTile = QPixmap(32, 128);
      
      lg = QLinearGradient(QPoint(0,0), QPoint(256, 0));
      QGradientStops stops;
      const QColor c1 = c.dark(106);
      
      // left
      p.begin(&set->topTile);
      stops << QGradientStop(0, c1) << QGradientStop(1, c);
      lg.setStops(stops); p.fillRect(set->topTile.rect(), lg);
      stops.clear(); p.end();
      // right
      p.begin(&set->btmTile);
      stops << QGradientStop(0, c) << QGradientStop(1, c1);
      lg.setStops(stops); p.fillRect(set->btmTile.rect(), lg);
      stops.clear(); p.end();
      // left corner right corner
      QPixmap *mask, *pix, *blend;
      for (int cnr = 0; cnr < 2; ++cnr) {
         if (cnr) {
            pix = &set->rCorner; blend = &set->btmTile;
         }
         else {
            pix = &set->lCorner; blend = &set->topTile;
         }
         pix->fill(c);
         mask = new QPixmap(256,32);
         lg = QLinearGradient(0,0, 0,32);
         lg.setColorAt(0, Qt::white);
#ifndef QT_NO_XRENDER
         mask->fill(Qt::transparent);
         lg.setColorAt(1, Qt::transparent);
         p.begin(mask); p.fillRect(mask->rect(), lg); p.end();
         OXRender::composite(*blend, mask->x11PictureHandle(),
                             *pix, 0, 0, 0, 0, 0, 0, 256, 32, PictOpOver);
#else
         lg.setColorAt(1, Qt::black);
         p.begin(mask); p.fillRect(mask->rect(), lg); p.end();
         QPixmap fill(pix->size());
         p.begin(&fill); p.drawTiledPixmap(fill.rect(), *blend); p.end();
         fill.setAlphaChannel(*mask);
         p.begin(pix); p.drawPixmap(0,0, fill); p.end();
#endif
         delete mask;
      }
      lg = QLinearGradient(QPoint(0,0), QPoint(0, 128));
      lg.setColorAt(0, c); lg.setColorAt(1, c1);
      p.begin(&set->cornerTile);
      p.fillRect(set->cornerTile.rect(), lg); p.end();
      break;
   }
   default:
      break;
   }
   _bgSet.insert(c.rgb(), set, costs(set));
   return *set;
}

void Gradients::init(BgMode mode) {
   _mode = mode;
   _bgSet.setMaxCost( 900<<10 ); // 832 should be enough - we keep some safety
   for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < Gradients::TypeAmount; ++j)
         gradients[i][j].setMaxCost( 1024<<10 );
   }
   _btnAmbient.setMaxCost( 64<<10 );
   _tabShadow.setMaxCost( 64<<10 );
   _groupLight.setMaxCost( 256<<10 );
}

void Gradients::wipe() {
   _bgSet.clear();
   for (int i = 0; i < 2; ++i)
      for (int j = 0; j < Gradients::TypeAmount; ++j)
         gradients[i][j].clear();
   _btnAmbient.clear();
   _tabShadow.clear();
   _groupLight.clear();
}
