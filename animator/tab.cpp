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
#include <QTabWidget>

#include <cmath>

#include "../oxrender.h"
#include "../eventkiller.h"

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

TabInfo::TabInfo(QObject* parent, QWidget *current, int idx) :
QObject(parent), progress(0.0), currentWidget(current), index(idx) {}

bool
TabInfo::eventFilter( QObject* object, QEvent* event )
{
   if (event->type() != QEvent::Paint || clock.isNull())
      return false;
   QPainter p((QWidget*)object);
   p.drawPixmap(0,0, tabPix[2]);
   p.end();
   return true;
}

void
TabInfo::updatePixmaps(Transition transition)
{
   uint ms = clock.elapsed();
   switch (transition) {
      #ifndef QT_NO_XRENDER
      default:
      case CrossFade: { //  TODO accelerate animation!
         float quote = (float)_timeStep / (_duration-ms);
         progress += quote;
         OXRender::blend(tabPix[1], tabPix[2], quote);
         break;
      }
      #else
      default:
      #endif
      case ScanlineBlend: {
         QPainter p(&tabPix[2]);
         const int numStep = _duration/_timeStep;
         const int h = lround(_timeStep * (numStep-progress) / (_duration-ms));
         for (int i = (int)progress; i < tabPix[2].height(); i+=numStep)
            p.drawPixmap(0, i, tabPix[1], 0, i, tabPix[1].width(), h);
         progress += h;
         break;
      }
      case SlideIn: {
         //TODO handle different bar positions (currently assumes top)
         QPainter p(&tabPix[2]);
         const int h = ms*tabPix[1].height()/_duration;
         p.drawPixmap(0, 0, tabPix[1], 0, tabPix[1].height() - h,
                       tabPix[1].width(), h);
         break;
      }
      case SlideOut: {
         tabPix[2] = tabPix[1];
         //TODO handle different bar positions (currently assumes top)
         QPainter p(&tabPix[2]);
         int off = ms*tabPix[0].height()/_duration;
         p.drawPixmap(0, 0, tabPix[0], 0, off,
                       tabPix[0].width(), tabPix[0].height() - off);
         break;
      }
      case RollIn: {
         QPainter p(&tabPix[2]);
         int h = ms*tabPix[1].height()/(2*_duration);
         p.drawPixmap(0, 0, tabPix[1], 0, 0, tabPix[1].width(), h);
         p.drawPixmap(0, tabPix[1].height()-h, tabPix[1],
                       0, tabPix[1].height()-h, tabPix[1].width(), h);
         break;
      }
      case RollOut: {
         QPainter p(&tabPix[2]);
         int h = ms*tabPix[1].height()/_duration;
         int y = (tabPix[1].height()-h)/2;
         p.drawPixmap(0, y, tabPix[1], 0, y, tabPix[1].width(), h);
         break;
      }
      case OpenVertically: {
         tabPix[2] = tabPix[1];
         QPainter p(&tabPix[2]);
         const int off = ms*tabPix[0].height()/(2*_duration);
         const int h2 = tabPix[0].height()/2;
         p.drawPixmap(0,0, tabPix[0], 0,off, tabPix[0].width(),h2 - off);
         p.drawPixmap(0,h2+off, tabPix[0], 0,h2,
                       tabPix[0].width(),tabPix[0].height()-off);
         break;
      }
      case CloseVertically: {
         QPainter p(&tabPix[2]);
         int h = ms*tabPix[1].height()/(2*_duration);
         p.drawPixmap(0, 0, tabPix[1], 0, tabPix[1].height()/2-h,
                       tabPix[1].width(), h);
         p.drawPixmap(0, tabPix[1].height()-h, tabPix[1],
                       0, tabPix[1].height()/2, tabPix[1].width(), h);
         break;
      }
      case OpenHorizontally: {
         tabPix[2] = tabPix[1];
         QPainter p(&tabPix[2]);
         const int off = ms*tabPix[0].width()/(2*_duration);
         const int w2 = tabPix[0].width()/2;
         p.drawPixmap(0,0,tabPix[0],off,0, w2-off,tabPix[0].height());
         p.drawPixmap(w2+off,0,tabPix[0], w2,0,tabPix[0].width()-off,tabPix[0].height());
         break;
      }
      case CloseHorizontally: {
         QPainter p(&tabPix[2]);
         int w = ms*tabPix[1].width()/(2*_duration);
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
   QTabWidget *tw = qobject_cast<QTabWidget*>(w);
   QStackedWidget *sw; QWidget *cw = 0; int idx = -1;
   if (tw) {
      connect(tw, SIGNAL(currentChanged(int)), this, SLOT(changed(int)));
      cw = tw->currentWidget(); idx = tw->currentIndex();
   }
   else if ((sw = qobject_cast<QStackedWidget*>(w))) {
      connect(sw, SIGNAL(currentChanged(int)), this, SLOT(changed(int)));
      cw = sw->currentWidget(); idx = sw->currentIndex();
   }
   else
      return false;
   
   connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(release(QObject*)));
   items.insert(w, new TabInfo(this, cw, idx));
   return true;
}

void
Tab::_release(QWidget *w)
{
   if (QTabWidget *tw = qobject_cast<QTabWidget*>(w)) {
      disconnect(tw, SIGNAL(currentChanged(int)), this, SLOT(changed(int)));
   }
   else if (QStackedWidget *sw = qobject_cast<QStackedWidget*>(w)) {
      disconnect(sw, SIGNAL(currentChanged(int)), this, SLOT(changed(int)));
   }
   items.remove(w);
   if (items.isEmpty()) timer.stop();
}

void
Tab::changed(int index)
{
   if (_transition == Jump)
      return; // ugly nothing ;)

   // ensure this is a qtabwidget - we'd segfault otherwise
   QTabWidget* tw; QStackedWidget *sw;
   QWidget *w = 0, *cw = 0;
   if ((tw = qobject_cast<QTabWidget*>(sender()))) {
      w = tw; cw = tw->widget(index);
   }
   else if ((sw = qobject_cast<QStackedWidget*>(sender()))) {
      w = sw; cw = sw->widget(index);
   }
   if (!cw) return;

   qDebug() << "BESPIN:" << w << index;

   // find matching tabinfo
   TabInfo* tai;
   Items::iterator i = items.find(w);
   if (i == items.end())
      return; // not handled... why ever (i.e. should not happen by default)
   tai = i.value();
   tai->clock.restart();
   tai->progress = 0.0;

   // update from/to indices
   const int oldIdx = tai->index;
   QWidget *ow = tw ? tw->widget(tai->index) : sw->widget(tai->index);
   tai->currentWidget = cw;
   tai->index = index;
   if (!ow) return; // this is the first time the tab changes, nothing to blend

   if (ow == cw) { // this can happen on destruction etc...
      tai->clock = QTime();
      return; // or segfault!
   }

   // prepare the pixmaps we use to pretend the animation
   tai->tabPix[0] = tai->tabPix[1] =
      dumpBackground(w, QRect(ow->mapTo(w, QPoint(0,0)), ow->size()), qApp->style());
   grabWidget(ow, &tai->tabPix[0]);
//    qDebug() << "BESPIN: grabbed OLD content" << tai->clock.elapsed();
   if (tai->clock.elapsed() > _duration - timeStep) {
      qWarning("BESPIN: skipped animated tab transition after grabbing OLD content!");
      tai->tabPix[0] = tai->tabPix[1] = QPixmap();
      return; // all the effort for NOTHING! (we lost too much time here)
   }
   
   tai->tabPix[2] = tai->tabPix[0];
   grabWidget(cw, &tai->tabPix[1]);
//    qDebug() << "BESPIN: grabbed NEW content" << tai->clock.elapsed();
   if (tai->clock.elapsed() > _duration - timeStep) {
      qWarning("BESPIN: skipped animated tab transition after grabbing NEW content!");
      tai->tabPix[0] = tai->tabPix[1] = tai->tabPix[2] = QPixmap();
      return; // all the effort for NOTHING! (we lost too much time here)
   }
   // overtake widgets painting (this MUST NOT be shifted above, as we
   // couldn't grab widgets anymore then...)
   cw->parentWidget()->installEventFilter(tai);
   _BLOCKEVENTS_(cw);
   QList<QWidget*> widgets = cw->findChildren<QWidget*>();
   foreach(QWidget *widget, widgets) {
      _BLOCKEVENTS_(widget);
      if (widget->autoFillBackground()) {
         tai->autofilling.append(widget);
         widget->setAutoFillBackground(false);
      }
      else if (widget->testAttribute(Qt::WA_OpaquePaintEvent)) {
         tai->opaque.append(widget);
         widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
      }
   }

   tai->updatePixmaps(_transition);
   // update
   cw->repaint();
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
   _activeTabs = 0;
   TabInfo* tai;
   QWidget *ctw = 0, *widget = 0; QList<QWidget*> widgets;
   int index;
   for (i = items.begin(); i != items.end(); i++) {
      tai = i.value();
      if (tai->clock.isNull()) // this tab is currently not animated
         continue;
      ctw = tai->currentWidget;

      // check if our desired duration has exceeded and stop this in case
      if (tai->clock.elapsed() >= _duration) {
         tai->clock = QTime(); // reset clock, this is IMPORTANT!
         // reset pixmaps
         tai->tabPix[2] = tai->tabPix[1] = tai->tabPix[0] = QPixmap();
         // release widget painting ---------------------
         ctw->parentWidget()->removeEventFilter(tai);
         _UNBLOCKEVENTS_(ctw);
         widgets = ctw->findChildren<QWidget*>();
//          ctw->repaint();
         foreach(widget, widgets) {
            index = tai->autofilling.indexOf(widget);
            if (index != -1) {
               tai->autofilling.removeAt(index);
               widget->setAutoFillBackground(true);
            }
            index = tai->opaque.indexOf(widget);
            if (index != -1) {
               tai->opaque.removeAt(index);
               widget->setAttribute(Qt::WA_OpaquePaintEvent, true);
            }
            _UNBLOCKEVENTS_(widget);
            widget->update(); //if necessary
         }
         // -----------------------
         ctw->repaint(); //asap
         tai->autofilling.clear();
         tai->opaque.clear();
         continue;
      }
      // normal action
      ++_activeTabs;
      tai->updatePixmaps(_transition);
      ctw->parentWidget()->repaint();
   }
   if (!_activeTabs) timer.stop();
}
