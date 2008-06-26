/*
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

#ifndef MENUBAR_H
#define MENUBAR_H

#include <QGraphicsWidget>

class XBar;
class QStyleOptionMenuItem;
class QGraphicsView;

class MenuBar : public QGraphicsWidget
{
   Q_OBJECT
public:
   MenuBar ( const QString &service = QString(), qlonglong key = 0, QGraphicsItem *parent = 0, QGraphicsView *view = 0 );
   QAction *addAction(const QString & text, int idx = -1, QMenu *menu = 0);
   const QRect &actionGeometry(int idx) const;
   void removeAction(int idx);
   void changeAction(int idx, const QString & text);
   void clear();
   QAction *action(int idx) const;
//    void show();
   void hide();
   QPointF mapFromGlobal(const QPoint &pt);
   QPoint mapToGlobal(const QPointF &pt);
   inline const QString& service() const { return d.service; }
   inline qlonglong key() const { return d.key; }
   inline int openPopup() const { return d.openPopup; }
signals:
   void hovered(int);
   void triggered(int);
protected:
   void initStyleOption(QStyleOptionMenuItem *option, int idx = -1) const;
   void hoverEnterEvent(QGraphicsSceneHoverEvent *ev);
   void hoverLeaveEvent(QGraphicsSceneHoverEvent *ev);
   void hoverMoveEvent(QGraphicsSceneHoverEvent *ev);
   void mousePressEvent(QGraphicsSceneMouseEvent *ev);
   void wheelEvent(QGraphicsSceneWheelEvent *);
   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0 );
   void timerEvent(QTimerEvent *event);
   friend class XBar;
   void popDown();
   void setOpenPopup(int popup);
private:
   QAction *action(const QPoint &pos) const;
   void mouseMoved(const QPointF &pos, const QPointF &lastPos);
   int index(const QPoint &pos);
   void updateSize();
private slots:
   void popupClosed();
private:
   struct {
      QList<QAction*> actions;
      QList<QRect> actionRects;
      QString service;
      qlonglong key;
      int hoverIndex;
      int openPopup;
      QGraphicsView *view;
   } d;
};

#endif //XBAR_H
