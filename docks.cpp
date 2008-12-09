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

#include <QDockWidget>
#include <QStyleOptionDockWidget>
#include "draw.h"

void
Style::drawDockBg(const QStyleOption * option, QPainter * painter, const QWidget *) const
{
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(Gradients::structure(FCOLOR(Window), true));
    painter->translate(RECT.topLeft());
    painter->drawRect(RECT);
    painter->restore();
}

void
Style::drawDockTitle(const QStyleOption * option, QPainter * painter, const QWidget *w) const
{

    ASSURE_OPTION(dwOpt, DockWidget);
    
    const QColor bg = FCOLOR(Window);

    painter->save();

    painter->setPen(bg.dark(120));
    painter->drawLine(RECT.topLeft(), RECT.topRight());

    painter->setPen(bg.light(114));
    painter->drawLine(RECT.left(), RECT.y()+1, RECT.right(), RECT.y()+1);

    if (dwOpt->title.isEmpty())
        { painter->restore(); return; }
        
    {
        OPT_ENABLED OPT_HOVER
        QRect textRect, rect = RECT;
        // adjust rect;
        const int bw = (dwOpt->closable +  dwOpt->floatable) * (16 + F(2));
        if (option->direction == Qt::LeftToRight)
            rect.setRight(rect.right() - bw);
        else
            rect.setLeft(rect.left() + bw);

        // text
        const int itemtextopts = Qt::AlignBottom | Qt::AlignHCenter | Qt::TextSingleLine | Qt::TextHideMnemonic;
        QString title = dwOpt->title; // " " + dwOpt->title + " "; // good for underlining
        setTitleFont(painter);
        if (Colors::value(bg) < 100)
        {   // emboss
            painter->setPen(Colors::mid(bg, Qt::black, 1, 4));
            drawItemText(painter, rect.adjusted(0,-1,0,-1), itemtextopts, PAL, isEnabled, title);
        }
        painter->setPen(hover ? FCOLOR(WindowText) : Colors::mid(bg, FCOLOR(WindowText), 1, 3));
        drawItemText(painter, rect, itemtextopts, PAL, isEnabled, title, QPalette::NoRole, &rect);

        // underline
        Tile::PosFlags pf = Tile::Center;
        if (option->direction == Qt::LeftToRight)
            { rect.setRight(RECT.right()); pf |= Tile::Left; }
        else
            { rect.setLeft(RECT.left()); pf |= Tile::Right; }
        shadows.line[0][Sunken].render(rect, painter, pf, true);
    }
    painter->restore();
}

void
Style::drawDockHandle(const QStyleOption * option, QPainter * painter, const QWidget *) const
{
    OPT_HOVER

    QPoint *points; int num;
    const int f12 = dpi.f12, f6 = dpi.f6;
    if (RECT.width() > RECT.height())
    {
        int x = RECT.left()+RECT.width()/3;
        int y = RECT.top()+(RECT.height()-f6)/2;
        num = RECT.width()/(3*f12);
        if ((RECT.width()/3) % f12)
            ++num;
        points = new QPoint[num];
        for (int i = 0; i < num; ++i)
            { points[i] = QPoint(x,y); x += f12; }
    }
    else
    {
        int x = RECT.left()+(RECT.width()-f6)/2;
        int y = RECT.top()+RECT.height()/3;
        num = RECT.height()/(3*f12);
        if ((RECT.height()/3) % f12)
            ++num;
        points = new QPoint[num];
        for (int i = 0; i < num; ++i)
            { points[i] = QPoint(x,y); y += f12; }
    }
    painter->save();
    painter->setPen(Qt::NoPen);
    const QPixmap *fill; int cnt = num/2, imp = hover ? 4 : 1;
    const QColor &bg = FCOLOR(Window);
    const QColor &fg = hover ? FCOLOR(Highlight) : FCOLOR(WindowText);
    if (num%2)
    {
        fill = &Gradients::pix(Colors::mid(bg, fg, 5, imp), f6, Qt::Vertical, Gradients::Sunken);
        fillWithMask(painter, points[cnt], *fill, masks.notch);
    }
    --num;
    for (int i = 0; i < cnt; ++i)
    {
        fill = &Gradients::pix(Colors::mid(bg, fg, 5+cnt-i, imp), f6, Qt::Vertical, Gradients::Sunken);
        fillWithMask(painter, points[i], *fill, masks.notch);
        fillWithMask(painter, points[num-i], *fill, masks.notch);
    }
    painter->restore();
    delete[] points;
}

void
Style::drawMDIControls(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    QStyleOptionButton btnOpt;
    btnOpt.QStyleOption::operator=(*option);
    OPT_SUNKEN

#define PAINT_MDI_BUTTON(_btn_)\
if (option->subControls & SC_Mdi##_btn_##Button)\
{\
    if (sunken && option->activeSubControls & SC_Mdi##_btn_##Button)\
    {\
        btnOpt.state |= State_Sunken;\
        btnOpt.state &= ~State_Raised;\
    }\
    else\
    {\
        btnOpt.state |= State_Raised;\
        btnOpt.state &= ~State_Sunken;\
    }\
    btnOpt.rect = subControlRect(CC_MdiControls, option, SC_Mdi##_btn_##Button, widget);\
    painter->drawPixmap(btnOpt.rect.topLeft(), standardPixmap(SP_TitleBar##_btn_##Button, &btnOpt, widget));\
}//

    PAINT_MDI_BUTTON(Close);
    PAINT_MDI_BUTTON(Normal);
    PAINT_MDI_BUTTON(Min);

#undef PAINT_MDI_BUTTON
}
