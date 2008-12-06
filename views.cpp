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
#include <QAbstractItemView>
#include <QTreeView>
#include "draw.h"

void
Style::drawHeader(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(header, Header);

    if (appType == GTK)
        const_cast<QStyleOption*>(option)->palette = qApp->palette();

    // init
    //    const QRegion clipRegion = painter->clipRegion();
    //    painter->setClipRect(RECT/*, Qt::IntersectClip*/);

    // base
    drawHeaderSection(header, painter, widget);

    // label
    drawHeaderLabel(header, painter, widget);

    // sort Indicator on sorting or (inverted) on hovered headers
    if (header->sortIndicator != QStyleOptionHeader::None)
    {
        QStyleOptionHeader subopt = *header;
        subopt.rect = subElementRect(SE_HeaderArrow, option, widget);
        drawHeaderArrow(&subopt, painter, widget);
    }
       
//    painter->setClipRegion(clipRegion);
}

void
Style::drawHeaderSection(const QStyleOption * option, QPainter * painter,
                               const QWidget *) const
{
    OPT_SUNKEN OPT_HOVER
    const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option);

    Qt::Orientation o = Qt::Vertical; int s = RECT.height();
    if (header && header->orientation == Qt::Vertical)
    {
        o = Qt::Horizontal;
        s = RECT.width();
    }

    QColor c =  (header->sortIndicator != QStyleOptionHeader::None) ?
                                        COLOR(config.view.sortingHeader_role[Bg]) :
                                        COLOR(config.view.header_role[Bg]);

    if (appType == GTK)
        sunken = option->state & State_HasFocus;
    if (sunken)
    {
        const QPixmap &sunk = Gradients::pix(c, s, o, Gradients::Sunken);
        painter->drawTiledPixmap(RECT, sunk);
        return;
    }

    const Gradients::Type gt =  (header->sortIndicator != QStyleOptionHeader::None) ?
                                config.view.sortingHeaderGradient : config.view.headerGradient;
    QRect r = RECT;
    if (o == Qt::Vertical)
        r.setBottom(r.bottom()-1);
    if (hover)
    {
        const bool sort = (header->sortIndicator != QStyleOptionHeader::None);
        c = Colors::mid(c, sort ? CCOLOR(view.sortingHeader, Fg) : CCOLOR(view.header, Fg),10,1);
    }
    if (gt == Gradients::None)
        painter->fillRect(r, c);
    else
        painter->drawTiledPixmap(r, Gradients::pix(c, s, o, gt));

    if (o == Qt::Vertical)
    {
        r.setLeft(r.right() - dpi.f1);
        painter->drawTiledPixmap(r, Gradients::pix(COLOR(config.view.header_role[Bg]), s, o, Gradients::Sunken));
        SAVE_PEN
        painter->setPen(Colors::mid(FCOLOR(Base), Qt::black, 8, 1));
        painter->drawLine(RECT.bottomLeft(), RECT.bottomRight());
        RESTORE_PEN
    }
}

void
Style::drawHeaderLabel(const QStyleOption * option, QPainter * painter, const QWidget *widget) const
{
    OPT_ENABLED

    const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option);
    QRect rect = widget ? RECT.intersected(widget->rect()) : RECT;

    // iconos
    if ( !header->icon.isNull() )
    {
        QPixmap pixmap = header->icon.pixmap( 22,22, isEnabled ? QIcon::Normal : QIcon::Disabled );
        int pixw = pixmap.width();
        int pixh = pixmap.height();
        // "pixh - 1" because of tricky integer division
        rect.setY( rect.center().y() - (pixh - 1) / 2 );
        drawItemPixmap ( painter, rect, Qt::AlignCenter, pixmap );
        rect = RECT; rect.setLeft( rect.left() + pixw + 2 );
    }

    if (header->text.isEmpty())
        return;

    // textos ;)
    painter->save();

    // this works around a possible Qt bug?!?
    QFont tmpFnt = painter->font(); tmpFnt.setBold(option->state & State_On);
    const QColor *bg, *fg;
    if (header->sortIndicator != QStyleOptionHeader::None)
    {
        tmpFnt.setBold(true);
        bg = &CCOLOR(view.sortingHeader, Bg);
        fg = &CCOLOR(view.sortingHeader, Fg);
    }
    else
    {
        bg = &CCOLOR(view.header, Bg);
        fg = &CCOLOR(view.header, Fg);
    }
    painter->setFont(tmpFnt);

    if (isEnabled)
    {   // dark background, let's paint an emboss
        rect.moveTop(rect.top()-1);
        painter->setPen(bg->dark(120));
        drawItemText ( painter, rect, Qt::AlignCenter, PAL, isEnabled, header->text);
        rect.moveTop(rect.top()+1);
    }

    painter->setPen(*fg);
    drawItemText ( painter, rect, Qt::AlignCenter, PAL, isEnabled, header->text);
    painter->restore();
}

void
Style::drawHeaderArrow(const QStyleOption * option, QPainter * painter, const QWidget *) const
{
    Navi::Direction dir = Navi::S;
    if (const QStyleOptionHeader* hopt = qstyleoption_cast<const QStyleOptionHeader*>(option))
    {
        if (hopt->sortIndicator == QStyleOptionHeader::None)
            return;
        if (hopt->sortIndicator == QStyleOptionHeader::SortUp)
            dir = Navi::N;
    }
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(Colors::mid(CCOLOR(view.sortingHeader, Bg), CCOLOR(view.sortingHeader, Fg)));
    drawArrow(dir, RECT, painter);
    painter->restore();
}

static const int decoration_size = 9;

void
Style::drawBranch(const QStyleOption * option, QPainter * painter, const QWidget *widget) const
{
    SAVE_PEN;
    int mid_h = RECT.x() + RECT.width() / 2;
    int mid_v = RECT.y() + RECT.height() / 2;
    int bef_h = mid_h;
    int bef_v = mid_v;
    int aft_h = mid_h;
    int aft_v = mid_v;


    QPalette::ColorRole bg = QPalette::Text, fg = QPalette::Base;
    if (widget)
        { bg = widget->backgroundRole(); fg = widget->foregroundRole(); }

    const bool firstCol = (RECT.x() ==  -1);

    if (option->state & State_Children)
    {
        SAVE_BRUSH
        int delta = decoration_size / 2 + 2;
        bef_h -= delta;
        bef_v -= delta;
        aft_h += delta;
        aft_v += delta;
        painter->setPen(Qt::NoPen);
        QRect rect = QRect(bef_h+2, bef_v+2, decoration_size, decoration_size);
        if (firstCol)
            rect.moveRight(RECT.right()-dpi.f1);
        if (option->state & State_Open)
        {
            if (option->state & State_Selected)
                painter->setBrush(FCOLOR(HighlightedText));
            else
                painter->setBrush(Colors::mid( COLOR(bg), COLOR(fg)));
            rect.translate(0,-decoration_size/6);
            if (option->direction == Qt::RightToLeft)
                drawSolidArrow(Navi::SW, rect, painter);
            else
                drawSolidArrow(Navi::SE, rect, painter);
        }
        else
        {
            if (option->state & State_Selected)
                painter->setBrush(FCOLOR(HighlightedText));
            else
                painter->setBrush(Colors::mid( COLOR(bg), COLOR(fg), 6, 1));
            if (option->direction == Qt::RightToLeft)
                drawArrow(Navi::W, rect, painter);
            else
                drawArrow(Navi::E, rect, painter);
        }
        RESTORE_BRUSH
    }

    // no line on the first column!
    if (firstCol)
    {
        RESTORE_PEN;
        return;
    }

    painter->setPen(Colors::mid( COLOR(bg), COLOR(fg), 40, 1));

    if (option->state & (State_Item | State_Sibling))
        painter->drawLine(mid_h, RECT.y(), mid_h, bef_v);
    if (option->state & State_Sibling)
        painter->drawLine(mid_h, aft_v, mid_h, RECT.bottom());
    if (option->state & State_Item)
    {
        if (option->direction == Qt::RightToLeft)
            painter->drawLine(RECT.left(), mid_v, bef_h, mid_v);
        else
            painter->drawLine(aft_h, mid_v, RECT.right(), mid_v);
    }
    RESTORE_PEN;
}

void
Style::drawTree(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
#ifdef QT3_SUPPORT
    ASSURE_OPTION(lv, Q3ListView);

    int i;
    if (lv->subControls & SC_Q3ListView)
        QCommonStyle::drawComplexControl(CC_Q3ListView, lv, painter, widget);
    if (!(lv->subControls & (SC_Q3ListViewBranch | SC_Q3ListViewExpand)))
        return;

    if (lv->items.isEmpty())
        return;
    QStyleOptionQ3ListViewItem item = lv->items.at(0);
    int y = lv->rect.y();
    int c;
    int dotoffset = 0;
    QPolygon dotlines;

    QPalette::ColorRole bg = QPalette::Text, fg = QPalette::Base;
    if (widget)
    { bg = widget->backgroundRole(); fg = widget->foregroundRole(); }
    QColor  cLine = Colors::mid( COLOR(bg), COLOR(fg), 40, 1),
            cIndi = Colors::mid( COLOR(bg), COLOR(fg), 6, 1),
            cIndiOpen = Colors::mid( COLOR(bg), COLOR(fg) );
            
    if ((lv->activeSubControls & SC_All) && (lv->subControls & SC_Q3ListViewExpand))
    {
        c = 2;
        dotlines.resize(2);
        dotlines[0] = QPoint(lv->rect.right(), lv->rect.top());
        dotlines[1] = QPoint(lv->rect.right(), lv->rect.bottom());
    }
    else
    {
        int linetop = 0, linebot = 0;
        // each branch needs at most two lines, ie. four end points
        dotoffset = (item.itemY + item.height - y) % 2;
        dotlines.resize(item.childCount * 4);
        c = 0;

        // skip the stuff above the exposed rectangle
        for (i = 1; i < lv->items.size(); ++i)
        {
            QStyleOptionQ3ListViewItem child = lv->items.at(i);
            if (child.height + y > 0)
                break;
            y += child.totalHeight;
        }
        int bx = lv->rect.width() / 2;

        painter->setPen(Qt::NoPen);
        while (i < lv->items.size() && y < lv->rect.height())
        {   // paint stuff in the magical area
            QStyleOptionQ3ListViewItem child = lv->items.at(i);
            if (child.features & QStyleOptionQ3ListViewItem::Visible)
            {
                int lh;
                if (!(item.features & QStyleOptionQ3ListViewItem::MultiLine))
                    lh = child.height;
                else
                    lh = painter->fontMetrics().height() + 2 * lv->itemMargin;
                    lh = qMax(lh, QApplication::globalStrut().height());
                if (lh % 2 > 0)
                    ++lh;
                linebot = y + lh / 2;
                if (child.features & QStyleOptionQ3ListViewItem::Expandable ||
                    child.childCount > 0 && child.height > 0)
                {
                    if (child.state & State_Open)
                    {
                        if (child.state & State_Selected)
                            painter->setBrush(FCOLOR(HighlightedText));
                        else
                            painter->setBrush(cIndiOpen);
                        if (option->direction == Qt::RightToLeft)
                            drawSolidArrow(Navi::SW, QRect(bx - 4, linebot - 4, 9, 9), painter);
                        else
                            drawSolidArrow(Navi::SE, QRect(bx - 4, linebot - 4, 9, 9), painter);
                    }
                    else
                    {
                        if (child.state & State_Selected)
                            painter->setBrush(FCOLOR(HighlightedText));
                        else
                            painter->setBrush(cIndi);
                        if (option->direction == Qt::RightToLeft)
                            drawArrow(Navi::W, QRect(bx - 4, linebot - 4, 9, 9), painter);
                        else
                            drawArrow(Navi::E, QRect(bx - 4, linebot - 4, 9, 9), painter);
                    }
                    // dotlinery
                    painter->setPen(cLine);
                    dotlines[c++] = QPoint(bx, linetop);
                    dotlines[c++] = QPoint(bx, linebot - 6);
                    dotlines[c++] = QPoint(bx + 5, linebot);
                    dotlines[c++] = QPoint(lv->rect.width(), linebot);
                    linetop = linebot + 11;
                    painter->setPen(Qt::NoPen);
                }
                else
                {   // just dotlinery
                    dotlines[c++] = QPoint(bx+1, linebot -1);
                    dotlines[c++] = QPoint(lv->rect.width(), linebot -1);
                }
                y += child.totalHeight;
            }
            ++i;
        }
        // Expand line height to edge of rectangle if there's any
        // visible child below
        while (i < lv->items.size() && lv->items.at(i).height <= 0)
            ++i;
        if (i < lv->items.size())
            linebot = lv->rect.height();

        if (linetop < linebot)
        {
            dotlines[c++] = QPoint(bx, linetop);
            dotlines[c++] = QPoint(bx, linebot);
        }
    }
    painter->setPen(cLine);

    int line; // index into dotlines
    if (lv->subControls & SC_Q3ListViewBranch)
    {
        for (line = 0; line < c; line += 2)
        {
            // assumptions here: lines are horizontal or vertical.
            // lines always start with the numerically lowest
            // coordinate.

            // point ... relevant coordinate of current point
            // end ..... same coordinate of the end of the current line
            // other ... the other coordinate of the current point/line
            if (dotlines[line].y() == dotlines[line+1].y())
            {
                int end = dotlines[line + 1].x(), point = dotlines[line].x(), other = dotlines[line].y();
                int i;
                while (point < end)
                {
                    i = 128;
                    if (i + point > end)
                        i = end - point;
                    painter->drawLine(point, other, point+i, other);
                    point += i;
                }
            }
            else
            {
                int end = dotlines[line + 1].y(), point = dotlines[line].y(), other = dotlines[line].x();
                int i;
                while(point < end)
                {
                    i = 128;
                    if (i + point > end)
                        i = end - point;
                    painter->drawLine(other, point, other, point+i);
                    point += i;
                }
            }
        }
    }
#endif
}
//    case PE_Q3CheckListController: // Qt 3 compatible Controller part of a list view item.

void
Style::drawRubberBand(const QStyleOption *option, QPainter *painter, const QWidget*) const
{
    painter->save();
    QColor c = FCOLOR(Highlight);
    painter->setPen(c);
    c.setAlpha(80); painter->setBrush(c);
#if 0
    if (config.fastRender)
        { c.setAlpha(80); painter->setBrush(c); }
    else
        painter->setBrush(QBrush(c, Qt::Dense4Pattern));
#endif
    painter->drawRect(RECT.adjusted(0,0,-1,-1));
    painter->restore();
}


void
Style::drawItem(const QStyleOption * option, QPainter * painter, const QWidget *widget) const
{
#if QT_VERSION >= 0x040400
    ASSURE_OPTION(item, ViewItemV4);
#else
    ASSURE_OPTION(item, ViewItemV2);
#endif
    OPT_HOVER

    const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>(widget);
    hover = hover && (!view || view->selectionMode() != QAbstractItemView::NoSelection);
    const bool selected = item->state & QStyle::State_Selected;
    QPalette::ColorRole bg = QPalette::Base, fg = QPalette::Text;
    if (view && view->viewport())
        { bg = view->viewport()->backgroundRole(); fg = view->viewport()->foregroundRole(); }
    else if (widget)
        { bg = widget->backgroundRole(); fg = widget->foregroundRole(); }

   // this could just leads to cluttered listviews...?!
//    QPalette::ColorGroup cg = item->state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
//    if (cg == QPalette::Normal && !(item->state & QStyle::State_Active))
//       cg = QPalette::Inactive;

   
    if (hover || selected)
    {
        // NOTE: single list/treeviews are typically single selected - but amarok doesn't set this..
        const QTreeView *tree = qobject_cast<const QTreeView*>(view);
        const bool single =  tree || view && view->selectionMode() == QAbstractItemView::SingleSelection;
        bool round = !tree; // looks ultimatly CRAP!
#if QT_VERSION >= 0x040400
        if  (item->viewItemPosition != QStyleOptionViewItemV4::OnlyOne)
            round = false;
#endif

        Gradients::Type gt = Gradients::None;
        if (round)
        {
            if (appType == KRunner)
                return; // ahhh... this has annoyed me from the beginning on...
            gt = hover ? Gradients::Button : Gradients::Sunken;
        }
        else if (selected && single)
            gt =  Gradients::Button;

        if (gt == Gradients::None)
        {
            const int contrast = qMax(1, Colors::contrast(FCOLOR(Highlight), COLOR(fg)));
            const QColor high = selected ? FCOLOR(Highlight) :
                                Colors::mid(COLOR(bg), FCOLOR(Highlight), 100/contrast, 4);
            painter->fillRect(RECT, high);
        }
        else
        {
            const QPixmap &fill = Gradients::pix(FCOLOR(Highlight), RECT.height(), Qt::Vertical, gt);
            if (round)
                masks.rect[true].render(RECT, painter, fill);
            else
                painter->drawTiledPixmap(RECT, fill);
        }
        // try to convince the itemview to use the proper fg color, WORKAROUND (kcategorizedview, mainly)
        painter->setPen(FCOLOR(HighlightedText));
    }
#if QT_VERSION >= 0x040400
#warning Compiling with Qt4.4 - do NOT use with lower versions
    else if (item->backgroundBrush.style() != Qt::NoBrush)
    {
        QPoint oldBO = painter->brushOrigin();
        painter->setBrushOrigin(item->rect.topLeft());
        painter->fillRect(item->rect, item->backgroundBrush);
        painter->setBrushOrigin(oldBO);
    }
#endif
    else if (item->features & QStyleOptionViewItemV2::Alternate)
        painter->fillRect(RECT, PAL.brush(QPalette::AlternateBase));
}
