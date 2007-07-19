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

#ifndef INLINEHELP_CPP
#define INLINEHELP_CPP

#include <QPalette>
#include <QTabBar>

#ifndef CLAMP
#define CLAMP(x,l,u) (x) < (l) ? (l) :\
(x) > (u) ? (u) :\
(x)
#endif

#ifndef QABS
#define QABS(x) (x) > 0 ? (x) : (-(x))
#endif

#ifndef _PRINTFLAGS_
#define _PRINTFLAGS_ option ? qWarning("State Flags:\n%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",\
option->state & State_Active ? "Active, " : "",\
option->state & State_AutoRaise ? "AutoRaise, " : "",\
option->state & State_Bottom ? "Bottom, " : "",\
option->state & State_Children ? "Children, " : "",\
option->state & State_None ? "None, " : "",\
option->state & State_DownArrow ? "DownArrow, " : "",\
option->state & State_Editing ? "Editing, " : "",\
option->state & State_Enabled ? "Enabled, " : "",\
option->state & State_FocusAtBorder ? "FocusAtBorder, " : "",\
option->state & State_HasFocus ? "HasFocus, " : "",\
option->state & State_Horizontal ? "Horizontal, " : "",\
option->state & State_Item ? "Item, " : "",\
option->state & State_MouseOver ? "MouseOver, " : "",\
option->state & State_NoChange ? "NoChange, " : "",\
option->state & State_Off ? "Off, " : "",\
option->state & State_On ? "On, " : "",\
option->state & State_Open ? "Open, " : "",\
option->state & State_Raised ? "Raised, " : "",\
option->state & State_Selected ? "Selected, " : "",\
option->state & State_Sibling ? "Sibling, " : "",\
option->state & State_Sunken ? "Sunken, " : "",\
option->state & State_Top ? "Top, " : "",\
option->state & State_UpArrow ? "UpArrow, " : "",\
option->state & State_KeyboardFocusChange ? "KeyboardFocusChange, " : "",\
option->state & State_ReadOnly ? "ReadOnly, " : "") : qWarning("MISSING OPTIONS")
#endif

extern Config config;

/**Internal calculates hsv v from color, faster than calling qcolor.hsv if you only want v*/
inline int colorValue(const QColor &c) {
   int v = c.red();
   if (c.green() > v) v = c.green();
   if (c.blue() > v) v = c.blue();
   return v;
}

/**Internal, calcs weighted mid of two colors*/
inline QColor midColor(const QColor &oc1, const QColor &c2, int w1 = 1, int w2 = 1) {
   int sum = (w1+w2);
   QColor c1 = oc1;
   int h,s, v = colorValue(c1);
   if (v < 70) {
      c1.getHsv(&h,&s,&v);
      c1.setHsv(h,s,70);
   }
   return QColor((w1*c1.red() + w2*c2.red())/sum,
                 (w1*c1.green() + w2*c2.green())/sum,
                 (w1*c1.blue() + w2*c2.blue())/sum,
                 (w1*c1.alpha() + w2*c2.alpha())/sum);
}

#define TMP_COLOR(_ROLE_) pal.color(QPalette::_ROLE_)
#define TMP_CONF_COLOR(_ROLE_) pal.color(config._ROLE_)

/**Internal, calcs button color depending on state*/

inline QColor btnBgColor(const QPalette &pal, bool isEnabled, int hasFocus = false, int step = 0) {
   if (!isEnabled)
      return TMP_COLOR(Window).dark(104);
   QColor c = (hasFocus) ? midColor(TMP_COLOR(Highlight),TMP_COLOR(Window), 1, 20) : TMP_COLOR(Window);
   if (step)
      return midColor(c, TMP_COLOR(Button), 36 - step, step);
   return c;
}

/**Internal, calcs buttonText color depending on state*/
inline QColor btnFgColor(const QPalette &pal, bool isEnabled, int hover, int step = 0) {
   if (!isEnabled)
      return midColor(TMP_COLOR(Window), TMP_COLOR(WindowText), 1, 3);
   if (hover && !step) step = 6;
   if (step)
      return midColor(TMP_COLOR(WindowText), TMP_COLOR(ButtonText),
                      step, config.hoverImpact - step);
   return TMP_CONF_COLOR(role_btn[1]);
}

#undef TMP_COLOR
#undef TMP_CONF_COLOR

/**Internal, calcs a contrasted color to a qcolor*/
inline QColor emphasize(const QColor &c, int value = 10) {
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

/**Internal, calcs a lightned version of a qcolor*/
inline QColor light(const QColor &c, int value)
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

inline bool verticalTabs(QTabBar::Shape shape) {
   return shape == QTabBar::RoundedEast ||
      shape == QTabBar::TriangularEast ||
      shape == QTabBar::RoundedWest ||
      shape == QTabBar::TriangularWest;
}

inline const QColor &bgcolor(const QPalette &pal, const QWidget *w) {
   if (w->parentWidget())
      return pal.color(w->parentWidget()->backgroundRole());
   if (w)
      return pal.color(w->backgroundRole());
   return pal.color(QPalette::Window);
}

#endif //INLINEHELP_CPP
