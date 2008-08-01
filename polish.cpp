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

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QLayout>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPen>
#include <QProgressBar>
#include <QScrollBar>
#include <QSplitterHandle>
#include <QToolBar>
#include <QToolTip>
#include <QTreeView>

#include "colors.h"

#ifdef Q_WS_X11
#include "xproperty.h"
#endif

#include "visualframe.h"
#include "bespin.h"
#include "hacks.h"

#include "macmenu.h"

#include "animator/hover.h"
#include "animator/aprogress.h"
#include "animator/tab.h"

#include "makros.h"
#undef CCOLOR
#undef FCOLOR
#define CCOLOR(_TYPE_, _FG_) PAL.color(QPalette::Active, config._TYPE_##_role[_FG_])
#define FCOLOR(_TYPE_) PAL.color(QPalette::Active, QPalette::_TYPE_)

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

void BespinStyle::polish ( QApplication * app )
{
    QPalette pal = app->palette();
    polish(pal);
    app->setPalette(pal);
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
   // AlternateBase
   pal.setColor(QPalette::AlternateBase,
                Colors::mid(pal.color(QPalette::Active, QPalette::Base),
                            pal.color(QPalette::Active, QPalette::Text),15,1));
   
   // highlight colors
   const int highlightGray = qGray(pal.color(QPalette::Active, QPalette::Highlight).rgb());
   const QColor grey(highlightGray,highlightGray,highlightGray);
   pal.setColor(QPalette::Disabled, QPalette::Highlight, grey);

#if QT_VERSION >= 0x040400
   // tooltip (NOTICE not configurable by qtconfig, kde can, let's see what we're gonna do on this...)
   pal.setColor(QPalette::ToolTipBase, pal.color(QPalette::Active, QPalette::WindowText));
   pal.setColor(QPalette::ToolTipText, pal.color(QPalette::Active, QPalette::Window));
#endif

   // inactive palette
   if (config.fadeInactive) { // fade out inactive foreground and highlight colors...
      pal.setColor(QPalette::Inactive, QPalette::Highlight,
                   Colors::mid(pal.color(QPalette::Active, QPalette::Highlight), grey, 2,1));
      pal.setColor(QPalette::Inactive, QPalette::WindowText,
                   Colors::mid(pal.color(QPalette::Active, QPalette::Window),
                               pal.color(QPalette::Active, QPalette::WindowText), 1,4));
      pal.setColor(QPalette::Inactive, QPalette::ButtonText,
                   Colors::mid(pal.color(QPalette::Active, QPalette::Button),
                               pal.color(QPalette::Active, QPalette::ButtonText), 1,4));
      pal.setColor(QPalette::Inactive, QPalette::Text,
                   Colors::mid(pal.color(QPalette::Active, QPalette::Base),
                               pal.color(QPalette::Active, QPalette::Text), 1,4));
   }

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

    // more on tooltips... (we force some colors...)
    QPalette toolPal = QToolTip::palette();
    const QColor bg = pal.color(QPalette::Active, QPalette::WindowText);
    const QColor fg = pal.color(QPalette::Active, QPalette::Window);
    toolPal.setColor(QPalette::Window, bg);
    toolPal.setColor(QPalette::WindowText, fg);
    toolPal.setColor(QPalette::Base, bg);
    toolPal.setColor(QPalette::Text, fg);
    toolPal.setColor(QPalette::Button, bg);
    toolPal.setColor(QPalette::ButtonText, fg);
    toolPal.setColor(QPalette::Highlight, fg); // sic!
    toolPal.setColor(QPalette::HighlightedText, bg); // sic!
    toolPal.setColor(QPalette::ToolTipBase, bg);
    toolPal.setColor(QPalette::ToolTipText, fg);
    QToolTip::setPalette(toolPal);
}

#if 0
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

   // !!! protect against polishing QObject trials ! (this REALLY happens from time to time...)
   if (!widget) return;

   // GTK-Qt gets a special handling - see above
   if (isGTK) {
      polishGTK(widget); return;
   }

   // NONONONONO!!!!! ;)
   if (qobject_cast<VisualFramePart*>(widget)) return;

   // apply any user selected hacks
   Hacks::add(widget);

   //BEGIN Window handling                                                                   -
   if (widget->isWindow() && !widget->inherits("QTipLabel")) {
      QPalette pal = widget->palette();

      // talk to kwin about colors, gradients, etc.
      setupDecoFor(widget);

      if (config.bg.mode > Scanlines)
         widget->setAttribute(Qt::WA_StyledBackground);

      //BEGIN Popup menu handling                                                                  -
      if (QMenu *menu = qobject_cast<QMenu *>(widget)) {
         // glass mode popups
         if (config.menu.glassy) {
            if (config.bg.mode == Scanlines) {
               QPalette pal = widget->palette();
               pal.setBrush(QPalette::Background, pal.color(QPalette::Active, QPalette::Background));
               menu->setPalette(pal);
            }
            menu->setAttribute(Qt::WA_MacBrushedMetal);
            menu->setAttribute(Qt::WA_StyledBackground);
         }
         // apple style popups
         if (config.bg.mode == Scanlines) {
            QPalette pal = menu->palette();
            QColor c = pal.color(QPalette::Active, QPalette::Window);
            if (!_scanlines[1]) makeStructure(&_scanlines[1], c, true);
            QBrush brush( c, *_scanlines[1] );
            pal.setBrush( QPalette::Window, brush );
            menu->setPalette(pal);
         }
         // opacity
         menu->setWindowOpacity( config.menu.opacity/100.0 );
         // color swapping
         menu->setAutoFillBackground(true);
         menu->setBackgroundRole ( config.menu.std_role[0] );
         menu->setForegroundRole ( config.menu.std_role[1] );
         if (config.menu.boldText) {
            QFont tmpFont = menu->font();
            tmpFont.setBold(true);
            menu->setFont(tmpFont);
         }
         // eventfiltering to reposition MDI windows and correct distance to menubars
//          if (menu->parentWidget() && menu->parentWidget()->inherits("QMdiSubWindow"))
            menu->installEventFilter(this); // reposition / repos MDI context / shaping
//          if (bar4popup(menu))
//             menu->installEventFilter(this); // reposition
#if 0
// NOTE this was intended to be for some menu mock from nuno where the menu reaches kinda ribbon-like into the bar
         if (bar4popup(menu)) {
            QAction *action = new QAction( menu->menuAction()->iconText(), menu );
            connect (action, SIGNAL(triggered(bool)), menu, SLOT(hide()));
            menu->insertAction(menu->actions().at(0), action);
//             menu->installEventFilter(this); // reposition/round corners
         }
#endif
      }
      //END Popup menu handling                                                                  -
      // modal dialogs, the modality isn't necessarily set yet, so we catch it on QEvent::Show, see bespin.cpp
      else if (config.bg.modal.invert || config.bg.modal.glassy || config.bg.modal.opacity < 100)
         widget->installEventFilter(this);
      
   }
   //END Window handling                                                                   -
   
#ifdef MOUSEDEBUG
   widget->installEventFilter(this);
#endif

   //BEGIN Hover widgets                                                                         -
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
//        || widget->inherits("QDockWidget")
//        || widget->inherits("QToolBar")
       || widget->inherits("QToolBarHandle")
//        || widget->inherits("QDockSeparator")
//        || widget->inherits("QToolBoxButton")
//        || widget->inherits("QDockWidgetSeparator")
       || widget->inherits("Q3DockWindowResizeHandle")
      )
      widget->setAttribute(Qt::WA_Hover);

   // Enable hover effects in listview, all itemviews like in kde is pretty annoying
   if (QAbstractItemView *itemView = qobject_cast<QAbstractItemView*>(widget) )
   if (!itemView->inherits("QTreeView")) // treeview hovering sucks, as the "tree" doesn't get an update
      itemView->viewport()->setAttribute(Qt::WA_Hover);
   //END Hover widgets                                                                         -

   // PUSHBUTTONS - hovering/animation
   if (qobject_cast<QAbstractButton*>(widget)) {
      if (widget->inherits("QToolBoxButton") ||
          widget->objectName() == "RenderFormElementWidget" ) { // KHtml
         widget->setAttribute(Qt::WA_Hover);
      }
      else
         Animator::Hover::manage(widget);
      if (widget->inherits("KMultiTabBarTab")) {
         // NOTICE this works around a bug? - at least this widget uses the style to paint the bg, but hardcodes the fg...
         // TODO: inform Joseph Wenninger <jowenn@kde.org> and really fix this
         // (fails all styles w/ Windowcolored ToolBtn and QPalette::ButtonText != QPalette::WindowText settings)
         QPalette pal = widget->palette();
         pal.setColor(QPalette::Active, QPalette::Button, pal.color(QPalette::Active, QPalette::Window));
         pal.setColor(QPalette::Inactive, QPalette::Button, pal.color(QPalette::Inactive, QPalette::Window));
         pal.setColor(QPalette::Disabled, QPalette::Button, pal.color(QPalette::Disabled, QPalette::Window));

         pal.setColor(QPalette::Active, QPalette::ButtonText, pal.color(QPalette::Active, QPalette::WindowText));
         pal.setColor(QPalette::Inactive, QPalette::ButtonText, pal.color(QPalette::Inactive, QPalette::WindowText));
         pal.setColor(QPalette::Disabled, QPalette::ButtonText, pal.color(QPalette::Disabled, QPalette::WindowText));
         widget->setPalette(pal);
      }
   }
   // COMBOBOXES - hovering/animation
   else if (widget->inherits("QComboBox")) {
      if (widget->objectName() == "RenderFormElementWidget") // KHtml
         widget->setAttribute(Qt::WA_Hover);
      else
         Animator::Hover::manage(widget);
   }
   
   else if (qobject_cast<QScrollBar *>(widget)) {
      // NOTICE QAbstractSlider::setAttribute(Qt::WA_OpaquePaintEvent) gains surprisinlgy little CPU
      // as the slider is usually not bound to e.g. a scrollarea
      // so that'd just gonna add more complexity... for literally nothing
         QWidget *dad = widget;
         if (!widget->parentWidget()) {
             // this catches e.g. plasma used graphicsproxywidgets...
             qWarning("Bespin, transparent scrollbar!");
             widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
         }
         while ((dad = dad->parentWidget())) {
            // btw: userer väter väter väter väter...? hail to Monty Python!
            if (dad->inherits("KHTMLView")) {
               // NOTICE this slows down things as it triggers a repaint of the frame
               // but it's necessary for KHtml scrollers...
               widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
               // this reenbles speed and currently does the job - TODO how's css/khtml policy on applying colors?
               widget->setAutoFillBackground ( true );
               widget->setBackgroundRole ( QPalette::Base ); // QPalette::Window looks wrong
               break;
            }
         }
         // Scrollarea hovering
         // QAbstractScrollArea is allready handled, but might not be the only scrollbar container!
         QWidget *area = 0;
         if (widget->parentWidget()) {
            area = widget->parentWidget();
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
   // PROGRESSBARS - animation and bold font
   else if (qobject_cast<QProgressBar*>(widget)) {
      QFont fnt = widget->font(); fnt.setBold(true);
      widget->setFont(fnt);
      Animator::Progress::manage(widget);
   }

   // Tab Transition
   // NOTICE do NOT(!) apply this on tabs explicitly, as they contain a stack!
   else if (widget->inherits("QStackedWidget"))
      Animator::Tab::manage(widget);
   // tabbar colors (to avoid flicker)
   else if (qobject_cast<QTabBar *>(widget)) {
      // the eventfilter overtakes the widget painting to allow tabs ABOVE the tabbar
      widget->installEventFilter(this);
   }
   
   // Menubars and toolbar default to QPalette::Button - looks crap and leads to flicker...?!
   bool isTopContainer = false // to simplify the #ifdefs
#ifndef QT_NO_MENUBAR
       || qobject_cast<QMenuBar *>(widget)
#endif
#ifdef QT3_SUPPORT
       || widget->inherits("Q3ToolBar")
#endif
#ifndef QT_NO_TOOLBAR
       || qobject_cast<QToolBar *>(widget)
#endif
    ;
    if (isTopContainer
#ifndef QT_NO_TOOLBAR
       || qobject_cast<QToolBar *>(widget->parent())
#endif
      )
      {
         widget->setBackgroundRole(QPalette::Window);
         widget->setForegroundRole(QPalette::WindowText);
         if (isTopContainer && config.bg.mode == Scanlines) {
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

   if (QMenuBar *mbar = qobject_cast<QMenuBar *>(widget))
      MacMenu::manage(mbar);

   //BEGIN Frames                                                                      -
   if (!widget->isWindow())
   if (QFrame *frame = qobject_cast<QFrame *>(widget)) {

      // sunken looks soo much nicer ;)
      if (frame->parentWidget() && frame->parentWidget()->inherits("KTitleWidget"))
         frame->setFrameShadow(QFrame::Sunken);
      
      // Kill ugly winblows frames... (qShadeBlablabla stuff)
      else if (frame->frameShape() == QFrame::Box || frame->frameShape() == QFrame::Panel ||
                                                frame->frameShape() == QFrame::WinPanel)
         frame->setFrameShape(QFrame::StyledPanel);

      // Kill ugly line look (we paint our styled v and h lines instead ;)
      else if (frame->frameShape() == QFrame::HLine || frame->frameShape() == QFrame::VLine)
         widget->installEventFilter(this);

      // scrollarea hovering
      if (qobject_cast<QAbstractScrollArea*>(frame)
#ifdef QT3_SUPPORT
        || frame->inherits("Q3ScrollView")
#endif
          ) {
         Animator::Hover::manage(frame);
         // allow all treeviews to be animated!
         if (config.hack.treeViews)
         if (QTreeView* tv = qobject_cast<QTreeView*>(frame))
            tv->setAnimated ( true );
      }

      // QToolBox handling - a shame they look that crap by default!
      if (widget->inherits("QToolBox")) {
         // get rid of QPalette::Button
         widget->setBackgroundRole(QPalette::Window);
         widget->setForegroundRole(QPalette::WindowText);
         // get rid of nasty indention
         if (widget->layout()) {
            widget->layout()->setMargin ( 0 );
            widget->layout()->setSpacing ( 0 );
         }
      }

      // "frame above content" look
      else if (isSpecialFrame(widget)) { // QTextEdit etc. can be handled more efficiently
         if (frame->lineWidth() == 1)
            frame->setLineWidth(dpi.f4); // but must have enough indention
      }
      else
         VisualFrame::manage(frame);
   }

   // this is for QToolBox kids - they're autofilled by default - what looks crap
   // canNOT(!) be handled above (they're just usually no frames...)
   if (widget->autoFillBackground() &&
       // dad
       widget->parentWidget() &&
       ( widget->parentWidget()->objectName() == "qt_scrollarea_viewport" ) &&
       //grampa
       widget->parentWidget()->parentWidget() &&
       qobject_cast<QAbstractScrollArea*>(widget->parentWidget()->parentWidget()) &&
       // grangrampa
       widget->parentWidget()->parentWidget()->parentWidget() &&
       widget->parentWidget()->parentWidget()->parentWidget()->inherits("QToolBox") ) {
         widget->parentWidget()->setAutoFillBackground(false);
         widget->setAutoFillBackground(false);
   }

   // KHtml css colors can easily get messed up
   // either because i'm unsure about what colors are set or KHtml does wrong OR (mainly) by html "designers"
   // the eventfilter watches palette changes and ensures contrasted foregrounds...
   if (widget->objectName() == "RenderFormElementWidget") {
      widget->installEventFilter(this);
      QEvent ev(QEvent::PaletteChange);
      eventFilter(widget, &ev);
   }

    // ... the widget uses daddies palette and foregroundrole, but tooltipbase for background
    // so this is Qt bug WORKAROUND
    if (widget->inherits("QWhatsThat"))
        widget->setPalette(QToolTip::palette());
   
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
   if (QMenuBar *mbar = qobject_cast<QMenuBar *>(widget))
      MacMenu::release(mbar);
/*   if (qobject_cast<VisualFramePart*>(widget)) {
      delete widget; widget = 0L; return
   }*/
   
   Animator::Hover::release(widget);
   Animator::Progress::release(widget);
   Animator::Tab::release(widget);
   Hacks::remove(widget);

   widget->removeEventFilter(this);
}
#undef CCOLOR
#undef FCOLOR
