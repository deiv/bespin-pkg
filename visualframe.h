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
#include <QFrame>

namespace VFrame
{
enum Side { North, South, West, East };
enum Type {Sunken, Plain, Raised};
}

class VisualFramePart : public QWidget {
   Q_OBJECT
public:
   VisualFramePart(QWidget *window, QFrame *parent, VFrame::Side side);
   VisualFramePart(){};
   void paintEvent ( QPaintEvent * event );
   inline const QFrame *frame() const {return _frame;}
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
   VFrame::Side _side;
   void passDownEvent(QEvent *ev, const QPoint &gMousePos);
};

class VisualFrame : public QObject {
   Q_OBJECT
public:
   bool eventFilter ( QObject * o, QEvent * ev );
   static void setGeometry(QFrame::Shadow shadow, const QRect &inner, const QRect &outer);
   static bool manage(QFrame *frame);
   static void release(QFrame *frame);
public slots:
   void show();
   void hide();
   void raise();
   void update();
   void repaint();
private:
   VisualFrame( QFrame *parent );
   ~VisualFrame();
   void updateShape();
   QFrame *_frame; // parent, to avoid nasty casting
   QFrame::Shape _style;
   QWidget *_window;
   VisualFramePart *top, *bottom, *left, *right;
   bool hidden;
private slots:
   void correctPosition();
};

#endif //VISUALFRAME_H
