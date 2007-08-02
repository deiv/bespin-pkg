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

#include <QAction>
#include <QApplication>
#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QComboBox>
#include <QDesktopWidget>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QStyleOptionButton>
#include <QPainter>
#include <QPainterPath>

#ifdef Q_WS_X11
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <fixx11h.h>
#endif

#include <cmath>

#include "bespin.h"
#ifndef QT_NO_XRENDER
#include "oxrender.h"
#endif
#include "visualframe.h"

#include <QtDebug>

using namespace Bespin;

extern Config config;
extern Dpi dpi;

#include "inlinehelp.cpp"
#include "makros.h"

static int radius(const QRect &r) {
   const QPoint &c = r.center();
   return sqrt(pow(c.x()-r.x(),2)+pow(c.y()-r.y(),2));
}

#ifndef QT_NO_XRENDER
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

static inline bool drawTiledBackground(const QRect &r, QPainter *p, const QPalette &pal, const QWidget * widget) {
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
   bgYoffset = pal.color(widget->backgroundRole()).rgb();
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

void BespinStyle::drawPrimitive ( PrimitiveElement pe, const QStyleOption * option, QPainter * painter, const QWidget * widget) const {
   Q_ASSERT(option);
   Q_ASSERT(painter);
   
   bool sunken = option->state & State_Sunken;
   bool isEnabled = option->state & State_Enabled;
   bool hover = isEnabled && (option->state & State_MouseOver);
   bool hasFocus = option->state & State_HasFocus;
   bool up = false;
   
   
   switch ( pe ) {
   case PE_Widget: {
      
      if (!(widget && widget->isWindow())) break; // can't do anything here

      switch (config.bgMode) {
#ifndef QT_NO_XRENDER
      case ComplexLights:
         if (drawTiledBackground(RECT, painter, PAL, widget))
            break;
#endif
      case BevelV: { // also fallback for ComplexLights
         const QColor &c = PAL.color(widget->backgroundRole());
         const QPixmap &upper = Gradients::bg(c);
         const QPixmap &lower = Gradients::bg(c, true);
         int s1 = upper.height();
         int s2 = qMin(s1, (RECT.height()+1)/2);
         painter->drawTiledPixmap( RECT.x(), RECT.y(), RECT.width(), s2,
                                   upper, 0, s1-s2 );
         s1 = lower.height();
         s2 = qMin(s1, (RECT.height())/2);
         painter->drawTiledPixmap( RECT.x(), RECT.bottom() - s2,
                                   RECT.width(), s2, lower );
         break;
      }
      case BevelH: {
         const QColor &c = PAL.color(widget->backgroundRole());
         const QPixmap &left = Gradients::bg(c);
         const QPixmap &right = Gradients::bg(c, true);
         int s1 = left.width();
         int s2 = qMin(s1, (RECT.width()+1)/2);
         painter->drawTiledPixmap( RECT.x(), RECT.y(), s2, RECT.height(),
                                   left, s1-s2, 0 );
         s1 = right.width();
         s2 = qMin(s1, (RECT.width())/2);
         painter->drawTiledPixmap( RECT.right() - s2, 0, s2, RECT.height(),
                                   right );
         break;
      }
      case LightV: {
         const QPixmap &center =
               Gradients::bg(PAL.color(widget->backgroundRole()));
         int s1 = qMin(center.height(), RECT.height());
         int s2 = (RECT.height() - center.height())/2;
         painter->drawTiledPixmap( RECT.x(), RECT.y() + qMax(s2,0),
                                   RECT.width(), s1, center, 0, qMin(0, s2) );
         break;
      }
      case LightH: {
         const QPixmap &center =
               Gradients::bg(PAL.color(widget->backgroundRole()));
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
      break;
   }
   case PE_FrameDefaultButton: // This frame around a default button, e.g. in a dialog.
      // we swap colors instead, frame only on focus!
      break;
   case PE_PanelButtonCommand: // Button used to initiate an action, for example, a QPushButton.
   case PE_PanelButtonBevel: { // Generic panel with a button bevel.
      const QStyleOptionButton* opt =
         qstyleoption_cast<const QStyleOptionButton*>(option);
      
      const int f1 = dpi.f1, f2 = dpi.f2;
      QRect r = RECT;
      int step = sunken ? 6 : animator->hoverStep(widget);
      QColor c = btnBgColor(PAL, isEnabled, hasFocus, step);
      if (config.fullButtonHover && (hover || step))
         c = midColor(c, COLOR(Button), 6-step, step);
      
      Gradients::Type gt = isEnabled ? config.gradButton : Gradients::None;
      
      // sunken variant
      if (config.sunkenButtons) {
         r.setBottom(r.bottom()-f2);
         masks.button.render(r, painter, Gradients::brush(c, r.height(),
            Qt::Vertical, sunken ? Gradients::Sunken : config.gradButton));
         if (!config.fullButtonHover && (hover || step)) {
            const QRect ir = r.adjusted(dpi.f3, f2, -dpi.f3, -f2 );
            c = midColor(c, COLOR(Button), 6-step, step);
            masks.button.render(ir, painter, Gradients::brush(c, r.height(),
                                Qt::Vertical, sunken ? Gradients::Sunken : gt),
                                Tile::Full, false, QPoint(0,f2));
         }
         if (hasFocus)
            masks.button.outline(r, painter, midColor(c, COLOR(Highlight)),
                                 config.strongFocus);
         shadows.lineEdit[isEnabled].render(RECT, painter);
         break;
      }
      
      // normal buttons
      // shadow
      if (sunken) {
         r.adjust(f1, f1, -f1, -f2);
         shadows.button[true][isEnabled].render(r, painter);
         r.adjust(f1, f1, -f1, -f1);
      }
      else {
         shadows.button[false][isEnabled].render(r, painter);
         r.adjust(f2, f2, -f2, -dpi.f3);
         if (hasFocus)
            lights.button.render(RECT, painter, COLOR(Highlight));
      }
      
      // backlight & plate
      masks.button.render(r, painter, Gradients::brush(c, r.height(),
         Qt::Vertical, gt));
      
      // outline
      masks.button.outline(r, painter, midColor(c, Qt::white,1,2), true);
      
      if (!config.fullButtonHover && (hover || step)) {
         const QRect ir = r.adjusted(dpi.f3, f2, -dpi.f3, -f2 );
         c = midColor(c, COLOR(Button), 6-step, step);
         masks.button.render(ir, painter,
                             Gradients::brush(c, ir.height(), Qt::Vertical, gt),
                             Tile::Full, false, QPoint(0,f2));
      }
      
      // ambient?
      if (!sunken)
         painter->drawPixmap(QPoint(r.right()+1-16*r.height()/9, r.top()),
                             Gradients::ambient(r.height()));

      break;
   }
   case PE_PanelButtonTool: { // Panel for a Tool button, used with QToolButton.
      if (!isEnabled)
         break;
      bool isOn = option->state & State_On;
      int step = animator->hoverStep(widget);
      const QColor &c = bgcolor(PAL, widget);
      if (isOn && (!hover || step < 6))
         masks.tab.render(RECT, painter,
                          Gradients::pix(c, RECT.height(), Qt::Vertical, Gradients::Sunken));
      if (hover || step || sunken) {
         QRect r = RECT;
         if (!sunken && step) {
            step = 6 - step;
            int dx = step*r.width()/18; int dy = step*r.height()/18;
            r.adjust(dx, dy, -dx, -dy);
         }
         masks.tab.render(r, painter,
                          Gradients::pix(c, r.height(), Qt::Vertical, sunken ?
                                Gradients::Sunken : Gradients::Button));
      }
      if (isOn)
         shadows.tabSunken.render(RECT, painter);
      break;
   }
   case PE_PanelLineEdit: { // Panel for a QLineEdit.
      // spinboxes and combos allready have a lineedit as global frame
      if (qstyleoption_cast<const QStyleOptionFrame *>(option) &&
          static_cast<const QStyleOptionFrame *>(option)->lineWidth < 1) {
         if (!widget || widget->autoFillBackground())
            painter->fillRect(RECT, COLOR(Base));
         break;
      }
      QRect r = RECT;
      if (isEnabled) {
         if (hasFocus) {
            r.adjust(0,0,0,-dpi.f2);
            masks.button.render(r, painter, COLOR(Base).light(112));
//             r.setBottom(r.bottom()+dpi.f1);
            QColor h = midColor(COLOR(Base), COLOR(Highlight), 2, 1);
            masks.button.outline(r, painter, h, config.strongFocus, Tile::Ring, dpi.f3);
         }
         else {
            r.setBottom(r.y()+r.height()/2);
            const QPixmap &fill =
               Gradients::pix(COLOR(Base), r.height(), Qt::Vertical, Gradients::Sunken);
            masks.button.render(r, painter, fill, Tile::Full & ~Tile::Bottom);
            r.setTop(r.bottom()+1); r.setBottom(RECT.bottom()-dpi.f2);
            masks.button.render(r, painter, COLOR(Base).light(112),
                                Tile::Full & ~Tile::Top);
         }
      }
      shadows.lineEdit[isEnabled].render(RECT, painter);
      break;
   }
   case PE_FrameFocusRect: { // Generic focus indicator.
      painter->save();
      painter->setBrush(Qt::NoBrush);
      painter->setPen(COLOR(Highlight));
      painter->drawLine(RECT.bottomLeft(), RECT.bottomRight());
      painter->restore();
      break;
   }
   case PE_IndicatorArrowUp: // Generic Up arrow.
   case PE_IndicatorSpinUp: // Up symbol for a spin widget, for example a QSpinBox.
   case PE_IndicatorSpinPlus: // Increase symbol for a spin widget.
      up = true;
   case PE_IndicatorHeaderArrow: // Arrow used to indicate sorting on a list or table header
   case PE_IndicatorButtonDropDown: // indicator for a drop down button, for example, a tool button that displays a menu.
   case PE_IndicatorArrowDown: // Generic Down arrow.
   case PE_IndicatorSpinDown: // Down symbol for a spin widget.
   case PE_IndicatorSpinMinus: { // Decrease symbol for a spin widget.
      if (const QStyleOptionHeader* hopt =
          qstyleoption_cast<const QStyleOptionHeader*>(option)) {
         if (hopt->sortIndicator == QStyleOptionHeader::None)
            break;
         up = hopt->sortIndicator == QStyleOptionHeader::SortUp;
      }
      SAVE_ANTIALIAS;
      bool hadNoBrush = painter->brush() == Qt::NoBrush;
      painter->setRenderHint(QPainter::Antialiasing);
      if (hadNoBrush)
         painter->setBrush(painter->pen().brush());
      
      int w = RECT.width();
      
      // we want a golden mean cut arrow ;) 1:1.6180339887498948482
      int x[3], y[2];
      if (w <  8*RECT.height()/5) {
         if (w%2) --w;
         x[0] = RECT.x(); x[1] = RECT.right(); x[2] = x[0] + w/2;
         int h = 5*w/8;  if (!(h%2)) --h;
         y[0] = RECT.y() + (RECT.height()-h)/2; y[1] = y[0] + h;
      }
      else {
         w = 8*RECT.height()/5;
         if (w%2) --w;
         x[0] = RECT.x() + (RECT.width()-w)/2; x[1] = x[0] + w; x[2] = x[0] + w/2;
         y[0] = RECT.y(); y[1] = RECT.bottom();
      }
      if (up) {
         const QPoint points[3] =  { QPoint(x[0], y[1]), QPoint(x[1], y[1]), QPoint(x[2], y[0]) };
         painter->drawPolygon(points, 3);
      }
      else {
         const QPoint points[3] =  { QPoint(x[0], y[0]), QPoint(x[1], y[0]), QPoint(x[2], y[1]) };
         painter->drawPolygon(points, 3);
      }
      if (hadNoBrush)
         painter->setBrush(Qt::NoBrush);
      RESTORE_ANTIALIAS;
      break;
   }
   case PE_IndicatorArrowRight: // Generic Right arrow.
      up = true;
   case PE_IndicatorArrowLeft: { // Generic Left arrow.
      SAVE_ANTIALIAS;
      bool hadNoBrush = painter->brush() == Qt::NoBrush;
      painter->setRenderHint(QPainter::Antialiasing);
      if (hadNoBrush)
         painter->setBrush(painter->pen().brush());
      int x[2], y[3], h = RECT.height();
      if (h <  8*RECT.width()/5) {
         if (h%2) --h;
         y[0] = RECT.y(); y[1] = RECT.bottom(); y[2] = y[0] + h/2;
         int w = 5*h/8; if (!(w%2)) --w;
         x[0] = RECT.x() + (RECT.width()-w)/2; x[1] = x[0] + w;
      }
      else {
         h = 8*RECT.width()/5;
         if (h%2) --h;
         y[0] = RECT.y() + (RECT.height()-h)/2; y[1] = y[0] + h; y[2] = y[0] + h/2;
         x[0] = RECT.x(); x[1] = RECT.right();
      }
      if (up) { //right
         const QPoint points[3] =  { QPoint(x[0], y[0]), QPoint(x[0], y[1]), QPoint(x[1], y[2]) };
         painter->drawPolygon(points, 3);
      }
      else {
         const QPoint points[3] =  { QPoint(x[0], y[2]), QPoint(x[1], y[0]), QPoint(x[1], y[1]) };
         painter->drawPolygon(points, 3);
      }
      if (hadNoBrush)
         painter->setBrush(Qt::NoBrush);
      RESTORE_ANTIALIAS;
      break;
   }
   case PE_IndicatorCheckBox: { // On/off indicator, for example, a QCheckBox.
      bool _fullHover = config.fullButtonHover;
//       if (config.checkType == 2)
         config.fullButtonHover = true;
      drawPrimitive(PE_PanelButtonBevel, option, painter, widget);
      config.fullButtonHover = _fullHover;
      
      if (!(sunken || (option->state & State_Off))) {
         painter->save();
         painter->setRenderHint(QPainter::Antialiasing);
         const int d = config.sunkenButtons ? dpi.f4 : dpi.f6;
         QRect r = RECT.adjusted(d, d, -d, -d);
         const QColor fill = btnFgColor(PAL, isEnabled, hover);
         switch (config.checkType) {
         case 0: {
            QPen pen(fill, r.width()/5, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
            painter->setPen(pen);
            int d = r.height()/8, c = r.height()/2, s = r.width(),
            x = r.x(), y = r.y();
            if (option->state & State_On) {
               const QPoint points[8] = {
               QPoint(x+c,y+c-d), QPoint(x,y),
               QPoint(x+c-d,y+c), QPoint(x,y+s),
               QPoint(x+c,y+c+d), QPoint(x+s,y+s),
               QPoint(x+c+d,y+c), QPoint(x+s,y)
               };
               painter->drawPolygon(points, 8);
            }
            else {
               const QPoint points[5] = {
               QPoint(x+c,y+c-d), QPoint(x,y), QPoint(x+c-d,y+c),
               QPoint(x+s,y+s), QPoint(x+c+d,y+c),
               };
               painter->drawPolygon(points, 5);
            }
            break;
         }
         default:
         case 1: {
            QPen pen(fill, r.width()/5, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
            painter->setPen(pen);
            const QPoint points[4] = {
            QPoint(r.right(), r.top()),
            QPoint(r.x()+r.width()/3, r.bottom()),
            QPoint(r.x(), r.bottom()-r.height()/3),
            QPoint(r.x()+r.width()/3, r.bottom()-r.height()/5)
            };
            painter->drawPolygon(points, (option->state & State_On)?4:2);
            break;
         }
         case 2:
            if (option->state & State_On)
               masks.button.render(r, painter, fill);
         }
         painter->restore();
      }
      break;
   }
   case PE_IndicatorRadioButton: { // Exclusive on/off indicator, for example, a QRadioButton.
      bool isOn = option->state & State_On;
      const int f2 = dpi.f2, f1 = dpi.f1;
      int sz = dpi.ExclusiveIndicator;
      QPoint xy = RECT.topLeft();
      
      Gradients::Type gt = isEnabled ? config.gradButton : Gradients::None;
      
      if (isOn) {
         hover = hasFocus = false;
      }
      else if (hover && sunken)
         isOn = true;
      int step = isOn ? 0 : animator->hoverStep(widget);
      
      
      QColor c = btnBgColor(PAL, isEnabled, hasFocus, step);
      if (config.fullButtonHover)
         c = midColor(c, COLOR(Button), 6-step, step);
      
      if (config.sunkenButtons) {
         QRect r = RECT.adjusted(dpi.f1,0,-dpi.f1,-dpi.f2);
         masks.tab.render(r, painter,
                          Gradients::brush( c, r.height(), Qt::Vertical,
                                            sunken || isOn ? Gradients::Sunken : gt));
         r.setBottom(RECT.bottom());
         shadows.tabSunken.render(r, painter);
         xy += QPoint(f1, 0);
      }
      else {
         
         if (!sunken && hasFocus) {
            painter->save();
            painter->setBrush(midColor(COLOR(Window), COLOR(Highlight),
                                       24-step, step));
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing);
            painter->drawEllipse(RECT);
            painter->restore();
         }
         
         // shadow
         sunken = sunken || isOn;
         painter->drawPixmap(sunken ? xy + QPoint(f1,f1) : xy,
                           shadows.radio[sunken][isEnabled]);
         
         // plate
         xy += QPoint(f2,f1);
         fillWithMask(painter, xy,
                     Gradients::brush( c, dpi.ExclusiveIndicator, Qt::Vertical,
                                       gt), masks.radio);
         
         if (isEnabled) {
            sz = dpi.ExclusiveIndicator - dpi.f4;
            painter->save();
            painter->setBrush(Qt::NoBrush);
            painter->setPen(Qt::white);
            painter->setRenderHint(QPainter::Antialiasing);
            painter->drawEllipse(xy.x(), xy.y(), sz, sz);
            painter->restore();
         }
      }
      // drop
      if (isOn) step = 18;
      if (step) {
         c = midColor(c, (hover && config.fullButtonHover) ? COLOR(ButtonText) :
               COLOR(WindowText), 18-step, step);
         xy += QPoint(dpi.f4, dpi.f4);
         fillWithMask(painter, xy, c, masks.radioIndicator);
      }
      
      break;
   }
   case PE_Q3DockWindowSeparator: // Item separator for Qt 3 compatible dock window and toolbar contents.
      break;
   case PE_Frame: { // Generic frame; see also QFrame.
      if (widget) {
         // handle the
         if (widget->inherits("QComboBoxPrivateContainer")) {
            SAVE_PEN;
            painter->setPen(COLOR(Base));
            painter->drawRect(RECT.adjusted(0,0,-1,-1));
            RESTORE_PEN;
            break;
         }
         
         bool niceFrame = false;
         QRect rect = RECT;
         if (sunken)
            rect.adjust(0,0,0,-dpi.f2);
         else if (option->state & State_Raised)
            rect.adjust(dpi.f2,dpi.f1,-dpi.f2,-dpi.f4);
         else
            rect.adjust(dpi.f2,dpi.f2,-dpi.f2,-dpi.f2);
         
         const Tile::Mask *mask = 0L; const Tile::Set *shadow = 0L;
         if (sunken) {
            shadow = &shadows.lineEdit[isEnabled];
            mask = &masks.button;
         }
         else if (option->state & State_Raised) {
            shadow = &shadows.tab[1][0];
            mask = &masks.tab;
         }
         QPoint zero; bool fastFocus = false;
         const QBrush *brush = &PAL.brush(widget->backgroundRole());
         if (qobject_cast<const QFrame*>(widget)) { // frame, can be killed unless...
            if (widget->inherits("QTextEdit")) { // ...it's a TextEdit!
               niceFrame = true; fastFocus = true;
               brush = &PAL.brush(QPalette::Base);
            }
            else { // maybe we need to corect a textlabels margin
               if (const QLabel* label = qobject_cast<const QLabel*>(widget))
                  if (label->text() != QString() && label->margin() < dpi.f3)
                     const_cast<QLabel*>(label)->setMargin(dpi.f3);
               break; // painted on visual frame
            }
         }
         else if (qobject_cast<const VisualFrame*>(widget)) {
            if (widget->parentWidget() && widget->parentWidget()->parentWidget())
               brush = &PAL.brush(widget->parentWidget()->parentWidget()->backgroundRole());
            niceFrame = true;
            zero = widget->mapTo(widget->window(), QPoint(0,0));
         }
         if (niceFrame) {
            if (fastFocus)
               mask->render(rect, painter, *brush, Tile::Full, false, zero);
            if (hasFocus) {
               QColor h;
               if (fastFocus)
                  h = midColor(COLOR(Base), COLOR(Highlight), 2, 1);
               else {
                  h = COLOR(Highlight); h.setAlpha(80);
               }
               rect = RECT.adjusted(0,0,0,-dpi.f2);
               mask->outline(rect, painter, h, config.strongFocus, Tile::Ring, dpi.f3);
            }
            if (shadow)
               shadow->render(RECT, painter);
            else { // plain frame
               //horizontal
               shadows.line[false][Sunken].render(RECT, painter, Tile::Full, false);
               shadows.line[false][Sunken].render(RECT, painter, Tile::Full, true);
               //vertical
               shadows.line[true][Sunken].render(RECT, painter, Tile::Full, false);
               shadows.line[true][Sunken].render(RECT, painter, Tile::Full, true);
               break;
            }
            break;
         }
      }
      // fallback, we cannot paint shaped frame contents
      if (sunken)
         shadows.sunken.render(RECT,painter);
      else if (option->state & State_Raised)
//          shadows.raised.render(RECT,painter);
         break;
      else {
         //horizontal
         shadows.line[false][Sunken].render(RECT, painter, Tile::Full, false);
         shadows.line[false][Sunken].render(RECT, painter, Tile::Full, true);
         //vertical
         shadows.line[true][Sunken].render(RECT, painter, Tile::Full, false);
         shadows.line[true][Sunken].render(RECT, painter, Tile::Full, true);
      }
      break;
   }
   case PE_FrameMenu: { // Frame for popup windows/menus; see also QMenu.
#if 0
      SAVE_PEN;
      QPalette::ColorRole role = QPalette::Window;
      if (widget)
         role = widget->inherits("QComboBox") ? // this is a combo popup
         QPalette::WindowText : widget->backgroundRole();
      QColor c = PAL.color(role);
      painter->setPen(c);
      painter->drawLine(RECT.x(), RECT.top(), RECT.right(), RECT.top());
      painter->setPen(c.dark(110));
      painter->drawLine(RECT.x(), RECT.top(), RECT.x(), RECT.bottom());
      painter->drawLine(RECT.x(), RECT.bottom(), RECT.right(), RECT.bottom());
      painter->drawLine(RECT.right(), RECT.top(), RECT.right(), RECT.bottom());
      RESTORE_PEN;
#endif
      if (config.menuShadow) {
         QRect rect = RECT.adjusted(0,0, // don't ask...
                                    shadows.line[true][Sunken].thickness()+1,
                                    shadows.line[false][Sunken].thickness()+1);
         //horizontal
         shadows.line[false][Sunken].render(rect, painter, Tile::Full, false);
         shadows.line[false][Sunken].render(rect, painter, Tile::Full, true);
         //vertical
         shadows.line[true][Sunken].render(rect, painter, Tile::Full, false);
         shadows.line[true][Sunken].render(rect, painter, Tile::Full, true);
      }
      break;
   }
   case PE_PanelMenuBar: // Panel for menu bars.
   case PE_FrameDockWidget: // Panel frame for dock windows and toolbars.
      break;
   case PE_FrameTabWidget: // Frame for tab widgets.
      if (const QStyleOptionTabWidgetFrame *twf =
          qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
             
//          Tile::PosFlags pf = Tile::Ring;
         QLine line[2];
         QStyleOptionTabBarBase tbb; tbb.initFrom(widget);
         tbb.shape = twf->shape; tbb.rect = twf->rect;
         
#define SET_BASE_HEIGHT(_o_) \
   baseHeight = twf->tabBarSize._o_(); \
   if (baseHeight < 0) \
      baseHeight = pixelMetric( PM_TabBarBaseHeight, option, widget )
                
         int baseHeight;
         switch (twf->shape) {
         case QTabBar::RoundedNorth: case QTabBar::TriangularNorth:
            SET_BASE_HEIGHT(height);
            tbb.rect.setHeight(baseHeight);
            line[0] = line[1] = QLine(RECT.bottomLeft(), RECT.bottomRight());
            line[0].translate(0,-1);
            break;
         case QTabBar::RoundedSouth: case QTabBar::TriangularSouth:
            SET_BASE_HEIGHT(height);
            tbb.rect.setTop(tbb.rect.bottom()-baseHeight);
            line[0] = line[1] = QLine(RECT.topLeft(), RECT.topRight());
            line[1].translate(0,1);
            break;
         case QTabBar::RoundedEast: case QTabBar::TriangularEast:
            SET_BASE_HEIGHT(width);
            tbb.rect.setLeft(tbb.rect.right()-baseHeight);
            line[0] = line[1] = QLine(RECT.topLeft(), RECT.bottomLeft());
            line[1].translate(1,0);
            break;
         case QTabBar::RoundedWest: case QTabBar::TriangularWest:
            SET_BASE_HEIGHT(width);
            tbb.rect.setWidth(baseHeight);
            line[0] = line[1] = QLine(RECT.topRight(), RECT.bottomRight());
            line[0].translate(-1,0);
            break;
         }
#undef SET_BASE_HEIGHT

         // the "frame"
         painter->save();
         painter->setPen(COLOR(Window).dark(120));
         painter->drawLine(line[0]);
         painter->setPen(COLOR(Window).light(114));
         painter->drawLine(line[1]);
         painter->restore();

         // the bar
         drawPrimitive ( PE_FrameTabBarBase, &tbb, painter, widget);
      }
      break;
   case PE_FrameLineEdit: { // Panel frame for line edits.
      QRect r = RECT.adjusted(0,0,0,-dpi.f2);
      shadows.lineEdit[isEnabled].render(r, painter);
      if (hasFocus) {
         QColor h = COLOR(Highlight); h.setAlpha(80);
//          r.setBottom(r.bottom()+dpi.f1);
         masks.button.outline(r, painter, h, config.strongFocus, Tile::Ring, dpi.f3);
      }
      break;
   }
   case PE_FrameGroupBox: { // Panel frame around group boxes.
      QRect rect = RECT.adjusted(dpi.f4,dpi.f2,-dpi.f4,0);
      rect.setHeight(qMin(2*dpi.f32, RECT.height()));
      fillWithMask(painter, rect, Gradients::light(rect.height()), &masks.button,
                   Tile::Full&~Tile::Bottom);
      rect.setBottom(RECT.bottom()-dpi.f32);
      shadows.group.render(RECT, painter, Tile::Ring);
      masks.button.outline(rect, painter, COLOR(Window).light(120), true,
                           Tile::Full&~Tile::Bottom);
      break;
   }
//    case PE_FrameButtonBevel: // Panel frame for a button bevel
//    case PE_FrameButtonTool: // Panel frame for a tool button
   case PE_FrameStatusBar: // Frame for a section of a status bar; see also QStatusBar.
      break;
   case PE_FrameWindow: // Frame around a MDI window or a docking window.
   {
      painter->save();
      painter->setPen(PAL.color(QPalette::Window).dark(110));
      painter->drawRect(RECT);
      painter->restore();
      break;
   }
   case PE_Q3Separator: // Qt 3 compatible generic separator.
      break;
   case PE_IndicatorViewItemCheck: // On/off indicator for a view item
   case PE_Q3CheckListIndicator: // Qt 3 compatible Checbox part of a list view item.
   case PE_IndicatorMenuCheckMark: { // Check mark used in a menu.
//       if (option->state & State_NoChange)
//          break;
      
      bool selected = false;
      QRect rect = RECT.adjusted(dpi.f2, dpi.f2, -dpi.f2, -dpi.f2);
      if (rect.width() > rect.height())
         rect.setWidth(rect.height());
      else
         rect.setHeight(rect.width());
      int off = rect.width()/4;
      
      painter->save();
      painter->setBrush(Qt::NoBrush);
      QColor fg;
      if (pe == PE_IndicatorMenuCheckMark) {
         selected = option->state & State_Selected && isEnabled;
         if (widget)
            fg = selected ? PAL.color(widget->backgroundRole()) :
            PAL.color(widget->foregroundRole());
         else
            fg = selected ? COLOR(Window) : COLOR(WindowText);
      }
      else
         fg = COLOR(Text);
      
      painter->setPen(fg);
      painter->drawRect(rect.adjusted(0, off, -off, 0));
      
      if (!(option->state & State_Off)) {
         painter->setRenderHint(QPainter::Antialiasing);
         if (pe != PE_IndicatorMenuCheckMark) {
            fg = midColor(COLOR(Highlight), COLOR(HighlightedText));
            painter->setPen(fg);
         }
         painter->setBrush(fg);
         const QPoint points[4] = {
            QPoint(rect.right(), rect.top()),
            QPoint(rect.x()+rect.width()/3, rect.bottom()),
            QPoint(rect.x(), rect.bottom()-rect.height()/3),
               QPoint(rect.x()+rect.width()/3, rect.bottom()-rect.height()/5)
         };
         painter->drawPolygon(points, 4);
      }
      painter->restore();
      break;
   }
   case PE_Q3CheckListExclusiveIndicator: // Qt 3 compatible Radio button part of a list view item.
      painter->save();
      painter->setRenderHint ( QPainter::Antialiasing );
      painter->drawEllipse ( RECT );
      _PRINTFLAGS_;
      if (option->state & State_On) {
         painter->setBrush ( painter->pen().color() );
         painter->drawEllipse ( RECT.adjusted(RECT.width()/4, RECT.height()/4, -RECT.width()/4, -RECT.height()/4) );
      }
      painter->restore();
      break;
//    case PE_IndicatorProgressChunk: // Section of a progress bar indicator; see also QProgressBar.
//    case PE_Q3CheckListController: // Qt 3 compatible Controller part of a list view item.
   case PE_IndicatorBranch: // Lines used to represent the branch of a tree in a tree view.
   {
      SAVE_PEN;
      int mid_h = RECT.x() + RECT.width() / 2;
      int mid_v = RECT.y() + RECT.height() / 2;
      int bef_h = mid_h;
      int bef_v = mid_v;
      int aft_h = mid_h;
      int aft_v = mid_v;
      
      painter->setPen(widget ?
                      midColor( PAL.color(widget->backgroundRole()), PAL.color(widget->foregroundRole())) :
                      midColor( PAL.color(QPalette::Base), PAL.color(QPalette::Text)) );
      static const int decoration_size = 9;
      if (option->state & State_Children) {
         int delta = decoration_size / 2 + 2;
         bef_h -= delta;
         bef_v -= delta;
         aft_h += delta;
         aft_v += delta;
         QStyleOption tmpOpt = *option;
         tmpOpt.rect = QRect(bef_h+2, bef_v+2, decoration_size, decoration_size);
         drawPrimitive(option->state & State_Open ? PE_IndicatorArrowDown :
                       option->direction == Qt::RightToLeft ? PE_IndicatorArrowLeft :
                       PE_IndicatorArrowRight, &tmpOpt, painter, widget);
      }
      if (RECT.x() ==  -1) { // this is for the first col and i don't see why we'd need a line here
         RESTORE_PEN;
         break;
      }
      
      if (option->state & (State_Item | State_Sibling))
         painter->drawLine(mid_h, RECT.y(), mid_h, bef_v);
      if (option->state & State_Sibling)
         painter->drawLine(mid_h, aft_v, mid_h, RECT.bottom());
      if (option->state & State_Item) {
         if (option->direction == Qt::RightToLeft)
            painter->drawLine(RECT.left(), mid_v, bef_h, mid_v);
         else
            painter->drawLine(aft_h, mid_v, RECT.right(), mid_v);
      }
      RESTORE_PEN;
      break;
   }
   case PE_IndicatorDockWidgetResizeHandle: // Resize handle for dock windows.
   {
      QPoint *points; int num;
      const int f12 = dpi.f12, f6 = dpi.f6;
      if (RECT.width() > RECT.height()) {
         int x = RECT.left()+RECT.width()/3;
         int y = RECT.top()+(RECT.height()-f6)/2;
         num = RECT.width()/(3*f12);
         if ((RECT.width()/3) % f12) ++num;
         points = new QPoint[num];
         for (int i = 0; i < num; ++i) {
            points[i] = QPoint(x,y); x += f12;
         }
      }
      else {
         int x = RECT.left()+(RECT.width()-f6)/2;
         int y = RECT.top()+RECT.height()/3;
         num = RECT.height()/(3*f12);
         if ((RECT.height()/3) % f12) ++num;
         points = new QPoint[num];
         for (int i = 0; i < num; ++i) {
            points[i] = QPoint(x,y); y += f12;
         }
      }
      painter->save();
      painter->setPen(Qt::NoPen);
      const QPixmap *fill; int cnt = num/2, imp = 1;
      QPalette::ColorRole role = QPalette::WindowText;
      if (hover) {
         role = QPalette::Highlight;
         imp = 8;
      }
      if (num%2)
      {
         fill = &Gradients::pix(midColor(COLOR(Window), PAL.color(role), 3, imp), f6, Qt::Vertical, Gradients::Sunken);
         fillWithMask(painter, points[cnt], *fill, masks.notch);
      }
      --num;
      for (int i = 0; i < cnt; ++i)
      {
         fill = &Gradients::pix(midColor(COLOR(Window), PAL.color(role), 3+cnt-i, imp), f6, Qt::Vertical, Gradients::Sunken);
         fillWithMask(painter, points[i], *fill, masks.notch);
         fillWithMask(painter, points[num-i], *fill, masks.notch);
      }
      painter->restore();
      delete[] points;
      break;
   }
   case PE_IndicatorToolBarHandle: // The handle of a toolbar.
      if (!(widget && widget->parentWidget()) ||
          widget->parentWidget()->underMouse()) {
         painter->save();
         QRect rect = RECT; bool line = false; int dx(0), dy(0);
         if (RECT.width() > RECT.height()) {
            line = (RECT.width() > 9*RECT.height()/2);
            if (line) {
               dx = 3*RECT.height()/2; dy = 0;
            }
            rect.setLeft(rect.left()+(rect.width()-rect.height())/2);
            rect.setWidth(rect.height());
         }
         else {
            line = (RECT.height() > 3*RECT.width());
            if (line) {
               dx = 0; dy = 3*RECT.width()/2;
            }
            rect.setTop(rect.top()+(rect.height()-rect.width())/2);
            rect.setHeight(rect.width());
         }
         QColor c = hover ? COLOR(Highlight) : COLOR(Window).dark(110);
         painter->setRenderHint(QPainter::Antialiasing);
         painter->setBrush(Gradients::pix(c, rect.height(), Qt::Vertical, Gradients::Sunken));
         painter->setPen(Qt::NoPen);
         painter->setBrushOrigin(rect.topLeft());
         painter->drawEllipse(rect);
         if (line) {
            const int f1 = dpi.f1;
            rect.adjust(f1,f1,-f1,-f1);
            painter->setBrush(Gradients::pix(c, rect.height(), Qt::Vertical, Gradients::Sunken));
            rect.translate(-dx,-dy);
            painter->setBrushOrigin(rect.topLeft());
            painter->drawEllipse(rect);
            rect.translate( 2*dx, 2*dy);
            painter->setBrushOrigin(rect.topLeft());
            painter->drawEllipse(rect);
         }
         painter->restore();
      }
      break;
   case PE_IndicatorToolBarSeparator: // The separator in a toolbar.
      break;
   case PE_PanelToolBar: // The panel for a toolbar.
      break;
//    case PE_PanelTipLabel: // The panel for a tip label.
   case PE_FrameTabBarBase:
      if (widget &&
          (qobject_cast<const QTabBar*>(widget) || // we alter the paintevent
          (widget->parentWidget() &&
           qobject_cast<QTabWidget*>(widget->parentWidget())))) // KDE abuse, allready has a nice base
         break;
      
      if (const QStyleOptionTabBarBase *tbb
            = qstyleoption_cast<const QStyleOptionTabBarBase *>(option)) {
         QRect rect = RECT.adjusted(dpi.f2, 0, -dpi.f2, -dpi.f2);
         int size = RECT.height(); Qt::Orientation o = Qt::Vertical;

         switch (tbb->shape) {
         case QTabBar::RoundedSouth: case QTabBar::TriangularSouth:
         case QTabBar::RoundedNorth: case QTabBar::TriangularNorth:
            break;
         case QTabBar::RoundedWest: case QTabBar::TriangularWest:
         case QTabBar::RoundedEast: case QTabBar::TriangularEast:
            o = Qt::Horizontal; size = RECT.width();
            break;
         }
         
         masks.tab.render(rect, painter, Gradients::brush(
                          midColor(CONF_COLOR(tab[0][0]), COLOR(Window), 2, 1),
         size, o, config.gradTab));
         rect.setBottom(rect.bottom()+dpi.f2);
         shadows.tabSunken.render(rect, painter);
      }
      break;
   case PE_IndicatorTabTear: // An indicator that a tab is partially scrolled out of the visible tab bar when there are many tabs.
      break;
   default:
      QCommonStyle::drawPrimitive( pe, option, painter, widget );
   } // switch
}
