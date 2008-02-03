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

#include <cmath>
#include "draw.h"

void
BespinStyle::drawSlider(const QStyleOptionComplex *option, QPainter *painter,
                        const QWidget * widget) const
{
   const QStyleOptionSlider *slider =
      qstyleoption_cast<const QStyleOptionSlider *>(option);
   if (!slider) return;

   B_STATES;

   if (isEnabled && slider->subControls & SC_SliderTickmarks) {
      int ticks = slider->tickPosition;
      if (ticks != QSlider::NoTicks) {
      
         int available = pixelMetric(PM_SliderSpaceAvailable, slider, widget);
         int interval = slider->tickInterval;
         if (interval < 1) interval = slider->pageStep;
         if (interval) {
            const int thickness =
               pixelMetric(PM_SliderControlThickness, slider, widget);
            const int len =
               pixelMetric(PM_SliderLength, slider, widget);
            const int fudge = len / 2;
            int pos, v = slider->minimum, nextInterval;
            // Since there is no subrect for tickmarks do a translation here.
            painter->save();
            painter->translate(RECT.x(), RECT.y());

#define DRAW_TICKS(_X1_, _Y1_, _X2_, _Y2_) \
            while (v <= slider->maximum) { \
               pos = sliderPositionFromValue(slider->minimum,\
                  slider->maximum, v, available) + fudge;\
               painter->drawLine(_X1_, _Y1_, _X2_, _Y2_);\
               nextInterval = v + interval;\
               if (nextInterval < v) break;\
               v = nextInterval; \
            } // skip semicolon

            painter->setPen(Colors::mid(BGCOLOR, FGCOLOR, 3,1));
            if (slider->orientation == Qt::Horizontal) {
               const int y = RECT.height()/2;
               if (ticks == QSlider::TicksAbove) {
                  DRAW_TICKS(pos, 0, pos, y); }
               else if (ticks == QSlider::TicksBelow) {
                  DRAW_TICKS(pos, y, pos, RECT.height()); }
               else {
                  DRAW_TICKS(pos, RECT.y(), pos, RECT.height()); }
            }
            else {
               const int x = RECT.width()/2;
               if (ticks == QSlider::TicksAbove) {
                  DRAW_TICKS(0, pos, x, pos); }
               else if (ticks == QSlider::TicksBelow) {
                  DRAW_TICKS(x, pos, RECT.width(), pos); }
               else {
                  DRAW_TICKS(0, pos, RECT.width(), pos); }
            }
            painter->restore();
         }
      }
   }

   QRect groove = subControlRect(CC_Slider, slider, SC_SliderGroove, widget);
   QRect handle = subControlRect(CC_Slider, slider, SC_SliderHandle, widget);

   isEnabled = isEnabled && (slider->maximum > slider->minimum);
   hover = isEnabled && hover && (slider->activeSubControls & SC_SliderHandle);
   sunken = sunken && (slider->activeSubControls & SC_SliderHandle);
   const int ground = 0;

   // groove
   if ((slider->subControls & SC_SliderGroove) && groove.isValid()) {
      QStyleOption grooveOpt = *option;
      grooveOpt.rect = groove;
      drawScrollBarGroove(&grooveOpt, painter, widget);
#if 0
      painter->save();

      QRect r;
      const QColor c = Colors::mid(FCOLOR(Window), CONF_COLOR(progress.std, 0), 3,1);
      if ( slider->orientation == Qt::Horizontal ) {

         int y = groove.center().y();
         painter->setPen(FCOLOR(Window).dark(115));
         painter->drawLine(groove.x(),y,groove.right(),y);
         ++y;
         painter->setPen(FCOLOR(Window).light(108));
         painter->drawLine(groove.x(),y,groove.right(),y);

         // the "temperature"
         if (slider->sliderPosition != ground &&
             slider->maximum > slider->minimum) {
            --y;

            int groundX = groove.width() * (ground - slider->minimum) /
               (slider->maximum - slider->minimum);
            bool rightSide = slider->sliderPosition > ground;

            if (slider->upsideDown) {
               rightSide = !rightSide;
               groundX = groove.right() - groundX;
            }
            else
               groundX += groove.left();

            if (rightSide) {
               groove.setLeft(groundX);
               groove.setRight(handle.center().x());
            }
            else {
               groove.setLeft(handle.center().x());
               groove.setRight(groundX);
            }
            painter->setPen(c.dark(115));
            painter->drawLine(groove.x(), y, groove.right(), y);
            ++y;
            painter->setPen(c.light(108));
            painter->drawLine(groove.x(), y, groove.right(), y);
         }
      }
      else { // Vertical

         int x = groove.center().x();
         painter->setPen(FCOLOR(Window).dark(115));
         painter->drawLine(x, groove.y(), x, groove.bottom());
         ++x;
         painter->setPen(FCOLOR(Window).light(108));
         painter->drawLine(x, groove.y(), x, groove.bottom());

         // the "temperature"
         if (slider->sliderPosition != ground &&
             slider->maximum > slider->minimum) {
            --x;

            int groundY = groove.height() * (ground - slider->minimum) /
               (slider->maximum - slider->minimum);
            bool upside = slider->sliderPosition > ground;

            if (slider->upsideDown) {
               upside = !upside;
               groundY = groove.bottom() - groundY;
            }
            else
               groundY += groove.top();

            if (upside) {
               groove.setBottom(handle.center().y());
               groove.setTop(groundY);
            }
            else {
               groove.setBottom(groundY);
               groove.setTop(handle.center().y());
            }

            painter->setPen(c.dark(115));
            painter->drawLine(x, groove.y(), x, groove.bottom());
            ++x;
            painter->setPen(c.light(108));
            painter->drawLine(x, groove.y(), x, groove.bottom());

         }
         // for later (cosmetic)
         if (!slider->upsideDown)
            handle.translate(dpi.f6, 0);
      }
      painter->restore();
#endif
   }

//    int direction = 0;
//    if (slider->orientation == Qt::Vertical)
//       ++direction;

   // handle
   if (slider->subControls & SC_SliderHandle) {
      int step;
      if (sunken)
         step = 6;
      else {
         step = 0;
         const ComplexHoverFadeInfo *info = animator->fadeInfo(widget,
            slider->activeSubControls & SC_SliderHandle);
         if (info && (info->fadeIns & SC_SliderHandle ||
                      info->fadeOuts & SC_SliderHandle))
            step = info->step(SC_SliderHandle);
         if (hover && !step)
            step = 6;
      }

      // shadow
      QPoint xy = handle.topLeft();
      if (sunken) xy += QPoint(dpi.f1, 0);
      painter->drawPixmap(xy, shadows.slider[isEnabled][sunken]);
      if (hasFocus && !sunken)
         fillWithMask(painter, xy, FCOLOR(Highlight), shadows.slider[true][false]);

      // gradient
      xy += QPoint(sunken ? dpi.f1 : dpi.f2, dpi.f1);
      
      QColor bc = CONF_COLOR(btn.std, Bg);
      QColor fc;
      if (config.btn.fullHover) {
         bc = Colors::mid(bc, CONF_COLOR(btn.active, Bg), 6-step, step);
         fc = Colors::mid(bc, CONF_COLOR(btn.active, Fg), 9-step, step+3);
      }
      else
         fc = Colors::mid(CONF_COLOR(btn.std, Bg),
                          CONF_COLOR(btn.std, Fg), 9-step, step+3);

      const QPixmap &fill =
         Gradients::pix(bc, masks.slider.height(), Qt::Vertical,
                        isEnabled ? GRAD(scroll) : Gradients::None);
      fillWithMask(painter, xy, fill, masks.slider);
      xy += QPoint(dpi.f5, dpi.f5);
      fillWithMask(painter, xy, fc, masks.notch);
#if 0
      SAVE_PEN;
      painter->setPen(fc);
      int x1, x2, y1, y2;
      if (slider->orientation == Qt::Horizontal) {
         x1 = x2 = handle.center().x();
         y1 = handle.top()+dpi.f4; y2 = handle.bottom()-dpi.f5;
      }
      else {
         x1 = handle.left()+dpi.f4; x2 = handle.right()-dpi.f4;
         y1 = y2 = handle.center().y();
      }
      painter->drawLine(x1, y1, x2, y2);
      RESTORE_PEN;
#endif
   }
}

void
BespinStyle::drawDial(const QStyleOptionComplex *option, QPainter *painter,
                      const QWidget *) const
{
   const QStyleOptionSlider *dial =
      qstyleoption_cast<const QStyleOptionSlider *>(option);
   if (!dial) return;

   B_STATES
   
   painter->save();
   QRect rect = RECT;
   if (rect.width() > rect.height()) {
      rect.setLeft(rect.x()+(rect.width()-rect.height())/2);
      rect.setWidth(rect.height());
   }
   else {
      rect.setTop(rect.y()+(rect.height()-rect.width())/2);
      rect.setHeight(rect.width());
   }
       
   int d = qMax(rect.width()/6, dpi.f10);
   int r = (rect.width()-d)/2;
   qreal a;
   if (dial->maximum == dial->minimum)
      a = M_PI / 2;
   else if (dial->dialWrapping)
      a = M_PI * 3 / 2 - (dial->sliderValue - dial->minimum) * 2 * M_PI
      / (dial->maximum - dial->minimum);
   else
      a = (M_PI * 8 - (dial->sliderValue - dial->minimum) * 10 * M_PI
         / (dial->maximum - dial->minimum)) / 6;

   QPoint cp((int)(r * cos(a)), -(int)(r * sin(a)));
   cp += rect.center();

   // the huge ring
   r = d/2; rect.adjust(r,r,-r,-r);
   painter->setPen(FCOLOR(Window).dark(115));
   painter->setRenderHint( QPainter::Antialiasing );
   painter->drawEllipse(rect);
   rect.translate(0, 1);
   painter->setPen(FCOLOR(Window).light(108));
   painter->drawEllipse(rect);
   // the value
   QFont fnt = painter->font();
   fnt.setPixelSize( rect.height()/3 );
   painter->setFont(fnt);
   painter->setBrush(Qt::NoBrush);
   painter->setPen(Colors::mid(PAL.background().color(),
                               PAL.foreground().color(),2,1));
   drawItemText(painter, rect,  Qt::AlignCenter, PAL, isEnabled,
                QString::number(dial->sliderValue));
   
   // the drop
   painter->setPen(Qt::NoPen);
   rect = QRect(0,0,d,d);
   rect.moveCenter(cp);
   painter->setBrush(QColor(0,0,0,50));
   painter->drawEllipse(rect);
   rect.adjust(dpi.f2,dpi.f1,-dpi.f2,-dpi.f2);
   painter->setPen(QPen(CONF_COLOR(btn.std, 0), dpi.f2));
   painter->setBrushOrigin(rect.topLeft());
   const QColor c = hover ? CONF_COLOR(btn.active, 0) : hasFocus ?
      FCOLOR(Highlight) : CONF_COLOR(btn.std, 0);
   const QPixmap &fill =
      Gradients::pix(c, rect.height(), Qt::Vertical, GRAD(scroll));
   painter->setBrush(fill);
   painter->drawEllipse(rect);
   painter->restore();
}
