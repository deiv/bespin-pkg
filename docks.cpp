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
Style::drawDockBg(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    bool needRestore = false;
    if (config.bg.mode == Scanlines)
    {
        painter->save(); needRestore = true;
        
        painter->setPen(Qt::NoPen);
        painter->setBrush(Gradients::structure(FCOLOR(Window), true));
        painter->translate(RECT.topLeft());
        painter->drawRect(RECT);
    }
    
    if (widget && widget->isWindow())
    {
        if (!needRestore) painter->save();

        painter->setPen(Colors::mid(FCOLOR(Window), Qt::black, 3,1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(RECT.adjusted(0,0,-1,-1));

        painter->restore(); needRestore = false;
    }
    if (needRestore) painter->restore();
}

void
Style::drawDockTitle(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(dock, DockWidget);
    
    QColor bg = FCOLOR(Window);
    bg.setAlpha(config.bg.opacity);
    const bool floating = widget && widget->isWindow();
#if 0
    if ((dock->floatable || dock->movable) && !floating)
    {
        Tile::setShape(Tile::Full & ~Tile::Bottom);
        masks.rect[true].render(RECT, painter, Gradients::light(RECT.height()));
        Tile::reset();
    }
#endif
    if (dock->title.isEmpty())
        return;

    OPT_ENABLED
    QRect rect = RECT;
    const int bw = (dock->closable +  dock->floatable) * (16 + F(2));
    if (option->direction == Qt::LeftToRight)
        rect.adjust(F(8), 0, -bw, 0);
    else
        rect.adjust(bw, 0, -F(8), 0);

    // text
    const int itemtextopts = Qt::AlignCenter | Qt::TextSingleLine | Qt::TextHideMnemonic;

    QPen pen = painter->pen();
    if (floating && widget->isActiveWindow())
        painter->setPen(FCOLOR(WindowText));
    else
        painter->setPen(Colors::mid(bg, FCOLOR(WindowText), 2, 1+isEnabled));
    drawItemText(painter, rect, itemtextopts, PAL, isEnabled, dock->title);
    painter->setPen(pen);
}

void
Style::drawDockHandle(const QStyleOption *option, QPainter *painter, const QWidget *) const
{
    OPT_HOVER
#if 0
    const QColor fg = Colors::mid(FCOLOR(Window), hover ? FCOLOR(Highlight) : FCOLOR(WindowText), 6, 1 + 2*hover );
    QRect rect;
    if (RECT.width() > RECT.height())
        rect.setRect(0,0,F(32),F(2));
    else
        rect.setRect(0,0,F(2),F(32));
    rect.moveCenter( RECT.center() );
    painter->drawPixmap( rect.left(), rect.top(),
                         Gradients::pix(fg, rect.height(), Qt::Vertical, Gradients::Sunken),
                         0, 0, rect.width(), rect.height() );

#else
    QPoint *points; int num;
    const int f12 = F(12), f6 = F(6);
    if (RECT.width() > RECT.height())
    {
        int x = RECT.left()+2*RECT.width()/5;
        int y = RECT.top()+(RECT.height()-f6)/2;
        num = RECT.width()/(5*f12);
        if ((2*RECT.width()/5) % f12)
            ++num;
        points = new QPoint[num];
        for (int i = 0; i < num; ++i)
            { points[i] = QPoint(x,y); x += f12; }
    }
    else
    {
        int x = RECT.left()+(RECT.width()-f6)/2;
        int y = RECT.top()+2*RECT.height()/5;
        num = RECT.height()/(5*f12);
        if ((2*RECT.height()/5) % f12)
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
#endif
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
