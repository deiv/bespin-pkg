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

#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

#include <QtDebug>
#include "colors.h"
#include "hacks.h"

using namespace Bespin;

static Hacks *bespinHacks = new Hacks;

static bool
hackMessageBox(QMessageBox* box, QEvent *e)
{
   switch (e->type()) {
   case QEvent::Paint: {
      int s = qMin(164, 3*box->height()/2);
      QStyleOption opt; opt.rect = QRect(0,0,s,s); opt.palette = box->palette();
      QStyle::StandardPixmap logo = (QStyle::StandardPixmap)0;
      switch (box->icon()){
      case QMessageBox::NoIcon:
      default:
         break;
      case QMessageBox::Question:
         logo = QStyle::SP_MessageBoxQuestion; break;
      case QMessageBox::Information:
         logo = QStyle::SP_MessageBoxInformation; break;
      case QMessageBox::Warning:
         logo = QStyle::SP_MessageBoxWarning; break;
      case QMessageBox::Critical:
         logo = QStyle::SP_MessageBoxCritical; break;
      }
      if (logo) {
         QPainter p(box);
         const int y = (box->height()-s)/2 - qMax(0,(box->height()-164)/3);
         p.drawPixmap(-s/3,y, box->style()->standardPixmap ( logo, &opt, box ));
         p.end();
      }
      return true;
   }
   case QEvent::Show: {
//       qDebug() << box->windowFlags() << "|" << Qt::Dialog << "-->" << Qt::FramelessWindowHint;
//       box->setWindowFlags ( Qt::FramelessWindowHint );
      QLabel *icon = box->findChild<QLabel*>("qt_msgboxex_icon_label");
      if (icon) {
         icon->setPixmap(QPixmap());
         icon->setFixedSize(box->height()/3,10);
      }
      QLabel *text = box->findChild<QLabel*>("qt_msgbox_label");
      if (text) {
         text->setAutoFillBackground(true);
         QPalette pal = text->palette();
         QColor c = pal.color(QPalette::Base);
         c.setAlpha(220);
         pal.setColor(QPalette::Base, c);
         text->setPalette(pal);
         text->setBackgroundRole(QPalette::Base);
         text->setForegroundRole(QPalette::Text);
         if (!text->text().contains("<h2>")) {
            QString head = "<qt><h2>" + box->windowTitle() + "</h2>";
//             switch (box->icon()){
//             case QMessageBox::NoIcon:
//             default:
//                break;
//             case QMessageBox::Question:
//                head = "<qt><h2>Question...</h2>"; break;
//             case QMessageBox::Information:
//                head = "<qt><h2>Info:</h2>"; break;
//             case QMessageBox::Warning:
//                head = "<qt><h2>Warning!</h2>"; break;
//             case QMessageBox::Critical:
//                head = "<qt><h2>Error!</h2>"; break;
//             }
            QString newText = text->text();
            newText.replace(QRegExp("^(<qt>)*"), head);
            if (!newText.endsWith("</qt>")) newText.append("</qt>");
            text->setText(newText);
            text->setMargin(4);
         }
         text->setFrameStyle ( QFrame::StyledPanel | QFrame::Sunken );
         text->setLineWidth ( 0 );
      }
      QLabel *info = box->findChild<QLabel*>("qt_msgbox_informativelabel");
      if (info) {
         info->setAutoFillBackground(true);
         QPalette pal = info->palette();
         QColor c = pal.color(QPalette::Base);
         c.setAlpha(220);
         pal.setColor(QPalette::Base, c);
         c = Colors::mid(pal.color(QPalette::Base), pal.color(QPalette::Text),1,3);
         pal.setColor(QPalette::Text, c);
         info->setPalette(pal);
         info->setBackgroundRole(QPalette::Base);
         info->setForegroundRole(QPalette::Text);
         info->setMargin(4);
      }
      return false;
   }
   default:
      return false;
   }
   return false;
}

bool
Hacks::eventFilter(QObject *o, QEvent *e)
{
   if (QMessageBox* box = qobject_cast<QMessageBox*>(o))
      return hackMessageBox(box, e);
   return false;
}

bool
Hacks::add(QWidget *w) {
   if (qobject_cast<QMessageBox*>(w)) {
      w->removeEventFilter(bespinHacks); // just to be sure
      w->installEventFilter(bespinHacks);
      return true;
   }
   return false;
}

void
Hacks::remove(QWidget *w) {
   w->removeEventFilter(bespinHacks);
}
