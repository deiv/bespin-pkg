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
BespinStyle::drawDockTitle(const QStyleOption * option, QPainter * painter,
                           const QWidget *) const
{
   ASSURE_OPTION(dwOpt, DockWidget);
   OPT_ENABLED

   QRect textRect;
   int x3 = RECT.right()-7;
   if (dwOpt->floatable)
      x3 -= 18;
   if (dwOpt->closable)
      x3 -= 18;
   int x2 = x3;
   if (!dwOpt->title.isEmpty()) {
      int itemtextopts = Qt::AlignCenter | Qt::TextShowMnemonic;
      drawItemText(painter, RECT, itemtextopts, PAL, isEnabled, dwOpt->title, QPalette::WindowText);
      textRect = painter->boundingRect ( RECT, itemtextopts, dwOpt->title );
      x2 = textRect.x()-8;
   }

   const Tile::Line &line = shadows.line[0][Sunken];
   textRect.setTop(textRect.top()+(textRect.height()-line.thickness())/2);
   int x = textRect.right()+dpi.f4;
   textRect.setRight(textRect.left()-dpi.f4);
   textRect.setLeft(qMin(RECT.x()+RECT.width()/4,textRect.x()-(textRect.x()-RECT.x())/2));
   line.render(textRect, painter, Tile::Left|Tile::Center);
   textRect.setLeft(x);
   textRect.setRight(qMax(RECT.right()-RECT.width()/4,x+(RECT.right()-x)/2));
   line.render(textRect, painter, Tile::Right|Tile::Center);
   //TODO: hover?
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
   const QPixmap *fill; int cnt = num/2, imp = hover ? 8 : 1;
   const QColor &bg = FCOLOR(Window);
   const QColor &fg = hover ? FCOLOR(Highlight) : FCOLOR(WindowText);
   if (num%2) {
      fill = &Gradients::pix(Colors::mid(bg, fg, 3, imp), f6,
                             Qt::Vertical, Gradients::Sunken);
      fillWithMask(painter, points[cnt], *fill, masks.notch);
   }
   --num;
   for (int i = 0; i < cnt; ++i) {
      fill = &Gradients::pix(Colors::mid(bg, fg, 3+cnt-i, imp), f6,
                             Qt::Vertical, Gradients::Sunken);
      fillWithMask(painter, points[i], *fill, masks.notch);
      fillWithMask(painter, points[num-i], *fill, masks.notch);
   }
   painter->restore();
   delete[] points;
}
