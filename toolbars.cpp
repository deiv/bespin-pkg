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

#include "draw.h"

void
BespinStyle::drawToolButton(const QStyleOptionComplex * option,
                            QPainter * painter, const QWidget * widget) const
{
   B_STATES
      
   // special handling for the tabbar scrollers ------------------------------
   if (widget && widget->parentWidget() &&
      qobject_cast<QTabBar*>(widget->parent()))
   {
      QColor c = widget->parentWidget()->palette().color(config.tab.std_role[0]);
      QColor c2 = widget->parentWidget()->palette().color(config.tab.std_role[1]);
      if (sunken) {
         int dy = (RECT.height()-RECT.width())/2;
         QRect r = RECT.adjusted(dpi.f2,dy,-dpi.f2,-dy);
         painter->drawTiledPixmap(r, Gradients::pix(c, r.height(), Qt::Vertical, Gradients::Sunken));
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

   hover = isEnabled && (bflags & (State_Sunken | State_On |
                                 State_Raised | State_HasFocus));

   QStyleOption tool(0); tool.palette = toolbutton->palette;

   // frame around whole button
   /*if (hover)*/ {
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
   B_STATES
      
   if (!isEnabled)
      return;

   bool isOn = option->state & State_On;
   int step = animator->hoverStep(widget);
   const QColor &c = Colors::bg(PAL, widget);
   if (isOn && (!hover || step < 6))
      masks.tab.render(RECT, painter, Gradients::Sunken, Qt::Vertical, c);
   if (hover || step || sunken) {
      QRect r = RECT;
      if (!sunken && step) {
         step = 6 - step;
         int dx = step*r.width()/18; int dy = step*r.height()/18;
         r.adjust(dx, dy, -dx, -dy);
      }
      masks.tab.render(r, painter, sunken ? Gradients::Sunken : Gradients::Button,
                       Qt::Vertical, c);
   }
   if (isOn)
      shadows.tabSunken.render(RECT, painter);
}

void
BespinStyle::drawToolButtonLabel(const QStyleOption * option,
                                 QPainter * painter, const QWidget *) const
{
   const QStyleOptionToolButton *toolbutton
      = qstyleoption_cast<const QStyleOptionToolButton *>(option);
   if (!toolbutton) return;

   OPT_ENABLED
      
   // Arrow type always overrules and is always shown
   bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
   if ((!hasArrow && toolbutton->icon.isNull()) &&
         !toolbutton->text.isEmpty() ||
         toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {
      drawItemText(painter, RECT, Qt::AlignCenter | Qt::TextShowMnemonic, PAL,
                   isEnabled, toolbutton->text, QPalette::WindowText);
      return;
   }

   OPT_HOVER
      
   QPixmap pm;
   QSize pmSize = toolbutton->iconSize;
   if (!toolbutton->icon.isNull()) {
      QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
      QIcon::Mode mode;
      if (!isEnabled)
         mode = QIcon::Disabled;
      else if (hover && (option->state & State_AutoRaise))
         mode = QIcon::Active;
      else
         mode = QIcon::Normal;
      pm = toolbutton->icon.pixmap(RECT.size().boundedTo(toolbutton->iconSize),
                                   mode, state);
      pmSize = pm.size();
   }

   if (toolbutton->toolButtonStyle != Qt::ToolButtonIconOnly) {
      painter->setFont(toolbutton->font);
      QRect pr = RECT, tr = RECT;
      int alignment = Qt::TextShowMnemonic;

      if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
         int fh = painter->fontMetrics().height();
         pr.adjust(0, dpi.f3, 0, -fh - dpi.f5);
         tr.adjust(0, pr.bottom(), 0, -dpi.f3);
         if (!hasArrow)
            drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
         else
            drawSolidArrow(Navi::S, pr, painter);
         alignment |= Qt::AlignCenter;
      }
      else {
         pr.setWidth(pmSize.width() + dpi.f8);
         tr.adjust(pr.right(), 0, 0, 0);
         if (!hasArrow)
            drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
         else
            drawSolidArrow(Navi::S, pr, painter);
         alignment |= Qt::AlignLeft | Qt::AlignVCenter;
      }
      drawItemText(painter, tr, alignment, PAL, isEnabled, toolbutton->text, QPalette::WindowText);
      return;
   }

   if (hasArrow) {
      const int f5 = dpi.f5;
      drawSolidArrow(Navi::S, RECT.adjusted(f5,f5,-f5,-f5), painter);
   }
   else
      drawItemPixmap(painter, RECT, Qt::AlignCenter, pm);
}

void
BespinStyle::drawToolBarHandle(const QStyleOption * option, QPainter * painter,
                               const QWidget * widget) const
{
   if (!(widget && widget->parentWidget()) ||
       widget->parentWidget()->underMouse())
      return; // toolbar is not hovered

   OPT_HOVER
   
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
   QColor c = hover ? FCOLOR(Highlight) : FCOLOR(Window).dark(110);
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
