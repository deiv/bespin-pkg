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

#include <QAbstractButton>
#include "oxrender.h"
#include "draw.h"
#include "animator/hover.h"

static int step;

void
BespinStyle::drawToolButton(const QStyleOptionComplex * option,
                            QPainter * painter, const QWidget * widget) const
{
   B_STATES
      
   // special handling for the tabbar scrollers ------------------------------
   if (widget && widget->parentWidget() &&
      qobject_cast<QTabBar*>(widget->parent())) {
      QColor c = widget->parentWidget()->palette().color(config.tab.std_role[0]);
      QColor c2 = widget->parentWidget()->palette().color(config.tab.std_role[1]);
      if (sunken) {
         int dy = (RECT.height()-RECT.width())/2;
         QRect r = RECT.adjusted(dpi.f2,dy,-dpi.f2,-dy);
         masks.rect[true].render(r, painter, Gradients::Sunken, Qt::Vertical, c);
      }
      painter->save();
      painter->setPen( isEnabled ? c2 : Colors::mid(c, c2) );
      drawToolButtonLabel(option, painter, widget);
      painter->restore();
      return;
   } // --------------------------------------------------------------------

   const QStyleOptionToolButton *toolbutton
      = qstyleoption_cast<const QStyleOptionToolButton *>(option);
   if (!toolbutton) return;


   QRect menuarea = subControlRect(CC_ToolButton, toolbutton, SC_ToolButtonMenu, widget);
   QRect button = subControlRect(CC_ToolButton, toolbutton, SC_ToolButton, widget);
   State bflags = toolbutton->state;

   if ((bflags & State_AutoRaise) && !hover)
      bflags &= ~State_Raised;

   State mflags = bflags;

   if (toolbutton->activeSubControls & SC_ToolButton)
      bflags |= State_Sunken;

   hover = isEnabled && (bflags & (State_Sunken | State_On | State_Raised | State_HasFocus));

   QStyleOption tool(0); tool.palette = toolbutton->palette;

   step = Animator::Hover::step(widget);

   // frame around whole button
   if (option->state & State_On)
   {
      tool.rect = RECT; tool.state = bflags;
      drawToolButtonShape(&tool, painter, widget);
   }

   // don't paint a dropdown arrow iff the button's really pressed
   if (!(bflags & State_Sunken) &&
      (toolbutton->subControls & SC_ToolButtonMenu)) {
         if (toolbutton->activeSubControls & SC_ToolButtonMenu)
            painter->drawTiledPixmap(menuarea, Gradients::pix(FCOLOR(Window),
               menuarea.height(), Qt::Vertical,
               Gradients::Sunken));
         QPen oldPen = painter->pen();
         painter->setPen(Colors::mid(FCOLOR(Window), FCOLOR(WindowText), 2, 1));
//          tool.rect = menuarea; tool.state = mflags;
         drawSolidArrow(Navi::S, menuarea, painter);
         painter->setPen(oldPen);
         if (hover) {
            menuarea.setLeft(button.right()-shadows.line[1][Sunken].thickness()/2);
            shadows.line[1][Sunken].render(menuarea, painter);
         }
      }

   // label in the toolbutton area
   QStyleOptionToolButton label = *toolbutton;
   label.rect = button;
   drawToolButtonLabel(&label, painter, widget);
}

void
BespinStyle::drawToolButtonShape(const QStyleOption * option,
                                 QPainter * painter, const QWidget * widget) const
{
   OPT_ENABLED; OPT_SUNKEN;
      
   if (!isEnabled)
      return;
   
   const bool isOn = option->state & State_On;
   const QColor &c = Colors::bg(PAL, widget);
   if (isOn) {
      masks.rect[true].render(RECT, painter, Gradients::Sunken, Qt::Vertical, c);
//    if (step || sunken) {
//       QRect r = RECT;
//       if (!sunken) {
//          step = 6 - step;
//          const int dx = step*r.width()/20, dy = step*r.height()/20;
//          r.adjust(dx, dy, -dx, -dy);
//          step = 6 - step;
//       }
//       const Gradients::Type gt = sunken ? Gradients::Sunken :
//          (Colors::value(c) < 108 ? Gradients::Simple : Gradients::Button);
//       masks.rect[true].render(r, painter, gt, Qt::Vertical, c);
//    }
//    if (isOn)
      shadows.sunken[true][true].render(RECT, painter);
   }
}

static QPixmap scaledIcon, emptyIcon;
qint64 lastIconPix = 0;
static QPixmap &
icon(QPixmap &pix, int step)
{
   if (pix.cacheKey() != lastIconPix) {
      scaledIcon = pix.scaledToHeight ( pix.height() + dpi.f4, Qt::SmoothTransformation );
      if (emptyIcon.size() != scaledIcon.size())
         emptyIcon = QPixmap(scaledIcon.size());
      lastIconPix = pix.cacheKey();
   }
#ifndef QT_NO_XRENDER
   emptyIcon.fill(Qt::transparent);
   OXRender::composite(pix, NULL, emptyIcon,
                       0, 0, 0, 0, dpi.f2, dpi.f2,
                       pix.width(), pix.height(), PictOpOver);
   OXRender::blend(scaledIcon, emptyIcon, step/6.0);
   return emptyIcon;
#else
   return step ? scaledIcon : pix;
#endif
}

void
BespinStyle::drawToolButtonLabel(const QStyleOption * option,
                                 QPainter * painter, const QWidget *widget) const
{
   const QStyleOptionToolButton *toolbutton
      = qstyleoption_cast<const QStyleOptionToolButton *>(option);
   if (!toolbutton) return;
   OPT_ENABLED

   // Arrow type always overrules and is always shown
   const bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
   const bool justText = (!hasArrow && toolbutton->icon.isNull()) &&
                         !toolbutton->text.isEmpty() ||
                         toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly;

   OPT_SUNKEN
      
   if (justText) { // the most simple way
      painter->setPen(Colors::mid(FCOLOR(WindowText), FCOLOR(Highlight), 6-step, step));
      QFont fnt = toolbutton->font;
      if (sunken) fnt.setBold(true);
      painter->setFont(fnt);
      drawItemText(painter, RECT, Qt::AlignCenter | Qt::TextShowMnemonic, PAL, isEnabled, toolbutton->text);
      return;
   }

   OPT_HOVER
      
   QPixmap pm;
   QSize pmSize = RECT.size() - QSize(dpi.f4, dpi.f4);
   pmSize = pmSize.boundedTo(toolbutton->iconSize);

   if (!toolbutton->icon.isNull()) {
      const QIcon::State state =
         toolbutton->state & State_On ? QIcon::On : QIcon::Off;
      QIcon::Mode mode;
      if (!isEnabled)
         mode = QIcon::Disabled;
//       else if (hover && (option->state & State_AutoRaise))
//          mode = QIcon::Active; // gamma thing looks dumb and i cannot turn it off in kde...
      else
         mode = QIcon::Normal;

      pm = toolbutton->icon.pixmap(RECT.size().boundedTo(pmSize), mode, state);
		if (step && !sunken && !pm.isNull())
#ifndef QT_NO_XRENDER
         pm = icon(pm, step);
#else
         pm = icon(pm, hover);
#endif
      pmSize = pm.size();
   }

   if (!(toolbutton->text.isEmpty() ||
         toolbutton->toolButtonStyle == Qt::ToolButtonIconOnly)) {
      QColor c = FCOLOR(WindowText);
      if (pm.isNull())
         c = Colors::mid(c, FCOLOR(Highlight), 6-step, step);
      painter->setPen(c);
            
//       QFont fnt = toolbutton->font;
//       if (hover) fnt.setUnderline(true);
      painter->setFont(toolbutton->font);
      
      QRect pr = RECT, tr = RECT;
      int alignment = Qt::TextShowMnemonic;

      if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
         int fh = painter->fontMetrics().height();
         pr.adjust(0, 0, 0, -fh - dpi.f2);
         tr.adjust(0, pr.bottom(), 0, -dpi.f3);
         if (!hasArrow)
            drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
         else
            drawSolidArrow(Navi::S, pr, painter);
         alignment |= Qt::AlignCenter;
      }
      else {
         pr.setWidth(toolbutton->iconSize.width() + dpi.f4);
         
         if (!hasArrow)
            drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
         else
            drawSolidArrow(Navi::S, pr, painter);

         tr.adjust(pr.width() + dpi.f4, 0, 0, 0);
         alignment |= Qt::AlignLeft | Qt::AlignVCenter;
      }
      drawItemText(painter, tr, alignment, PAL, isEnabled, toolbutton->text);
      return;
   }

   if (hasArrow) {
      const int f5 = dpi.f5;
      drawSolidArrow(Navi::Direction(toolbutton->arrowType),
                     RECT.adjusted(f5,f5,-f5,-f5), painter);
   }
   else
      drawItemPixmap(painter, RECT, Qt::AlignCenter, pm);
}

void
BespinStyle::drawToolBarHandle(const QStyleOption * option, QPainter * painter,
                               const QWidget * widget) const
{

   OPT_HOVER
   if (!hover) return;
   
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
   QColor c = FCOLOR(Window).dark(110);
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
