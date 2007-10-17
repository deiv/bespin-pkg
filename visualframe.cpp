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

#include "visualframe.h"
#include <QBitmap>
#include <QCoreApplication>
#include <QFrame>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>
#include <QStyle>
#include <QWheelEvent>
#include <QTimer>

#include <QtDebug>

using namespace VFrame;

static QRegion corner[4];

VisualFramePart::VisualFramePart(QWidget * parent, QFrame *frame, Side side,
                                 uint t, int e,
                                 uint o1, uint o2, uint o3, uint o4) :
QWidget(parent)
{
   _frame = frame;
   _side = side;
   _ext = e;
   _thickness = t;
   _off[0] = o1; _off[1] = o2; _off[2] = o3; _off[3] = o4;
   connect(_frame, SIGNAL(destroyed(QObject*)), this, SLOT(hide()));
   connect(_frame, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
//    setMouseTracking ( true );
//    setAcceptDrops(true);
}

#include <QtDebug>

void
VisualFramePart::paintEvent ( QPaintEvent * event )
{
   QPainter p(this);
   p.setClipRegion(event->region(), Qt::IntersectClip);
   QStyleOption opt;
   if (_frame->frameShadow() == QFrame::Raised)
      opt.state |= QStyle::State_Raised;
   else if (_frame->frameShadow() == QFrame::Sunken)
      opt.state |= QStyle::State_Sunken;
   if (_frame->hasFocus())
      opt.state |= QStyle::State_HasFocus;
   if (_frame->isEnabled())
      opt.state |= QStyle::State_Enabled;
   opt.rect = _frame->frameRect();
   switch (_side) {
   case North:
      opt.rect.setWidth(opt.rect.width()+_off[0]+_off[1]);
      opt.rect.setHeight(opt.rect.height()+_ext);
      opt.rect.moveTopLeft(rect().topLeft());
      break;
   case South:
      opt.rect.setWidth(opt.rect.width()+_off[0]+_off[1]);
      opt.rect.setHeight(opt.rect.height()+_ext);
      opt.rect.moveBottomLeft(rect().bottomLeft());
      break;
   case West:
      opt.rect.setWidth(opt.rect.width()+_ext);
      opt.rect.setHeight(opt.rect.height()+_off[2]+_off[3]);
      opt.rect.moveTopLeft(QPoint(0, -_off[2]));
      break;
   case East:
      opt.rect.setWidth(opt.rect.width()+_ext);
      opt.rect.setHeight(opt.rect.height()+_off[2]+_off[3]);
      opt.rect.moveTopRight(QPoint(width()-1, -_off[2]));
      break;
   }
   style()->drawPrimitive(QStyle::PE_Frame, &opt, &p, this);
   p.end();
}

void
VisualFramePart::passDownEvent(QEvent *ev, const QPoint &gMousePos)
{
   // the raised frames don't look like you could click in, we'll see if this should be changed...
   if (_frame->frameShadow() == QFrame::Raised)
      return;
   QList<QWidget *> candidates = _frame->findChildren<QWidget *>();
   QList<QWidget *>::const_iterator i = candidates.constEnd();
   QWidget *match = 0;
   while (i != candidates.constBegin()) {
      --i;
      if (*i == this)
         continue;
      if ((*i)->rect().contains((*i)->mapFromGlobal(gMousePos))) {
         match = *i;
         break;
      }
   }
   if (!match) match = _frame;
   QEvent *nev = 0;
   if (ev->type() == QEvent::Wheel) {
      QWheelEvent *wev = static_cast<QWheelEvent *>(ev);
      QWheelEvent wev2( match->mapFromGlobal(gMousePos), gMousePos,
                        wev->delta(), wev->buttons(), wev->modifiers(),
                        wev->orientation() );
      nev = &wev2;
   }
   else {
      QMouseEvent *mev = static_cast<QMouseEvent *>(ev);
      QMouseEvent mev2( mev->type(), match->mapFromGlobal(gMousePos), gMousePos,
                        mev->button(), mev->buttons(), mev->modifiers() );
      nev = &mev2;
   }
   QCoreApplication::sendEvent( match, nev );
}

#define HANDLE_EVENT(_EV_) \
void VisualFramePart::_EV_ ( QMouseEvent * event ) {\
   passDownEvent((QEvent *)event, event->globalPos());\
}

HANDLE_EVENT(mouseDoubleClickEvent)
HANDLE_EVENT(mouseMoveEvent)
HANDLE_EVENT(mousePressEvent)
HANDLE_EVENT(mouseReleaseEvent)

void
VisualFramePart::wheelEvent ( QWheelEvent * event ) {
   passDownEvent((QEvent *)event, event->globalPos());
}

#undef HANDLE_EVENT

class StdChildAdd : public QObject
{
public:
   bool eventFilter( QObject *, QEvent *ev) {
      return (ev->type() == QEvent::ChildAdded);
   }
};

static StdChildAdd *stdChildAdd = 0L;

VisualFrame::VisualFrame(QFrame *parent, uint (&sizes)[4], int (&exts)[4]) :
QObject(parent)
{
   if (!(parent &&
         (sizes[0] || sizes[1] ||
          sizes[2] || sizes[3]))) {
      deleteLater();
      return;
   }
   // generate corner regions

   if (corner[North].isEmpty()) {
      int f5 = 4;
      QBitmap bm(2*f5, 2*f5);
      bm.fill(Qt::black);
      QPainter p(&bm);
      p.setPen(Qt::NoPen);
      p.setBrush(Qt::white);
      p.drawEllipse(0,0,2*f5,2*f5);
      p.end();
      QRegion circle(bm);
      corner[North] = circle & QRegion(0,0,f5,f5); // tl
      corner[South] = circle & QRegion(f5,0,f5,f5); // tr
      corner[South].translate(-corner[South].boundingRect().left(), 0);
      corner[West] = circle & QRegion(0,f5,f5,f5); // bl
      corner[West].translate(0, -corner[West].boundingRect().top());
      corner[East] = circle & QRegion(f5,f5,f5,f5); // br
      corner[East].translate(-corner[East].boundingRect().topLeft());
   }

   // create frame elements
   _frame = parent;
   _window = _frame;
   while (_window->parentWidget() &&
          !(_window->isWindow() || _window->inherits("QMdiSubWindow"))) {
      _window->installEventFilter(this);
      _window = _window->parentWidget();
   }
   
   parent->installEventFilter(stdChildAdd);
   for (int i = 0; i < 4; ++i) {
      _s[i] = sizes[i]; _e[i] = exts[i];
   }
   top = new VisualFramePart(_window, _frame, North, _s[North], _e[North],
                             _e[West], _e[East]);
   bottom = new VisualFramePart(_window, _frame, South, _s[South], _e[South],
                                _e[West], _e[East]);
   left = new VisualFramePart(_window, _frame, West, _s[West], _e[West],
                              _s[North]-_e[North], _s[South]-_e[South],
                              _s[North], _s[South]);
   right = new VisualFramePart(_window, _frame, East, _s[East], _e[East],
                               _s[North]-_e[North], _s[South]-_e[South],
                               _s[North], _s[South]);
   parent->removeEventFilter(stdChildAdd);

   // manage events
   top->installEventFilter(this);
   if (_frame->isVisible())
      show();
   else
      hide();
   QTimer::singleShot(0, this, SLOT(correctPosition()));
}

void
VisualFrame::correctPosition()
{
   QRect rect = _frame->frameRect();
   // TODO: this works around a Qt rtl (bug(?))!
   if ((_frame->layoutDirection() == Qt::RightToLeft) &&
       rect.right() != _frame->rect().right() &&
       _frame->inherits("QAbstractScrollArea"))
      rect.moveLeft(rect.x() + (_frame->rect().right() - rect.right()));
   rect.translate(_frame->mapTo(_window, QPoint(0,0)));
//    int offs = _off[0]+_off[1];

   // mask
   int x,y,r,b;
   _frame->frameRect().getRect(&x, &y, &r, &b);
   r += (x+1); b += (y+1);
   QRegion mask(_frame->rect());
   mask -= corner[North].translated(x, y); // tl
   QRect br = corner[South].boundingRect();
   mask -= corner[South].translated(r-br.width(), y); // tr
   br = corner[West].boundingRect();
   mask -= corner[West].translated(x, b-br.height()); // bl
   br = corner[East].boundingRect();
   mask -= corner[East].translated(r-br.width(), b-br.height()); // br
   _frame->setMask(mask);

   int offs = _e[West] + _e[East];
   // north element
   top->resize(rect.width()+offs, _s[North]);
   top->move(rect.x()-_e[West], rect.y()-_e[North]);

   // South element
   bottom->resize(rect.width()+offs, _s[South]);
   bottom->move(rect.x()-_e[West], rect.bottom()+1 + _e[South]-_s[South]);

   offs = (_s[North] + _s[South]) - (_e[North] + _e[South]);
   // West element
   left->resize(_s[West], rect.height()-offs);
   left->move(rect.x()-_e[West], rect.y()+_s[North]-_e[North]);

   // East element
   right->resize(_s[East], rect.height()-offs);
   right->move(rect.right()+1-_s[East]+_e[East], rect.y()+_s[North]-_e[North]);
}

void
VisualFrame::show()
{
   top->show();
   left->show();
   right->show();
   bottom->show();
}

void
VisualFrame::hide()
{
   top->hide();
   left->hide();
   right->hide();
   bottom->hide();
}

void
VisualFrame::raise()
{
   QWidgetList widgets = _window->findChildren<QWidget*>();
   QWidget *up = 0; int cnt = widgets.size()-1;
   for (int i = 0; i < cnt; ++i)
      if (widgets.at(i) == _frame) {
         up = widgets.at(i+1);
         break;
      }
   if (up) {
      top->stackUnder(up);
      left->stackUnder(up);
      right->stackUnder(up);
      bottom->stackUnder(up);
   }
   else {
      top->raise();
      left->raise();
      right->raise();
      bottom->raise();
   }
}

void
VisualFrame::repaint() {
   top->repaint();
   left->repaint();
   right->repaint();
   bottom->repaint();
}

void
VisualFrame::update()
{
   top->update();
   left->update();
   right->update();
   bottom->update();
}

bool
VisualFrame::eventFilter ( QObject * o, QEvent * ev )
{
   if (o == top) {
      if (ev->type() == QEvent::ZOrderChange)
         raise();
      return false;
   }
   
   if (ev->type() == QEvent::Move) { // for everyone between frame and parent...
      correctPosition();
      return false;
   }
   
   if (o != _frame) { // now we're only interested in frame events
//       o->removeEventFilter(this);
      return false;
   }

   if (ev->type() == QEvent::Show) {
      correctPosition();
      show();
      return false;
   }
   
   if (ev->type() == QEvent::Resize ||
      ev->type() == QEvent::LayoutDirectionChange) {
      correctPosition();
      return false;
   }
   
   if (ev->type() == QEvent::FocusIn ||
       ev->type() == QEvent::FocusOut) {
      update();
      return false;
   }
   
   if (ev->type() == QEvent::ZOrderChange) { // might be necessary for all widgets from frame to parent?
      raise();
      return false;
   }
   
   if (ev->type() == QEvent::Hide) {
      hide();
      return false;
   }
   
   return false;
//    if (ev->type() == QEvent::ParentChange) {
//       qWarning("parent changed?");
//       _frame->parentWidget() ?
//          setParent(_frame->parentWidget() ) :
//          setParent(_frame );
//       raise();
//    }
//    return false;
}
