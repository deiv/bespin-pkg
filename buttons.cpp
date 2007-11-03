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
#include <QStyleOptionButton>

#include "draw.h"

static int animStep = -1;
static bool isCheckbox = false;

void
BespinStyle::drawPushButton(const QStyleOption * option, QPainter * painter,
                            const QWidget * widget) const
{
   ASSURE_OPTION(btn, Button);
   OPT_SUNKEN;

   QStyleOptionButton tmpBtn = *btn;
   if (btn->features & QStyleOptionButton::Flat) {// more like a toolbtn
      if (option->state & State_Enabled) {
         if (option->state & State_HasFocus)
            masks.button.outline(RECT.adjusted(0,0,0,-dpi.f2), painter,
                                 Colors::mid(FCOLOR(Window), FCOLOR(Highlight)),
                                 dpi.f3);
         if (sunken)
            shadows.lineEdit[true].render(RECT, painter);
         else
            shadows.relief.render(RECT, painter);
      }
   }
   else {
      if (sunken && config.btn.layer == 1 && !config.btn.cushion)
         tmpBtn.rect.adjust(dpi.f1,dpi.f1,-dpi.f1,0);
      drawPushButtonBevel(&tmpBtn, painter, widget);
   }
//    tmpBtn.rect = subElementRect(SE_PushButtonContents, btn, widget);
   tmpBtn.rect.adjust(dpi.f6,dpi.f4,-dpi.f6,-dpi.f4);
   drawPushButtonLabel(&tmpBtn, painter, widget);
}

void
BespinStyle::drawPushButtonBevel(const QStyleOption * option,
                                 QPainter * painter,
                                 const QWidget * widget) const
{
   ASSURE_OPTION(btn, Button);
   OPT_SUNKEN

   animStep = sunken ? 6 : animator->hoverStep(widget);
   drawButtonFrame(option, painter, widget);
   if (btn->features & QStyleOptionButton::HasMenu) {
      int sz = (RECT.height()-dpi.f6)/2;
      QRect rect = RECT;
      rect.setLeft(RECT.right() - (dpi.f10+sz));
      shadows.line[1][Sunken].render(rect, painter);
      rect.setLeft(rect.left() + dpi.f4);
      rect.setWidth(sz);
      painter->save();
      const QColor c =
         Colors::mid(Colors::mid(CCOLOR(btn.std, Bg), CCOLOR(btn.std, Fg)),
                     Colors::mid(CCOLOR(btn.active, Bg), CCOLOR(btn.active, Fg)),
                     6-animStep, animStep);
      painter->setPen(c); painter->setBrush(c);
      drawArrow(Navi::S, rect, painter);
      painter->restore();
   }
   // toggle indicator
   ASSURE(widget);
   ASSURE_WIDGET(b, QAbstractButton);
   
   if (b->isCheckable()) {
      QRect r = RECT;
      const int h = r.height()/3;
      r.setTop(r.top() + h);
      r.setLeft(r.right()- h -dpi.f6);
      r.setWidth(h); r.setHeight(h);

      painter->save();
      painter->setRenderHint(QPainter::Antialiasing);
      if (option->state & State_On) {
         const QPixmap &fill =
            Gradients::pix(Colors::mid(CCOLOR(btn.std, Fg),
                                       CCOLOR(btn.active, Fg), 6-animStep, animStep),
                           r.height(), Qt::Vertical, GRAD(btn));
         painter->setBrush(fill);
      }
      else
         painter->setBrush(Qt::NoBrush);
      painter->setPen(CCOLOR(btn.std, Bg).dark(124));
      painter->setBrushOrigin(r.topLeft());
      painter->drawEllipse(r);
      painter->restore();
   }
}

void
BespinStyle::drawButtonFrame(const QStyleOption * option,
                             QPainter * painter, const QWidget * widget) const
{
   B_STATES;

   const QAbstractButton* btn = qobject_cast<const QAbstractButton*>(widget);
   const int f1 = dpi.f1, f2 = dpi.f2;
   const bool toggled = !hover && option->state & State_On;
   const bool round = !isCheckbox && (config.btn.round ||
                                      (btn && btn->isCheckable()));
   QRect r = RECT;
   if (animStep < 0)
      animStep = sunken ? 6 : animator->hoverStep(widget);

   QColor c = Colors::btnBg(PAL, isEnabled, hasFocus, animStep);
   QColor iC;

   bool drawInner = false;
   Gradients::Type gt = GRAD(btn);
   if (animStep) {
      if (config.btn.fullHover && !config.btn.backLightHover) {
         iC = Colors::mid(c, CCOLOR(btn.active, Bg), 6-animStep, animStep);
         c = iC;
      }
      else {
         if (config.btn.backLightHover) {
            drawInner = false;
            iC = Colors::mid(FCOLOR(Window), CCOLOR(btn.active, Bg), 6-animStep, animStep);
         }
         else {
            drawInner = true;
            iC = Colors::mid(c, CCOLOR(btn.active, Bg), 6-animStep, animStep);
         }
      }
      if ((config.btn.cushion && sunken) || toggled) {
         gt = Gradients::Sunken;
         drawInner = true;
      }
   }

   // sunken variant
   if (config.btn.layer) {
      if (isEnabled) {
         if (config.btn.layer == 1)
            r.adjust(f2, f1,-f2,-f2);
         else
            r.setBottom(r.bottom()-f2);
         masks.button.render(r, painter, GRAD(btn), Qt::Vertical, c);
         if (drawInner) {
            const int f3 = dpi.f3;
            const QRect ir =
               r.adjusted(f3, (config.btn.layer == 1) ? f2 : f3, -f3, -f3 );
            masks.button.render(ir, painter, gt, Qt::Vertical, iC,
                                r.height(), QPoint(0,f2));
         }
         if (hasFocus) {
            r = RECT; r.setBottom(r.bottom()-f1);
            masks.button.outline(r, painter, FCOLOR(Highlight), dpi.f3);
         }
      }
      if (config.btn.layer == 2 || (sunken && !config.btn.cushion) || !isEnabled )
         shadows.lineEdit[isEnabled].render(RECT, painter);
      else
         shadows.relief.render(RECT, painter);
      return;
   }
   
   // normal buttons ---------------
   const Tile::Set *mask, *light;
   if (round) { mask = &masks.tab; light = &lights.tab; }
   else { mask = &masks.button; light = &lights.button; }

   if (hasFocus) {// focus?
      const int contrast = Colors::contrast(FCOLOR(Window), FCOLOR(Highlight));
      if (!config.btn.cushion && sunken) r.setBottom(r.bottom()-dpi.f1);
      light->render(r, painter, Colors::mid(FCOLOR(Window),
         FCOLOR(Highlight), contrast/10, 1));
      r = RECT;
   }
   else if (config.btn.backLightHover && animStep) {
      light->render(RECT, painter, iC); // backlight
   }
   // shadow
   if (sunken && !config.btn.cushion) {
      r.adjust(f1, f1, -f1, -f2);
      round ? shadows.tab[isEnabled][true].render(r, painter) :
         shadows.button[isEnabled][true].render(r, painter);
      r.adjust(f1, f1, -f1, -f1);
   }
   else {
      r.adjust(0, f1, 0, 0);
      round ? shadows.tab[isEnabled][false].render(r, painter) :
         shadows.button[isEnabled][false].render(r, painter);
      r.adjust(f2, f1, -f2, -dpi.f3);
   }
   
   // plate
   mask->render(r, painter, toggled ? Gradients::Sunken : GRAD(btn), Qt::Vertical, c);

   // outline
   mask->outline(r.adjusted(f1,f1,-f1,-f1), painter,
                 Colors::mid(c, Qt::white), f1);

   if (isEnabled) {
      
      if (drawInner) {
         const QRect ir = isCheckbox ? r.adjusted(f2, f2, -f2, -f2 ) :
            r.adjusted(dpi.f3, f2, -dpi.f3, -f2 );
         mask->render(ir, painter, gt, Qt::Vertical,
                             config.btn.backLightHover ? c : iC,
                             r.height(), QPoint(0,f2));
      }
      
      // ambient?
      if (config.btn.ambientLight && !(sunken || isCheckbox))
         painter->drawPixmap(QPoint(r.right()+1-16*r.height()/9, r.top()),
                           Gradients::ambient(r.height()));
   }
}

void
BespinStyle::drawPushButtonLabel(const QStyleOption * option,
                                 QPainter * painter,
                                 const QWidget * widget) const
{
   OPT_ENABLED OPT_FOCUS OPT_HOVER;
   ASSURE_OPTION(btn, Button);

   QRect ir = btn->rect;
   uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
   if (!styleHint(SH_UnderlineShortcut, btn, widget))
      tf |= Qt::TextHideMnemonic;

   if (!btn->icon.isNull()) {
      QIcon::Mode mode = isEnabled ? QIcon::Normal
         : QIcon::Disabled;
      if (mode == QIcon::Normal && hasFocus)
         mode = QIcon::Active;
      QIcon::State state = QIcon::Off;
      if (btn->state & State_On)
         state = QIcon::On;
      QPixmap pixmap = btn->icon.pixmap(btn->iconSize, mode, state);
      int pixw = pixmap.width();
      int pixh = pixmap.height();

      //Center the icon if there is no text
      QPoint point;
      if (btn->text.isEmpty())
         point = QPoint(ir.x() + ir.width() / 2 - pixw / 2, ir.y() + ir.height() / 2 - pixh / 2);
      else
         point = QPoint(ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2);

      if (btn->direction == Qt::RightToLeft)
         point.rx() += pixw;

      painter->drawPixmap(visualPos(btn->direction, btn->rect, point), pixmap);

      if (btn->direction == Qt::RightToLeft)
         ir.translate(-4, 0);
      else
         ir.translate(pixw + 4, 0);
      ir.setWidth(ir.width() - (pixw + 4));
            // left-align text if there is
      if (!btn->text.isEmpty())
         tf |= Qt::AlignLeft;
   }
   else
      tf |= Qt::AlignHCenter;

   if (btn->text.isEmpty())
      return;

   if (btn->features & QStyleOptionButton::HasMenu) {
      ir.setRight(ir.right() - ir.height()/2 - dpi.f10);
   }
   else if (widget)
      if (const QAbstractButton* btn =
         qobject_cast<const QAbstractButton*>(widget))
         if (btn->isCheckable())
            ir.setRight(ir.right() - ir.height()/2 - dpi.f10);

   painter->save();
   const bool flat = btn->features & QStyleOptionButton::Flat;
   if ((flat && hover ) || hasFocus) {
//       ir.translate(0,-1);
      QFont tmpFnt = painter->font();
      tmpFnt.setBold(true);
      QFontMetrics fm(tmpFnt);
      QRect tr = fm.boundingRect ( btn->text );
      if (!tr.isValid()) {
         painter->restore();
         return;
      }
//       const int oPtS = tmpFnt.pointSize();
//       const int oPxS = tmpFnt.pixelSize();
      while (tr.width() > ir.width()) {
         if (tmpFnt.pointSize() > -1)
            tmpFnt.setPointSize(tmpFnt.pointSize()*ir.width()/tr.width());
         else
            tmpFnt.setPixelSize(tmpFnt.pixelSize()*ir.width()/tr.width());
         fm = QFontMetrics(tmpFnt);
         tr = fm.boundingRect ( btn->text );
      }
//       int dy = (oPtS > -1) ? (oPtS-tmpFnt.pointSize())/2 :
//          (oPxS-tmpFnt.pixelSize())/2;
//       tr.translate(0,dy);
      painter->setFont(tmpFnt);
   }
   
   QColor fg;
   if (config.btn.backLightHover)
      hover = animStep = 0;
   if (flat)
      fg = FCOLOR(WindowText);
   else
      fg = Colors::btnFg(PAL, isEnabled, hover, animStep);
   const QColor &bg = flat ? FCOLOR(Window) :
      (hover ? CCOLOR(btn.active, Bg) : CCOLOR(btn.std, Bg));

   if (!flat && isEnabled) {
      painter->setPen(bg.dark(120));
      ir.translate(0,-1);
      drawItemText(painter, ir, tf, PAL, isEnabled, btn->text);
      ir.translate(0,1);
   }

   painter->setPen(fg);
   drawItemText(painter, ir, tf, PAL, isEnabled, btn->text);
   painter->restore();
   animStep = -1;
}

void
BespinStyle::drawCheckBox(const QStyleOption * option, QPainter * painter,
                          const QWidget * widget) const
{
   OPT_ENABLED OPT_SUNKEN OPT_HOVER;

   QStyleOption copy = *option;
   if (config.btn.layer == 1)
      copy.rect.adjust(0,dpi.f1,0,-dpi.f1); // get rect appereance again
   else if (config.btn.layer == 0)
      copy.rect.adjust(dpi.f1,dpi.f1,-dpi.f1,0); // get rect appereance again
   isCheckbox = true;
   drawButtonFrame(&copy, painter, widget);
   isCheckbox = false;
   
   if (!(sunken || (option->state & State_Off))) {
      painter->save();
      if (config.btn.backLightHover) hover = animStep = 0;
      const QPoint center = copy.rect.center() - QPoint(0,dpi.f1);
      painter->setBrush(Colors::btnFg(PAL, isEnabled, hover, animStep));
      const int d = dpi.f5 - (config.btn.checkType + config.btn.layer) * dpi.f1;
      copy.rect.adjust(d, d, -d, -d);
      if (copy.rect.width() > copy.rect.height())
         copy.rect.setWidth(copy.rect.height());
      else
         copy.rect.setHeight(copy.rect.width());
      copy.rect.moveCenter(center);
      drawCheckMark(&copy, painter, config.btn.checkType);
      painter->restore();
   }
   animStep = -1;
}

void
BespinStyle::drawRadio(const QStyleOption * option, QPainter * painter,
                             const QWidget * widget) const
{
   B_STATES

   bool isOn = option->state & State_On;
   const int f2 = dpi.f2, f1 = dpi.f1, f4 = dpi.f4;
   QPoint xy = RECT.topLeft();
   
   Gradients::Type gt = isEnabled ? GRAD(btn) : Gradients::None;
   
   if (isOn) hover = /*hasFocus = */false;
//       else if (hover && sunken) isOn = true;
   
   animStep = isOn ? 0 : animator->hoverStep(widget);
   QColor bc = Colors::btnBg(PAL, isEnabled, hasFocus, animStep);
   QColor c = bc;
   if (animStep)
      c = Colors::mid(c, CCOLOR(btn.active, Bg), 6-animStep, animStep);
   
   if (config.btn.layer == 2) { // sunken ==================
      QRect r = RECT.adjusted(dpi.f1,0,-dpi.f1,-f2);
      masks.tab.render(r, painter, sunken || isOn ? Gradients::Sunken : gt,
                       Qt::Vertical, c);
      r.setBottom(RECT.bottom());
      shadows.tabSunken.render(r, painter);
      xy += QPoint(f1, 0);

   } else if (config.btn.layer) { // embedded ==================
      
      QRect r = RECT.adjusted(2,2,-2,-2);
      painter->save();
      painter->setRenderHint(QPainter::Antialiasing);
      painter->setBrushOrigin(r.topLeft());
      painter->setBrush(Gradients::brush( c, r.height(), Qt::Vertical, gt));
      if (!sunken && hasFocus)
         painter->setPen(Colors::mid(FCOLOR(Window), FCOLOR(Highlight),
                                     24-animStep, animStep));
      else
         painter->setPen(Qt::NoPen);
      painter->drawEllipse(r);
      painter->setBrush(Qt::NoBrush); r.adjust(-1,-1,1,1);
      painter->setPen(QColor(0,0,0,70)); painter->drawEllipse(r);
      r.translate(0,1);
      painter->setPen(QColor(255,255,255,90)); painter->drawEllipse(r);
      painter->restore();
      xy += QPoint(config.btn.layer*f1,f1);

   } else { //raised ==================

      sunken = sunken || isOn;

      painter->save();
      if (hasFocus || (animStep && config.btn.backLightHover)) {
         if (hasFocus) {
            const int contrast = Colors::contrast(FCOLOR(Window), FCOLOR(Highlight));
            painter->setBrush(Colors::mid(FCOLOR(Window), FCOLOR(Highlight), contrast/5, 1));
         }
         else
            painter->setBrush(Colors::mid(FCOLOR(Window), CCOLOR(btn.active, Bg),
                                          40/animStep, animStep));
         painter->setPen(Qt::NoPen);
         painter->setRenderHint(QPainter::Antialiasing);
         QRect r = RECT; if (sunken) r.setBottom(r.bottom()-f1);
         painter->drawEllipse(r);
      }
      
      // shadow
      painter->drawPixmap(sunken ? xy + QPoint(f1,f1) : xy,
                          shadows.radio[isEnabled][sunken]);
      
      // plate
      xy += QPoint(f2,f1); const int sz = dpi.ExclusiveIndicator - f4;
      if (config.btn.fullHover && !config.btn.backLightHover)
         fillWithMask(painter, xy, Gradients::brush(c, sz, Qt::Vertical, gt), masks.radio);
      else {
         fillWithMask(painter, xy, Gradients::brush(bc, sz, Qt::Vertical, gt), masks.radio);
         if (animStep) {
            fillWithMask(painter, xy + QPoint(f4, f4),
                         Gradients::brush(c, sz, Qt::Vertical, gt),
                         masks.radioIndicator, QPoint(0, dpi.f4));
         }
      }
      if (isEnabled) {
         painter->setPen(QPen(Colors::mid(bc, Qt::white),f1));
         painter->setBrush(Qt::NoBrush);
         painter->setRenderHint(QPainter::Antialiasing);
         QRect r(xy, QSize(sz, sz));
         r.adjust(f1,f1,-f1,-f1); painter->drawEllipse(r);
      }
      painter->restore();
   }
   // drop
   if (isOn) {
      xy += QPoint(f4, f4);
      fillWithMask(painter, xy,
                   Colors::btnFg(PAL, isEnabled, hover, animStep),
                   masks.radioIndicator);
   }
   animStep = -1;
}

//    case PE_FrameButtonBevel: // Panel frame for a button bevel

void
BespinStyle::drawRadioItem(const QStyleOption * option, QPainter * painter,
                           const QWidget * widget) const
{
   ASSURE_OPTION(btn, Button);
   QStyleOptionButton subopt = *btn;
   subopt.rect = subElementRect(SE_RadioButtonIndicator, btn, widget);
   drawRadio(&subopt, painter, widget);
   subopt.rect = subElementRect(SE_RadioButtonContents, btn, widget);
   drawCheckLabel(&subopt, painter, widget);
}

void
BespinStyle::drawCheckBoxItem(const QStyleOption * option, QPainter * painter,
                              const QWidget * widget) const
{
   ASSURE_OPTION(btn, Button);
   QStyleOptionButton subopt = *btn;
   subopt.rect = subElementRect(SE_CheckBoxIndicator, btn, widget);
   drawCheckBox(&subopt, painter, widget);
   subopt.rect = subElementRect(SE_CheckBoxContents, btn, widget);
   drawCheckLabel(&subopt, painter, widget);
}

void
BespinStyle::drawCheckLabel(const QStyleOption * option, QPainter * painter,
                            const QWidget * /*widget*/) const
{
   ASSURE_OPTION(btn, Button);
   OPT_ENABLED;
   
   uint alignment =
      visualAlignment(btn->direction, Qt::AlignLeft | Qt::AlignVCenter);

   QRect textRect = RECT;
   if (!btn->icon.isNull()) {
      const QPixmap pix =
         btn->icon.pixmap(btn->iconSize,
                          isEnabled ? QIcon::Normal : QIcon::Disabled);
      drawItemPixmap(painter, btn->rect, alignment, pix);
      if (btn->direction == Qt::RightToLeft)
         textRect.setRight(textRect.right() - btn->iconSize.width() - dpi.f4);
      else
         textRect.setLeft(textRect.left() + btn->iconSize.width() + dpi.f4);
   }
   if (!btn->text.isEmpty())
      drawItemText(painter, textRect, alignment | Qt::TextShowMnemonic,
                   PAL, isEnabled, btn->text, QPalette::WindowText);
}
