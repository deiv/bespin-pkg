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

#include <QToolButton>
#include "draw.h"
#include "animator/hoverindex.h"


inline static bool
verticalTabs(QTabBar::Shape shape)
{
    return  shape == QTabBar::RoundedEast ||
            shape == QTabBar::TriangularEast ||
            shape == QTabBar::RoundedWest ||
            shape == QTabBar::TriangularWest;
}

void
Style::drawTabWidget(const QStyleOption *option, QPainter *painter, const QWidget * widget) const
{
    if (appType == GTK)
    {
        shadows.sunken[true][true].render(RECT, painter);
        return;
    }
   
    ASSURE_OPTION(twf, TabWidgetFrame);

    QLine line[2];
    QStyleOptionTabBarBase tbb;
    if (widget)
        tbb.initFrom(widget);
    else
        tbb.QStyleOption::operator=(*twf);
    tbb.shape = twf->shape; tbb.rect = twf->rect;

#define SET_BASE_HEIGHT(_o_) \
baseHeight = twf->tabBarSize._o_(); \
if (baseHeight < 0) \
    baseHeight = pixelMetric( PM_TabBarBaseHeight, option, widget )
         
    int baseHeight;
    switch (twf->shape)
    {
    case QTabBar::RoundedNorth: case QTabBar::TriangularNorth:
        SET_BASE_HEIGHT(height);
        tbb.rect.setHeight(baseHeight);
        line[0] = line[1] = QLine(RECT.bottomLeft(), RECT.bottomRight());
        line[0].translate(0,-1);
        break;
    case QTabBar::RoundedSouth: case QTabBar::TriangularSouth:
        SET_BASE_HEIGHT(height);
        tbb.rect.setTop(tbb.rect.bottom()-baseHeight);
        line[0] = line[1] = QLine(RECT.topLeft(), RECT.topRight());
        line[1].translate(0,1);
        break;
    case QTabBar::RoundedEast: case QTabBar::TriangularEast:
        SET_BASE_HEIGHT(width);
        tbb.rect.setLeft(tbb.rect.right()-baseHeight);
        line[0] = line[1] = QLine(RECT.topLeft(), RECT.bottomLeft());
        line[1].translate(1,0);
        break;
    case QTabBar::RoundedWest: case QTabBar::TriangularWest:
        SET_BASE_HEIGHT(width);
        tbb.rect.setWidth(baseHeight);
        line[0] = line[1] = QLine(RECT.topRight(), RECT.bottomRight());
        line[0].translate(-1,0);
        break;
    }
#undef SET_BASE_HEIGHT
   
    // the "frame"
    painter->save();
    painter->setPen(FCOLOR(Window).dark(120));
    painter->drawLine(line[0]);
    painter->setPen(FCOLOR(Window).light(114));
    painter->drawLine(line[1]);
    painter->restore();
    // the bar
    drawTabBar(&tbb, painter, widget);
}
#include <QtDebug>
void
Style::drawTabBar(const QStyleOption *option, QPainter *painter,
                        const QWidget * widget) const
{
    ASSURE_OPTION(tbb, TabBarBase);

    QWidget *win = 0;
    if (widget)
    {
        if (widget->parentWidget() && qobject_cast<QTabWidget*>(widget->parentWidget()))
        {
            // in general we don't want a tabbar on a tabwidget
            // that's nonsense, looks crap... and still used by some KDE apps
            // the konqueror / kdevelop guys however want the konqueror tabbar to look
            // somewhat like Bespin =)
            // so permit the proxystyle solution
            if (widget->parentWidget()->style() == this)
                return; // otherwise it's a proxystyle like on konqueror / kdevelop...
        }
        else if (qobject_cast<const QTabBar*>(widget))
            return; // usually we alter the paintevent by eventfiltering
        win = widget->window();
    }

    QRect rect = RECT.adjusted(0, 0, 0, -dpi.f2);
    int size = RECT.height(); Qt::Orientation o = Qt::Vertical;

    QRect winRect;
    if (win)
    {
        winRect = win->rect();
        winRect.moveTopLeft(widget->mapFrom(win, winRect.topLeft()));
    }
    else
        winRect = tbb->tabBarRect; // we set this from the eventfilter QEvent::Paint

    Tile::PosFlags pf = Tile::Full;
    if (verticalTabs(tbb->shape))
    {
        if (RECT.bottom() >= winRect.bottom())
            pf &= ~Tile::Bottom; // we do NEVER shape away the top - assuming deco here...!
        o = Qt::Horizontal; size = RECT.width();
    }
    else
    {
        if (RECT.width() >= winRect.width())
            pf &= ~(Tile::Left | Tile::Right);
        else
        {
            if (RECT.left() <= winRect.left()) pf &= ~Tile::Left;
            if (RECT.right() >= winRect.right()) pf &= ~Tile::Right;
        }
    }
    Tile::setShape(pf);

    masks.rect[true].render(rect, painter, GRAD(tab), o, CCOLOR(tab.std, Bg), size);
    rect.setBottom(rect.bottom()+dpi.f2);
    shadows.sunken[true][true].render(rect, painter);
    Tile::reset();
}

static int animStep = -1;

void
Style::drawTab(const QStyleOption *option, QPainter *painter,
                     const QWidget * widget) const
{
    ASSURE_OPTION(tab, Tab);

    // do we have to exclude the scrollers?
    bool needRestore = false;
    if (widget && qobject_cast<const QTabBar*>(widget))
    {
        QRegion clipRgn = painter->clipRegion();
        if (clipRgn.isEmpty())
            clipRgn = RECT;
            
        QList<QToolButton*> buttons = widget->findChildren<QToolButton*>();
        foreach (QToolButton* button, buttons)
        {
            if (button->isVisible())
                clipRgn -= QRect(button->pos(), button->size());
        }
        if (!clipRgn.isEmpty())
        {
            painter->save();
            needRestore = true;
            painter->setClipRegion(clipRgn);
        }
    }

   
    // paint shape and label
    QStyleOptionTab copy = *tab;
    // NOTICE: workaround for e.g. konsole,
    // which sets the tabs bg, but not the fg color to the palette, but just
    // presets the painter and hopes for the best... tststs
    // TODO: bug Konsole/Konqueror authors
    if (widget)
        copy.palette = widget->palette();

    copy.rect.adjust(0, F(1), 0, -F(2));

    if (appType == GTK)
    {
        QRect r = copy.rect;
        r.adjust(0, F(1), 0, -F(1));
        switch (tab->position)
        {
        default:
        case QStyleOptionTab::OnlyOneTab:
            r.adjust(F(2), 0, -F(2), 0); break;
        case QStyleOptionTab::Beginning:
            r.setLeft(r.left()+F(2));
            Tile::setShape(Tile::Full &~ Tile::Right); break;
        case QStyleOptionTab::End:
            r.setRight(r.right()-F(2));
            Tile::setShape(Tile::Full &~ Tile::Left); break;
        case QStyleOptionTab::Middle:
            Tile::setShape(Tile::Full &~ (Tile::Left | Tile::Right)); break;
        }
        shadows.raised[true][true][false].render(RECT, painter);
        masks.rect[true].render(r, painter, GRAD(tab), Qt::Vertical, CCOLOR(tab.std, Bg), r.height());
//         shadows.sunken[true][true].render(RECT, painter);
        Tile::reset();
    }

    if (tab->position != QStyleOptionTab::OnlyOneTab || appType == GTK)
    {
        OPT_SUNKEN OPT_ENABLED OPT_HOVER
        sunken = sunken || (option->state & State_Selected);
        animStep = 0;
        // animation stuff
        if (isEnabled && !sunken)
        {
            Animator::IndexInfo *info = 0;
            int index = -1, hoveredIndex = -1;
            if (widget)
            if (const QTabBar* tbar = qobject_cast<const QTabBar*>(widget))
            {
                // NOTICE: the index increment is IMPORTANT to make sure it's not "0"
                index = tbar->tabAt(RECT.topLeft()) + 1; // is the action for this item!
                hoveredIndex = hover ? index :
                tbar->tabAt(tbar->mapFromGlobal(QCursor::pos())) + 1;
                info = const_cast<Animator::IndexInfo*>
                (Animator::HoverIndex::info(widget, hoveredIndex));
            }
            if (info)
                animStep = info->step(index);
            if (hover && !animStep) animStep = 6;
        }
        drawTabShape(&copy, painter, widget);
    }
    drawTabLabel(&copy, painter, widget);
    if (needRestore)
        painter->restore();
}

void
Style::drawTabShape(const QStyleOption *option, QPainter *painter,
                          const QWidget *) const
{
    ASSURE_OPTION(tab, Tab);
    OPT_SUNKEN

    QRect rect = RECT;

    if (appType == GTK) {
        rect.translate(0, F(3));
        sunken = option->state & State_Selected;
    }
    else if (tab->position == QStyleOptionTab::OnlyOneTab)
        sunken = false;
    else
        sunken = sunken || (option->state & State_Selected);

    // maybe we're done here?!
    if (!(animStep || sunken))
        return;
       
    const int f2 = F(2);
    int size = RECT.height()+F(3);
    Qt::Orientation o = Qt::Vertical;
    const bool vertical = verticalTabs(tab->shape);
    if (vertical)
    {
        rect.adjust(F(3), F(1), -F(4), -F(1));
        o = Qt::Horizontal;
        size = RECT.width();
    }
    else
        rect.adjust(F(1), F(3), -F(1), -F(4));

    QColor c;
    if (sunken)
    {
        c = CCOLOR(tab.active, Bg);
        if (config.tab.activeTabSunken)
        {
            if (vertical)
                rect.adjust(0, F(1), 0, -f2);
            else
                rect.adjust(f2, -F(1), -f2, 0);
        }
        else
        {
            if (vertical)
                rect.adjust(-F(1), f2, F(1), -f2);
            else
                rect.adjust(f2, -F(1), -f2, F(1));
        }
    }
    else
    {
    //       c = CCOLOR(tab.std, Bg);
    //       int quota = 6 + (int) (.16 * Colors::contrast(c, CCOLOR(tab.active, 0)));
    //       c = Colors::mid(c, CCOLOR(tab.active, 0), quota, animStep);
        c = Colors::mid(CCOLOR(tab.std, Bg), CCOLOR(tab.active, Bg), 8-animStep, animStep);
    }

    Gradients::Type gt = GRAD(tab);
    if (sunken && config.tab.active_role[Bg] == config.tab.std_role[Bg])
    {   // active tab has same color as inactive one, we must do sth. on the gradient...
        if (gt == Gradients::Sunken)
            gt = Gradients::Simple;
        else
            gt = Gradients::Sunken;
    }
    masks.rect[true].render(rect, painter, gt, o, c, size, rect.topLeft());
    if (config.tab.activeTabSunken && sunken)
    {
        rect.setBottom(rect.bottom()+f2);
        shadows.sunken[true][true].render(rect, painter);
    }
}

void
Style::drawTabLabel(const QStyleOption *option, QPainter *painter,
                          const QWidget *) const
{
    ASSURE_OPTION(tab, Tab);
    OPT_SUNKEN OPT_ENABLED OPT_HOVER
    if (tab->position == QStyleOptionTab::OnlyOneTab)
    {
        sunken = false;
        hover = false;
    }
    else
        sunken = sunken || (option->state & State_Selected);
    if (sunken) hover = false;

    painter->save();
    QRect tr = RECT;

    bool verticalTabs = false;
    bool east = false;


    int alignment = Qt::AlignCenter | BESPIN_MNEMONIC;

    switch(tab->shape)
    {
    case QTabBar::RoundedEast: case QTabBar::TriangularEast:
        east = true;
    case QTabBar::RoundedWest: case QTabBar::TriangularWest:
        verticalTabs = true;
        break;
    default:
        break;
    }

    if (verticalTabs)
    {
        int newX, newY, newRot;
        if (east)
        {
            newX = tr.width(); newY = tr.y(); newRot = 90;
        }
        else
        {
            newX = 0; newY = tr.y() + tr.height(); newRot = -90;
        }
        tr.setRect(0, 0, tr.height(), tr.width());
        QMatrix m; m.translate(newX, newY); m.rotate(newRot);
        painter->setMatrix(m, true);
    }
       
    if (!tab->icon.isNull())
    {
        QSize iconSize;
        if (const QStyleOptionTabV2 *tabV2 = qstyleoption_cast<const QStyleOptionTabV2*>(tab))
            iconSize = tabV2->iconSize;
        if (!iconSize.isValid())
        {
            int iconExtent = pixelMetric(PM_SmallIconSize);
            iconSize = QSize(iconExtent, iconExtent);
        }
        QPixmap tabIcon = tab->icon.pixmap(iconSize, (isEnabled) ? QIcon::Normal : QIcon::Disabled);
        painter->drawPixmap(tr.left() + F(9), tr.center().y() - tabIcon.height() / 2 + F(1), tabIcon);
        tr.setLeft(tr.left() + iconSize.width() + F(12));
        alignment = Qt::AlignLeft | Qt::AlignVCenter | BESPIN_MNEMONIC;
    }

    // color adjustment
    QColor cF, cB;
    if (sunken)
    {
        cF = CCOLOR(tab.active, Fg);
        cB = CCOLOR(tab.active, Bg);
    }
    else if (animStep)
    {
        cF = cF = CCOLOR(tab.std, Fg);
        cB = Colors::mid(CCOLOR(tab.std, Bg ), CCOLOR(tab.active, Bg), 8-animStep, animStep);
        if (Colors::contrast(CCOLOR(tab.active, Fg), cB) > Colors::contrast(cF, cB))
            cF = CCOLOR(tab.active, Fg);
    }
    else
    {
        cB = Colors::mid(CCOLOR(tab.std, Bg), FCOLOR(Window), 2, 1);
        cF = Colors::mid(cB, CCOLOR(tab.std, Fg), 1,4);
    }

    // dark background, let's paint an emboss
    if (isEnabled)
    {
        painter->setPen(cB.dark(120));
        tr.moveTop(tr.top()-1);
        drawItemText(painter, tr, alignment, PAL, isEnabled, tab->text);
        tr.moveTop(tr.top()+1);
    }
    painter->setPen(cF);
    drawItemText(painter, tr, alignment, PAL, isEnabled, tab->text);

    painter->restore();
    animStep = -1;
}

void
Style::drawToolboxTab(const QStyleOption *option, QPainter *painter,
                            const QWidget * widget) const
{
    ASSURE_OPTION(tbt, ToolBox);

    // color fix...
    if (widget && widget->parentWidget())
        const_cast<QStyleOption*>(option)->palette = widget->parentWidget()->palette();

    drawToolboxTabShape(tbt, painter, widget);
    QStyleOptionToolBox copy = *tbt;
    copy.rect.setBottom(copy.rect.bottom()-dpi.f2);
    drawToolboxTabLabel(&copy, painter, widget);
}

void
Style::drawToolboxTabShape(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (option->state & State_Selected)
        return; // plain selected items
    OPT_HOVER OPT_SUNKEN

    QRect r = RECT; Tile::PosFlags pf = Tile::Full;
    if (const QStyleOptionToolBoxV2* tbt = qstyleoption_cast<const QStyleOptionToolBoxV2*>(option))
    {
        switch (tbt->position)
        {
        case QStyleOptionToolBoxV2::Beginning:
            pf &= ~Tile::Bottom; break;
        case QStyleOptionToolBoxV2::Middle:
            pf &= ~(Tile::Top | Tile::Bottom); break;
        case QStyleOptionToolBoxV2::End:
            pf &= ~Tile::Top; // fallthrough
        default: // single
            r.setBottom(r.bottom()-dpi.f2);
        }

        // means we touch the displayed box bottom
        switch (tbt->selectedPosition)
        {
        case QStyleOptionToolBoxV2::PreviousIsSelected:
            pf |= Tile::Top; break;
        // means we touch the displayed box top
        case QStyleOptionToolBoxV2::NextIsSelected:
            pf |= Tile::Bottom; break;
        default: break;
        }

        // handle window relative position
        if (widget)
        if (QWidget *win = widget->window())
        {
            QRect winRect = win->rect();
            winRect.moveTopLeft(widget->mapFrom(win, winRect.topLeft()));
            if (RECT.width() >= winRect.width())
                pf &= ~(Tile::Left | Tile::Right);
            else
            {
                if (RECT.left() <= winRect.left()) pf &= ~Tile::Left;
                if (RECT.right() >= winRect.right()) pf &= ~Tile::Right;
            }
        }
    }

    QColor c = CCOLOR(tab.std, Bg);
    Gradients::Type gt = GRAD(tab);
    if (gt == Gradients::Sunken) // looks freaky
        gt = Gradients::Button;
    if (sunken)
        { c = FCOLOR(Window); gt = Gradients::Sunken; }
    else if (hover)
        c = Colors::mid(CCOLOR(tab.std, Bg), CCOLOR(tab.active, Bg), 4, 1);

    Tile::setShape(pf);
    masks.rect[true].render(r, painter, gt, Qt::Vertical, c);
    Tile::setShape(pf & ~Tile::Center);
    shadows.sunken[true][true].render(RECT, painter);
    Tile::reset();
}

void
Style::drawToolboxTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *) const
{
    ASSURE_OPTION(tbt, ToolBox);
    OPT_ENABLED OPT_SUNKEN
    const bool selected = option->state & (State_Selected);

    QColor cB = FCOLOR(Window), cF = FCOLOR(WindowText);
    painter->save();
    if (selected)
        setTitleFont(painter);
    else if (!sunken)
        { cB = CCOLOR(tab.std, Bg); cF = CCOLOR(tab.std, Fg); }

    // on dark background, let's paint an emboss
    const uint tf = Qt::AlignHCenter | (selected ? Qt::AlignBottom : Qt::AlignVCenter) | BESPIN_MNEMONIC;
    if (isEnabled && Colors::value(cB) < 100)
    {
        QRect tr = RECT;
        painter->setPen(cB.dark(120));
        tr.moveTop(tr.top()-1);
        drawItemText(painter, tr, tf, PAL, isEnabled, tbt->text);
        tr.moveTop(tr.top()+1);
    }
    painter->setPen(cF);
    if (selected)
    {
        QRect rect;
        drawItemText(painter, RECT, tf, PAL, isEnabled, tbt->text, QPalette::NoRole, &rect);
        Tile::PosFlags pf = Tile::Center;
        if (option->direction == Qt::LeftToRight)
            { rect.setRight(RECT.right()); pf |= Tile::Left; }
        else
            { rect.setLeft(RECT.left()); pf |= Tile::Right; }
        shadows.line[0][Sunken].render(rect, painter, pf, true);
//         painter->drawLine(rect.x(), RECT.bottom(), RECT.right(), RECT.bottom());
    }
    else
        drawItemText(painter, RECT, tf, PAL, isEnabled, tbt->text);
    painter->restore();
}
