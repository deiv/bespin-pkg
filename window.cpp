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
#include <QtDebug>
#include "draw.h"

#ifdef Q_WS_X11
#include "xproperty.h"
#else
#define QT_NO_XRENDER #
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

static QPainterPath glasPath;
static QSize glasSize;

void
BespinStyle::drawWindowBg(const QStyleOption * option, QPainter * painter,
                          const QWidget * widget) const
{
    // cause of scrollbars - kinda optimization
    if (config.bg.mode < BevelV) return;


    if (!(widget && widget->isWindow()))
        return; // can't do anything here
    if (PAL.brush(widget->backgroundRole()).style() > 1)
        return; // we'd cover a gradient/pixmap/whatever

    // glassy Modal dialog/Popup menu ==========
    const QColor &c = PAL.color(widget->backgroundRole());
    if (widget->testAttribute(Qt::WA_MacBrushedMetal))
    {   // we just kinda abuse this mac only attribute... ;P
        if (widget->size() != glasSize)
        {
            const QRect &wr = widget->rect();
            glasSize = widget->size();
            glasPath = QPainterPath();
            glasPath.moveTo(wr.topLeft());
            glasPath.lineTo(wr.topRight());
            glasPath.quadTo(wr.center()/2, wr.bottomLeft());
        }
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(c.light(115-Colors::value(c)/20));
        painter->translate(RECT.topLeft());
        painter->drawPath(glasPath);
        painter->restore();
        return;
    }
    // ===================
    const BgSet &set = Gradients::bgSet(c); QRect rect = RECT;

#ifndef QT_NO_XRENDER
    uint decoDim = 0;
    XProperty::get(widget->winId(), XProperty::decoDim, decoDim);
    if (decoDim)
    {
        Qt::HANDLE picture = set.topTile.x11PictureHandle();
        XProperty::set(widget->winId(), XProperty::topTile, picture);
        picture = set.btmTile.x11PictureHandle();
        XProperty::set(widget->winId(), XProperty::btmTile, picture);
        picture = set.cornerTile.x11PictureHandle();
        XProperty::set(widget->winId(), XProperty::cnrTile, picture);
        picture = set.lCorner.x11PictureHandle();
        XProperty::set(widget->winId(), XProperty::lCorner, picture);
        picture = set.rCorner.x11PictureHandle();
        XProperty::set(widget->winId(), XProperty::rCorner, picture);
        rect.adjust(-((decoDim >> 24) & 0xff), -((decoDim >> 16) & 0xff), (decoDim >> 8) & 0xff, decoDim & 0xff);
    }
#endif

    switch (config.bg.mode)
    {
    case BevelV:
    {   // also fallback for ComplexLights
        int s1 = set.topTile.height();
        int s2 = qMin(s1, (rect.height()+1)/2);
        s1 -= s2;
        painter->drawTiledPixmap( rect.x(), rect.y(), rect.width(), s2, set.topTile, 0, s1 );
        if (Colors::value(c) < 245)
        {   // no sense otherwise
            const int w = rect.width()/4 - 128;
            if (w > 0)
            {
                s2 = 128-s1;
                painter->drawTiledPixmap( rect.x(), rect.y(), w, s2, set.cornerTile, 0, s1 );
                painter->drawTiledPixmap( rect.right()+1-w, rect.y(), w, s2, set.cornerTile, 0, s1 );
            }
            painter->drawPixmap(rect.x()+w, rect.y(), set.lCorner, 0, s1, 128, s2);
            painter->drawPixmap(rect.right()-w-127, rect.y(), set.rCorner, 0, s1, 128, s2);
        }
        s1 = set.btmTile.height();
        s2 = qMin(s1, (rect.height())/2);
        painter->drawTiledPixmap( rect.x(), rect.bottom() - s2, rect.width(), s2, set.btmTile );
        break;
    }
    case BevelH:
    {
        int s1 = set.topTile.width();
        int s2 = qMin(s1, (rect.width()+1)/2);
        const int h = qMin(128+32, rect.height()/8);
        const int y = rect.y()+h;
        painter->drawTiledPixmap( rect.x(), y, s2, rect.height()-h, set.topTile, s1-s2, 0 );
        painter->drawPixmap(rect.x(), y-32, set.lCorner, s1-s2, 0,0,0);
        s1 = set.btmTile.width();
        s2 = qMin(s1, (rect.width())/2);
        painter->drawTiledPixmap( rect.right() - s2, y , s2, rect.height()-h, set.btmTile );
        painter->drawPixmap(rect.right() - s2, y-32, set.rCorner);
        painter->drawTiledPixmap( rect.x(), y-(128+32), rect.width(), 128, set.cornerTile );
        break;
    }
//    case Plain: // should not happen anyway...
//    case Scanlines: // --"--
    default:
        break;
    }
}

void
BespinStyle::drawToolTip(const QStyleOption * option, QPainter * painter, const QWidget *) const
{
   painter->save();
   
#if QT_VERSION < 0x040400
#define ToolTipBase WindowText
#define ToolTipText Window
#endif

//    painter->setBrush(Gradients::pix(FCOLOR(ToolTipBase), RECT.height(), Qt::Vertical, Gradients::Button));
   painter->setBrush(FCOLOR(ToolTipBase));
//    painter->setPen(Qt::NoPen);
//    painter->drawRect(RECT);
   const int f1 = dpi.f1;
//    QPen pen(Colors::mid(FCOLOR(ToolTipBase), FCOLOR(ToolTipText),6,1), f1);
   QPen pen(FCOLOR(ToolTipText), f1);
   painter->setPen(pen);
   painter->drawRect(RECT.adjusted(f1/2,f1/2,-f1,-f1));

#if QT_VERSION < 0x040400
#undef ToolTipBase
#undef ToolTipText
#endif

   painter->restore();
}

#define PAINT_WINDOW_BUTTON(_btn_) {\
   tmpOpt.rect =\
      subControlRect(CC_TitleBar, tb, SC_TitleBar##_btn_##Button, widget);\
   if (!tmpOpt.rect.isNull()) { \
      if (tb->activeSubControls & SC_TitleBar##_btn_##Button)\
         tmpOpt.state = tb->state;\
      else\
         tmpOpt.state &= ~(State_Sunken | State_MouseOver);\
      painter->drawPixmap(tmpOpt.rect.topLeft(), standardPixmap(SP_TitleBar##_btn_##Button, &tmpOpt, widget));\
   }\
}

void
BespinStyle::drawTitleBar(const QStyleOptionComplex * option,
                          QPainter * painter, const QWidget * widget) const
{
   const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(option);
   if (!tb) return;

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
BespinStyle::drawSizeGrip(const QStyleOption * option, QPainter * painter, const QWidget *) const
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
