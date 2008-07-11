#include <QProgressBar>
#include <QTimerEvent>
#include "aprogress.h"

#include <QtDebug>

using namespace Animator;

bool animationUpdate;

INSTANCE(Progress)
MANAGE(Progress)
RELEASE(Progress)
STEP(Progress)

int
Progress::_step(const QWidget *widget, long int index) const
{
   return qAbs(info(widget, index).step(index));
}

void
Progress::timerEvent(QTimerEvent * event)
{
   if (event->timerId() != timer.timerId() || noAnimations())
      return;

   //Update the registered progressbars.
   Items::iterator iter;
   QProgressBar *pb;
   bool mkProper = false;
   animationUpdate = true;
   for (iter = items.begin(); iter != items.end(); iter++) {
       if (!iter.key())
       {
           mkProper = true;
           continue; // not a progressbar - shouldn't be in items, btw...
       }
       pb = const_cast<QProgressBar*>(qobject_cast<const QProgressBar*>(iter.key()));
       if (!pb)
            continue; // not a progressbar - shouldn't be in items, btw...
      
      if (pb->maximum() != 0 || pb->minimum() != 0 ||
          pb->paintingActive() || !pb->isVisible())
         continue; // no paint necessary
      
      ++iter.value();
      
      // dump pb geometry
      int x,y,l,t, *step = &iter.value()._step;
      if ( pb->orientation() == Qt::Vertical ) // swapped values
         pb->rect().getRect(&y,&x,&t,&l);
      else
         pb->rect().getRect(&x,&y,&l,&t);

      if (*step > l/3)
         *step = l/36-l/3;
      else if (*step == -1)
         *step = l/36-1;
      
      int s = qMin(qMax(l / 10, /*dpi.f*/16), t /*16*t/10*/);
      int ss = (10*s)/16;
      int n = l/s;
      if ( pb->orientation() == Qt::Vertical) {
         x = pb->rect().bottom(); x -= (l - n*s)/2 + ss;
//          s = -s;
      }
      else
         x += (l - n*s)/2;
//       s = qAbs(s);
      
      x += qMax(3*qAbs(*step)*n*s/l - s, 0);
      if ( pb->orientation() == Qt::Vertical )
         pb->repaint(y,x-s,s,3*s);
      else
         pb->repaint(x-s,y,3*s,s);
   }
   animationUpdate = false;
   if (mkProper)
       _release(NULL);
}
