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
#include <QListView>
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
#include <QTreeView>

#include "colors.h"
#include "xproperty.h"
#include "visualframe.h"
#include "eventkiller.h"
#include "bespin.h"
#include "hacks.h"

#include "animator/hover.h"
#include "animator/aprogress.h"
#include "animator/tab.h"

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
   int i;
   switch (config.bg.structure)
   {
   default:
   case 0: // scanlines
      pix->fill( c.light(config.bg.intensity).rgb() );
      i = 100 + (light?6:3)*(config.bg.intensity - 100)/10;
      p.setPen(c.light(i));
      for ( i = 1; i < 64; i += 4 ) {
         p.drawLine( 0, i, 63, i );
         p.drawLine( 0, i+2, 63, i+2 );
      }
      p.setPen( c );
      for ( i = 2; i < 63; i += 4 )
         p.drawLine( 0, i, 63, i );
      break;
   case 1: //checkboard
      p.setPen(Qt::NoPen);
      i = 100 + 2*(config.bg.intensity - 100)/10;
      p.setBrush(c.light(i));
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
      p.setBrush(c.dark(i));
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
   case 2:  // fat scans
      i = (config.bg.intensity - 100);
      pix->fill( c.light(100+3*i/10).rgb() );
      p.setPen(QPen(light ? c.light(100+i/10) : c, 3));
      p.setBrush( c.dark(100+2*i/10) );
      p.drawRect(-3,8,70,8);
      p.drawRect(-3,24,70,8);
      p.drawRect(-3,40,70,8);
      p.drawRect(-3,56,70,8);
      break;
   case 3: // "blue"print
      i = (config.bg.intensity - 100);
      pix->fill( c.dark(100+i/10).rgb() );
      p.setPen(c.light(100+(light?4:2)*i/10));
      for ( i = 0; i < 64; i += 16 )
         p.drawLine( 0, i, 63, i );
      for ( i = 0; i < 64; i += 16 )
         p.drawLine( i, 0, i, 63 );
      break;
   case 4: // verticals
      i = (config.bg.intensity - 100);
      pix->fill( c.light(100+i).rgb() );
      p.setPen(c.light(100+(light?6:3)*i/10));
      for ( i = 1; i < 64; i += 4 ) {
         p.drawLine( i, 0, i, 63 );
         p.drawLine( i+2, 0, i+2, 63 );
      }
      p.setPen( c );
      for ( i = 2; i < 63; i += 4 )
         p.drawLine( i, 0, i, 63 );
      break;
   case 5: // diagonals
      i = config.bg.intensity - 100;
      pix->fill( c.light(100+i).rgb() );
      p.setPen(QPen(c.dark(100 + i/(2*(light+1))), 11));
      p.setRenderHint(QPainter::Antialiasing);
      p.drawLine(-64,64,64,-64);
      p.drawLine(0,64,64,0);
      p.drawLine(0,128,128,0);
      p.drawLine(32,64,64,32);
      p.drawLine(0,32,32,0);
      break;
   }
   p.end();
}

void BespinStyle::polish ( QApplication * app ) {

//    if (timer && !timer->isActive())
//       timer->start(50);
   QPalette pal = app->palette();
   polish(pal);
   app->setPalette(pal);
//    app->installEventFilter(this);
}

#define _SHIFTCOLOR_(clr) clr = QColor(CLAMP(clr.red()-10,0,255),CLAMP(clr.green()-10,0,255),CLAMP(clr.blue()-10,0,255))

#include "makros.h"
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
   // AlternateBase
   pal.setColor(QPalette::AlternateBase,
                Colors::mid(pal.color(QPalette::Active, QPalette::Base),
                            pal.color(QPalette::Active, QPalette::Text),15,1));
   
   // highlight colors
   const int highlightGray = qGray(pal.color(QPalette::Active, QPalette::Highlight).rgb());
   const QColor grey(highlightGray,highlightGray,highlightGray);
   pal.setColor(QPalette::Disabled, QPalette::Highlight, grey);


#if 0 // inactive palette
   pal.setColor(QPalette::Inactive, QPalette::Highlight,
                Colors::mid(pal.color(QPalette::Active, QPalette::Highlight),
                            grey,2,1));
   pal.setColor(QPalette::Inactive, QPalette::WindowText,
                Colors::mid(pal.color(QPalette::Active, QPalette::Window),
                            pal.color(QPalette::Active, QPalette::WindowText),1,4));
   pal.setColor(QPalette::Inactive, QPalette::Text,
                Colors::mid(pal.color(QPalette::Active, QPalette::Base),
                            pal.color(QPalette::Active, QPalette::Text),1,4));
#endif

   // fade disabled palette
   pal.setColor(QPalette::Disabled, QPalette::WindowText,
                Colors::mid(pal.color(QPalette::Active, QPalette::Window),
                            pal.color(QPalette::Active, QPalette::WindowText),2,1));
   pal.setColor(QPalette::Disabled, QPalette::Base,
                Colors::mid(pal.color(QPalette::Active, QPalette::Window),
                            pal.color(QPalette::Active, QPalette::Base),1,2));
   pal.setColor(QPalette::Disabled, QPalette::Text,
                Colors::mid(pal.color(QPalette::Active, QPalette::Base),
                            pal.color(QPalette::Active, QPalette::Text)));
   pal.setColor(QPalette::Disabled, QPalette::AlternateBase,
                Colors::mid(pal.color(QPalette::Disabled, QPalette::Base),
                            pal.color(QPalette::Disabled, QPalette::Text),15,1));
}


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
#include <QtDebug>
#undef PAL
#define PAL pal

inline static void polishGTK(QWidget * widget)
{
   enum MyRole{Bg = BespinStyle::Bg, Fg = BespinStyle::Fg};
   if (widget->objectName() == "QPushButton" ||
       widget->objectName() == "QComboBox" ||
       widget->objectName() == "QCheckBox" ||
       widget->objectName() == "QRadioButton" )
   {
      QPalette pal = widget->palette();
      pal.setColor(QPalette::Disabled, QPalette::Button,
                   Colors::mid(Qt::black, FCOLOR(Window),5,100));
      pal.setColor(QPalette::Inactive, QPalette::Button, CCOLOR(btn.std, Bg));
      pal.setColor(QPalette::Active, QPalette::Button, CCOLOR(btn.active, Bg));
      
      pal.setColor(QPalette::Disabled, QPalette::ButtonText,
                   Colors::mid(FCOLOR(Window), FCOLOR(WindowText),3,1));
      pal.setColor(QPalette::Inactive, QPalette::ButtonText, CCOLOR(btn.std, Fg));
      pal.setColor(QPalette::Active, QPalette::ButtonText, CCOLOR(btn.active, Fg));
      widget->setPalette(pal);
   }
   if (widget->objectName() == "QTabWidget" ||
       widget->objectName() == "QTabBar")
   {
      QPalette pal = widget->palette();
      pal.setColor(QPalette::Inactive, QPalette::Window, CCOLOR(tab.std, Bg));
      pal.setColor(QPalette::Active, QPalette::Window, CCOLOR(tab.active, Bg));
      
      pal.setColor(QPalette::Disabled, QPalette::WindowText,
                   Colors::mid(CCOLOR(tab.std, Bg), CCOLOR(tab.std, Fg),3,1));
      pal.setColor(QPalette::Inactive, QPalette::WindowText, CCOLOR(tab.std, Fg));
      pal.setColor(QPalette::Active, QPalette::WindowText, CCOLOR(tab.active, Fg));
      widget->setPalette(pal);
   }
   
   if (widget->objectName() == "QMenuBar" )
   {
      QPalette pal = widget->palette();
      pal.setColor(QPalette::Inactive, QPalette::Window,
                   Colors::mid(FCOLOR(Window), CCOLOR(menu.bar, Bg)));
      pal.setColor(QPalette::Active, QPalette::Window, CCOLOR(menu.active, Bg));
      
      pal.setColor(QPalette::Inactive, QPalette::WindowText, CCOLOR(menu.bar, Fg));
      pal.setColor(QPalette::Active, QPalette::WindowText, CCOLOR(menu.active, Fg));
      widget->setPalette(pal);
   }
   
   if (widget->objectName() == "QMenu" )
   {
      QPalette pal = widget->palette();
      pal.setColor(QPalette::Inactive, QPalette::Window, CCOLOR(menu.std, Bg));
      pal.setColor(QPalette::Active, QPalette::Window, CCOLOR(menu.active, Bg));
      
      pal.setColor(QPalette::Inactive, QPalette::WindowText, CCOLOR(menu.std, Fg));
      pal.setColor(QPalette::Active, QPalette::WindowText, CCOLOR(menu.active, Fg));
      widget->setPalette(pal);
   }
}

void BespinStyle::polish( QWidget * widget ) {
   
   if (!widget) return; // !!! protect against polishing QObject trials !

   if (isGTK) {
      polishGTK(widget);
      return;
   }

   if (qobject_cast<VisualFramePart*>(widget)) return;
//    qDebug() << widget;
   Hacks::add(widget);
   
   if (widget->isWindow()) {
      QPalette pal = widget->palette();

      uint info = XProperty::encode(FCOLOR(Window), FCOLOR(WindowText), config.bg.mode);
      XProperty::set(widget->winId(), XProperty::bgInfo, info);
      info = XProperty::encode(CCOLOR(kwin.active, Bg),
                                CCOLOR(kwin.active, Fg), GRAD(kwin)[1]);
      XProperty::set(widget->winId(), XProperty::actInfo, info);
      const QColor fg = Colors::mid(CCOLOR(kwin.inactive, Bg),
                                     CCOLOR(kwin.inactive, Fg), 2, 1);
      info = XProperty::encode(CCOLOR(kwin.inactive, Bg), fg, GRAD(kwin)[0]);
      XProperty::set(widget->winId(), XProperty::inactInfo, info);
      
      bool freakModals = config.bg.modal.invert ||
         config.bg.modal.glassy ||
         config.bg.modal.opacity < 100;
      if (freakModals)
         widget->installEventFilter(this);
      if (config.bg.mode > Scanlines)
         widget->setAttribute(Qt::WA_StyledBackground);
//       widget->setAutoFillBackground(true);
      if (QMenu *menu = qobject_cast<QMenu *>(widget)) {
         if (config.menu.glassy) {
            if (config.bg.mode == Scanlines) {
               QPalette pal = widget->palette();
               pal.setBrush( QPalette::Background,
                             pal.color(QPalette::Active, QPalette::Background) );
               widget->setPalette(pal);
            }
            widget->setAttribute(Qt::WA_StyledBackground);
         }
         if (config.bg.mode == Scanlines) {
            QPalette pal = widget->palette();
            QColor c = pal.color(QPalette::Active, QPalette::Window);
            if (!_scanlines[1]) makeStructure(&_scanlines[1], c, true);
            QBrush brush( c, *_scanlines[1] );
            pal.setBrush( QPalette::Window, brush );
            widget->setPalette(pal);
         }
         widget->setWindowOpacity( config.menu.opacity/100.0 );
         // swap qmenu colors - in case
         widget->setAutoFillBackground(true);
         widget->setBackgroundRole ( config.menu.std_role[0] );
         widget->setForegroundRole ( config.menu.std_role[1] );
         if (config.menu.boldText) {
            QFont tmpFont = widget->font();
            tmpFont.setBold(true);
            widget->setFont(tmpFont);
         }
         if (!freakModals && widget->parentWidget() &&
             widget->parentWidget()->inherits("QMdiSubWindow"))
            widget->installEventFilter(this);
         if (bar4popup(menu))
            menu->installEventFilter(this); // reposition
#if SHAPE_POPUP
// WARNING: compmgrs like e.g. beryl/emerald deny to shadow shaped windows,
// if we cannot find a way to get ARGB menus independent from the app settings, the compmgr must handle the round corners here
         if (bar4popup(menu)) {
            QAction *action = new QAction( menu->menuAction()->iconText(), menu );
            connect (action, SIGNAL(triggered(bool)), menu, SLOT(hide()));
            menu->insertAction(menu->actions().at(0), action);
//             menu->installEventFilter(this); // reposition/round corners
         }
#endif
      }
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

   // Enable hover effects in listview, all itemviews like in kde is pretty annoying
   if (QListView *listView = qobject_cast<QListView*>(widget) )
      listView->viewport()->setAttribute(Qt::WA_Hover);
   
   if (qobject_cast<QAbstractButton*>(widget)) {
      widget->setBackgroundRole ( QPalette::Window );
      widget->setForegroundRole ( QPalette::WindowText );
      if (widget->inherits("QToolBoxButton") ||
          widget->objectName() == "RenderFormElementWidget" ) {
         widget->setAttribute(Qt::WA_Hover);
      }
      else
         Animator::Hover::manage(widget);
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
      if (widget->objectName() == "RenderFormElementWidget")
         widget->setAttribute(Qt::WA_Hover);
      else
         Animator::Hover::manage(widget);
   }
   
   else if (qobject_cast<QAbstractSlider *>(widget)) {
      // NOTICE we could get slight more performance by this and cache ourselve,
      // but that's gonna add more complexity, and as the slider is usually not
      // bound to e.g. a scrollarea surprisinlgy little CPU gain...
//       widget->setAttribute(Qt::WA_OpaquePaintEvent);
      if (qobject_cast<QScrollBar *>(widget)) {
         // NOTICE slows down things as it triggers a repaint of the frame
//          widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
         // ================
         QWidget *area = 0;
         if (widget->parentWidget()) {
            area = widget->parentWidget();
//             if (isSpecialFrame(area))
//                widget->installEventFilter(this);
            if (area->parentWidget()) {
               area = area->parentWidget();
               if (qobject_cast<QAbstractScrollArea*>(area))
                  area = 0; // this is handled with the scrollarea, but...
               else // konsole, kate, etc. need a special handling!
                  area = widget->parentWidget();
            }
         }
         if (area)
            Animator::Hover::manage(area, true);
      }
   }
   
   else if (qobject_cast<QProgressBar*>(widget)) {
      QFont fnt = widget->font();
      fnt.setBold(true);
      widget->setFont(fnt);
//       widget->setBackgroundRole ( config.progress.role[0] );
//       widget->setForegroundRole ( config.progress.role[1] );
      Animator::Progress::manage(widget);
   }

   // do NOT! apply this on tabs explicitly, as they contain a stack!
   // theroretically we can handle all stacked widgets, but this can lead to couple of trouble
   // 1: designer segfaulst on startup -> ignore parentWidget()->inherits("QSplitter")
   // 2: konsole painting doesn't work (ok, that's no SOO much a problem, just appears to be slow)
   // 3: e.g. konqueror setup dialog shows only the first entry - and i don't know why...
   // 4: systemsettings crashes
   // the core problem seems to be that i block events on the widgets, maybe better inject a widget, raise it paint there
   // and leave other widgets alone
   else if (widget->inherits("QStackedWidget"))
      Animator::Tab::manage(widget);
   
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
          qobject_cast<Q3ScrollView*>(frame)) {
         Animator::Hover::manage(frame);
         if (config.hack.treeViews)
         if (QTreeView* tv = qobject_cast<QTreeView*>(frame))
            tv->setAnimated ( true );
      }

   // map a toolbox frame to it's elements
//       if (qobject_cast<QAbstractScrollArea*>(frame) &&
//             frame->parentWidget() && frame->parentWidget()->inherits("QToolBox"))
//          frame->setFrameStyle( static_cast<QFrame*>(frame->parentWidget())->frameStyle() );

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
      else if (isSpecialFrame(widget)) {
            if (frame->lineWidth() == 1)
               frame->setLineWidth(dpi.f4);
      }
      else
         VisualFrame::manage(frame);
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
   //========================
#define LACK_CONTRAST(_C_) Colors::contrast(pal.color(QPalette::Active, QPalette::_C_), pal.color(QPalette::Active, QPalette::_C_##Text)) < 20
#define HARD_CONTRAST(_C_) Colors::value(pal.color(QPalette::Active, QPalette::_C_)) < 128 ? Qt::white : Qt::black
   if (widget->objectName() == "RenderFormElementWidget")
	{
		QPalette pal = widget->palette();
		if (LACK_CONTRAST(Window))
			pal.setColor(QPalette::WindowText, HARD_CONTRAST(Window));
		if (LACK_CONTRAST(Button))
			pal.setColor(QPalette::ButtonText, HARD_CONTRAST(Button));
		widget->setPalette(pal);
	}
   
}
#undef PAL
void BespinStyle::unPolish( QApplication *app )
{
   app->setPalette(QPalette());
}

void BespinStyle::unPolish( QWidget *widget )
{
   if (QFrame *frame = qobject_cast<QFrame *>(widget))
      VisualFrame::release(frame);
/*   if (qobject_cast<VisualFramePart*>(widget)) {
      delete widget; widget = 0L; return
   }*/
   
   Animator::Hover::release(widget);
   Animator::Progress::release(widget);
   Animator::Tab::release(widget);
   Hacks::remove(widget);

   widget->removeEventFilter(this);
}
