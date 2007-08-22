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

/**================== Qt4 includes ======================*/
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QEvent>
#include <QList>
#include <QResizeEvent>
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QGroupBox>
#include <QPixmap>
#include <QImage>
#include <QDesktopWidget>
#include <QX11Info>
#include <QStylePlugin>
#include <QProgressBar>
#include <QMenu>
#include <QMenuBar>
#include <QStyleOptionProgressBarV2>
#include <QLayout>
#include <QListWidget>
#include <QAbstractButton>
#include <QPushButton>
#include <QScrollBar>
#include <QTabBar>
#include <QTabWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QSplitterHandle>
#include <QToolBar>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QAbstractScrollArea>
#include <QProcess>
/**============= Qt3 support includes ======================*/
#include <Q3ScrollView>
/**========================================================*/

/**============= System includes ==========================*/

#ifdef Q_WS_X11
// #include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
// #include <fixx11h.h>
#endif
#ifndef QT_NO_XRENDER
#include <X11/extensions/Xrender.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cmath>
/**========================================================*/

/**============= DEBUG includes ==========================*/
#undef DEBUG
#ifdef DEBUG
#define MOUSEDEBUG 1
#include <QtDebug>
#include "debug.h"
#define oDEBUG qDebug()
#include <QTime>
#include <QTimer>
#define _PROFILESTART_ QTime timer; int time; timer.start();
#define _PROFILERESTART_ timer.restart();
#define _PROFILESTOP_(_STRING_) time = timer.elapsed(); qDebug("%s: %d",_STRING_,time);
#else
#define oDEBUG //
#undef MOUSEDEBUG
#endif

/**========================================================*/

/**============= Bespin includes ==========================*/
#include "bespin.h"
#include "colors.h"
#include "makros.h"
#ifndef QT_NO_XRENDER
#include "oxrender.h"
#endif
#include "visualframe.h"
/**=========================================================*/


/**============= extern C stuff ==========================*/
class BespinStylePlugin : public QStylePlugin
{
public:
   QStringList keys() const {
      return QStringList() << "Bespin";
   }
   
   QStyle *create(const QString &key) {
      if (key == "bespin")
         return new Bespin::BespinStyle;
      return 0;
   }
};

Q_EXPORT_PLUGIN2(BespinStyle, BespinStylePlugin)
/**=========================================================*/
using namespace Bespin;

/** static config object */
Config config;
Dpi dpi;
Gradients ::Type _progressBase;

/** Let's try if we can supply round frames that shape their content */

/** Get some excluded code */

void BespinStyle::makeStructure(int num, const QColor &c)
{
   if (!_scanlines[num])
      _scanlines[num] = new QPixmap(64, 64);
   QPainter p(_scanlines[num]);
   switch (config.bg.structure)
   {
   default:
   case 0: { // scanlines
      _scanlines[num]->fill( c.light(110).rgb() );
      p.setPen( (num == 1) ? c.light(106) : c.light(103) );
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
      if (num == 1) {
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
      if (num == 1) {
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
      _scanlines[num]->fill( c.light(103).rgb() );
      p.setPen(QPen((num == 1) ? c.light(101) : c, 3));
      p.setBrush( c.dark(102) );
      p.drawRect(-3,8,70,8);
      p.drawRect(-3,24,70,8);
      p.drawRect(-3,40,70,8);
      p.drawRect(-3,56,70,8);
      break;
   case 3: // "blue"print
      _scanlines[num]->fill( c.dark(101).rgb() );
      p.setPen((num == 1) ? c.light(104) : c.light(102));
      for ( int i = 0; i < 64; i += 16 )
         p.drawLine( 0, i, 63, i );
      for ( int i = 0; i < 64; i += 16 )
         p.drawLine( i, 0, i, 63 );
      break;
   case 4: // verticals
      _scanlines[num]->fill( c.light(110).rgb() );
      p.setPen( (num == 1) ? c.light(106) : c.light(103) );
      for ( int i = 1; i < 64; i += 4 ) {
         p.drawLine( i, 0, i, 63 );
         p.drawLine( i+2, 0, i+2, 63 );
      }
      p.setPen( c );
      for ( int i = 2; i < 63; i += 4 )
         p.drawLine( i, 0, i, 63 );
      break;
   }
   p.end();
}

#define readRole(_ENTRY_, _VAR_, _DEF1_, _DEF2_)\
config._VAR_##_role[0] = (QPalette::ColorRole) iSettings->value(_ENTRY_, QPalette::_DEF1_).toInt();\
Colors::counterRole(config._VAR_##_role[0], config._VAR_##_role[1], QPalette::_DEF1_, QPalette::_DEF2_)

#define gradientType(_ENTRY_, _DEF_)\
(Gradients::Type) iSettings->value(_ENTRY_, Gradients::_DEF_).toInt();

static void updatePalette(QPalette &pal, QPalette::ColorGroup group, const QStringList &list) {
   for (int i = 0; i < QPalette::NColorRoles; i++)
      pal.setColor(group, (QPalette::ColorRole) i, list.at(i));
}

static QStringList colors(const QPalette &pal, QPalette::ColorGroup group) {
   QStringList list;
   for (int i = 0; i < QPalette::NColorRoles; i++)
      list << pal.color(group, (QPalette::ColorRole) i).name();
   return list;
}

void BespinStyle::readSettings(const QSettings* settings)
{
   bool delSettings = false;
   QSettings *iSettings = const_cast<QSettings*>(settings);
   if (!iSettings) {
      delSettings = true;
      char *preset = getenv("BESPIN_PRESET");
      if (preset) {
         iSettings = new QSettings("Bespin", "Store");
         if (iSettings->childGroups().contains(preset)) {
            iSettings->beginGroup(preset);
            // set custom palette!
            QPalette pal;
            iSettings->beginGroup("QPalette");
            QStringList list =
                  iSettings->value ( "active", colors(pal,
                                     QPalette::Active) ).toStringList();
            updatePalette(pal, QPalette::Active, list);
            list = iSettings->value ( "inactive", colors(pal,
                                      QPalette::Inactive) ).toStringList();
            updatePalette(pal, QPalette::Inactive, list);
            list = iSettings->value ( "disabled", colors(pal,
                                      QPalette::Disabled) ).toStringList();
            updatePalette(pal, QPalette::Disabled, list);
            polish(pal);
            qApp->setPalette(pal);
            iSettings->endGroup();
         }
         else {
            delete iSettings; iSettings = 0L;
         }
         free(preset);
      }
      if (!iSettings) {
         iSettings = new QSettings("Bespin", "Style");
         iSettings->beginGroup("Style");
      }
   }
   else
      qWarning("Bespin: WARNING - reading EXTERNAL settings!!!");
   
   
   // Background ===========================
   config.bg.mode = (BGMode) iSettings->value("Bg.Mode", BevelV).toInt();

#ifndef QT_NO_XRENDER
   if (config.bg.mode > LightH)
      config.bg.mode = BevelV;
   else if(config.bg.mode == ComplexLights &&
           !QFile::exists(QDir::tempPath() + "bespinPP.lock"))
      QProcess::startDetached ( iSettings->value("Bg.Daemon", "bespin pusher").toString() );
#else
   if (config.bg.mode == ComplexLights) config.bg.mode = BevelV;
#endif
   
   if (config.bg.mode == Scanlines)
      config.bg.structure = iSettings->value("Bg.Structure", 0).toInt();
   
   // Buttons ===========================
   config.btn.layer = CLAMP(iSettings->value("Btn.Layer", 0).toInt(), 0, 2);
   config.btn.checkType = iSettings->value("Btn.CheckType", 0).toInt();
   
   GRAD(btn) = gradientType("Btn.Gradient", Button);
   _progressBase = GRAD(btn);

   if (config.btn.layer == 2 && GRAD(btn) == Gradients::Sunken) // NO!
      GRAD(btn) = Gradients::None;
   
   if (config.btn.layer == 2)
      config.btn.cushion = true;
   else if (GRAD(btn) ==  Gradients::Sunken)
      config.btn.cushion = false;
   else
      config.btn.cushion = iSettings->value("Btn.Cushion", true).toBool();
   config.btn.fullHover = iSettings->value("Btn.FullHover", true).toBool();
   readRole("Btn.Role", btn.std, Window, WindowText);
   readRole("Btn.ActiveRole", btn.active, Button, ButtonText);
   Colors::setButtonRoles(config.btn.std_role[0], config.btn.std_role[1],
                       config.btn.active_role[0], config.btn.active_role[1]);
   
   // Choosers ===========================
   GRAD(chooser) = gradientType("Chooser.Gradient", Sunken);
   
   // PW Echo Char ===========================
   config.input.pwEchoChar =
         ushort(iSettings->value("Input.PwEchoChar", 0x26AB).toUInt());
   
   // Menus ===========================
   config.menu.itemGradient = gradientType("Menu.ItemGradient", None);
   config.menu.showIcons = iSettings->value("Menu.ShowIcons", false).toBool();
   config.menu.shadow = iSettings->value("Menu.Shadow", false).toBool();
   readRole("Menu.ActiveRole", menu.active, Highlight, HighlightedText);
   readRole("Menu.Role", menu.std, Window, WindowText);
   
   // Progress ===========================
   GRAD(progress) = gradientType("Progress.Gradient", Gloss);
   readRole("Progress.Role", progress.std, Highlight, HighlightedText);
   
   // ScrollStuff ===========================
   GRAD(scroll) =
         (Gradients::Type) iSettings->value("Scroll.Gradient", GRAD(btn)).toInt();
   config.scroll.showButtons =
         iSettings->value("Scroll.ShowButtons", false).toBool();
   config.scroll.sunken =
         iSettings->value("Scroll.Sunken", false).toBool();
   config.scroll.groove = (!config.scroll.sunken) ? false :
         iSettings->value("Scroll.Groove", false).toBool();
   
   // Tabs ===========================
   readRole("Tab.ActiveRole", tab.active, Highlight, HighlightedText);
   config.tab.animSteps =
         CLAMP(iSettings->value("Tab.AnimSteps", 5).toUInt(), 2, 18);
   GRAD(tab) = gradientType("Tab.Gradient", Button);
   readRole("Tab.Role", tab.std, Window, WindowText);
   config.tab.transition =
      (TabAnimInfo::TabTransition) iSettings->value("Tab.Transition",
         TabAnimInfo::ScanlineBlend).toInt();

   // ToolBoxes
   config.toolbox.active_role[0] = (QPalette::ColorRole)
         iSettings->value("ToolBox.ActiveRole", config.tab.std_role[Bg]).toInt();
   Colors::counterRole(config.toolbox.active_role[Bg],
                       config.toolbox.active_role[Fg],
                       config.tab.std_role[Bg], config.tab.std_role[Fg]);
   GRAD(toolbox) = (Gradients::Type) iSettings->value("Tab.ActiveGradient", GRAD(tab)).toInt();

   // Views ===========================
   readRole("View.HeaderRole", view.header, Text, Base);
   readRole("View.SortingHeaderRole", view.sortingHeader, Text, Base);
   config.view.headerGradient = gradientType("View.HeaderGradient", Button);
   config.view.sortingHeaderGradient = gradientType("View.SortingHeaderGradient", Sunken);
   
   // General ===========================
   config.scale = iSettings->value("Scale", 1.0).toDouble();

   
   if (delSettings)
      delete iSettings;
}

#undef readRole
#undef gradientType

#define SCALE(_N_) lround(_N_*config.scale)

#include "genpixmaps.cpp"

void BespinStyle::initMetrics()
{
   dpi.f1 = SCALE(1); dpi.f2 = SCALE(2);
   dpi.f3 = SCALE(3); dpi.f4 = SCALE(4);
   dpi.f5 = SCALE(5); dpi.f6 = SCALE(6);
   dpi.f7 = SCALE(7); dpi.f8 = SCALE(8);
   dpi.f9 = SCALE(9); dpi.f10 =SCALE(10);
   
   dpi.f12 = SCALE(12); dpi.f13 = SCALE(13);
   dpi.f16 = SCALE(16); dpi.f18 = SCALE(18);
   dpi.f20 = SCALE(20); dpi.f32 = SCALE(32);
   dpi.f80 = SCALE(80);
   
   dpi.ScrollBarExtent = SCALE(17);
   dpi.ScrollBarSliderMin = SCALE(40);
   dpi.SliderThickness = SCALE(24);
   dpi.SliderControl = SCALE(19);
   dpi.Indicator = config.btn.layer == 2 ? SCALE(16) : SCALE(20);
   dpi.ExclusiveIndicator = config.btn.layer ? SCALE(16) : SCALE(19);
}

#undef SCALE

void BespinStyle::init(const QSettings* settings) {
   readSettings(settings);
   initMetrics();
   generatePixmaps();
   Gradients::init(config.bg.mode > ComplexLights ?
                   (Gradients::BgMode)config.bg.mode : Gradients::BevelV);
}

/**THE STYLE ITSELF*/
BespinStyle::BespinStyle() : QCommonStyle(), mouseButtonPressed_(false),
internalEvent_(false) {
   _scanlines[0] = _scanlines[1] = 0L;
   init();

   //====== TOOLTIP ======================
//    tooltipPalette = qApp->palette();
//    tooltipPalette.setBrush( QPalette::Background, QColor( 255, 255, 220 ) );
//    tooltipPalette.setBrush( QPalette::Foreground, Qt::black );
   //=======================================
   

   // start being animated
   animator = new StyleAnimator(this, config.tab.transition, config.tab.animSteps);

}

BespinStyle::~BespinStyle() {
   Gradients::wipe();
}

void BespinStyle::fillWithMask(QPainter *painter, const QPoint &xy, const QBrush &brush, const QPixmap &mask, QPoint offset) const
{
   QPixmap qPix(mask.size());
   if (brush.texture().isNull())
      qPix.fill(brush.color());
   else {
      QPainter p(&qPix);
      p.drawTiledPixmap(mask.rect(),brush.texture(),offset);
      p.end();
   }
#ifndef QT_NO_XRENDER
   qPix = OXRender::applyAlpha(qPix, mask);
#else
#warning no XRender - performance will suffer!
   qPix.setAlphaChannel(mask);
#endif
   painter->drawPixmap(xy, qPix);
}


/**======================================*/

/**QStyle reimplementation ========================================= */

void BespinStyle::polish ( QApplication * app ) {
//    if (timer && !timer->isActive())
//       timer->start(50);
   QPalette pal = app->palette();
   polish(pal);
   app->setPalette(pal);
//    app->installEventFilter(this);
}

#define _SHIFTCOLOR_(clr) clr = QColor(CLAMP(clr.red()-10,0,255),CLAMP(clr.green()-10,0,255),CLAMP(clr.blue()-10,0,255))
#include <QtDebug>
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
      makeStructure(0, c);
      QBrush brush( c, *_scanlines[0] );
      pal.setBrush( QPalette::Background, brush );
   }
   
   int highlightGray = qGray(pal.color(QPalette::Active, QPalette::Highlight).rgb());
   pal.setColor(QPalette::Disabled, QPalette::Highlight,
                QColor(highlightGray,highlightGray,highlightGray));
   pal.setColor(QPalette::Active, QPalette::AlternateBase,
                Colors::mid(pal.color(QPalette::Active, QPalette::Base),
                         pal.color(QPalette::Active, QPalette::Text),15,1));
   
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
   
   config.strongFocus =
         Colors::haveContrast(pal.color(QPalette::Active, QPalette::Window),
                                pal.color(QPalette::Active, QPalette::Highlight));
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

#ifdef Q_WS_X11
static Atom winTypePopup = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_POPUP_MENU", False);
static Atom winType = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False);
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
   
   else if (widget->inherits("QMdiSubWindow"))
      widget->installEventFilter(this);
   else if (widget->inherits("QWorkspace"))
      connect(this, SIGNAL(MDIPopup(QPoint)), widget, SLOT(_q_popupOperationMenu(QPoint)));

   
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
            makeStructure(1, c);
         QBrush brush( c, *_scanlines[1] );
         pal.setBrush( QPalette::Window, brush );
         widget->setPalette(pal);
      }
   }
   
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
            QWidget *grampa = frame;
            while (grampa->parentWidget() &&
                   !(grampa->isWindow() || grampa->inherits("QMdiSubWindow")))
               grampa = grampa->parentWidget();
               
//             if (!grampa) grampa = frame;
            QList<VisualFrame*> vfs = grampa->findChildren<VisualFrame*>();
            bool addVF = true;
            foreach (VisualFrame* vf, vfs)
               if (vf->frame() == frame) { addVF = false; break; }
            if (addVF) {
               int f2 = dpi.f2, f3 = dpi.f3, f4 = dpi.f4, f6 = dpi.f6;
               int s[4]; uint t[4]; // t/b/l/r
               if (frame->frameShadow() == QFrame::Sunken) {
                  s[0] = s[2] = s[3] = 0; s[1] = f3;
                  t[0] = t[1] = t[2] = t[3] = f4;
               }
               else if (frame->frameShadow() == QFrame::Raised) {
                  s[0] = f2; s[1] = f4; s[2] = s[3] = f2;
                  t[0] = t[2] = t[3] = f4; t[1] = f6;
               }
               else { // plain
                  s[0] = s[1] = s[2] = s[3] = f2;
                  t[0] = t[1] = t[2] = t[3] = f2;
               }
               new VisualFrame(grampa, frame, VisualFrame::North,
                               t[0], s[0], s[2], s[3]);
               new VisualFrame(grampa, frame, VisualFrame::South,
                               t[1], s[1], s[2], s[3]);
               new VisualFrame(grampa, frame, VisualFrame::West,
                               t[2], s[2], t[0]-s[0], t[1]-s[1], t[0], t[1]);
               new VisualFrame(grampa, frame, VisualFrame::East,
                               t[3], s[3], t[0]-s[0], t[1]-s[1], t[0], t[1]);
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
   if (QMenu * menu = qobject_cast<QMenu *>(widget)) {
#ifdef Q_WS_X11
      // this should tell beryl et. al this is a popup - doesn't work... yet
      XChangeProperty(QX11Info::display(), widget->winId(), winType,
                      XA_CARDINAL, 32, PropModeReplace, (const unsigned char*)&winTypePopup, 1L);
#endif
      // WARNING: compmgrs like e.g. beryl/emerald deny to shadow shaped windows,
      // if we cannot find a way to get ARGB menus independent from the app settings, the compmgr must handle the round corners here
//       widget->setAutoFillBackground (true);
      widget->setBackgroundRole ( config.menu.std_role[0] );
      widget->setForegroundRole ( config.menu.std_role[1] );
//       if (qGray(widget->palette().color(QPalette::Active, widget->backgroundRole()).rgb()) < 100) {
//          QFont tmpFont = widget->font();
//          tmpFont.setBold(true);
//          widget->setFont(tmpFont);
//       }
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

bool BespinStyle::eventFilter( QObject *object, QEvent *ev ) {
   switch (ev->type()) {
   case QEvent::MouseMove:
   case QEvent::Timer:
   case QEvent::Move:
      return false; // just for performance - they can occur really often
   case QEvent::Paint: {
      if (QFrame *frame = qobject_cast<QFrame*>(object)) {
         if (frame->frameShape() == QFrame::HLine ||
             frame->frameShape() == QFrame::VLine) {
            if (frame->isVisible()) {
               QPainter p(frame);
               Orientation3D o3D =
                  (frame->frameShadow() == QFrame::Sunken) ? Sunken:
                  (frame->frameShadow() == QFrame::Raised) ? Raised : Relief;
               const bool v = frame->frameShape() == QFrame::VLine;
               shadows.line[v][o3D].render(frame->rect(), &p);
               p.end();
            }
            return true;
         }
      }
      else if (QTabBar *tabBar = qobject_cast<QTabBar*>(object)) {
         if (tabBar->parentWidget() &&
             qobject_cast<QTabWidget*>(tabBar->parentWidget()))
            return false; // no extra tabbar here please...
         QPainter p(tabBar);
         QStyleOptionTabBarBase opt;
         opt.initFrom(tabBar);
         drawPrimitive ( PE_FrameTabBarBase, &opt, &p);
         p.end();
         return false;
      }
      return false;
   }
#if SHAPE_POPUP
   case QEvent::Resize: {
      if (QMenu *menu = qobject_cast<QMenu*>(object)) {
         QAction *head = menu->actions().at(0);
         QRect r = menu->fontMetrics().boundingRect(menu->actionGeometry(head),
         Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine | Qt::TextExpandTabs | Qt::TextShowMnemonic,
         head->iconText());
         r.adjust(-dpi.f5, -dpi.f2, dpi.f5, dpi.f2);
         QResizeEvent *rev = (QResizeEvent*)ev;
         QRegion mask(menu->rect());
         mask -= QRect(0,0,menu->width(),r.bottom());
         mask += r;
         mask -= masks.corner[0]; // tl
         QRect br = masks.corner[1].boundingRect();
         mask -= masks.corner[1].translated(r.right()-br.width(), 0); // tr
         br = masks.corner[2].boundingRect();
         mask -= masks.corner[2].translated(0, menu->height()-br.height()); // bl
         br = masks.corner[3].boundingRect();
         mask -= masks.corner[3].translated(menu->width()-br.width(),
                                            menu->height()-br.height()); // br
         menu->setMask(mask);
      }
      return false;
   }
#endif
   case QEvent::MouseButtonPress: {
      QMouseEvent *mev = (QMouseEvent*)ev;
#ifdef MOUSEDEBUG
      qDebug() << object;
#endif
      if (( mev->button() == Qt::LeftButton) &&
          object->inherits("QMdiSubWindow")) {
         //TODO this is a hack to get the popupmenu to the right side. bug TT to query the position with a SH
         QWidget *MDI = (QWidget*)object;
         // check for menu button
//          QWidget *MDI = qobject_cast<QWidget*>(widget->parent()); if (!MDI) return false; //this is elsewhat...
         /// this does not work as TT keeps the flag in a private to the titlebar (for no reason?)
//             if (!(widget->windowFlags() & Qt::WindowSystemMenuHint)) return false;
         // check if we clicked it..
//          if (mev->x() < widget->width()-widget->height()-2) return false;
         // find popup
         MDI = qobject_cast<QWidget*>(MDI->parent()); if (!MDI) return false; //this is elsewhat...
         MDI = MDI->findChild<QMenu *>("qt_internal_mdi_popup");
         if (!MDI) {
            qWarning("MDI popup not found, unable to calc menu position");
            return false;
         }
         // calc menu position
         emit MDIPopup(mev->globalPos());
//                        MDI->mapToGlobal( QPoint(widget->width() - MDI->sizeHint().width(), widget->height())));
         return true;
      }
      return false;
   }
#if SHAPE_POPUP
   case QEvent::Show:
      if (QMenu * menu = qobject_cast<QMenu*>(object)) {
         QMenuBar *bar = bar4popup(menu);
         if (bar) {
            QPoint pos(dpi.f3, dpi.f1);
            pos += bar->actionGeometry(menu->menuAction()).topLeft();
            menu->move(bar->mapToGlobal(pos));
            menu->setActiveAction(menu->actions().at(0));
         }
         return false;
      }
      return false;
#endif
   default:
      return false;
   }
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

QPalette BespinStyle::standardPalette () const
{
   QPalette pal ( Qt::black, Qt::white, // windowText, button
                     Qt::white, QColor(200,201,202), QColor(221,222,223), //light, dark, mid
                     Qt::black, Qt::white, //text, bright_text
                     Qt::white, QColor(226,227,231) ); //base, window
   pal.setColor(QPalette::ButtonText, Qt::black);
   pal.setColor(QPalette::Highlight, QColor(78, 156, 234));
   pal.setColor(QPalette::HighlightedText, Qt::white);
   return pal;
}

/** eventcontrol slots*/
#if 0
void BespinStyle::fakeMouse()
{
   if (mouseButtonPressed_) // delayed mousepress for move event
   {
      QCursor::setPos ( cursorPos_ );
      XTestFakeButtonEvent(QX11Info::display(),1, false, 0);
      XTestFakeKeyEvent(QX11Info::display(),XKeysymToKeycode(QX11Info::display(), XK_Alt_L), true, 0);
      XTestFakeButtonEvent(QX11Info::display(),1, true, 0);
      XTestFakeKeyEvent(QX11Info::display(),XKeysymToKeycode(QX11Info::display(), XK_Alt_L), false, 0);
      XFlush(QX11Info::display());
   }
}
#endif
