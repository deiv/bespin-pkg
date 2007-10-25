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

//    case PE_PanelTipLabel: // The panel for a tip label.

#include <QApplication>
#include <QDesktopWidget>
#include "draw.h"

#ifndef QT_NO_XRENDER

#include <X11/Xatom.h>

static const Atom bespin_decoDim =
XInternAtom(QX11Info::display(), "BESPIN_DECO_DIM", False);
static const Atom bespin_bgYoff =
XInternAtom(QX11Info::display(), "BESPIN_BG_Y_OFFSET", False);
static const Atom bespin_bgPicture =
XInternAtom(QX11Info::display(), "BESPIN_BG_PICTURE", False);
static const Atom bespin_bgColor =
XInternAtom(QX11Info::display(), "BESPIN_BG_COLOR", False);

inline void *qt_getClipRects( const QRegion &r, int &num )
{
   return r.clipRectangles( num );
}
extern Drawable qt_x11Handle(const QPaintDevice *pd);

static inline uint decoTopSize(uint decoDim) {
   return decoDim & 0xff;
}

static inline uint decoBottomSize(uint decoDim) {
   return (decoDim >> 8) & 0xff;
}

static inline uint decoLeftSize(uint decoDim) {
   return (decoDim >> 16) & 0xff;
}

static inline uint decoRightSize(uint decoDim) {
   return (decoDim >> 24) & 0xff;
}

static inline bool
drawTiledBackground(const QRect &r, QPainter *p, const QColor &c,
                    const QWidget * widget)
{
   Display *dpy = QX11Info::display();
   QWidget *desktop = QApplication::desktop();
   
   unsigned char *data = 0;
   Atom actual;
   int format, result;
   unsigned long n, left;
   uint bgPicture = 0;
   uint bgYoffset = 0;
   
   // get bg picture color
   result = XGetWindowProperty(dpy, desktop->winId(), bespin_bgColor, 0L, 1L,
                               False, XA_CARDINAL, &actual, &format, &n,
                               &left, &data);
   if (result == Success && data != X::None) {
      memcpy (&bgPicture, data, sizeof (unsigned int));
   }
   else
      return false;
   
   // check color range for usability
   bgYoffset = c.rgb();
   if (qRed(bgPicture) > qRed(bgYoffset) + 5) return false;
   if (qRed(bgPicture) < qRed(bgYoffset) - 5) return false;
   if (qGreen(bgPicture) > qGreen(bgYoffset) + 5) return false;
   if (qGreen(bgPicture) < qGreen(bgYoffset) - 5) return false;
   if (qBlue(bgPicture) > qBlue(bgYoffset) + 5) return false;
   if (qBlue(bgPicture) < qBlue(bgYoffset) - 5) return false;
   
   // get bgPicture
   bgPicture = 0;
   result = XGetWindowProperty(dpy, desktop->winId(), bespin_bgPicture, 0L, 1L,
                               False, XA_CARDINAL, &actual, &format, &n,
                               &left, &data);
   if (result == Success && data != X::None) {
      memcpy (&bgPicture, data, sizeof (unsigned int));
   }
   
   if (!bgPicture)
      return false;
   
   // ok, get bgYoff...
   bgYoffset = 0;
   result = XGetWindowProperty(dpy, desktop->winId(), bespin_bgYoff, 0L, 1L,
                               False, XA_CARDINAL, &actual, &format, &n,
                               &left, &data);
   if (result == Success && data != X::None) {
      memcpy (&bgYoffset, data, sizeof (unsigned int));
   }
   // ... and decodim
   uint decoDim = 0;
   result = XGetWindowProperty(dpy, widget->winId(), bespin_decoDim, 0L, 1L,
                               False, XA_CARDINAL, &actual, &format, &n,
                               &left, &data);
   if (result == Success && data != X::None) {
      memcpy (&decoDim, data, sizeof (unsigned int));
   }
   
   // we create a tmp pixmap, paint the tiles on in, paint the pixmap with
   // painter and delete it - would be better to XRenderComposite on the
   // painter device directly - but does not work (for now - you can't see
   // anything)
   
   QPixmap *tmp = new QPixmap(r.size());
   Qt::HANDLE dst = tmp->x11PictureHandle();
   // painter has no clips, thus this doesn't help at all - for the moment
//       int numRects = 0;
//       XRectangle *rects =
//          static_cast<XRectangle*>(qt_getClipRects( p->clipRegion(), numRects ));
//       XRenderSetPictureClipRectangles (dpy, dst, 0, 0, rects, numRects);
   // =========
   
   // just for docu ==============
   // XRenderComposite (dpy, PictOpSrc, _bgPicture_, 0, dst,
   // sx, sy, 0, 0, dx, dy, w, h); - "0" cause we don't use a mask
   // ======================
   int x,y;
   
   // get the top center tile
   x = (desktop->width() - r.width())/2;
   // 'y'  is here more 'h' - which is NOT directly proportional to the window
   // height (we prefer some upper part)
   bgYoffset -= decoTopSize(decoDim); // save the titlebar...
   y = (r.height()*bgYoffset/desktop->height() + bgYoffset)/2;
   bgYoffset += decoTopSize(decoDim); // just revert upper change
   
   XRenderComposite (dpy, PictOpSrc, bgPicture, 0, dst,
                     x, bgYoffset - y, 0, 0, 0, 0,
                     r.width(), y);
   
   // the bottom top/right aligned picture tile
   x = desktop->width() - r.width() - decoRightSize(decoDim) - 1;
   XRenderComposite (dpy, PictOpSrc, bgPicture, 0, dst,
                     x, bgYoffset, 0, 0, 0, y,
                     r.width(), r.height() - y);
   XFlush (dpy);  // this is IMPORTANT - keep it there
   p->drawPixmap(r.topLeft(), *tmp);
   delete tmp;
   return true;
}
#endif

void
BespinStyle::drawWindowFrame(const QStyleOption * option, QPainter * painter,
                             const QWidget *) const
{
   painter->save();
   painter->setPen(PAL.color(QPalette::Window).dark(110));
   painter->drawRect(RECT);
   painter->restore();
}

void
BespinStyle::drawWindowBg(const QStyleOption * option, QPainter * painter,
                          const QWidget * widget) const
{
   if (!(widget && widget->isWindow()))
      return; // can't do anything here
   if (!widget->isActiveWindow())
      return; // experiment - plain inactives...
   if (PAL.brush(widget->backgroundRole()).style() > 1)
      return; // we'd cover a gradient/pixmap/whatever
   
   const QColor &c = PAL.color(widget->backgroundRole());
   
   switch (config.bg.mode) {
#ifndef QT_NO_XRENDER
   case ComplexLights:
      if (drawTiledBackground(RECT, painter, c, widget))
         break;
#endif
   case BevelV: { // also fallback for ComplexLights
      const BgSet &set = Gradients::bgSet(c);
      int s1 = set.topTile.height();
      int s2 = qMin(s1, (RECT.height()+1)/2);
      s1 -= s2;
      painter->drawTiledPixmap( RECT.x(), RECT.y(), RECT.width(), s2,
                                set.topTile, 0, s1 );
      if (Colors::value(c) < 245) { // no sense otherwise
         const int w = RECT.width()/4 - 128;
         if (w > 0) {
            painter->drawTiledPixmap( RECT.x(), RECT.y(), w, 128,
                                      set.cornerTile, 0, s1 );
            painter->drawTiledPixmap( RECT.right()+1-w, RECT.y(), w, 128,
                                      set.cornerTile, 0, s1 );
         }
         painter->drawPixmap(RECT.x()+w, RECT.y(), set.lCorner, 0, s1, 128, s2);
         painter->drawPixmap(RECT.right()-w-127, RECT.y(), set.rCorner, 0, s1, 128, s2);
      }
      s1 = set.btmTile.height();
      s2 = qMin(s1, (RECT.height())/2);
      painter->drawTiledPixmap( RECT.x(), RECT.bottom() - s2,
                                RECT.width(), s2, set.btmTile );
      break;
   }
   case BevelH: {
      const BgSet &set = Gradients::bgSet(c);
      int s1 = set.topTile.width();
      int s2 = qMin(s1, (RECT.width()+1)/2);
      painter->drawTiledPixmap( RECT.x(), RECT.y(), s2, RECT.height(),
                                set.topTile, s1-s2, 0 );
      s1 = set.btmTile.width();
      s2 = qMin(s1, (RECT.width())/2);
      painter->drawTiledPixmap( RECT.right() - s2, 0, s2, RECT.height(),
                                set.btmTile );
      break;
   }
   case LightV: {
      const QPixmap &center = Gradients::bgSet(c).topTile;
      int s1 = qMin(center.height(), RECT.height());
      int s2 = (RECT.height() - center.height())/2;
      painter->drawTiledPixmap( RECT.x(), RECT.y() + qMax(s2,0),
                                RECT.width(), s1, center, 0, qMin(0, s2) );
      break;
   }
   case LightH: {
      const QPixmap &center = Gradients::bgSet(c).topTile;
      int s1 = qMin(center.width(), RECT.width());
      int s2 = (RECT.width() - center.width())/2;
      painter->drawTiledPixmap( RECT.x() + qMax(s2,0), RECT.y(),
                                s1, RECT.height(), center, qMin(0, s2), 0 );
      break;
   }
   case Plain: // should not happen anyway...
   case Scanlines: // --"--
   default:
      break;
   }
}

#define PAINT_WINDOW_BUTTON(_btn_) {\
   tmpOpt.rect =\
      subControlRect(CC_TitleBar, tb, SC_TitleBar##_btn_##Button, widget);\
   if (!tmpOpt.rect.isNull()) { \
      if (tb->activeSubControls & SC_TitleBar##_btn_##Button)\
         tmpOpt.state = tb->state;\
      else\
         tmpOpt.state &= ~(State_Sunken | State_MouseOver);\
      painter->drawPixmap(tmpOpt.rect.topLeft(), \
            standardPixmap ( SP_TitleBar##_btn_##Button, &tmpOpt, widget ));\
   }\
}

void
BespinStyle::drawTitleBar(const QStyleOptionComplex * option,
                          QPainter * painter, const QWidget * widget) const
{
   const QStyleOptionTitleBar *tb =
      qstyleoption_cast<const QStyleOptionTitleBar *>(option);
   if (!tb) return;

//    painter->fillRect(RECT, PAL.brush(QPalette::Window));
   QRect ir;
       
   // the label
   if (option->subControls & SC_TitleBarLabel) {
      ir = subControlRect(CC_TitleBar, tb, SC_TitleBarLabel, widget);
      painter->setPen(PAL.color(QPalette::WindowText));
      ir.adjust(dpi.f2, 0, -dpi.f2, 0);
      painter->drawText(ir, Qt::AlignCenter | Qt::TextSingleLine, tb->text);
   }
   
   QStyleOptionTitleBar tmpOpt = *tb;
   if (tb->subControls & SC_TitleBarCloseButton)
      PAINT_WINDOW_BUTTON(Close)

   if (tb->subControls & SC_TitleBarMaxButton &&
       tb->titleBarFlags & Qt::WindowMaximizeButtonHint) {
      if (tb->titleBarState & Qt::WindowMaximized)
         PAINT_WINDOW_BUTTON(Normal)
      else
         PAINT_WINDOW_BUTTON(Max)
   }
       
   if (tb->subControls & SC_TitleBarMinButton &&
       tb->titleBarFlags & Qt::WindowMinimizeButtonHint) {
      if (tb->titleBarState & Qt::WindowMinimized)
         PAINT_WINDOW_BUTTON(Normal)
      else
         PAINT_WINDOW_BUTTON(Min)
   }
       
   if (tb->subControls & SC_TitleBarNormalButton &&
       tb->titleBarFlags & Qt::WindowMinMaxButtonsHint)
      PAINT_WINDOW_BUTTON(Normal)

   if (tb->subControls & SC_TitleBarShadeButton)
      PAINT_WINDOW_BUTTON(Shade)

   if (tb->subControls & SC_TitleBarUnshadeButton)
      PAINT_WINDOW_BUTTON(Unshade)

   if (tb->subControls & SC_TitleBarContextHelpButton &&
       tb->titleBarFlags & Qt::WindowContextHelpButtonHint)
      PAINT_WINDOW_BUTTON(ContextHelp)

   if (tb->subControls & SC_TitleBarSysMenu &&
       tb->titleBarFlags & Qt::WindowSystemMenuHint) {
      if (!tb->icon.isNull()) {
         ir = subControlRect(CC_TitleBar, tb, SC_TitleBarSysMenu, widget);
         tb->icon.paint(painter, ir);
      }
//    else
//       PAINT_WINDOW_BUTTON(SC_TitleBarSysMenu, SP_TitleBarMenuButton)
   }
#undef PAINT_WINDOW_BUTTON
}

void
BespinStyle::drawSizeGrip(const QStyleOption * option, QPainter * painter,
                          const QWidget *) const
{
   Qt::Corner corner;
   if (const QStyleOptionSizeGrip *sgOpt =
       qstyleoption_cast<const QStyleOptionSizeGrip *>(option))
      corner = sgOpt->corner;
   else if (option->direction == Qt::RightToLeft)
      corner = Qt::BottomLeftCorner;
   else
      corner = Qt::BottomRightCorner;

   QRect rect = RECT;
   rect.setWidth(7*RECT.width()/4);
   rect.setHeight(7*RECT.height()/4);
   painter->save();
   painter->setRenderHint(QPainter::Antialiasing);
   int angle = 90<<4;
   painter->setPen(Qt::NoPen);
   switch (corner) {
   default:
   case Qt::BottomLeftCorner:
      angle = 0;
      rect.moveRight(RECT.right());
   case Qt::BottomRightCorner:
      painter->setBrush(Gradients::pix(FCOLOR(Window).dark(120), rect.height(),
                                       Qt::Vertical, Gradients::Sunken));
//       painter->setBrush(FCOLOR(Window).dark(120));
//       painter->setPen(FCOLOR(Window).dark(140));
      break;
   case Qt::TopLeftCorner:
      angle += 90<<4;
      rect.moveBottomRight(RECT.bottomRight());
   case Qt::TopRightCorner:
      angle += 90<<4;
      rect.moveBottom(RECT.bottom());
      painter->setBrush(FCOLOR(Window).dark(110));
      painter->setPen(FCOLOR(Window).dark(116));
      painter->drawPie(RECT, -(90<<4), 90<<4);
      break;
   }
   painter->drawPie(rect, angle, 90<<4);
   painter->restore();
}
