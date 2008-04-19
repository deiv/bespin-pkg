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
BespinStyle::drawMenuBarBg(const QStyleOption * option, QPainter * painter,
                           const QWidget *) const
{
   if (config.menu.bar_role[Bg] != QPalette::Window ||
       config.menu.barGradient != Gradients::None)
      painter->fillRect(RECT.adjusted(0,0,0,-dpi.f2),
                        Gradients::brush(Colors::mid(FCOLOR(Window),
                                                     CCOLOR(menu.bar, Bg),1,4),
                                         RECT.height()-dpi.f2,
                                         Qt::Vertical, config.menu.barGradient));
   if (config.menu.barSunken) {
      Tile::setShape(Tile::Top | Tile::Bottom);
      shadows.sunken[false][false].render(RECT, painter);
      Tile::reset();
   }
}

void
BespinStyle::drawMenuBarItem(const QStyleOption * option, QPainter * painter,
                             const QWidget * widget) const
{
   drawMenuBarBg(option, painter, widget);
   const QStyleOptionMenuItem *mbi =
      qstyleoption_cast<const QStyleOptionMenuItem *>(option);
   if (!mbi) return;

   if (mbi->menuRect.height() > mbi->rect.height()) {
      QStyleOptionMenuItem copy = *mbi;
      copy.rect.setHeight(mbi->menuRect.height());
      drawMenuBarBg( &copy, painter, widget );
   }
   else
      drawMenuBarBg(option, painter, widget);

   B_STATES;

   ROLES(menu.active);
   hover = option->state & State_Selected;
   Animator::IndexInfo *info = 0;
   QAction *action = 0, *activeAction = 0;
   int step = 0;
   if (sunken)
      step = 6;
   else {
      if (widget)
         if (const QMenuBar* mbar =
             qobject_cast<const QMenuBar*>(widget)) {
            action = mbar->actionAt(RECT.topLeft()); // is the action for this item!
            activeAction = mbar->activeAction();
            info = const_cast<Animator::IndexInfo*>
               (Animator::HoverIndex::info(widget, (long int)activeAction));
         }
      if (info && (!activeAction || !activeAction->menu() ||
                  activeAction->menu()->isHidden()))
         step = info->step((long int)action);
   }
   QRect r = RECT.adjusted(0, dpi.f2, 0, -dpi.f4);
   if (step || hover) {
      if (!step) step = 6;
      QColor c = (config.menu.bar_role[Bg] == QPalette::Window) ?
         FCOLOR(Window) :
         Colors::mid(FCOLOR(Window), CCOLOR(menu.bar, Bg),1,4);
      c = Colors::mid(c, COLOR(ROLE[Bg]), 9-step, step);
//       int dy = 0;
//       if (!sunken) {
//          step = 6-step;
//          int dx = step*r.width()/18;
//          dy = step*r.height()/18;
//          r.adjust(dx, dy, -dx, -dy);
//          step = 6-step;
//       }
      const Gradients::Type gt =
         sunken ? Gradients::Sunken : config.menu.itemGradient;
      masks.rect[round_].render(r, painter, gt, Qt::Vertical, c, r.height()/*, QPoint(0,-dy)*/);
      if (config.menu.activeItemSunken && sunken) {
         r.setBottom(r.bottom()+dpi.f2);
         shadows.sunken[round_][true].render(r, painter);
         r.adjust(0,dpi.f1,0,-dpi.f1); // -> text repositioning
      }
      else if (step == 6 && config.menu.itemSunken)
         shadows.sunken[round_][false].render(r, painter);
   }
   QPixmap pix =
      mbi->icon.pixmap(pixelMetric(PM_SmallIconSize), isEnabled ?
                     QIcon::Normal : QIcon::Disabled);
   const uint alignment =
      Qt::AlignCenter | Qt::TextShowMnemonic |
      Qt::TextDontClip | Qt::TextSingleLine;
   if (!pix.isNull())
      drawItemPixmap(painter,r, alignment, pix);
   else
      drawItemText(painter, r, alignment, mbi->palette, isEnabled,
                   mbi->text, (hover || step > 2) ? ROLE[Fg] :
                   config.menu.bar_role[Fg]);
}

void
BespinStyle::drawMenuFrame(const QStyleOption * option, QPainter * painter,
                           const QWidget *) const
{
   if (!config.menu.shadow)
      return;
   const int f1 = dpi.f1;
   QPen pen(Colors::mid(CCOLOR(menu.std, Bg), CCOLOR(menu.std, Fg),4,1), f1);
   painter->save();
   painter->setBrush(Qt::NoBrush);
   painter->setPen(pen);
   painter->drawRect(RECT.adjusted(f1/2,f1/2,-f1,-f1));
   painter->restore();
}

static const int windowsItemFrame   = 1; // menu item frame width
static const int windowsItemHMargin = 3; // menu item hor text margin
static const int windowsItemVMargin = 1; // menu item ver text margin
static const int windowsRightBorder = 12; // right border on windows

void
BespinStyle::drawMenuItem(const QStyleOption * option, QPainter * painter,
                          const QWidget * widget) const
{
   const QStyleOptionMenuItem *menuItem =
      qstyleoption_cast<const QStyleOptionMenuItem *>(option);
   if (!menuItem) return;

   ROLES(menu.std);
   B_STATES; if (isGTK) sunken = false;
       
   // separator
   if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
      int dx = RECT.width()/10,
         dy = (RECT.height()-shadows.line[0][Sunken].thickness())/2;
      painter->save();
      const QRegion rgn =
         QRegion(RECT).subtract( painter->
                                 boundingRect( RECT, Qt::AlignCenter, menuItem->text).
                                 adjusted(-dpi.f4,0,dpi.f4,0));
      painter->setClipRegion(rgn, Qt::IntersectClip);
      shadows.line[0][Sunken].render(RECT.adjusted(dx,dy,-dx,-dy), painter);
      painter->restore();
      if (!menuItem->text.isEmpty()) {
         painter->setFont(menuItem->font);
         drawItemText(painter, RECT, Qt::AlignCenter, PAL, isEnabled,
                     menuItem->text, ROLE[Fg]);
      }
      return;
   }
       
   QRect r = RECT.adjusted(0,0,-1,-1);
   bool selected = menuItem->state & State_Selected;

   QColor bg = COLOR(ROLE[Bg]);
   QColor fg = isEnabled ? COLOR(ROLE[Fg]) :
      Colors::mid(COLOR(ROLE[Bg]), COLOR(ROLE[Fg]), 2,1);

   painter->save();
   bool checkable =
      (menuItem->checkType != QStyleOptionMenuItem::NotCheckable);
   bool checked = checkable && menuItem->checked;

   // selected bg
   if (selected && isEnabled) {
      if (ROLE[Bg] == QPalette::Window) {
         bg = Colors::mid(COLOR(ROLE[Bg]), CCOLOR(menu.active, Bg), 1, 2);
         fg = CCOLOR(menu.active, Fg);
      }
      else {
         bg = Colors::mid(COLOR(ROLE[Bg]), COLOR(ROLE[Fg]), 1, 2);
         fg = COLOR(ROLE[Bg]);
      }
      masks.rect[round_].render(r, painter, sunken ? Gradients::Sunken :
                                config.menu.itemGradient, Qt::Vertical, bg);
      if (config.menu.itemSunken)
         shadows.sunken[round_][false].render(r, painter);
//             masks.tab.outline(r, painter, QColor(0,0,0,45));
   }
       
   // Text and icon, ripped from windows style
   const QStyleOptionMenuItem *menuitem = menuItem;
   int iconCol = config.menu.showIcons*menuitem->maxIconWidth;

   if (config.menu.showIcons && !menuItem->icon.isNull()) {
      QRect vCheckRect = visualRect(option->direction, r,
                                    QRect(r.x(), r.y(), iconCol, r.height()));
      QIcon::Mode mode = isEnabled ? (selected ? QIcon::Active :
                                    QIcon::Normal) : QIcon::Disabled;
      QPixmap pixmap = menuItem->icon.pixmap(pixelMetric(PM_SmallIconSize),
                                             mode, checked ? QIcon::On : QIcon::Off);

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
                  w - xm - menuItem->menuHasCheckableItems*(cDim+dpi.f7) -
                  windowsRightBorder - tab + 1,
                  h - 2 * windowsItemVMargin);
   QRect vTextRect = visualRect(option->direction, r, textRect);
   QString s = menuitem->text;
   if (!s.isEmpty()) {
      // draw text
      int t = s.indexOf('\t');
      const int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic |
         Qt::TextDontClip | Qt::TextSingleLine;
      if (t >= 0) {
         QRect vShortcutRect = visualRect(option->direction, r,
                                          QRect(textRect.topRight(),
                                             QPoint(textRect.right()+tab, textRect.bottom())));
         painter->setPen(Colors::mid(bg, fg));
         painter->drawText(vShortcutRect, text_flags | Qt::AlignRight, s.mid(t + 1));
         painter->setPen(fg);
         s = s.left(t);
      }
      if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem) {
         QFont font = menuitem->font;
         font.setBold(true);
         painter->setFont(font);
      }
      painter->drawText(vTextRect, text_flags | Qt::AlignLeft, s.left(t));
   }
         
   // Arrow
   if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {
      // draw sub menu arrow
      Navi::Direction dir = (option->direction == Qt::RightToLeft) ?
         Navi::W : Navi::E;
      int dim = 5*r.height()/12;
      xpos = r.right() - dpi.f4 - dim;
      QStyleOptionMenuItem tmpOpt = *menuItem;
      tmpOpt.rect = visualRect(option->direction, r,
                              QRect(xpos, r.y() +
                                    (r.height() - dim)/2, dim, dim));
      painter->setBrush(Colors::mid(bg, fg, 1, 2));
      painter->setPen(painter->brush());
      drawArrow(dir, tmpOpt.rect, painter);
   }
   else if (checkable) { // Checkmark
      xpos = r.right() - dpi.f4 - cDim;
      QStyleOptionMenuItem tmpOpt = *menuItem;
      tmpOpt.rect = QRect(xpos, r.y() + (r.height() - cDim)/2, cDim, cDim);
      tmpOpt.rect = visualRect(menuItem->direction, menuItem->rect, tmpOpt.rect);
      tmpOpt.state &= ~State_Selected; // cause of color, not about checkmark!
      if (checked) {
         tmpOpt.state |= State_On;
         tmpOpt.state &= ~State_Off;
      }
      else {
         tmpOpt.state |= State_Off;
         tmpOpt.state &= ~State_On;
      }
      painter->setPen(Colors::mid(bg, fg));
      painter->setBrush(fg);
      if (menuItem->checkType & QStyleOptionMenuItem::Exclusive) {
         const int d = cDim/7;
         tmpOpt.rect.adjust(d,d,-d,-d);
         drawExclusiveCheck(&tmpOpt, painter, widget); // Radio button
      }
      else {
         // Check box
         drawMenuCheck(&tmpOpt, painter, widget);
      }
   }
   painter->restore();
}

void
BespinStyle::drawMenuScroller(const QStyleOption * option, QPainter * painter,
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

