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

#include <QAbstractButton>
#include <QStyleOptionComboBox>
#include <QStyleOptionMenuItem>
#include "bespin.h"

using namespace Bespin;
extern Dpi dpi;

extern Config config;
static const int windowsArrowHMargin = 6; // arrow horizontal margin

QSize BespinStyle::sizeFromContents ( ContentsType ct, const QStyleOption * option, const QSize & contentsSize, const QWidget * widget ) const
{
   switch ( ct ) {
//    case CT_CheckBox: // A check box, like QCheckBox
   case CT_ComboBox: // A combo box, like QComboBox
      if (const QStyleOptionComboBox *cb =
          qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
	 int hgt = contentsSize.height();
         if ( cb->frame )
            hgt += (cb->editable || config.btn.fullHover) ? dpi.f2 : dpi.f4;
         if ( !cb->currentIcon.isNull()) hgt += dpi.f2;
         return QSize(contentsSize.width()+dpi.f10+(int)(hgt/1.1), hgt);
      }
//    case CT_DialogButtons: //
//       return QSize((contentsSize.width()+16 < 80) ? 80 : contentsSize.width()+16, contentsSize.height()+10);
//    case CT_Q3DockWindow: //  
   case CT_HeaderSection: // A header section, like QHeader
      if (const QStyleOptionHeader *hdr =
          qstyleoption_cast<const QStyleOptionHeader *>(option)) {
         QSize sz;
         int margin = dpi.f2;
         int iconSize = hdr->icon.isNull() ? 0 :
                pixelMetric(QStyle::PM_SmallIconSize, hdr, widget);
         QSize txt = hdr->fontMetrics.size(0, hdr->text);
         sz.setHeight(qMax(iconSize, txt.height()) + dpi.f4);
         sz.setWidth((iconSize?margin+iconSize:0) +
                     (hdr->text.isNull()?0:margin+txt.width()) +
//                      ((hdr->sortIndicator == QStyleOptionHeader::None) ? 0 :
//                       margin+8*option->rect.height()/5) +
                     margin);
         return sz;
      }
   case CT_LineEdit: // A line edit, like QLineEdit
      return contentsSize + QSize(dpi.f4,dpi.f2);
   case CT_MenuBarItem: { // A menu bar item, like the buttons in a QMenuBar
      const int h = contentsSize.height()+dpi.f6;
      return QSize(qMax(contentsSize.width()+dpi.f18, h*8/5), h);
   }
   case CT_MenuItem: // A menu item, like QMenuItem
      if (const QStyleOptionMenuItem *menuItem =
          qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
         
         if (menuItem->menuItemType == QStyleOptionMenuItem::Separator)
            return QSize(10, menuItem->text.isEmpty() ?
                         dpi.f6 : menuItem->fontMetrics.lineSpacing());
             
         bool checkable = menuItem->menuHasCheckableItems;
         int maxpmw = config.menu.showIcons*menuItem->maxIconWidth;
         int w = contentsSize.width();
         int h = qMin(contentsSize.height(),
                      menuItem->fontMetrics.lineSpacing()) +dpi.f2;
         
         if (config.menu.showIcons && !menuItem->icon.isNull())
            h = qMax(h, menuItem->icon.pixmap(pixelMetric(PM_SmallIconSize),
                                              QIcon::Normal).height() + dpi.f2);
         if (menuItem->text.contains('\t'))
            w += dpi.f12;
         if (maxpmw > 0)
            w += maxpmw + dpi.f6;
         if (checkable)
            w += 2*(h - dpi.f4)/3 + dpi.f7;
         w += (checkable + (maxpmw > 0))*dpi.f2;
         w += dpi.f12;
         if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu)
            w += 2 * windowsArrowHMargin;
         if (menuItem->menuItemType == QStyleOptionMenuItem::DefaultItem) {
            // adjust the font and add the difference in size.
            // it would be better if the font could be adjusted in the
            // getStyleOptions qmenu func!!
            QFontMetrics fm(menuItem->font);
            QFont fontBold = menuItem->font;
            fontBold.setBold(true);
            QFontMetrics fmBold(fontBold);
            w += fmBold.width(menuItem->text) - fm.width(menuItem->text);
         }
         return QSize(w, h);
      }
      break;
   case CT_PushButton: // A push button, like QPushButton
      if (const QStyleOptionButton *btn =
          qstyleoption_cast<const QStyleOptionButton *>(option)) {
         if (btn->text.isEmpty())
//             3px for shadow & outline + 1px padding -> 4px per side
            return ( QSize( contentsSize.width() + dpi.f8, contentsSize.height() + dpi.f8 ) );
         else {
            int w = contentsSize.width() + dpi.f20;
            if (btn->features & QStyleOptionButton::HasMenu)
               w += contentsSize.height()/2+dpi.f10;
            else
            if (widget)
            if (const QAbstractButton* abn =
                qobject_cast<const QAbstractButton*>(widget))
            if (abn->isCheckable())
               w += contentsSize.height()/2+dpi.f10;
	         int h = contentsSize.height() + (config.btn.layer ? dpi.f4 : dpi.f6);
            if (!btn->icon.isNull()) {w += dpi.f10; h += dpi.f2;}
	         if (config.btn.round) { w += dpi.f8; h -= dpi.f2; }
            if (w < dpi.f80) w = dpi.f80;
            return QSize(w, h);
         }
      }
//    case CT_RadioButton: // A radio button, like QRadioButton
//    case CT_SizeGrip: // A size grip, like QSizeGrip

   case CT_Menu: // A menu, like QMenu
   case CT_Q3Header: // A Qt 3 header section, like Q3Header
   case CT_MenuBar: // A menu bar, like QMenuBar
   case CT_ProgressBar: // A progress bar, like QProgressBar
   case CT_Slider: // A slider, like QSlider
   case CT_ScrollBar: // A scroll bar, like QScrollBar
      return contentsSize;
   case CT_SpinBox: // A spin box, like QSpinBox
      return contentsSize - QSize(0, dpi.f2);
//    case CT_Splitter: // A splitter, like QSplitter
   case CT_TabBarTab: // A tab on a tab bar, like QTabBar
      if (const QStyleOptionTab *tab =
          qstyleoption_cast<const QStyleOptionTab *>(option)) {
         int add = dpi.f8;
         switch (tab->shape) {
         case QTabBar::RoundedNorth: case QTabBar::TriangularNorth:
         case QTabBar::RoundedSouth: case QTabBar::TriangularSouth:
            return contentsSize + QSize(add, 0);
         case QTabBar::RoundedEast: case QTabBar::TriangularEast:
         case QTabBar::RoundedWest: case QTabBar::TriangularWest:
            return contentsSize + QSize(0, add);
         }
      }
      return contentsSize + QSize(dpi.f8, dpi.f6);
   case CT_TabWidget: // A tab widget, like QTabWidget
      return contentsSize + QSize(dpi.f8,dpi.f6);
   case CT_ToolButton: { // A tool button, like QToolButton
      const QStyleOptionToolButton *toolbutton
         = qstyleoption_cast<const QStyleOptionToolButton *>(option);
      // get ~goldem mean ratio
      int w, h;
      if (toolbutton &&
          toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon)
         h = contentsSize.height() + dpi.f8;
      else
         h = contentsSize.height() + dpi.f6;
      w = qMax(contentsSize.width()+dpi.f4, h*16/11-dpi.f2); // 4/3 - 16/9
//      w = contentsSize.width()+dpi.f8;
      if (toolbutton && (toolbutton->subControls & SC_ToolButtonMenu))
         w += pixelMetric(PM_MenuButtonIndicator, option, widget) + dpi.f4;
      return QSize(w, h);
   }
   default: ;
   } // switch
   return QCommonStyle::sizeFromContents( ct, option, contentsSize, widget );
}
