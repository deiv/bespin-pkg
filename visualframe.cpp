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
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>
#include <QStyle>
#include <QWheelEvent>
#include <QTimer>

#define DEBUG_VF 0
#if DEBUG_VF
#include <QtDebug>
#define VDebug(_STREAM_) qDebug() << _STREAM_
#else
#define VDebug(_STREAM_) //
#define DEBUG //
#endif
#include <QtDebug>

QStyle *VisualFrame::ourStyle = 0L;

using namespace VFrame;

static QRegion corner[4];
static int sizes[3][4];
static int extends[3][4];
static int notInited = 0x7;

static VFrame::Type
type(QFrame::Shadow shadow)
{
   switch (shadow) {
   default:
   case QFrame::Plain: return VFrame::Plain;
   case QFrame::Raised: return VFrame::Raised;
   case QFrame::Sunken: return VFrame::Sunken;
   }
}

void
VisualFrame::setGeometry(QFrame::Shadow shadow, const QRect &inner, const QRect &outer)
{

   // first call, generate corner regions
   if (corner[North].isEmpty()) {
      int f5 = 4; // TODO: make this value dynamic!
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

   const Type t = type(shadow);
   notInited &= ~(1 << t);
   
   sizes[t][North] = inner.y() - outer.y();
   sizes[t][South] = outer.bottom() - inner.bottom();
   sizes[t][East] = outer.right() - inner.right();
   sizes[t][West] = inner.x() - outer.x();
   extends[t][North] = -outer.y();
   extends[t][South] = outer.bottom() - 99;
   extends[t][East] = outer.right() - 99;
   extends[t][West] = -outer.x();
}

class StdChildAdd : public QObject
{
public:
   bool eventFilter( QObject *, QEvent *ev) {
      return (ev->type() == QEvent::ChildAdded);
   }
};

static StdChildAdd stdChildAdd;

bool
VisualFrame::manage(QFrame *frame)
{
   if (!frame) return false;
   VDebug ("======= MANAGE" << frame << "===============");
   QList<VisualFrame*> vfs = frame->findChildren<VisualFrame*>();
   foreach (VisualFrame* vf, vfs) {
      VDebug ("test" << vf);
      if (vf->parent() == frame) {
         VDebug (frame << "is allready managed by" << vf);
         return false;
      }
   }
//    if (!vfs.isEmpty()) return false; // avoid double adds

   VDebug ("add new visual frame for" << frame);
   new VisualFrame(frame);
   return true;
}

void
VisualFrame::release(QFrame *frame)
{
   if (!frame) return;
   VDebug ("======= RELEASE" << frame << "===============");
   QList<VisualFrame*> vfs = frame->findChildren<VisualFrame*>();
   foreach (VisualFrame* vf, vfs) {
      VDebug ("test" << vf);
      if (vf->parent() == frame) {
         VDebug (frame << "matches" << vf << "... releasing");
         vf->hide(); vf->deleteLater();
      }
   }
}

// TODO: mange ALL frames and catch shape/shadow changes!!!
VisualFrame::VisualFrame(QFrame *parent) : QObject(parent),
top(0), bottom(0), left(0), right(0), hidden(true)
{
   if (notInited) {
      qWarning("You need to initialize the VisualFrames with\n\
               VisualFrame::setGeometry()\n\
               for all three QFrame::Shadow types first!\n\
               No Frame added.");
      deleteLater(); return;
   }
   if (!parent) {
      deleteLater(); return;
   }
   
   // create frame elements
   _frame = parent;
   _frame->installEventFilter(this);
   connect(_frame, SIGNAL(destroyed(QObject*)), this, SLOT(hide()));
   connect(_frame, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
   updateShape();
}

// VisualFrame::~VisualFrame()
// {
//    delete top; top = 0L;
//    delete bottom; bottom = 0L;
//    delete left; left = 0L;
//    delete right; right = 0L;
//    QObject::~QObject();
// }

void
VisualFrame::updateShape()
{
   _style = _frame->frameShape();
   if (_style != QFrame::StyledPanel) {
      delete top; top = 0L;
      delete bottom; bottom = 0L;
      delete left; left = 0L;
      delete right; right = 0L;

      QWidget *runner = _frame->parentWidget();
      while (runner && runner != _window) {
         runner->removeEventFilter(this);
         runner = runner->parentWidget();
      }
      hidden = true;
      return;
   }

   // we start out with _window == _frame, the actual parent widget is recalculated on each show()
   // to capture e.g. dock floats etc.
   _window = _frame;
   _window->installEventFilter(&stdChildAdd);
   top = new VisualFramePart(_window, _frame, this, North);
   bottom = new VisualFramePart(_window, _frame, this, South);
   left = new VisualFramePart(_window, _frame, this, West);
   right = new VisualFramePart(_window, _frame, this, East);
   _window->removeEventFilter(&stdChildAdd);

   // manage events
   top->installEventFilter(this);
   if (_frame->isVisible())
      show();
   else
      hide();
   QTimer::singleShot(0, this, SLOT(correctPosition()));
}

inline static QRect
correctedRect(QFrame *frame)
{
    QRect rect = frame->frameRect();
    if (frame->autoFillBackground() && frame->inherits("QLabel"))
    {
        int l,r,t,b;
        frame->getContentsMargins(&l, &t, &r, &b);
        rect.adjust(-l, -t, r, b);
    }
    // NOTICE: this works around a Qt rtl (bug(?))!
    else if (   (frame->layoutDirection() == Qt::RightToLeft) &&
                rect.right() != frame->rect().right() &&
                frame->inherits("QAbstractScrollArea") )
        rect.moveLeft(rect.x() + (frame->rect().right() - rect.right()));
    return rect;
}

void
VisualFrame::correctPosition()
{
    if (hidden) return;
    if (_style != QFrame::StyledPanel) return;

    VFrame::Type t = type(_frame->frameShadow());
    QRect rect = _frameRect = correctedRect(_frame);

    // mask
    int x,y,r,b;
    rect.getRect(&x, &y, &r, &b); r += x; b += y;
    QRegion mask(_frame->QWidget::rect());// _frame->mask().isEmpty() ? _frame->rect() : _frame->mask();
    mask -= corner[North].translated(x, y); // tl
    QRect br = corner[South].boundingRect();
    mask -= corner[South].translated(r-br.width(), y); // tr
    br = corner[West].boundingRect();
    mask -= corner[West].translated(x, b-br.height()); // bl
    br = corner[East].boundingRect();
    mask -= corner[East].translated(r-br.width(), b-br.height()); // br
    _frame->setMask(mask);

    // position
    rect.translate(_frame->mapTo(_window, QPoint(0,0)));
    rect.getRect(&x, &y, &r, &b);
    int offs = extends[t][West] + extends[t][East];

    // north element
    top->resize(rect.width()+offs, sizes[t][North]);
    top->move(x - extends[t][West], y - extends[t][North]);

    // South element
    bottom->resize(rect.width() +offs, sizes[t][South]);
    bottom->move(x - extends[t][West], rect.bottom() + 1 + extends[t][South] - sizes[t][South]);

    offs = (sizes[t][North] + sizes[t][South]) - (extends[t][North] + extends[t][South]);

    // West element
    left->resize(sizes[t][West], rect.height() - offs);
    left->move(x - extends[t][West], y + sizes[t][North] - extends[t][North]);

    // East element
    right->resize(sizes[t][East], rect.height() - offs);
    right->move(rect.right() + 1 - sizes[t][East] + extends[t][East], y + sizes[t][North] - extends[t][North]);
    //    qDebug() << _frame->frameRect() << rect << right->geometry();
}

#define PARTS(_FUNC_) top->_FUNC_; left->_FUNC_; right->_FUNC_; bottom->_FUNC_

void
VisualFrame::show()
{
    hidden = false;
    if (_style != QFrame::StyledPanel)
        return;

    QWidget *window = _frame;
    while (window->parentWidget())
    {
        window->removeEventFilter(this);
        window->installEventFilter(this);
        window = window->parentWidget();
        if ((window->isWindow() || window->inherits("QMdiSubWindow") ||
            (window != _frame && window->inherits("QAbstractScrollArea"))))
            break;
    }

    if (window != _window)
    {
        _window = window;
        _window->installEventFilter(&stdChildAdd);
        PARTS(setParent(_window));
        _window->removeEventFilter(&stdChildAdd);
    }

    correctPosition();
    PARTS(show());
}

void
VisualFrame::hide()
{
    hidden = true;
    if (_style != QFrame::StyledPanel)
        return;
    PARTS(hide());
}

void
VisualFrame::raise()
{
    if (_style != QFrame::StyledPanel)
        return;

    const bool show = !hidden;
    PARTS(setParent(_frame));
    PARTS(raise()); // ensure they're right above all other frame childs
    PARTS(setParent(_window)); // but place them back to window
    if (show)
    {   // MUST! be - Qt hides widgets on setParent()
        PARTS(show());
    }
}

void
VisualFrame::repaint()
{
    if (hidden || _style != QFrame::StyledPanel)
        return;
    PARTS(repaint());
}

void
VisualFrame::update()
{
    if (hidden || _style != QFrame::StyledPanel)
        return;
    PARTS(update());
}

#undef PARTS

bool
VisualFrame::eventFilter ( QObject * o, QEvent * ev )
{

    if (o == top)
    {   // "top" is the only monitored framepart!
        if (ev->type() == QEvent::ZOrderChange)
            raise();
        return false;
    }

    if (ev->type() == QEvent::Move)
    {   // for everyone between frame and parent...
        correctPosition();
        return false;
    }

    if (ev->type() == QEvent::ZOrderChange)
    {   // necessary for all widgets from frame to parent
        raise();
        return false;
    }

//    if (ev->type() == QEvent::ParentChange) {
//       qDebug() << "reparented" << o << _window << _frame->window();
//       return false;
//    }
   
    if (o != _frame)
    {   // now we're only interested in frame events
        return false;
    }

    if (ev->type() == QEvent::Paint)
    {
        if (_frame->frameShape() != _style)
            updateShape();
        return false;
    }

    if (_style != QFrame::StyledPanel)
        return false;

    if (ev->type() == QEvent::Show)
    {
        correctPosition();
        show();
        return false;
    }

    if (ev->type() == QEvent::Resize || ev->type() == QEvent::LayoutDirectionChange)
    {
        correctPosition();
        return false;
    }

    if (ev->type() == QEvent::FocusIn || ev->type() == QEvent::FocusOut)
    {
        update();
        return false;
    }

    if (ev->type() == QEvent::Hide)
    {
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

VisualFramePart::VisualFramePart(QWidget * parent, QFrame *frame, VisualFrame *vFrame, Side side) :
QWidget(parent), _frame(frame), _vFrame(vFrame), _side(side)
{
   connect(_frame, SIGNAL(destroyed(QObject*)), this, SLOT(hide()));
   connect(_frame, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
//    setMouseTracking ( true );
//    setAcceptDrops(true);
}

void
VisualFramePart::paintEvent ( QPaintEvent * event )
{
    if (!_vFrame->style())
        return;
    QPainter p(this);
    p.setClipRegion(event->region(), Qt::IntersectClip);
    QStyleOption opt; Type t;
    if (_frame->frameShadow() == QFrame::Raised)
    {
        opt.state |= QStyle::State_Raised;
        t = VFrame::Raised;
    }
    else if (_frame->frameShadow() == QFrame::Sunken)
    {
        opt.state |= QStyle::State_Sunken;
        t = VFrame::Sunken;
    }
    else
        t = VFrame::Plain;

    if (_frame->hasFocus())
        opt.state |= QStyle::State_HasFocus;

    if (_frame->isEnabled())
        opt.state |= QStyle::State_Enabled;

    opt.rect = _vFrame->frameRect();

    switch (_side)
    {
    case North:
        opt.rect.setWidth(opt.rect.width() + extends[t][West] + extends[t][East]);
        opt.rect.setHeight(opt.rect.height() + extends[t][North]);
        opt.rect.moveTopLeft(rect().topLeft());
        break;
    case South:
        opt.rect.setWidth(opt.rect.width() + extends[t][West] + extends[t][East]);
        opt.rect.setHeight(opt.rect.height() + extends[t][South]);
        opt.rect.moveBottomLeft(rect().bottomLeft());
        break;
    case West:
        opt.rect.setWidth(opt.rect.width() + extends[t][West]);
        opt.rect.setHeight(opt.rect.height() + sizes[t][North] + sizes[t][South]);
        opt.rect.moveTopLeft(QPoint(0, -sizes[t][North]));
        break;
    case East:
        opt.rect.setWidth(opt.rect.width() + extends[t][East]);
        opt.rect.setHeight(opt.rect.height() + sizes[t][North] + sizes[t][South]);
        opt.rect.moveTopRight(QPoint(width()-1, -sizes[t][North]));
        break;
    }
    _vFrame->style()->drawPrimitive(QStyle::PE_Frame, &opt, &p, this);
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
