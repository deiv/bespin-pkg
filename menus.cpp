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

#include <QAction>
#include <QMenuBar>
#include "draw.h"
#include "animator/hoverindex.h"

static const bool round_ = true;

void
Style::drawMenuBarBg(const QStyleOption * option, QPainter * painter, const QWidget *) const
{
    if (appType == Plasma)
        return;

    QRect rect = RECT;
    QColor c = FCOLOR(Window);
    if (config.menu.bar_role[Bg] != QPalette::Window)
    {
        if (config.menu.barSunken) rect.setBottom(rect.bottom()-F(2));
        c = Colors::mid(FCOLOR(Window), CCOLOR(menu.bar, Bg),1,6);
    }

    if (config.menu.barGradient != Gradients::None)
        painter->fillRect(rect, Gradients::brush(c, rect.height(), Qt::Vertical, config.menu.barGradient));
    else if (config.bg.mode == Scanlines)
        painter->fillRect(rect, Gradients::structure(c, true));
    else if (config.menu.bar_role[Bg] != QPalette::Window)
        painter->fillRect(rect, c);

    if (config.menu.barSunken)
    {
        Tile::setShape(Tile::Top | Tile::Bottom);
        shadows.sunken[false][false].render(RECT, painter);
        Tile::reset();
    }
}


void
Style::drawMenuBarItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(mbi, MenuItem);

#if 1 // was necessary once, not anymore?!
    if (appType != GTK && mbi->menuRect.height() > mbi->rect.height())
    {
        QStyleOptionMenuItem copy = *mbi;
        copy.rect.setTop(mbi->menuRect.top());
        copy.rect.setHeight(mbi->menuRect.height());
        drawMenuBarBg( &copy, painter, widget );
    }
    else
#endif
        drawMenuBarBg(option, painter, widget);

    OPT_SUNKEN OPT_ENABLED OPT_HOVER
    QPalette::ColorRole bg = config.menu.active_role[Bg];
    QPalette::ColorRole fg = config.menu.active_role[Fg];

    hover = option->state & State_Selected;
    Animator::IndexInfo *info = 0;
    int step = 0;
    if (sunken)
        step = 6;
    else
    {   // check for hover animation ==========================
        if (const QMenuBar* mbar = qobject_cast<const QMenuBar*>(widget))
        {
            QAction *action = mbar->actionAt(RECT.topLeft()); // is the action for this item!
            QAction *activeAction = mbar->activeAction();
            info = const_cast<Animator::IndexInfo*>(Animator::HoverIndex::info(widget, (long int)activeAction));
            if (info && (!(activeAction && activeAction->menu()) || activeAction->menu()->isHidden()))
                step = info->step((long int)action);
        }
        else if (appType == Plasma && widget)
        {
            // i abuse this property as xbar menus are no menus and the active state is s thing of it's own...
            int action = (mbi->menuItemType & 0xffff);
            int activeAction = ((mbi->menuItemType >> 16) & 0xffff);
            info = const_cast<Animator::IndexInfo*>(Animator::HoverIndex::info(widget, activeAction));
            if (info)
                step = info->step(action);
        }
        // ================================================
    }
    
    QRect r = RECT.adjusted(0, F(2), 0, -F(4));
    if (step || hover)
    {
        if (!step)
            step = 6;
        QColor c;
        if (appType == Plasma)
        {   // NOTICE: opaque Colors::mid() are too flickerious with several plasma bgs...
            bg = QPalette::WindowText;
            fg = QPalette::Window;
        }
        c = COLOR(bg);
        c.setAlpha(step*255/8);
        // NOTICE: scale code - currently unused
//       int dy = 0;
//       if (!sunken) {
//          step = 6-step;
//          int dx = step*r.width()/18;
//          dy = step*r.height()/18;
//          r.adjust(dx, dy, -dx, -dy);
//          step = 6-step;
//       }
        const Gradients::Type gt = sunken ? Gradients::Sunken : config.menu.itemGradient;
        masks.rect[round_].render(r, painter, gt, Qt::Vertical, c, r.height()/*, QPoint(0,-dy)*/);
        if (config.menu.activeItemSunken && sunken)
        {
            r.setBottom(r.bottom()+F(1));
            shadows.sunken[round_][true].render(r, painter);
            r.adjust(0,F(1),0,-F(1)); // -> text repositioning
        }
        else if (step == 6 && config.menu.itemSunken)
            shadows.sunken[round_][false].render(r, painter);
    }
    QPalette::ColorRole fg2 = (appType == Plasma) ? QPalette::WindowText : config.menu.bar_role[Fg];
    QPixmap pix = mbi->icon.pixmap(pixelMetric(PM_SmallIconSize), isEnabled ? QIcon::Normal : QIcon::Disabled);
    const uint alignment = Qt::AlignCenter | BESPIN_MNEMONIC | Qt::TextDontClip | Qt::TextSingleLine;
    if (!pix.isNull())
        drawItemPixmap(painter,r, alignment, pix);
    else
        drawItemText(painter, r, alignment, mbi->palette, isEnabled, mbi->text, (hover || step > 3) ? fg : fg2);
}

static const int windowsItemFrame   = 1; // menu item frame width
static const int windowsItemHMargin = 3; // menu item hor text margin
static const int windowsItemVMargin = 1; // menu item ver text margin
static const int windowsRightBorder = 12; // right border on windows

void
Style::drawMenuItem(const QStyleOption * option, QPainter * painter,
                          const QWidget * widget) const
{
    ASSURE_OPTION(menuItem, MenuItem);
    ROLES(menu.std);
    OPT_SUNKEN OPT_ENABLED

    if (appType == GTK)
        sunken = false;

    if (menuItem->menuItemType == QStyleOptionMenuItem::Separator)
    {   // separator ===============================
        int dx = RECT.width()/10,
            dy = (RECT.height()-shadows.line[0][Sunken].thickness())/2;
        painter->save();
        const QRegion rgn = QRegion(RECT).subtract( painter->
                                                    boundingRect( RECT, Qt::AlignCenter, menuItem->text).
                                                    adjusted(-F(4),0,F(4),0));
        painter->setClipRegion(rgn, Qt::IntersectClip);
        shadows.line[0][Sunken].render(RECT.adjusted(dx,dy,-dx,-dy), painter);
        painter->restore();
        if (!menuItem->text.isEmpty())
        {
            setBold(painter);
            drawItemText(painter, RECT, Qt::AlignCenter, PAL, isEnabled, menuItem->text, ROLE[Fg]);
            painter->setFont(menuItem->font);
        }
        return;
    }
       
    QRect r = RECT.adjusted(0,0,-1,-1);
    bool selected = isEnabled && menuItem->state & State_Selected;

    QColor bg = PAL.color(QPalette::Active, ROLE[Bg]);
    QColor fg = isEnabled ? COLOR(ROLE[Fg]) : Colors::mid(bg, PAL.color(QPalette::Active, ROLE[Fg]), 2,1);

    painter->save();
    bool checkable = (menuItem->checkType != QStyleOptionMenuItem::NotCheckable);
    bool checked = checkable && menuItem->checked;

    // selected bg
    if (selected)
    {
        if (config.menu.itemGradient != Gradients::None ||
            config.menu.bar_role[Bg] == ROLE[Bg] ||
            Colors::contrast(COLOR(ROLE[Bg]), CCOLOR(menu.active, Bg)) > 8) // enough to indicate hover
        {
            bg = Colors::mid(COLOR(ROLE[Bg]), CCOLOR(menu.active, Bg), 1, 6);
            fg = CCOLOR(menu.active, Fg);
        }
        else
        {
            bg = Colors::mid(COLOR(ROLE[Bg]), COLOR(ROLE[Fg]), 1, 2);
            fg = COLOR(ROLE[Bg]);
        }
        if (config.menu.itemGradient == Gradients::Glass || config.menu.itemGradient == Gradients::Gloss)
            Tile::setShape(Tile::Top|Tile::Bottom|Tile::Center);
        masks.rect[round_].render( r, painter, sunken ? Gradients::Sunken : config.menu.itemGradient, Qt::Vertical, bg);
        if (sunken && config.menu.itemSunken)
            shadows.sunken[round_][false].render(r, painter);
        Tile::reset();
    }
       
    // Text and icon, ripped from windows style
    const QStyleOptionMenuItem *menuitem = menuItem;
    int iconCol = config.menu.showIcons*menuitem->maxIconWidth;

    if (config.menu.showIcons && !menuItem->icon.isNull())
    {
        QRect vCheckRect =
        visualRect(option->direction, r, QRect(r.x(), r.y(), iconCol, r.height()));
        QIcon::Mode mode =
        isEnabled ? (selected ? QIcon::Active : QIcon::Normal) : QIcon::Disabled;
        QPixmap pixmap =
        menuItem->icon.pixmap(pixelMetric(PM_SmallIconSize), mode, checked ? QIcon::On : QIcon::Off);

        QRect pmr(QPoint(0, 0), pixmap.size());
        pmr.moveCenter(vCheckRect.center());

        painter->drawPixmap(pmr.topLeft(), pixmap);
    }
       
    painter->setPen ( fg );
    painter->setBrush ( Qt::NoBrush );

    int x, y, w, h;
    r.getRect(&x, &y, &w, &h);
    int tab = menuitem->tabWidth;
    int cDim = (r.height() - dpi.f6);
    int xm = windowsItemFrame + iconCol + windowsItemHMargin;
    int xpos = r.x() + xm;
    QRect textRect(xpos, y + windowsItemVMargin,
                    w - xm - menuItem->menuHasCheckableItems*(cDim+dpi.f7) - windowsRightBorder - tab + 1,
                    h - 2 * windowsItemVMargin);
    QRect vTextRect = visualRect(option->direction, r, textRect);
    QString s = menuitem->text;

    if (!s.isEmpty())
    {   // draw text
        int t = s.indexOf('\t');
        const int text_flags = Qt::AlignVCenter | BESPIN_MNEMONIC | Qt::TextDontClip | Qt::TextSingleLine;
        if (t >= 0)
        {
            QRect vShortcutRect = visualRect(option->direction, r, QRect(textRect.topRight(),
                                             QPoint(textRect.right()+tab, textRect.bottom())));
            painter->setPen(Colors::mid(bg, fg));
            drawItemText(painter, vShortcutRect, text_flags | Qt::AlignRight, PAL, isEnabled, s.mid(t + 1));
//          painter->drawText(vShortcutRect, text_flags | Qt::AlignRight, s.mid(t + 1));
            painter->setPen(fg);
            s = s.left(t);
        }
        if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem)
            setBold(painter);
        drawItemText(painter, vTextRect, text_flags | Qt::AlignLeft, PAL, isEnabled, s);
//       painter->drawText(vTextRect, text_flags | Qt::AlignLeft, s.left(t));
    }
         
    if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu)
    {   // draw sub menu arrow ================================

        Navi::Direction dir = (option->direction == Qt::RightToLeft) ? Navi::W : Navi::E;
        int dim = 5*r.height()/12;
        xpos = r.right() - dpi.f4 - dim;
        QStyleOptionMenuItem tmpOpt = *menuItem;
        tmpOpt.rect = visualRect(option->direction, r, QRect(xpos, r.y() + (r.height() - dim)/2, dim, dim));
        painter->setBrush(Colors::mid(bg, fg, 1, 2));
        painter->setPen(QPen(painter->brush(), painter->pen().widthF()));
        drawArrow(dir, tmpOpt.rect, painter);
    }
    else if (checkable)
    {   // Checkmark =============================
        xpos = r.right() - dpi.f4 - cDim;
        QStyleOptionMenuItem tmpOpt = *menuItem;
        tmpOpt.rect = QRect(xpos, r.y() + (r.height() - cDim)/2, cDim, cDim);
        tmpOpt.rect = visualRect(menuItem->direction, menuItem->rect, tmpOpt.rect);
        tmpOpt.state &= ~State_Selected; // cause of color, not about checkmark!
        if (checked)
        {
            tmpOpt.state |= State_On;
            tmpOpt.state &= ~State_Off;
        }
        else
        {
            tmpOpt.state |= State_Off;
            tmpOpt.state &= ~State_On;
        }
        painter->setPen(Colors::mid(bg, fg));
        painter->setBrush(fg);
        if (menuItem->checkType & QStyleOptionMenuItem::Exclusive)
        {
            const int d = cDim/7;
            tmpOpt.rect.adjust(d,d,-d,-d);
            drawExclusiveCheck(&tmpOpt, painter, widget); // Radio button
        }
        else // Check box
            drawMenuCheck(&tmpOpt, painter, widget);
    }
    painter->restore();
}

void
Style::drawMenuScroller(const QStyleOption * option, QPainter * painter,
                              const QWidget *) const
{
   OPT_SUNKEN
      
   QPoint offset;
   Navi::Direction dir = Navi::N;
   QPalette::ColorRole bg = config.menu.std_role[0];
   const QPixmap &gradient = Gradients::pix(PAL.color(QPalette::Active, bg),
                                            RECT.height()*2, Qt::Vertical,
                                            sunken ? Gradients::Sunken :
                                            Gradients::Button);
   if (option->state & State_DownArrow) {
      offset = QPoint(0,RECT.height());
      dir = Navi::S;
   }
   painter->drawTiledPixmap(RECT, gradient, offset);
   drawArrow(dir, RECT, painter);
}

//    case CE_MenuTearoff: // A menu item representing the tear off section of a QMenu

