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

// TODO remove tabwidget / replace with stackedwidget only...!?
#include <QAbstractScrollArea>
#include <QApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QStyleOption>
#include <QStackedWidget>

#include <cmath>

#include "../oxrender.h"

#define ANIMATOR_IMPL 1
#include "tab.h"

#include <QtDebug>

using namespace Animator;

INSTANCE(Tab)
MANAGE(Tab)
RELEASE(Tab)
SET_FPS(Tab)

#undef ANIMATOR_IMPL

static inline QAbstractScrollArea*
scrollAncestor(QWidget *w, QWidget *root)
{
   QWidget *parent = w;
   while (parent != root && (parent = parent->parentWidget())) {
      if (qobject_cast<QAbstractScrollArea*>(parent)) break;
   }
   if (parent != root) return static_cast<QAbstractScrollArea*>(parent);
   return 0L;
}

// to get an idea about what the bg of out tabs looks like - seems as if we
// need to paint it
static QPixmap dumpBackground(QWidget *target, const QRect &r, const QStyle *style) {
   if (!target) return QPixmap();
   QPoint zero(0,0);
   QPixmap pix(r.size());
   QWidgetList widgets; widgets << target;
   QWidget *w = target->parentWidget();
   while (w) {
      if (!w->isVisible()) { w = w->parentWidget(); continue; }
      widgets << w;
      if (w->isTopLevel() || w->autoFillBackground()) break;
      w = w->parentWidget();
   }
   
   QPainter p(&pix);
   const QBrush bg = w->palette().brush(w->backgroundRole());
   if (bg.style() == Qt::TexturePattern)
      p.drawTiledPixmap(pix.rect(), bg.texture(), target->mapTo(w, r.topLeft()));
   else
      p.fillRect(pix.rect(), bg);
   
   if (w->isTopLevel() && w->testAttribute(Qt::WA_StyledBackground)) {
      QStyleOption opt; opt.initFrom(w);// opt.rect = r;
      opt.rect.translate(-target->mapTo(w, r.topLeft()));
      style->drawPrimitive ( QStyle::PE_Widget, &opt, &p, w);
   }
   p.end();
   
   QPaintEvent e(r); int i = widgets.size();
   while (i) {
      w = widgets.at(--i);
      QPainter::setRedirected( w, &pix, target->mapTo(w, r.topLeft()) );
      e = QPaintEvent(QRect(zero, r.size()));
      QCoreApplication::sendEvent(w, &e);
      QPainter::restoreRedirected(w);
   }
   return pix;
}

// QPixmap::grabWidget(.) currently fails on the background offset,
// so we use our own implementation
static void
grabWidget(QWidget * root, QPixmap *pix)
{
   if (!root)
      return;
   
   QPoint zero(0,0);
   QSize sz = root->window()->size();
   
   QWidgetList widgets = root->findChildren<QWidget*>();
   
   // resizing (in case) -- NOTICE may be dropped for performance...?!
//    if (root->testAttribute(Qt::WA_PendingResizeEvent) ||
//        !root->testAttribute(Qt::WA_WState_Created)) {
//       QResizeEvent e(root->size(), QSize());
//       QApplication::sendEvent(root, &e);
//    }
//    foreach (QWidget *w, widgets) {
//       if (root->testAttribute(Qt::WA_PendingResizeEvent) ||
//          !root->testAttribute(Qt::WA_WState_Created)) {
//          QResizeEvent e(w->size(), QSize());
//          QApplication::sendEvent(w, &e);
//       }
//    }
   
   // painting ------------
   QPainter::setRedirected( root, pix );
   QPaintEvent e(QRect(zero, root->size()));
   QCoreApplication::sendEvent(root, &e);
   QPainter::restoreRedirected(root);
   
   bool hasScrollAreas = false;
   QAbstractScrollArea *scrollarea = 0;
   QPainter p; QRegion rgn;
   QPixmap *saPix = 0L;

   foreach (QWidget *w, widgets) {
      if (w->isVisibleTo(root)) {
         // solids
         if (w->autoFillBackground()) {
            const QBrush bg = w->palette().brush(w->backgroundRole());
            p.begin(pix);
            QRect wrect = QRect(zero, w->size()).translated(w->mapTo(root, zero));
            if (bg.style() == Qt::TexturePattern)
               p.drawTiledPixmap(wrect, bg.texture(),
                                 w->mapTo(root->window(), zero));
            else
               p.fillRect(wrect, bg);
            p.end();
         }
         
         // scrollarea workaround
         if ((scrollarea = qobject_cast<QAbstractScrollArea*>(w)))
            hasScrollAreas = true;
         if (hasScrollAreas && !qobject_cast<QScrollBar*>(w) &&
             (scrollarea = scrollAncestor(w, root))) {
                QRect rect = scrollarea->frameRect();
                rect.translate(scrollarea->mapTo(root, zero));
                if (!saPix || saPix->size() != rect.size()) {
                   delete saPix; saPix = new QPixmap(rect.size());
                }
                p.begin(saPix); p.drawPixmap(zero, *pix, rect);
                p.end();
                const QPoint &pt = scrollarea->frameRect().topLeft();
                w->render(saPix, w->mapTo(scrollarea, pt), w->rect(), 0);
                p.begin(pix); p.drawPixmap(rect.topLeft(), *saPix); p.end();
             }
         // default painting redirection
         else
            w->render(pix, w->mapTo(root, zero), w->rect(), 0);
      }
   }
   delete saPix;
}

static uint _duration = 350;
static Transition _transition = SlideIn;

class Animator::Curtain : public QWidget
{
public:
   Curtain(TabInfo *info, QWidget *parent) : QWidget(parent), _info(info)
   {
      setAttribute(Qt::WA_NoSystemBackground);
      raise();
   }
protected:
   void paintEvent( QPaintEvent *  )
   {
      if (_info->clock.isNull()) return; // should not happen
      QPainter p(this);
      p.drawPixmap(0,0, _info->tabPix[2]);
      p.end();
   }
private:
   TabInfo *_info;
};

class StdChildAdd : public QObject
{
   public:
      bool eventFilter( QObject *, QEvent *ev) {
         return (ev->type() == QEvent::ChildAdded);
      }
};


TabInfo::TabInfo(QObject* parent, QWidget *current, int idx) :
QObject(parent), curtain(0), progress(0.0), currentWidget(current), index(idx) {}

bool
TabInfo::proceed()
{
   if (clock.isNull()) // this tab is currently not animated
      return false;
   
   // check if our desired duration has exceeded and stop this in case
   uint ms = clock.elapsed();
   if (ms > duration - _timeStep) {
      rewind();
      return false;
   }
   
   // normal action
   updatePixmaps(_transition, ms);
   curtain->repaint();
   return true;  // for counter
}

void
TabInfo::rewind()
{
   clock = QTime(); // reset clock, this is IMPORTANT!
   delete curtain; curtain = 0; // get rid of curtain, and RESHOW CONTENT!
   tabPix[0] = tabPix[1] = tabPix[2] = QPixmap(); // reset pixmaps, saves space
}

void
TabInfo::switchTab(QStackedWidget *sw, int newIdx)
{
   progress = 0.0;
   // update from/to indices
   //    const int oldIdx = tai->index; // just for debug out later on
   QWidget *ow = sw->widget(index);
   QWidget *cw = sw->widget(newIdx);
   currentWidget = cw;
   index = newIdx;

   #define _RESET_SIZE_
   #define AVOID(_COND_) if (_COND_) { _RESET_SIZE_ rewind(); return; } //
   #define TOO_SLOW clock.elapsed() > (int)(_duration - _timeStep)
   
   AVOID(!ow); // this is the first time the tab changes, nothing to blend
   AVOID(ow == cw); // this can happen on destruction etc... and thus lead to segfaults...

// this works around a possible bug in some handcrafted scrollareas?
// not sure, but as soon as i call window->render(.) on them, the window resizes
// worse: this behaviour seems to be arbitrary (happens e.g. with kate/gwenview, but not everytime...)
// for the moment we just fix the window size during the grab and release it afterards, works, but:
// TODO: remove this once widget->render(.) does no more trigger resizes!
#undef _RESET_SIZE_
#define _RESET_SIZE_ window->setMinimumSize(minSz); window->setMaximumSize(maxSz);
   // fix the window size
   QWidget *window = sw->window();
   QSize minSz = window->minimumSize(), maxSz = window->maximumSize();
   window->setFixedSize(window->size());
//-----------------------------------------------------------------

   // prepare the pixmaps we use to pretend the animation
   QRect contentsRect(ow->mapTo(sw, QPoint(0,0)), ow->size());
   tabPix[1] = dumpBackground(sw, contentsRect, qApp->style());
   
   if (clock.isNull()) {
      clock.start();
      tabPix[0] = tabPix[1];
      grabWidget(ow, &tabPix[0]);
      tabPix[2] = tabPix[0];
      AVOID(TOO_SLOW);
   }
   else { // humm?? very fast tab change... maybe the user changed his mind...
      clock.restart();
      tabPix[0] = tabPix[2];
   }
   
   grabWidget(cw, &tabPix[1]);
   AVOID(TOO_SLOW);

   duration = _duration - clock.elapsed() + _timeStep;
   clock.restart(); clock.addMSecs(_timeStep);
   updatePixmaps(_transition, _timeStep);

   // make curtain and first update ----------------
   if (!curtain) {
      // prevent w from doing freaky things with the curtain
      // (e.g. QSplitter would add a new section...)
      StdChildAdd *stdChildAdd = new StdChildAdd;
      sw->installEventFilter(stdChildAdd);
      
      curtain = new Curtain(this, sw);
      curtain->move(contentsRect.topLeft());
      curtain->resize(contentsRect.size());
      curtain->show();

      sw->removeEventFilter(stdChildAdd);
      delete stdChildAdd;
   }
   else
      curtain->raise();

   _RESET_SIZE_

}

void
TabInfo::updatePixmaps(Transition transition, uint ms)
{
   switch (transition) {
      #ifndef QT_NO_XRENDER
      default:
      case CrossFade: {
         float quote = (float)_timeStep / (duration-ms);
//          progress += quote;
         OXRender::blend(tabPix[1], tabPix[2], quote);
         break;
      }
      #else
      default:
      #endif
      case ScanlineBlend: {
         QPainter p(&tabPix[2]);
         const int numStep = duration/_timeStep;
         const int h = lround(_timeStep * (numStep-progress) / (duration-ms));
         for (int i = (int)progress; i < tabPix[2].height(); i+=numStep)
            p.drawPixmap(0, i, tabPix[1], 0, i, tabPix[1].width(), h);
         progress += h;
         break;
      }
      case SlideIn: {
         //TODO handle different bar positions (currently assumes top)
         QPainter p(&tabPix[2]);
         const int h = ms*tabPix[1].height()/duration;
         p.drawPixmap(0, 0, tabPix[1], 0, tabPix[1].height() - h, tabPix[1].width(), h);
         break;
      }
      case SlideOut: {
         tabPix[2] = tabPix[1];
         //TODO handle different bar positions (currently assumes top)
         QPainter p(&tabPix[2]);
         int off = ms*tabPix[0].height()/duration;
         p.drawPixmap(0, 0, tabPix[0], 0, off, tabPix[0].width(), tabPix[0].height() - off);
         break;
      }
      case RollIn: {
         QPainter p(&tabPix[2]);
         int h = ms*tabPix[1].height()/(2*duration);
         p.drawPixmap(0, 0, tabPix[1], 0, 0, tabPix[1].width(), h);
         p.drawPixmap(0, tabPix[1].height()-h, tabPix[1], 0, tabPix[1].height()-h, tabPix[1].width(), h);
         break;
      }
      case RollOut: {
         QPainter p(&tabPix[2]);
         int h = ms*tabPix[1].height()/duration;
         int y = (tabPix[1].height()-h)/2;
         p.drawPixmap(0, y, tabPix[1], 0, y, tabPix[1].width(), h);
         break;
      }
      case OpenVertically: {
         tabPix[2] = tabPix[1];
         QPainter p(&tabPix[2]);
         const int off = ms*tabPix[0].height()/(2*duration);
         const int h2 = tabPix[0].height()/2;
         p.drawPixmap(0,0, tabPix[0], 0,off, tabPix[0].width(),h2 - off);
         p.drawPixmap(0,h2+off, tabPix[0], 0,h2, tabPix[0].width(),tabPix[0].height()-off);
         break;
      }
      case CloseVertically: {
         QPainter p(&tabPix[2]);
         int h = ms*tabPix[1].height()/(2*duration);
         p.drawPixmap(0, 0, tabPix[1], 0, tabPix[1].height()/2-h, tabPix[1].width(), h);
         p.drawPixmap(0, tabPix[1].height()-h, tabPix[1], 0, tabPix[1].height()/2, tabPix[1].width(), h);
         break;
      }
      case OpenHorizontally: {
         tabPix[2] = tabPix[1];
         QPainter p(&tabPix[2]);
         const int off = ms*tabPix[0].width()/(2*duration);
         const int w2 = tabPix[0].width()/2;
         p.drawPixmap(0,0,tabPix[0],off,0, w2-off,tabPix[0].height());
         p.drawPixmap(w2+off,0,tabPix[0], w2,0,tabPix[0].width()-off,tabPix[0].height());
         break;
      }
      case CloseHorizontally: {
         QPainter p(&tabPix[2]);
         int w = ms*tabPix[1].width()/(2*duration);
         p.drawPixmap(0, 0, tabPix[1], tabPix[1].width()/2-w, 0, w, tabPix[1].height());
         p.drawPixmap(tabPix[1].width()-w, 0, tabPix[1], tabPix[1].width()/2, 0, w, tabPix[1].height());
         break;
      }
   }
}

void
Tab::setDuration(uint ms)
{
   _duration = ms;
}

void
Tab::setTransition(Transition t)
{
   _transition = t;
}

Tab::Tab() : Basic(), _activeTabs(0)
{
   timeStep = _timeStep; // yes! otherwise we'd inherit general timestep
}

bool
Tab::_manage (QWidget* w)
{
   // the tabs need to be kept in a list, as currentChanged() does not allow us
   // to identify the former tab... unfortunately.
   QStackedWidget *sw = qobject_cast<QStackedWidget*>(w);
   if (!sw) return false;
   connect(sw, SIGNAL(currentChanged(int)), this, SLOT(changed(int)));
   connect(sw, SIGNAL(destroyed(QObject*)), this, SLOT(release(QObject*)));
   items.insert(sw, new TabInfo(this, sw->currentWidget(), sw->currentIndex()));
   return true;
}

void
Tab::_release(QWidget *w)
{
   QStackedWidget *sw = qobject_cast<QStackedWidget*>(w);
   if (!sw) return;

   disconnect(sw, SIGNAL(currentChanged(int)), this, SLOT(changed(int)));
   items.remove(sw);
   if (items.isEmpty()) timer.stop();
}

void
Tab::changed(int index)
{
   if (_transition == Jump)
      return; // ugly nothing ;)

   // ensure this is a qtabwidget - we'd segfault otherwise
   QStackedWidget *sw = qobject_cast<QStackedWidget*>(sender());
   if (!(sw && sw->isVisible())) return;

   // find matching tabinfo
   Items::iterator i = items.find(sw);
   if (i == items.end())
      return; // not handled... why ever (i.e. should not happen by default)
   // init transition
   i.value()->switchTab(sw, index);
   
   // _activeTabs is counted in the timerEvent(), so if this is the first
   // changing tab in a row, it's currently '0'
   if (!_activeTabs) timer.start(timeStep, this);
}


void
Tab::timerEvent(QTimerEvent *event)
{
   if (event->timerId() != timer.timerId() || items.isEmpty())
      return;
   
   Items::iterator i;
   _activeTabs = 0; // reset counter
   for (i = items.begin(); i != items.end(); i++) {
      if (i.value()->proceed())
         ++_activeTabs;
   }
   if (!_activeTabs) timer.stop();
}
