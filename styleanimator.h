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

#ifndef STYLEANIMATOR_H
#define STYLEANIMATOR_H

#include <QStyle>
#include <QHash>
#include <QList>
#include <QPixmap>
#include <QTime>
#include <QTabWidget>

class QTimer;

class HoverFadeInfo {
public:
   HoverFadeInfo(int s = 0, bool fI = true) {step = s; fadeIn = fI; }
   int step;
   bool fadeIn;
};

class ComplexHoverFadeInfo {
public:
   ComplexHoverFadeInfo() {
      activeSubControls = fadeIns = fadeOuts = QStyle::SC_None;
   }
   QStyle::SubControls activeSubControls, fadeIns, fadeOuts;
   inline int step(QStyle::SubControl sc) const {return steps.value(sc);}
private:
   friend class StyleAnimator;
   QHash<QStyle::SubControl, int> steps;
};

class IndexedFadeInfo {
public:
   IndexedFadeInfo(long int index) { this->index = index; }
   int step(long int index) const;
private:
   friend class StyleAnimator;
   QHash<long int, int> fadeIns, fadeOuts;
   long int index;
};

class TabAnimInfo : public QObject {
   Q_OBJECT
public:
   enum TabTransition {Jump = 0, ScanlineBlend, SlideIn, SlideOut,
   RollIn, RollOut, OpenVertically, CloseVertically, OpenHorizontally,
   CloseHorizontally
#ifndef QT_NO_XRENDER
         , CrossFade
#endif
   };
   TabAnimInfo(QObject *parent = 0, int currentTab = -1) :
      QObject(parent), lastTab(currentTab), animStep(0){}
   void updatePixmaps(TabTransition transition);
protected:
   bool eventFilter( QObject* object, QEvent* event );
public:
   QList < QWidget* > autofillingWidgets;
   int lastTab, animStep;
   QPixmap tabPix[3];
   QTime lastTabUpdate;
};

#define OUT false

class StyleAnimator : public QObject {
   Q_OBJECT
public:
   StyleAnimator(QObject *parent, TabAnimInfo::TabTransition tabTransition =
                 TabAnimInfo::CrossFade);
   ~StyleAnimator();
   
   void registrate(QWidget *w);
   void unregistrate(QWidget *w);
   
   void addScrollArea(QWidget *area);
   
   int hoverStep(const QWidget *widget) const;
   int progressStep(const QWidget *w) const;
   
   const ComplexHoverFadeInfo *
      fadeInfo(const QWidget *widget, QStyle::SubControls activeSubControls) const;
   const IndexedFadeInfo *
      fadeInfo(const QWidget *widget, long int index) const;
   
   inline bool handlesArea(QWidget *widget) {return _scrollAreas.contains(widget);}
   
public slots:
   void tabChanged(int index);
protected:
   bool eventFilter( QObject *object, QEvent *event );
private:
   void addProgressBar(QWidget *progressBar);
   void addTab(QTabWidget* tab, int currentTab = -1);
   void fade(QWidget *widget, bool in = true);
   void remove(QWidget *w);
private slots:
   void destroyed(QObject*);
   void updateProgressbars();
   void updateTabAnimation();
   void updateFades();
   void updateComplexFades();
   void updateIndexedFades();
private:
   QTimer* timer;
   TabAnimInfo::TabTransition tabTransition;
   QObjectList _scrollAreas;
};

#endif // STYLEANIMATOR_H
