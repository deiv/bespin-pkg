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

#include <Q3ScrollView>
#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QLayout>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPen>
#include <QProgressBar>
#include <QSplitterHandle>
#include <QToolBar>

#ifdef Q_WS_X11
#include <X11/Xatom.h>
#endif

#include "colors.h"
#include "visualframe.h"
#include "eventkiller.h"
#include "bespin.h"
using namespace Bespin;

extern Config config;
extern Dpi dpi;

static void
makeStructure(QPixmap **pixp, const QColor &c, bool light)
{
   if (!(*pixp))
      (*pixp) = new QPixmap(64, 64);
   QPixmap *pix = (*pixp);
   QPainter p(pix);
   switch (config.bg.structure)
   {
   default:
   case 0: { // scanlines
      pix->fill( c.light(110).rgb() );
      p.setPen( light ? c.light(106) : c.light(103) );
      int i;
      for ( i = 1; i < 64; i += 4 ) {
         p.drawLine( 0, i, 63, i );
         p.drawLine( 0, i+2, 63, i+2 );
      }
      p.setPen( c );
      for ( i = 2; i < 63; i += 4 )
         p.drawLine( 0, i, 63, i );
      break;
   }
   case 1: { //checkboard
      p.setPen(Qt::NoPen);
      p.setBrush(c.light(102));
      if (light) {
         p.drawRect(0,0,16,16); p.drawRect(32,0,16,16);
         p.drawRect(16,16,16,16); p.drawRect(48,16,16,16);
         p.drawRect(0,32,16,16); p.drawRect(32,32,16,16);
         p.drawRect(16,48,16,16); p.drawRect(48,48,16,16);
      }
      else {
         p.drawRect(0,0,32,32);
         p.drawRect(32,32,32,32);
      }
      p.setBrush(c.dark(102));
      if (light) {
         p.drawRect(16,0,16,16); p.drawRect(48,0,16,16);
         p.drawRect(0,16,16,16); p.drawRect(32,16,16,16);
         p.drawRect(16,32,16,16); p.drawRect(48,32,16,16);
         p.drawRect(0,48,16,16); p.drawRect(32,48,16,16);
      }
      else {
         p.drawRect(32,0,32,32);
         p.drawRect(0,32,32,32);
      }
      break;
   }
   case 2: // fat scans
      pix->fill( c.light(103).rgb() );
      p.setPen(QPen(light ? c.light(101) : c, 3));
      p.setBrush( c.dark(102) );
      p.drawRect(-3,8,70,8);
      p.drawRect(-3,24,70,8);
      p.drawRect(-3,40,70,8);
      p.drawRect(-3,56,70,8);
      break;
   case 3: // "blue"print
      pix->fill( c.dark(101).rgb() );
      p.setPen(light ? c.light(104) : c.light(102));
      for ( int i = 0; i < 64; i += 16 )
         p.drawLine( 0, i, 63, i );
      for ( int i = 0; i < 64; i += 16 )
         p.drawLine( i, 0, i, 63 );
      break;
   case 4: // verticals
      pix->fill( c.light(110).rgb() );
      p.setPen( light ? c.light(106) : c.light(103) );
      for ( int i = 1; i < 64; i += 4 ) {
         p.drawLine( i, 0, i, 63 );
         p.drawLine( i+2, 0, i+2, 63 );
      }
      p.setPen( c );
      for ( int i = 2; i < 63; i += 4 )
         p.drawLine( i, 0, i, 63 );
      break;
   case 5: { // diagonals
      pix->fill( c.light(102).rgb() );
      QPen pen(c.dark(102-light), 11);
      p.setPen(pen);
      p.setRenderHint(QPainter::Antialiasing);
      p.drawLine(-64,64,64,-64);
      p.drawLine(0,64,64,0);
      p.drawLine(0,128,128,0);
      p.drawLine(32,64,64,32);
      p.drawLine(0,32,32,0);
      break;
   }
   }
   p.end();
}

#ifdef Q_WS_X11
static Atom winTypePopup = 0;
static Atom winTypeMenu = 0;
static Atom winType = 0;
#define SET_WINDOW_TYPE(_WINDOW_, _TYPE_)\
XChangeProperty(QX11Info::display(), _WINDOW_->winId(), winType,\
XA_CARDINAL, 32, PropModeReplace, (const unsigned char*)&_TYPE_, 1L)
#endif

void BespinStyle::polish ( QApplication * app ) {
#ifdef Q_WS_X11
#define ENSURE_ATOM(_VAR_, _TYPE_)\
   if (!_VAR_)\
      _VAR_ = XInternAtom(QX11Info::display(), _TYPE_, False)

   ENSURE_ATOM(winTypePopup, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU");
   ENSURE_ATOM(winTypeMenu, "_NET_WM_WINDOW_TYPE_MENU");
   ENSURE_ATOM(winType, "_NET_WM_WINDOW_TYPE");
   
#undef ENSURE_ATOM
#endif
//    if (timer && !timer->isActive())
//       timer->start(50);
   QPalette pal = app->palette();
   polish(pal);
   app->setPalette(pal);
//    app->installEventFilter(this);
}

#define _SHIFTCOLOR_(clr) clr = QColor(CLAMP(clr.red()-10,0,255),CLAMP(clr.green()-10,0,255),CLAMP(clr.blue()-10,0,255))

void BespinStyle::polish( QPalette &pal )
{
   QColor c = pal.color(QPalette::Active, QPalette::Background);
   if (config.bg.mode > Scanlines) {
      int h,s,v;
      c.getHsv(&h,&s,&v);
      if (v < 70) // very dark colors won't make nice backgrounds ;)
         c.setHsv(h,s,70);
      pal.setColor( QPalette::Window, c );
   }
   if (_scanlines[0]) delete _scanlines[0]; _scanlines[0] = 0;
   if (_scanlines[1]) delete _scanlines[1]; _scanlines[1] = 0;
   QLinearGradient lg; QPainter p;
   if (config.bg.mode == Scanlines) {
      QColor c = pal.color(QPalette::Active, QPalette::Background);
      makeStructure(&_scanlines[0], c, false);
      QBrush brush( c, *_scanlines[0] );
      pal.setBrush( QPalette::Background, brush );
   }
   
   // disabled palette
   int highlightGray = qGray(pal.color(QPalette::Active, QPalette::Highlight).rgb());
   pal.setColor(QPalette::Disabled, QPalette::Highlight,
                QColor(highlightGray,highlightGray,highlightGray));
   pal.setColor(QPalette::Active, QPalette::AlternateBase,
                Colors::mid(pal.color(QPalette::Active, QPalette::Base),
                            pal.color(QPalette::Active, QPalette::Text),15,1));
   pal.setColor(QPalette::Inactive, QPalette::WindowText,
                Colors::mid(pal.color(QPalette::Active, QPalette::Window),
                            pal.color(QPalette::Active, QPalette::WindowText),2,1));
   pal.setColor(QPalette::Inactive, QPalette::Base,
                Colors::mid(pal.color(QPalette::Active, QPalette::Window),
                            pal.color(QPalette::Active, QPalette::Base),1,2));
   pal.setColor(QPalette::Inactive, QPalette::Text,
                Colors::mid(pal.color(QPalette::Active, QPalette::Base),
                            pal.color(QPalette::Active, QPalette::Text)));
   
   //inactive palette
   pal.setColor(QPalette::Inactive, QPalette::WindowText,
                Colors::mid(pal.color(QPalette::Active, QPalette::Window),
                            pal.color(QPalette::Active, QPalette::WindowText)));
   pal.setColor(QPalette::Inactive, QPalette::Text,
                Colors::mid(pal.color(QPalette::Active, QPalette::Base),
                            pal.color(QPalette::Active, QPalette::Text),1,3));
   pal.setColor(QPalette::Inactive, QPalette::Highlight,
                Colors::mid(pal.color(QPalette::Active, QPalette::Highlight),
                            pal.color(QPalette::Disabled, QPalette::Highlight),3,1));
   pal.setColor(QPalette::Inactive, QPalette::AlternateBase,
                Colors::mid(pal.color(QPalette::Active, QPalette::AlternateBase),
                            pal.color(QPalette::Active, QPalette::Base),3,1));
}

#if SHAPE_POPUP
static QMenuBar *bar4popup(QMenu *menu) {
   if (!menu->menuAction())
      return 0;
   if (menu->menuAction()->associatedWidgets().isEmpty())
      return 0;
   foreach (QWidget *w, menu->menuAction()->associatedWidgets())
      if (qobject_cast<QMenuBar*>(w))
         return static_cast<QMenuBar *>(w);
   return 0;
}
#endif

void BespinStyle::polish( QWidget * widget) {
   
   if (!widget) return; // !
   
   if (widget->isWindow() && config.bg.mode > Scanlines) {
      widget->setAutoFillBackground(true);
      widget->setAttribute(Qt::WA_StyledBackground);
   }
   
#ifdef MOUSEDEBUG
   widget->installEventFilter(this);
#endif
   
   if (false
#ifndef QT_NO_SPINBOX
       || qobject_cast<QAbstractSpinBox *>(widget)
#endif
       || widget->inherits("QHeaderView")
#ifndef QT_NO_SPLITTER
       || qobject_cast<QSplitterHandle *>(widget)
#endif
#ifndef QT_NO_TABBAR
       || qobject_cast<QTabBar *>(widget)
#endif
       || qobject_cast<QProgressBar*>(widget)
       || qobject_cast<QAbstractSlider*>(widget)
       || widget->inherits("QWorkspaceTitleBar")
       || widget->inherits("QDockWidget")
       || widget->inherits("QToolBar")
       || widget->inherits("QToolBarHandle")
       || widget->inherits("QDockSeparator")
       || widget->inherits("QToolBoxButton")
       || widget->inherits("QDockWidgetSeparator")
       || widget->inherits("Q3DockWindowResizeHandle")
      )
      widget->setAttribute(Qt::WA_Hover);
   
   if (qobject_cast<QAbstractButton*>(widget)) {
      widget->setBackgroundRole ( QPalette::Window );
      widget->setForegroundRole ( QPalette::WindowText );
      if (!widget->inherits("QToolBoxButton"))
         animator->registrate(widget);
   }
   else if (QComboBox *box = qobject_cast<QComboBox *>(widget)) {
      if (box->isEditable()) {
         widget->setBackgroundRole ( QPalette::Base );
         widget->setForegroundRole ( QPalette::Text );
      }
      else {
         widget->setBackgroundRole ( QPalette::Window );
         widget->setForegroundRole ( QPalette::WindowText );
      }
      animator->registrate(widget);
   }
   
   else if (qobject_cast<QAbstractSlider *>(widget)) {
      widget->installEventFilter(this);
      if (qobject_cast<QScrollBar *>(widget)) {
         widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
         QWidget *area = 0;
         if (widget->parentWidget()) {
            area = widget->parentWidget();
            if (area->parentWidget()) {
               area = area->parentWidget();
               if (qobject_cast<QAbstractScrollArea*>(area)) {
                  if (area->inherits("QComboBoxListView"))
                     widget->setAttribute(Qt::WA_OpaquePaintEvent, true);
                  area = 0;
               }
               else // konsole, kate, etc.
                  area = widget->parentWidget();
            }
         }
         if (area)
            animator->addScrollArea(area);
      }
   }
   
   else if (qobject_cast<QProgressBar*>(widget)) {
      QFont fnt = widget->font();
      fnt.setBold(true);
      widget->setFont(fnt);
//       widget->setBackgroundRole ( config.progress.role[0] );
//       widget->setForegroundRole ( config.progress.role[1] );
      animator->registrate(widget);
   }
   
   else if (qobject_cast<QTabWidget*>(widget))
      animator->registrate(widget);
   
   else if (qobject_cast<QTabBar *>(widget)) {
      widget->setBackgroundRole ( config.tab.std_role[0] );
      widget->setForegroundRole ( config.tab.std_role[1] );
      widget->installEventFilter(this);
   }
   
   
   if (false // to simplify the #ifdefs
#ifndef QT_NO_MENUBAR
       || qobject_cast<QMenuBar *>(widget)
#endif
#ifdef QT3_SUPPORT
       || widget->inherits("Q3ToolBar")
#endif
#ifndef QT_NO_TOOLBAR
       || qobject_cast<QToolBar *>(widget)
       || (widget && qobject_cast<QToolBar *>(widget->parent()))
#endif
      ) {
         widget->setBackgroundRole(QPalette::Window);
         if (config.bg.mode == Scanlines) {
            widget->setAutoFillBackground ( true );
            QPalette pal = widget->palette();
            QColor c = pal.color(QPalette::Active, QPalette::Window);
            
            if (!_scanlines[1])
               makeStructure(&_scanlines[1], c, true);
            QBrush brush( c, *_scanlines[1] );
            pal.setBrush( QPalette::Window, brush );
            widget->setPalette(pal);
         }
      }
#if 0
#ifdef Q_WS_X11
   if (qobject_cast<QMenuBar*>(widget)) {
      widget->setParent(widget->parentWidget(), Qt::Window | Qt::Tool |
                        Qt::FramelessWindowHint);
      widget->move(0,0);
      SET_WINDOW_TYPE(widget, winTypeMenu);
      if( widget->parentWidget())
         XSetTransientForHint( QX11Info::display(), widget->winId(),
                               widget->parentWidget()->topLevelWidget()->winId());
   }
#endif
#endif
   if (!widget->isWindow())
   if (QFrame *frame = qobject_cast<QFrame *>(widget)) {
   // kill ugly winblows frames...
      if (frame->frameShape() == QFrame::Box ||
            frame->frameShape() == QFrame::Panel ||
            frame->frameShape() == QFrame::WinPanel)
         frame->setFrameShape(QFrame::StyledPanel);

      if (qobject_cast<QAbstractScrollArea*>(frame) ||
            qobject_cast<Q3ScrollView*>(frame))
         animator->registrate(frame);

   // map a toolbox frame to it's elements
      if (qobject_cast<QAbstractScrollArea*>(frame) &&
            frame->parentWidget() && frame->parentWidget()->inherits("QToolBox"))
         frame->setFrameStyle( static_cast<QFrame*>(frame->parentWidget())->frameStyle() );

   // overwrite ugly lines
      if (frame->frameShape() == QFrame::HLine ||
            frame->frameShape() == QFrame::VLine)
         widget->installEventFilter(this);

   // toolbox handling - a shame they look that crap by default!
      else if (widget->inherits("QToolBox")) {
         widget->setBackgroundRole(QPalette::Window);
         widget->setForegroundRole(QPalette::WindowText);
         if (widget->layout()) {
            widget->layout()->setMargin ( 0 );
            widget->layout()->setSpacing ( 0 );
         }
      }
      else if (frame->frameShape() == QFrame::StyledPanel) {

         if (widget->inherits("QTextEdit") && frame->lineWidth() == 1)
            frame->setLineWidth(dpi.f4);
         else {
            QList<VisualFrame*> vfs = frame->findChildren<VisualFrame*>();
            if (vfs.isEmpty()) { // avoid double adds
               int exts[4]; uint sizes[4]; // t/b/l/r
               const int f2 = dpi.f2, f3 = dpi.f3, f4 = dpi.f4, f6 = dpi.f6;
               if (frame->frameShadow() == QFrame::Sunken) {
                  exts[0] = exts[2] = exts[3] = 0; exts[1] = f3;
                  sizes[0] = sizes[1] = sizes[2] = sizes[3] = f4;
               }
               else if (frame->frameShadow() == QFrame::Raised) {
                  exts[0] = f2; exts[1] = f4; exts[2] = exts[3] = f2;
                  sizes[0] = sizes[2] = sizes[3] = f4; sizes[1] = f6;
               }
               else { // plain
                  exts[0] = exts[1] = exts[2] = exts[3] = f2;
                  sizes[0] = sizes[1] = sizes[2] = sizes[3] = f2;
               }
               new VisualFrame(frame, sizes, exts);
            }
         }
      }
   }
   
   if (widget->autoFillBackground() &&
       // dad
       widget->parentWidget() &&
       ( widget->parentWidget()->objectName() == "qt_scrollarea_viewport" ) &&
       //grampa
       widget->parentWidget()->parentWidget() &&
       qobject_cast<QAbstractScrollArea*>(widget->parentWidget()->parentWidget()) &&
       // grangrampa
       widget->parentWidget()->parentWidget()->parentWidget() &&
       widget->parentWidget()->parentWidget()->parentWidget()->inherits("QToolBox")
      ) {
         widget->parentWidget()->setAutoFillBackground(false);
         widget->setAutoFillBackground(false);
      }
   
   // swap qmenu colors
   if (qobject_cast<QMenu *>(widget)) {
#ifdef Q_WS_X11
      // tell beryl et. al this is a popup TODO: doesn't work yet...
      SET_WINDOW_TYPE(widget, winTypePopup);
// WARNING: compmgrs like e.g. beryl/emerald deny to shadow shaped windows,
// if we cannot find a way to get ARGB menus independent from the app settings, the compmgr must handle the round corners here
#endif
      widget->setBackgroundRole ( config.menu.std_role[0] );
      widget->setForegroundRole ( config.menu.std_role[1] );
      if (config.menu.boldText) {
         QFont tmpFont = widget->font();
         tmpFont.setBold(true);
         widget->setFont(tmpFont);
      }
      if (widget->parentWidget() &&
          widget->parentWidget()->inherits("QMdiSubWindow"))
         widget->installEventFilter(this);
      // hmmmm... =)
#if SHAPE_POPUP
      if (bar4popup(menu)) {
         QAction *action = new QAction( menu->menuAction()->iconText(), menu );
         connect (action, SIGNAL(triggered(bool)), menu, SLOT(hide()));
         menu->insertAction(menu->actions().at(0), action);
         menu->installEventFilter(this); // reposition/round corners
      }
#endif
   }
   
   //========================
   
}

void BespinStyle::unPolish( QApplication *app )
{
   app->setPalette(QPalette());
}

void BespinStyle::unPolish( QWidget *widget )
{
   animator->unregistrate(widget);
   
   if (qobject_cast<VisualFrame*>(widget))
      delete widget; widget = 0L;
   
   widget->removeEventFilter(this);
   
}
