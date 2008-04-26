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
#include <QWidget>

#define ANIMATOR_IMPL 1
#include "basic.h"

#include <QtDebug>

using namespace Animator;

Info::Info(int s, bool bwd) : _step(s), backwards(bwd) {}
int Info::step(long int) const { return _step; }
bool Info::bwd() const { return backwards; }
void Info::init(int s, bool bwd) {_step = s; backwards = bwd;}
Info defInfo;

INSTANCE(Basic)
MANAGE(Basic)
RELEASE(Basic)
STEP(Basic)
SET_FPS(Basic)

#undef ANIMATOR_IMPL

Basic::Basic() : QObject(), timeStep(_timeStep), count(0) {}

bool
Basic::_manage(QWidget *w)
{
   // just to be sure...
   disconnect(w, SIGNAL(destroyed(QObject*)), this, SLOT(release(QObject*)));
   w->removeEventFilter(this);

   connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(release(QObject*)));
   if (w->isVisible()) {
      QEvent ev(QEvent::Show);
      eventFilter(w, &ev);
   }
   w->installEventFilter(this);
   return true;
}

void
Basic::_release(QWidget *w)
{
   w->removeEventFilter(instance);
   items.remove(w);
   if (noAnimations()) {
      timer.stop();
//       delete instance; instance = 0; // nope, TODO check whether the eventFilter is used at all!
   }
}

void
Basic::play(QWidget *widget, bool bwd)
{
   if (!widget) return;
//    if (items.contains(w)) return;
   const bool needTimer = noAnimations();
   items[widget].init(0, bwd);
   if (needTimer) timer.start(timeStep, this);
}

void
Basic::_setFPS(uint fps)
{
   timeStep = 1000/fps;
   if (timer.isActive())
      timer.start(timeStep, this);
}

int
Basic::_step(const QWidget *widget, long int index) const
{
   return info(widget, index).step(index);
}

const Info &
Basic::info(const QWidget *widget, long int) const
{
   Items::const_iterator it = items.find(widget);
   if (it == items.end()) return defInfo;
   return *it;
}

bool
Basic::noAnimations() const
{
   return items.isEmpty();
}

void
Basic::release(QObject *obj)
{
   _release(qobject_cast<QWidget*>(obj));
}

void
Basic::timerEvent(QTimerEvent * event)
{
   if (event->timerId() != timer.timerId() || noAnimations())
      return;
   //Update the registered progressbars.
   QHash<const QWidget*, Info>::iterator iter;
   const QWidget *w;
   for (iter = items.begin(); iter != items.end(); iter++) {
      w = iter.key();
      if (w->paintingActive() || !w->isVisible())
         continue;
      ++iter.value();
      const_cast<QWidget*>(w)->repaint();
   }
}

bool
Basic::eventFilter( QObject* object, QEvent *e ) {
   
   QWidget* widget = qobject_cast<QWidget*>(object);
   if (!(widget && widget->isVisible()))
      return false;
   
   switch (e->type()) {
   case QEvent::MouseMove:
   case QEvent::Timer:
   case QEvent::Move:
   case QEvent::Paint:
      return false; // just for performance - they can occur really often
      
   case QEvent::Show:
      if (widget->isEnabled()) play(widget);
      return false;
   case QEvent::Hide:
      _release(widget);
      return false;
   case QEvent::EnabledChange:
      if (widget->isEnabled()) play(widget);
      else _release(widget);
      return false;
   default:
      return false;
   }
}

