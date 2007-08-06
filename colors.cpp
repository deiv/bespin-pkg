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

#include "colors.h"
#include "makros.h"
#include <QWidget>

using namespace Bespin;

const QColor &Colors::bg(const QPalette &pal, const QWidget *w) {
   if (w->parentWidget())
      return pal.color(w->parentWidget()->backgroundRole());
   if (w)
      return pal.color(w->backgroundRole());
   return pal.color(QPalette::Window);
}

#define TMP_COLOR(_ROLE_) pal.color(QPalette::_ROLE_)

QColor Colors::btnBg(const QPalette &pal, bool isEnabled, int hasFocus, int step) {
   if (!isEnabled)
      return TMP_COLOR(Window).dark(104);
   QColor c = (hasFocus) ?
      mid(TMP_COLOR(Highlight),TMP_COLOR(Window),
               1, contrast(TMP_COLOR(Highlight),TMP_COLOR(Window))/2) :
      TMP_COLOR(Window);
   if (step)
      return mid(c, TMP_COLOR(Button), 36 - step, step);
   return c;
}

QColor Colors::btnFg(const QPalette &pal, bool isEnabled, int hover, int step) {
   if (!isEnabled)
      return mid(TMP_COLOR(Window), TMP_COLOR(WindowText), 1, 3);
   if (hover && !step) step = 6;
   if (step)
      return mid(TMP_COLOR(WindowText), TMP_COLOR(ButtonText), 6 - step, step);
   return TMP_COLOR(WindowText);
}

#undef TMP_COLOR

int Colors::contrast(const QColor &a, const QColor &b) {
   int ar,ag,ab,br,bg,bb;
   a.getRgb(&ar,&ag,&ab);
   b.getRgb(&br,&bg,&bb);
   
   int diff = qAbs(299*(ar-br) + 587*(ag-bg) + 114*(ab-bb));
   int perc = diff / 2550;
   
   diff = qMax(ar,br) + qMax(ag,bg) + qMax(ab,bb)
      - (qMin(ar,br) + qMin(ag,bg) + qMin(ab,bb));
   
   perc *= diff;
   perc /= 765;
   
   return perc;
}

bool Colors::counterRole(QPalette::ColorRole &from, QPalette::ColorRole &to,
                         QPalette::ColorRole defFrom, QPalette::ColorRole defTo) {
   switch (from) {
   case QPalette::WindowText: //0
      to = QPalette::Window; break;
   case QPalette::Window: //10
      to = QPalette::WindowText; break;
   case QPalette::Base: //9
      to = QPalette::Text; break;
   case QPalette::Text: //6
      to = QPalette::Base; break;
   case QPalette::Button: //1
      to = QPalette::ButtonText; break;
   case QPalette::ButtonText: //8
      to = QPalette::Button; break;
   case QPalette::Highlight: //12
      to = QPalette::HighlightedText; break;
   case QPalette::HighlightedText: //13
      to = QPalette::Highlight; break;
   default:
      from = defFrom;
      to = defTo;
      return false;
   }
   return true;
}

QColor Colors::emphasize(const QColor &c, int value) {
   int h,s,v;
   QColor ret;
   c.getHsv(&h,&s,&v);
   if (v < 75+value) {
      ret.setHsv(h,s,CLAMP(85+value,85,255));
      return ret;
   }
   if (v > 200) {
      if (s > 30) {
         h -= 5; if (h < 0) h = 360 + h;
         s = (s<<3)/9;
         v += value;
         ret.setHsv(h,CLAMP(s,30,255),CLAMP(v,0,255));
         return ret;
      }
      if (v > 230) {
         ret.setHsv(h,s,CLAMP(v-value,0,255));
         return ret;
      }
   }
   if (v > 128)
      ret.setHsv(h,s,CLAMP(v+value,0,255));
   else
      ret.setHsv(h,s,CLAMP(v-value,0,255));
   return ret;
}

QColor Colors::light(const QColor &c, int value)
{
   int h,s,v;
   c.getHsv(&h,&s,&v);
   QColor ret;
   if (v < 255-value) {
      ret.setHsv(h,s,CLAMP(v+value,0,255)); //value could be negative
      return ret;
   }
   // psychovisual uplightning, i.e. shift hue and lower saturation
   if (s > 30) {
      h -= (value*5/20); if (h < 0) h = 400 + h;
      s = CLAMP((s<<3)/9,30,255);
      ret.setHsv(h,s,255);
      return ret;
   }
   else // hue shifting has no sense, half saturation (btw, white won't get brighter :)
      ret.setHsv(h,s>>1,255);
   return ret;
}

QColor Colors::mid(const QColor &oc1, const QColor &c2, int w1, int w2) {
   int sum = (w1+w2);
   QColor c1 = oc1;
   int h,s, v = value(c1);
   if (v < 70) {
      c1.getHsv(&h,&s,&v);
      c1.setHsv(h,s,70);
   }
   return QColor((w1*c1.red() + w2*c2.red())/sum,
                 (w1*c1.green() + w2*c2.green())/sum,
                 (w1*c1.blue() + w2*c2.blue())/sum,
                 (w1*c1.alpha() + w2*c2.alpha())/sum);
}

int Colors::value(const QColor &c) {
   int v = c.red();
   if (c.green() > v) v = c.green();
   if (c.blue() > v) v = c.blue();
   return v;
}
