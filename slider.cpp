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
#include "animator/hovercomplex.h"
#include "draw.h"

void
BespinStyle::drawSliderHandle(const QRect &handle, const QStyleOption *option, QPainter *painter,
                              int step) const
{
    OPT_SUNKEN OPT_ENABLED OPT_FOCUS
    bool fullHover = config.btn.fullHover && !config.btn.backLightHover;

    // shadow
    QPoint xy = handle.topLeft();
    if (hasFocus && !sunken)
        fillWithMask(painter, xy, FCOLOR(Highlight), lights.slider);
    else if (step && config.btn.backLightHover)
        fillWithMask(painter, xy, Colors::mid(FCOLOR(Window), CCOLOR(btn.active, Bg), 6-step, step), lights.slider);
    if (sunken)
        xy += QPoint(dpi.f1, 0);
    painter->drawPixmap(xy, shadows.slider[isEnabled][sunken]);

    // gradient
    xy += QPoint(sunken ? dpi.f1 : dpi.f2, dpi.f1);

    QColor bc = CCOLOR(btn.std, Bg);
    if (fullHover)
        bc = Colors::mid(bc, CCOLOR(btn.active, Bg), 6-step, step);

    const QPixmap &fill = Gradients::pix(bc, masks.slider.height(), Qt::Vertical,
                                         isEnabled ? GRAD(scroll) : Gradients::None);
    fillWithMask(painter, xy, fill, masks.slider);
    if (isEnabled)
    {
        const QColor fc = Colors::mid(hasFocus ? FCOLOR(Highlight) : bc, CCOLOR(btn.std, Fg), 6-step, step+3);
        xy += QPoint(dpi.f5, dpi.f5);
        fillWithMask(painter, xy, fc, masks.notch);
    }
}

void
BespinStyle::drawSlider(const QStyleOptionComplex *option, QPainter *painter,
                        const QWidget * widget) const
{
   const QStyleOptionSlider *slider =
      qstyleoption_cast<const QStyleOptionSlider *>(option);
   if (!slider) return;

   OPT_SUNKEN OPT_ENABLED OPT_HOVER

   if (isEnabled && slider->subControls & SC_SliderTickmarks) {
      int ticks = slider->tickPosition;
      if (ticks != QSlider::NoTicks) {
      
         int available = pixelMetric(PM_SliderSpaceAvailable, slider, widget);
         int interval = slider->tickInterval;
         if (interval < 1) interval = slider->pageStep;
         if (interval) {
//             const int thickness =
//                pixelMetric(PM_SliderControlThickness, slider, widget);
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
//    const int ground = 0;

   // groove
   if ((slider->subControls & SC_SliderGroove) && groove.isValid()) {
      QStyleOption grooveOpt = *option;
      grooveOpt.rect = groove;

      const Groove::Mode gType = config.scroll.groove;
      if (gType) config.scroll.groove = Groove::Groove;
      drawScrollBarGroove(&grooveOpt, painter, widget);
      config.scroll.groove = gType;
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
      int step = 0;
      if (sunken)
         step = 6;
      else if (isEnabled) {
         const Animator::ComplexInfo *info =
               Animator::HoverComplex::info(widget, slider->activeSubControls & SC_SliderHandle);
         if (info && (info->fades[Animator::In] & SC_SliderHandle ||
                      info->fades[Animator::Out] & SC_SliderHandle))
            step = info->step(SC_SliderHandle);
         if (hover && !step)
            step = 6;
      }

   drawSliderHandle(handle, option, painter, step);

   }
}

void
BespinStyle::drawDial(const QStyleOptionComplex *option, QPainter *painter,
                      const QWidget *) const
{
   const QStyleOptionSlider *dial = qstyleoption_cast<const QStyleOptionSlider *>(option);
   if (!dial) return;

   OPT_ENABLED OPT_HOVER OPT_FOCUS
   
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
       
   int d = qMin(2*rect.width()/5, dpi.SliderThickness);
   int r;
   // angle calculation from qcommonstyle.cpp (c) Trolltech 1992-2007, ASA.
   qreal a;
   if (dial->maximum == dial->minimum)
      a = M_PI / 2;
   else if (dial->dialWrapping)
      a = M_PI * 3 / 2 - (dial->sliderValue - dial->minimum) * 2 * M_PI /
                                                            (dial->maximum - dial->minimum);
   else
      a = (M_PI * 8 - (dial->sliderValue - dial->minimum) * 10 * M_PI /
                                                            (dial->maximum - dial->minimum)) / 6;

   QPoint cp = rect.center();

   bool small = false;
   // fallback for small dials
   if (small = (rect.width() < 5*dpi.SliderThickness/2)) {
      painter->setRenderHint( QPainter::Antialiasing );
      painter->setPen(Qt::NoPen);
      painter->setBrush(QColor(0,0,0,50));
      painter->drawEllipse(rect);
      rect.adjust(dpi.f2,dpi.f1,-dpi.f2,-dpi.f2);
      painter->setBrushOrigin(rect.topLeft());
      const QPixmap &fill = Gradients::pix(FCOLOR(Window), rect.height(), Qt::Vertical, GRAD(scroll));
      painter->setBrush(fill);
      painter->drawEllipse(rect);
      QColor c = hasFocus ? FCOLOR(Highlight) : FCOLOR(WindowText);
      if (!hover)
         c = Colors::mid(FCOLOR(Window), c, 1, 1+isEnabled);
      d = qMax(dpi.f3, d/4);
      r = (rect.width()-d)/2;
      cp += QPoint((int)(r * cos(a)), -(int)(r * sin(a)));
      painter->setPen(QPen(c, d, Qt::SolidLine, Qt::RoundCap));
      painter->drawPoint(cp);
   }

   // the value
   QFont fnt = painter->font();
   int h = rect.height()/2;
   h -= 2 * (h - qMin(h, painter->fontMetrics().xHeight())) / 3;
   fnt.setPixelSize( h );
   painter->setFont(fnt);
   painter->setBrush(Qt::NoBrush);
   painter->setPen(Colors::mid(PAL.background().color(), PAL.foreground().color(),1,2));
   drawItemText(painter, rect,  Qt::AlignCenter, PAL, isEnabled, QString::number(dial->sliderValue));

   if (small) return;

   r = (rect.width()-d)/2;
   cp += QPoint((int)(r * cos(a)), -(int)(r * sin(a)));

   // the huge ring
   r = d/2; rect.adjust(r,r,-r,-r);
   painter->setPen(FCOLOR(Window).dark(115));
   painter->setRenderHint( QPainter::Antialiasing );
   painter->drawEllipse(rect);
   rect.translate(0, 1);
   painter->setPen(FCOLOR(Window).light(108));
   painter->drawEllipse(rect);

   // the drop
   rect = QRect(0,0,d,d);
   rect.moveCenter(cp);
   drawSliderHandle(rect, option, painter, hover * 6);

   painter->restore();
}
