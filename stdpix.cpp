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

#include <QStyleOptionTitleBar>
#include <QApplication>
#include <QPainter>
#include <QPen>

#include <cmath>

#include "bespin.h"
#include "colors.h"

#define COLOR(_TYPE_) pal.color(QPalette::_TYPE_)

using namespace Bespin;

extern Dpi dpi;
extern Config config;

QPixmap BespinStyle::standardPixmap ( StandardPixmap standardPixmap, const QStyleOption * option, const QWidget * widget ) const
{
   bool sunken = false, isEnabled = false, hover = false;
   if (option) {
      sunken = option->state & State_Sunken;
      isEnabled = option->state & State_Enabled;
      hover = isEnabled && (option->state & State_MouseOver);
   }
   
   QRect rect; QPalette pal;
   const QStyleOptionTitleBar *opt =
      qstyleoption_cast<const QStyleOptionTitleBar *>(option);
   if (opt) {
      if (opt->rect.isNull())
         return QPixmap();
      pal = opt->palette;
      rect = opt->rect; rect.moveTo(0,0);
      if (rect.width() > rect.height())
         rect.setWidth(rect.height());
      else
         rect.setHeight(rect.width());
   }
   else {
      rect = QRect(0,0,14,14);
      pal = qApp->palette();
   }
   const int sz = rect.width();
   const int s2 = lround(sz/2.0), s3 = lround(sz/3.0),
      s4 = lround(sz/4.0), s6 = lround(sz/6.0);

   QPixmap pm(rect.size()); pm.fill(Qt::transparent);
   QPainter painter(&pm);
   
   bool needShape = true; QVector <QRect> shape;
   
   switch (standardPixmap) {
   case SP_DockWidgetCloseButton:
   case SP_TitleBarCloseButton:
      needShape = false;
      shape << QRect(0,0,sz,s4) << QRect(0,sz-s4,sz,s4) <<
         QRect(0,s4,s4,sz-2*s4) << QRect(sz-s4,s4,s4,sz-2*s4) <<
         QRect(s3, s3, sz-2*s3, sz-2*s3);
   case SP_TitleBarMinButton:
      if (needShape) {
         needShape = false;
         shape << QRect(0,0,s4,sz) << QRect(s4,sz-s4,sz-s4,s4) << QRect(sz-s4,0,s4,s4);
      }
   case SP_TitleBarMaxButton:
      if (needShape) {
         needShape = false;
         shape << QRect(0,0,sz,s4) << QRect(sz-s4,s4,s4,sz-s4) << QRect(0,sz-s4,s4,s4);
      }
   case SP_TitleBarMenuButton:
      if (needShape) {
         needShape = false;
         shape << QRect(0,0,sz,s4) << QRect(sz-s3,s4,s3,sz-s4);
      }
   case SP_TitleBarShadeButton:
      if (needShape) {
         needShape = false;
         shape << QRect(0,0,sz,s6);
      }
   case SP_TitleBarNormalButton:
   case SP_TitleBarUnshadeButton:
      if (needShape) {
         needShape = false;
         shape << QRect(0,0,sz,s6) << QRect(0,sz-s6,sz,s6) <<
            QRect(0,s6,s6,sz-2*s6) << QRect(sz-s6,s6,s6,sz-2*s6);
      }
   case SP_TitleBarContextHelpButton: {
      if (needShape)
         shape << QRect(0,0,s2,s4) << QRect(sz-s2-s4,s4,s4,sz-2*s4-s6) <<
            QRect(sz-s2-s4,sz-s4,s4,s4);
      
      QPalette::ColorRole bg = QPalette::Window, fg = QPalette::WindowText;
      if (widget) {
         bg = widget->backgroundRole(); fg = widget->foregroundRole();
      }

      const QColor c =
         Colors::mid(pal.color(bg), pal.color(fg), (!sunken)*(4-2*hover), 1);

      painter.setPen(Qt::NoPen); painter.setBrush(c);
      for (int r = 0; r < shape.size(); ++r)
         painter.drawRect(shape.at(r));
      break;
   }
   case SP_MessageBoxInformation: { //  9  The "information" icon
      QFont fnt = painter.font(); fnt.setPixelSize ( rect.height()-2 ); painter.setFont(fnt);
      painter.setRenderHint ( QPainter::Antialiasing );
      painter.drawEllipse(rect);
      painter.drawText(rect, Qt::AlignHCenter | Qt::AlignBottom, "!");
      break;
   }
   case SP_MessageBoxWarning: { //  10  The "warning" icon
      QFont fnt = painter.font(); fnt.setPixelSize ( rect.height()-2 ); painter.setFont(fnt);
      painter.setRenderHint ( QPainter::Antialiasing );
      int hm = rect.x()+rect.width()/2;
      const QPoint points[4] = {
         QPoint(hm, rect.top()),
         QPoint(rect.left(), rect.bottom()),
         QPoint(rect.right(), rect.bottom()),
         QPoint(hm, rect.top())
      };
      painter.drawPolyline(points, 4);
      painter.drawLine(hm, rect.top()+4, hm, rect.bottom() - 6);
      painter.drawPoint(hm, rect.bottom() - 3);
      break;
   }
//    case SP_MessageBoxCritical: //  11  The "critical" icon
//       QFont fnt = painter.font(); fnt.setPixelSize ( rect.height() ); painter.setFont(fnt);
//       const QPoint points[3] =
//       {
//          QPoint(rect.width()/3, rect.top()),
//          QPoint(2*rect.width()/3, rect.top()),
//          QPoint(rect.right(), rect.height()/2),
//          QPoint(rect.right(), rect.bottom())
//       };
//       painter.drawPolyLine(points);
//       painter.drawText(rect, Qt::AlignCenter, "-");
   case SP_MessageBoxQuestion: { //  12  The "question" icon
      QFont fnt = painter.font(); fnt.setPixelSize ( rect.height() ); painter.setFont(fnt);
      painter.drawText(rect, Qt::AlignCenter, "?");
      break;
   }
//    case SP_DesktopIcon: //  13   
//    case SP_TrashIcon: //  14   
//    case SP_ComputerIcon: //  15   
//    case SP_DriveFDIcon: //  16   
//    case SP_DriveHDIcon: //  17   
//    case SP_DriveCDIcon: //  18   
//    case SP_DriveDVDIcon: //  19   
//    case SP_DriveNetIcon: //  20   
//    case SP_DirOpenIcon: //  21   
//    case SP_DirClosedIcon: //  22   
//    case SP_DirLinkIcon: //  23   
//    case SP_FileIcon: //  24   
//    case SP_FileLinkIcon: //  25   
//    case SP_FileDialogStart: //  28   
//    case SP_FileDialogEnd: //  29   
//    case SP_FileDialogToParent: //  30   
//    case SP_FileDialogNewFolder: //  31   
//    case SP_FileDialogDetailedView: //  32   
//    case SP_FileDialogInfoView: //  33   
//    case SP_FileDialogContentsView: //  34   
//    case SP_FileDialogListView: //  35   
//    case SP_FileDialogBack: //  36   
   ///    case SP_ToolBarHorizontalExtensionButton: //  26  Extension button for horizontal toolbars
   ///    case SP_ToolBarVerticalExtensionButton: //  27  Extension button for vertical toolbars
   default:
      return QCommonStyle::standardPixmap ( standardPixmap, option, widget );
   }
   painter.end();
#if 0
   QPixmapCache::insert(key, pm);
#endif
   return pm;
}

#undef COLOR
