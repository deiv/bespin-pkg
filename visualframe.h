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

#ifndef VISUALFRAME_H
#define VISUALFRAME_H

class QFrame;
class QMouseEvent;
class QWheelEvent;
class QPaintEvent;

#include <QWidget>
#include <QPoint>

namespace VFrame
{
enum Side { North, South, West, East };
}

class VisualFramePart : public QWidget {
   Q_OBJECT
public:
   VisualFramePart(QWidget *window, QFrame *parent, VFrame::Side side,
                   uint thickness = 0, int ext = 0,
                   uint off1 = 0, uint off2 = 0, uint off3 = 0, uint off4 = 0);
   VisualFramePart(){};
   void paintEvent ( QPaintEvent * event );
protected:
//    void enterEvent ( QEvent * event ) { passDownEvent(event, event->globalPos()); }
//    void leaveEvent ( QEvent * event ) { passDownEvent(event, event->globalPos()); }
   void mouseDoubleClickEvent ( QMouseEvent * event );
   void mouseMoveEvent ( QMouseEvent * event );
   void mousePressEvent ( QMouseEvent * event );
   void mouseReleaseEvent ( QMouseEvent * event );
   void wheelEvent ( QWheelEvent * event );
private:
   QFrame *_frame; // parent, to avoid nasty casting
   void passDownEvent(QEvent *ev, const QPoint &gMousePos);
   int _thickness;
   int _off[4];
   int _ext;
   VFrame::Side _side;
};

class VisualFrame : public QObject {
   Q_OBJECT
public:
   VisualFrame( QFrame *parent, uint (&sizes)[4], int (&exts)[4] );
   bool eventFilter ( QObject * o, QEvent * ev );
public slots:
   void show();
   void hide();
   void raise();
   void update();
private:
   QFrame *_frame; // parent, to avoid nasty casting
   QWidget *_window;
   VisualFramePart *top, *bottom, *left, *right;
   uint _s[4];
   int _e[4];
private slots:
   void correctPosition();
};

#endif //VISUALFRAME_H
