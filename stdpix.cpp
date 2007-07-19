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
#include "bespin.h"

#define COLOR(_TYPE_) pal.color(QPalette::_TYPE_)

using namespace Bespin;

extern Dpi dpi;
extern Config config;

#include "inlinehelp.cpp"

QPixmap BespinStyle::standardPixmap ( StandardPixmap standardPixmap, const QStyleOption * option, const QWidget * widget ) const
{

   QRect rect; QPalette pal; QPixmap pm;
   const QStyleOptionTitleBar *opt = qstyleoption_cast<const QStyleOptionTitleBar *>(option);
   if (opt) {
      if (opt->rect.isNull())
         return QPixmap();
      pal = opt->palette;
      rect = opt->rect; rect.moveTo(0,0);
      pm = QPixmap(opt->rect.size());
   }
   else {
      rect = QRect(0,0,16,16);
      pal = qApp->palette();
      pm = QPixmap(rect.size());
   }

   pm.fill(Qt::transparent); // make transparent by default
   QPainter painter(&pm);
   bool sunken = false, isEnabled = false, hover = false;
   if (option) {
      sunken = option->state & State_Sunken;
      isEnabled = option->state & State_Enabled;
      hover = isEnabled && (option->state & State_MouseOver);
   }

   int numPoints = 0;
   QPoint *points;
   switch (standardPixmap) {
   case SP_DockWidgetCloseButton:
   case SP_TitleBarCloseButton: {
      numPoints = 8;
      points = new QPoint[8];
      int d = rect.height()/8, d2 = 2*rect.height()/7,
         c = rect.height()/2, s = rect.width()-d2;
      points[0] = QPoint(c,c-d); points[1] = QPoint(d2,d2);
      points[2] = QPoint(c-d,c); points[3] = QPoint(d2,s);
      points[4] = QPoint(c,c+d); points[5] = QPoint(s,s);
      points[6] = QPoint(c+d,c); points[7] = QPoint(s,d2);
   }
   case SP_TitleBarMinButton:
      if (!numPoints) {
         numPoints = 3;
         points = new QPoint[3];
         int d = rect.height()/8, y = rect.height()/2, r = rect.width();
         points[0] = QPoint(d,y);
         points[1] = QPoint(r-2*d,y-d);
         points[2] = QPoint(r-3*d,y+d);
      }
   case SP_TitleBarNormalButton:
      if (!numPoints) {
         numPoints = 4;
         points = new QPoint[4];
         int d = rect.height()/3, s = rect.width()-d;
         points[0] = QPoint(d,d); points[1] = QPoint(s,d);
         points[2] = QPoint(s,s); points[3] = QPoint(d,s);
      }
   case SP_TitleBarMaxButton: {
      if (!numPoints) {
         numPoints = 8;
         points = new QPoint[8];
         int d = rect.height()/8, c = rect.height()/2, s = rect.width()-d;
         points[0] = QPoint(c,d); points[1] = QPoint(c-d,c-d);
         points[2] = QPoint(d,c); points[3] = QPoint(c-d,c+d);
         points[4] = QPoint(c,s); points[5] = QPoint(c+d,c+d);
         points[6] = QPoint(s,c); points[7] = QPoint(c+d,c-d);
      }
      painter.setRenderHint ( QPainter::Antialiasing );
      painter.setPen(Qt::NoPen);
      Gradients::Type type = Gradients::Button;
      if (hover && !sunken) {
         const QPixmap &fill = Gradients::pix(COLOR(Window), rect.height(),
                                             Qt::Vertical, Gradients::Button);
         painter.setBrush(fill);
         painter.drawEllipse(rect);
         type = Gradients::Sunken;
      }
      const QPixmap &fill = Gradients::pix(COLOR(WindowText), rect.height(),
                                           Qt::Vertical, type);
      painter.setBrush(fill);
      painter.drawPolygon(points, numPoints);
      delete [] points;
      break;
   }
   case SP_TitleBarMenuButton: { //  0  Menu button on a title bar
      QFont fnt = painter.font();
      fnt.setPixelSize ( rect.height() );
      painter.setFont(fnt);
      painter.setPen(pal.color(QPalette::WindowText));
      painter.drawText(rect, Qt::AlignCenter, ";P");
      break;
   }
   case SP_TitleBarShadeButton: //  5  Shade button on title bars
      painter.drawPoint(rect.center().x(), rect.top());
      break;
   case SP_TitleBarUnshadeButton: //  6  Unshade button on title bars
      painter.drawPoint(rect.center().x(), rect.top());
      painter.drawPoint(rect.center());
      break;
   case SP_TitleBarContextHelpButton: { //  7  The Context help button on title bars
      QFont fnt = painter.font();
      fnt.setPixelSize ( rect.height() );
      painter.setFont(fnt);
      painter.drawText(rect, Qt::AlignCenter, "?");
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
