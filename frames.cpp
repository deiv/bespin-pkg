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

#include <QLabel>
#include <QTableView>
#include <QTreeView>
#include "visualframe.h"
#include "draw.h"

bool
BespinStyle::isSpecialFrame(const QWidget *w)
{
   return
      w->inherits("QTextEdit") ||
      (w->parentWidget() && w->parentWidget()->inherits("KateView"));
}

void
BespinStyle::drawFocusFrame(const QStyleOption * option, QPainter * painter,
                            const QWidget *) const
{
   painter->save();
   painter->setBrush(Qt::NoBrush);
   painter->setPen(FCOLOR(Highlight));
   painter->drawLine(RECT.bottomLeft(), RECT.bottomRight());
   painter->restore();
}

void
BespinStyle::drawFrame(const QStyleOption * option, QPainter * painter,
                       const QWidget * widget) const
{
   B_STATES
      
   if (!widget) { // fallback, we cannot paint shaped frame contents
      if (sunken)
         shadows.sunken.render(RECT,painter);
      else if (option->state & State_Raised) //TODO!
//          shadows.raised.render(RECT,painter);
         return;
      else {
         //horizontal
         shadows.line[false][Sunken].render(RECT, painter, Tile::Full, false);
         shadows.line[false][Sunken].render(RECT, painter, Tile::Full, true);
         //vertical
         shadows.line[true][Sunken].render(RECT, painter, Tile::Full, false);
         shadows.line[true][Sunken].render(RECT, painter, Tile::Full, true);
      }
      return;
   }

   bool fastFocus = false;
   const QBrush *brush = 0;
   if (qobject_cast<const QFrame*>(widget)) { // frame, can be killed unless...
      if (isSpecialFrame(widget)) { // ...TextEdit, KateView, ...
         fastFocus = true;
         brush = &PAL.brush(QPalette::Base);
      }
      else { // maybe we need to corect a textlabels margin
         if (const QLabel* label = qobject_cast<const QLabel*>(widget))
            if (label->text() != QString() && label->margin() < dpi.f3)
               const_cast<QLabel*>(label)->setMargin(dpi.f3);
         return; // painted on visual frame
      }
   }
   else if (widget->inherits("QComboBoxPrivateContainer")) {
      // comob dropdowns
      SAVE_PEN;
      painter->setPen(FCOLOR(Base));
      painter->drawRect(RECT.adjusted(0,0,-1,-1));
      RESTORE_PEN;
      return;
   }

   QRect rect = RECT;
   if (sunken)
      rect.adjust(0,0,0,-dpi.f2);
   else if (option->state & State_Raised)
      rect.adjust(dpi.f2,dpi.f1,-dpi.f2,-dpi.f4);
   else
      rect.adjust(dpi.f2,dpi.f2,-dpi.f2,-dpi.f2);
   
   const Tile::Set *mask = 0L, *shadow = 0L;
   if (sunken) {
      shadow = &shadows.lineEdit[isEnabled];
      mask = &masks.button;
   }
   else if (option->state & State_Raised) {
      shadow = &shadows.group;
      mask = &masks.button;
   }
   
   if (brush)
      mask->render(rect, painter, *brush);
   if (hasFocus) {
      QColor h;
//       if (fastFocus)
//          h = Colors::mid(FCOLOR(Base), FCOLOR(Highlight), 3, 2);
//       else {
         h = FCOLOR(Highlight); h.setAlpha(102);
//       }
      if (const VisualFramePart* vfp =
          qobject_cast<const VisualFramePart*>(widget)) {
         Tile::setShape(Tile::Ring);
         QWidget *vHeader = 0, *hHeader = 0;
         if (const QTreeView* tv =
             qobject_cast<const QTreeView*>(vfp->frame()))
            hHeader = (QWidget*)tv->header();
         else if (const QTableView* table =
                  qobject_cast<const QTableView*>(vfp->frame())) {
            hHeader = (QWidget*)table->horizontalHeader();
            vHeader = (QWidget*)table->verticalHeader();
         }
         if (vHeader && vHeader->isVisible()) {
            Tile::setShape(Tile::shape() & ~Tile::Left);
            rect.setLeft(rect.left()+vHeader->width());
         }
         if (hHeader && hHeader->isVisible()) {
            Tile::setShape(Tile::shape() & ~Tile::Top);
            rect.setTop(rect.top()+hHeader->height());
         }
      }
      mask->outline(rect, painter, h, dpi.f3);
      Tile::reset();
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
   }
}

void
BespinStyle::drawGroupBox(const QStyleOptionComplex * option,
                          QPainter * painter, const QWidget * widget) const
{
   ASSURE_OPTION(groupBox, GroupBox);
   OPT_ENABLED
      
   // Frame
   if (groupBox->subControls & QStyle::SC_GroupBoxFrame) {
      QStyleOptionFrameV2 frame;
      frame.QStyleOption::operator=(*groupBox);
      frame.features = groupBox->features;
      frame.lineWidth = groupBox->lineWidth;
      frame.midLineWidth = groupBox->midLineWidth;
      frame.rect = subControlRect(CC_GroupBox, option, SC_GroupBoxFrame, widget);
      drawGroupBoxFrame(&frame, painter, widget);
   }

   // Title
   if ((groupBox->subControls & QStyle::SC_GroupBoxLabel) &&
      !groupBox->text.isEmpty()) {
      QColor textColor = groupBox->textColor;
      if (textColor.isValid()) painter->setPen(textColor);
      QFont tmpfnt = painter->font(); tmpfnt.setBold(true);
      painter->setFont ( tmpfnt );
      QStyleOptionGroupBox copy = *groupBox; copy.fontMetrics = QFontMetrics(tmpfnt);
      QRect textRect = subControlRect(CC_GroupBox, &copy, SC_GroupBoxLabel, widget);
      int alignment = Qt::AlignCenter; //int(groupBox->textAlignment);
      if (!styleHint(QStyle::SH_UnderlineShortcut, option, widget))
         alignment |= Qt::TextHideMnemonic;
      else
         alignment |= Qt::TextShowMnemonic;

      drawItemText(painter, textRect,  alignment, groupBox->palette, isEnabled, groupBox->text,
                  textColor.isValid() ? QPalette::NoRole : QPalette::Foreground);
      int x = textRect.bottom(); textRect = RECT; textRect.setTop(x);
      x = textRect.width()/4; textRect.adjust(x,0,-x,0);
      shadows.line[0][Sunken].render(textRect, painter);
   }
       
   // Checkbox
   // TODO: sth better - maybe a round thing in the upper left corner...?
   // also doesn't hover - yet.
   if (groupBox->subControls & SC_GroupBoxCheckBox) {
      QStyleOptionButton box;
      box.QStyleOption::operator=(*groupBox);
      box.rect = subControlRect(CC_GroupBox, option, SC_GroupBoxCheckBox, widget);
      drawCheckBox(&box, painter, widget);
   }
}

void
BespinStyle::drawGroupBoxFrame(const QStyleOption * option, QPainter * painter,
                               const QWidget *) const
{
   QRect rect = RECT.adjusted(dpi.f4,dpi.f2,-dpi.f4,0);
   rect.setHeight(qMin(2*dpi.f32, RECT.height()));
   Tile::setShape(Tile::Full & ~Tile::Bottom);
   masks.button.render(rect, painter, Gradients::light(rect.height()));
   rect.setBottom(RECT.bottom()-dpi.f32);
   Tile::setShape(Tile::Full);
   shadows.group.render(RECT, painter);
//       Tile::setShape(Tile::Full & ~Tile::Bottom);
//       masks.button.outline(rect, painter, FCOLOR(Window).light(120));
   Tile::reset();
}
