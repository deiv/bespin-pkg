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

#include <QApplication>
#include <QComboBox>
#include "draw.h"
#include "animator/hover.h"

void
BespinStyle::drawLineEditFrame(const QStyleOption * option, QPainter * painter,
                               const QWidget *) const
{
   OPT_ENABLED OPT_FOCUS
      
   QRect r = RECT.adjusted(0,0,0,-dpi.f2);
   if (hasFocus) {
      QColor h = FCOLOR(Highlight); h.setAlpha(128);
      masks.rect[false].outline(r, painter, h, dpi.f3);
   }
   shadows.sunken[false][isEnabled].render(r, painter);
}

void
BespinStyle::drawLineEdit(const QStyleOption * option, QPainter * painter,
                          const QWidget * widget) const
{

   // spinboxes and combos allready have a lineedit as global frame
   // TODO: exclude Q3Combo??
   if (qstyleoption_cast<const QStyleOptionFrame *>(option) &&
       static_cast<const QStyleOptionFrame *>(option)->lineWidth < 1) {
      if (widget && widget->parentWidget() &&
          ( qobject_cast<QComboBox*>(widget->parentWidget()) ||
            widget->parentWidget()->inherits("QAbstractSpinBox")))
         return;
      painter->fillRect(RECT, FCOLOR(Base));
      return;
   }

   OPT_ENABLED OPT_FOCUS
      
   isEnabled = isEnabled && !(option->state & State_ReadOnly);
   QRect r = RECT;
   if (isEnabled) {
      const Tile::Set &mask = masks.rect[false];
      if (hasFocus) {
         r.adjust(0,0,0,-dpi.f2);
         mask.render(r, painter, FCOLOR(Base).light(112));
         r.setBottom(r.bottom()+dpi.f1);
         QColor h = FCOLOR(Highlight); h.setAlpha(102);
//          Colors::mid(FCOLOR(Base), FCOLOR(Highlight), 3, 2);
         mask.outline(r, painter, h, dpi.f3);
      }
      else {
         r.setBottom(r.y()+r.height()/2);
         Tile::setShape(Tile::Full & ~Tile::Bottom);
         mask.render(r, painter, Gradients::Sunken,
                             Qt::Vertical, FCOLOR(Base));
         r.setTop(r.bottom()+1); r.setBottom(RECT.bottom()-dpi.f2);
         Tile::setShape(Tile::Full & ~Tile::Top);
         mask.render(r, painter, FCOLOR(Base).light(112));
         Tile::reset();
      }
   }
   shadows.sunken[false][isEnabled].render(RECT, painter);
}

void
BespinStyle::drawSpinBox(const QStyleOptionComplex * option, QPainter * painter,
                         const QWidget * widget) const
{
   ASSURE_OPTION(sb, SpinBox);
   B_STATES

   QStyleOptionSpinBox copy = *sb;
   // this doesn't work (for the moment, i assume...)
//    isEnabled = isEnabled && !(option->state & State_ReadOnly);
   if (isEnabled)
   if (widget)
   if (const QAbstractSpinBox *box =
         qobject_cast<const QAbstractSpinBox*>(widget)) {
      isEnabled = isEnabled && !box->isReadOnly();
      if (!isEnabled)
         copy.state &= ~State_Enabled;
   }

   if (sb->frame && (sb->subControls & SC_SpinBoxFrame))
      drawLineEdit(&copy, painter, widget);

   if (!isEnabled)
      return; // why bother the user with elements he can't use... ;)

   const int f2 = dpi.f2;

   int arrowHeight = -1;
   painter->setPen(Qt::NoPen);

   if (sb->subControls & SC_SpinBoxUp) {
      copy.subControls = SC_SpinBoxUp;
      copy.rect = subControlRect(CC_SpinBox, sb, SC_SpinBoxUp, widget);

      isEnabled = sb->stepEnabled & QAbstractSpinBox::StepUpEnabled;
      hover = isEnabled && (sb->activeSubControls == SC_SpinBoxUp);
      sunken = sunken && (sb->activeSubControls == SC_SpinBoxUp);

      arrowHeight = 2*copy.rect.height()/3;
      copy.rect.setTop(copy.rect.bottom()-arrowHeight);

      if (!sunken) {
         painter->setBrush(FCOLOR(Base).dark(108));
         copy.rect.translate(0, f2);
         drawArrow(Navi::N, copy.rect, painter);
         copy.rect.translate(0, -f2);
      }

      QColor c;
      if (hover)
         c = FCOLOR(Highlight);
      else if (isEnabled)
         c = Colors::mid(FCOLOR(Base), FCOLOR(Text));
      else
         c = Colors::mid(FCOLOR(Base), PAL.color(QPalette::Disabled, QPalette::Text));

      painter->setBrush(c);
      drawArrow(Navi::N, copy.rect, painter);
   }

   if (sb->subControls & SC_SpinBoxDown) {
      copy.subControls = SC_SpinBoxDown;
      copy.rect = subControlRect(CC_SpinBox, sb, SC_SpinBoxDown, widget);

      isEnabled = sb->stepEnabled & QAbstractSpinBox::StepDownEnabled;
      hover = isEnabled && (sb->activeSubControls == SC_SpinBoxDown);
      sunken = sunken && (sb->activeSubControls == SC_SpinBoxDown);

      if (arrowHeight < 0)
         arrowHeight = 2*copy.rect.height()/3;
      copy.rect.setBottom(copy.rect.top()+arrowHeight);

      if (!sunken) {
         painter->setBrush(FCOLOR(Base).dark(105));
         copy.rect.translate(0, f2);
         drawArrow(Navi::S, copy.rect, painter);
         copy.rect.translate(0, -f2);
      }

      QColor c;
      if (hover)
         c = FCOLOR(Highlight);
      else if (isEnabled)
         c = Colors::mid(FCOLOR(Base), FCOLOR(Text));
      else
         c = Colors::mid(FCOLOR(Base), PAL.color(QPalette::Disabled, QPalette::Text));

      painter->setBrush(c);
      drawArrow(Navi::S, copy.rect, painter);
   }
}

static int animStep = -1;
static bool round_ = true;

void
BespinStyle::drawComboBox(const QStyleOptionComplex * option,
                          QPainter * painter, const QWidget * widget) const
{
   ASSURE_OPTION(cmb, ComboBox);
   B_STATES
   
   const int f2 = dpi.f2, f3 = dpi.f3;
   QRect ar, r = RECT; r.setBottom(r.bottom()-f2);
   const QComboBox* combo = widget ?
      qobject_cast<const QComboBox*>(widget) : 0;
   const bool listShown = combo && combo->view() &&
      ((QWidget*)(combo->view()))->isVisible();
   QColor c = CONF_COLOR(btn.std, Bg);

   if (listShown) { // this messes up hover
      hover = hover || QRect(widget->mapToGlobal(RECT.topLeft()),
                              RECT.size()).contains(QCursor::pos());
   }

   // do we have an arrow?
   if (isEnabled &&
      (cmb->subControls & SC_ComboBoxArrow) &&
      (!combo || combo->count() > 0)) {
         ar = subControlRect(CC_ComboBox, cmb, SC_ComboBoxArrow, widget);
         ar.setBottom(ar.bottom()-f2);
   }

   // the frame
   if ((cmb->subControls & SC_ComboBoxFrame) && cmb->frame) {
   if (cmb->editable)
      drawLineEdit(option, painter, widget);
   else {
      if (!ar.isNull()) {
         const Tile::Set &mask = masks.rect[round_];
         // ground
         animStep = Animator::Hover::step(widget);
         if (listShown) animStep = 6;

         c = btnBg(PAL, isEnabled, hasFocus, animStep, config.btn.fullHover,
                   Gradients::isReflective(GRAD(chooser)));
         if (hasFocus) {
            const int contrast = Colors::contrast(c, FCOLOR(Highlight));
            c = Colors::mid(c, FCOLOR(Highlight), contrast/5, 1);
         }
         
         mask.render(r, painter, GRAD(chooser), Qt::Vertical, c);

//          if (hasFocus) {
//             const int contrast = Colors::contrast(c, FCOLOR(Highlight));
//             const QColor fc = Colors::mid(c, FCOLOR(Highlight), contrast/10, 1);
//             mask.outline(r, painter, fc, f3);
//          }

         // maybe hover indicator?
         if (!config.btn.fullHover && animStep) { // jupp ;)
            r.adjust(f3, f3, -f3, -f3);
            c = Colors::mid(c, CONF_COLOR(btn.active, Bg), 6-animStep, animStep);
            mask.render(r, painter, GRAD(chooser), Qt::Vertical, c,
                        RECT.height()-f2, QPoint(0,f3));
         }
      }
      shadows.sunken[round_][isEnabled].render(RECT, painter);
   }
   }

   // the arrow
   if (!ar.isNull()) {
      if (!(ar.width()%2)) ar.setWidth(ar.width()-1);
      const int dy = ar.height()/4;
      QRect rect = ar.adjusted(0, dy, 0, -dy);

      Navi::Direction dir;
      bool upDown = false;
      if (listShown)
         dir = (config.leftHanded) ? Navi::E : Navi::W;
      else if (combo) {
         if (combo->currentIndex() == 0)
            dir = Navi::S;
         else if (combo->currentIndex() == combo->count()-1)
            dir = Navi::N;
         else
            upDown = true;
      }
      else
         dir = Navi::S;

      painter->save();
      painter->setPen(Qt::NoPen);
      if (cmb->editable) {
         if (upDown || dir == Navi::N) dir = Navi::S;
         upDown = false; // shall never look like spinbox!
         hover = hover && (cmb->activeSubControls == SC_ComboBoxArrow);
         if (!sunken) {
            painter->setBrush(FCOLOR(Base).dark(105));
            rect.translate(0, f2);
            drawArrow(dir, rect, painter);
            rect.translate(0, -f2);
         }
         if (hover || listShown)
            painter->setBrush(FCOLOR(Highlight));
         else
            painter->setBrush( Colors::mid(FCOLOR(Base), FCOLOR(Text)) );
      }
      else {
         c = Colors::mid(c, CONF_COLOR(btn.active, Bg));
         c = Colors::mid(c, CONF_COLOR(btn.active, Bg), 6-animStep, animStep);
//          ar.adjust(f2, f3, -f2, -f3);
         masks.rect[round_].render(ar, painter, GRAD(chooser), Qt::Vertical, c,
                                 RECT.height()-f2, QPoint(0,dpi.f4));
         painter->setBrush(Colors::mid(c, CONF_COLOR(btn.active, Fg), 1,2));
      }
      if (upDown) {
         rect.setBottom(rect.y() + rect.height()/2);
         rect.translate(0, -1);
         drawArrow(Navi::N, rect, painter);
         rect.translate(0, rect.height());
         drawArrow(Navi::S, rect, painter);
      }
      else
         drawArrow(dir, rect, painter);
      painter->restore();
   }
}

void
BespinStyle::drawComboBoxLabel(const QStyleOption * option, QPainter * painter,
                               const QWidget * widget) const
{
   ASSURE_OPTION(cb, ComboBox);
   OPT_ENABLED OPT_HOVER

   QRect editRect = subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, widget);
   painter->save();
   painter->setClipRect(editRect);
   // icon
   if (!cb->currentIcon.isNull()) {
      QIcon::Mode mode = isEnabled ? QIcon::Normal : QIcon::Disabled;
      QPixmap pixmap = cb->currentIcon.pixmap(cb->iconSize, mode);
      QRect iconRect(editRect);
      iconRect.setWidth(cb->iconSize.width() + 4);
      iconRect = alignedRect(QApplication::layoutDirection(),
                             Qt::AlignLeft | Qt::AlignVCenter,
                             iconRect.size(), editRect);
//       if (cb->editable)
//          painter->fillRect(iconRect, opt->palette.brush(QPalette::Base));
      drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

      if (cb->direction == Qt::RightToLeft)
         editRect.translate(-4 - cb->iconSize.width(), 0);
      else
         editRect.translate(cb->iconSize.width() + 4, 0);
   }
   // text
   if (!cb->currentText.isEmpty() && !cb->editable) {
      if (cb->frame) {
         const QComboBox* combo = widget ?
            qobject_cast<const QComboBox*>(widget) : 0;
         hover = !config.btn.backLightHover &&
            (hover || animStep > 2 || ( combo && combo->view() &&
                                        ((QWidget*)(combo->view()))->isVisible()));
         int f3 = dpi.f3;
         editRect.adjust(f3,0, -f3, 0);
         painter->setPen(hover ? CCOLOR(btn.active, 1) : CCOLOR(btn.std, 1));
      }
      painter->drawText(editRect, Qt::AlignCenter, cb->currentText);
   }
   painter->restore();
   animStep = -1;
}
