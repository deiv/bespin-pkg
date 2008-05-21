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

#include <QStyleOptionDockWidget>
#include "draw.h"

void
BespinStyle::drawDockTitle(const QStyleOption * option, QPainter * painter, const QWidget *) const
{

   ASSURE_OPTION(dwOpt, DockWidget);
   OPT_ENABLED

//    masks.rect[true].render(RECT, painter, FCOLOR(Window));
//    shadows.line[false][Sunken].render(RECT, painter, Tile::Full, false);
   QRect r = RECT; r.setRight(r.x() + r.width()/3);
   shadows.line[false][Sunken].render(r, painter, Tile::Center | Tile::Right, true);
   r.setRight(RECT.right()); r.setLeft(r.right() - r.width()/3);
   shadows.line[false][Sunken].render(r, painter, Tile::Center | Tile::Left, true);

   if (!dwOpt->title.isEmpty()) {
      QFont fnt = painter->font(); bool noBold = fnt.bold();
      fnt.setBold(true);
      painter->setFont(fnt);
      int itemtextopts = Qt::AlignHCenter | Qt::AlignBottom | Qt::TextSingleLine | Qt::TextHideMnemonic;

      QPen pen = painter->pen();
      if (Colors::value(FCOLOR(WindowText)) > 140) {
         painter->setPen(Colors::mid(FCOLOR(Window), Qt::black, 1, 4));
         drawItemText(painter, RECT.adjusted(0,-1,0,-1), itemtextopts, PAL, isEnabled, dwOpt->title);
      }

      painter->setPen(FCOLOR(WindowText));
      drawItemText(painter, RECT, itemtextopts, PAL, isEnabled, dwOpt->title);
      painter->setPen(pen);

      if (noBold) {
         fnt.setBold(false); painter->setFont(fnt);
      }
   }
}

void
BespinStyle::drawDockHandle(const QStyleOption * option, QPainter * painter,
                            const QWidget *) const
{
   OPT_HOVER
      
   QPoint *points; int num;
   const int f12 = dpi.f12, f6 = dpi.f6;
   if (RECT.width() > RECT.height()) {
      int x = RECT.left()+RECT.width()/3;
      int y = RECT.top()+(RECT.height()-f6)/2;
      num = RECT.width()/(3*f12);
      if ((RECT.width()/3) % f12) ++num;
      points = new QPoint[num];
      for (int i = 0; i < num; ++i) {
         points[i] = QPoint(x,y); x += f12;
      }
   }
   else {
      int x = RECT.left()+(RECT.width()-f6)/2;
      int y = RECT.top()+RECT.height()/3;
      num = RECT.height()/(3*f12);
      if ((RECT.height()/3) % f12) ++num;
      points = new QPoint[num];
      for (int i = 0; i < num; ++i) {
         points[i] = QPoint(x,y); y += f12;
      }
   }
   painter->save();
   painter->setPen(Qt::NoPen);
   const QPixmap *fill; int cnt = num/2, imp = hover ? 4 : 1;
   const QColor &bg = FCOLOR(Window);
   const QColor &fg = hover ? FCOLOR(Highlight) : FCOLOR(WindowText);
   if (num%2) {
      fill = &Gradients::pix(Colors::mid(bg, fg, 5, imp), f6, Qt::Vertical, Gradients::Sunken);
      fillWithMask(painter, points[cnt], *fill, masks.notch);
   }
   --num;
   for (int i = 0; i < cnt; ++i) {
      fill = &Gradients::pix(Colors::mid(bg, fg, 5+cnt-i, imp), f6, Qt::Vertical, Gradients::Sunken);
      fillWithMask(painter, points[i], *fill, masks.notch);
      fillWithMask(painter, points[num-i], *fill, masks.notch);
   }
   painter->restore();
   delete[] points;
}
