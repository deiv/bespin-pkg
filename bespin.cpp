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

#include <QAbstractScrollArea>
#include <QEvent>
#include <QPainter>
#include <QStylePlugin>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QFrame>
#include <QToolButton>

/**============= System includes ==========================*/
#ifdef Q_WS_X11
// #include <X11/Xlib.h>
// #include <X11/keysym.h>
// #include <X11/extensions/XTest.h>
// #include <fixx11h.h>
#endif
/**========================================================*/


/**============= Bespin includes ==========================*/

#ifndef QT_NO_XRENDER
#include "oxrender.h"
#endif
// #include "debug.h"
#include "xproperty.h"
#include "colors.h"
#include "bespin.h"

/**=========================================================*/

#include <QtDebug>


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

#define N_PE 54
#define N_CE 50
#define N_CC 10
static void
(BespinStyle::*primitiveRoutine[N_PE])(const QStyleOption*, QPainter*, const QWidget*) const;
static void
(BespinStyle::*controlRoutine[N_CE])(const QStyleOption*, QPainter*, const QWidget*) const;
static void
(BespinStyle::*complexRoutine[N_CC])(const QStyleOptionComplex*, QPainter*, const QWidget*) const;

#define registerPE(_FUNC_, _ELEM_) primitiveRoutine[QStyle::_ELEM_] = &BespinStyle::_FUNC_
#define registerCE(_FUNC_, _ELEM_) controlRoutine[QStyle::_ELEM_] = &BespinStyle::_FUNC_
#define registerCC(_FUNC_, _ELEM_) complexRoutine[QStyle::_ELEM_] = &BespinStyle::_FUNC_

// static void registerPE(char *S0, ...)
// {
//    register char *s;
//    if (s=S0, s!=NULL)  { register char *sa;
//       va_list a;
//       for (va_start(a,S0);  (sa=va_arg(a,char*), sa!=NULL);  )
//          while (*s=*sa, *sa)  ++s,++sa;
//       va_end(a);
//    }
//    return ((int)(s-S0));
// }

void
BespinStyle::registerRoutines()
{
   for (int i = 0; i < N_PE; ++i)
      primitiveRoutine[i] = 0;
   for (int i = 0; i < N_CE; ++i)
      controlRoutine[i] = 0;
   for (int i = 0; i < N_CC; ++i)
      complexRoutine[i] = 0;
   
   // buttons.cpp
   registerPE(drawButtonFrame, PE_PanelButtonCommand);
   registerPE(drawButtonFrame, PE_PanelButtonBevel);
   registerPE(skip, PE_FrameDefaultButton);
   registerCE(drawPushButton, CE_PushButton);
   registerCE(drawPushButtonBevel, CE_PushButtonBevel);
   registerCE(drawPushButtonLabel, CE_PushButtonLabel);
   registerPE(drawCheckBox, PE_IndicatorCheckBox);
   registerPE(drawRadio, PE_IndicatorRadioButton);
   registerCE(drawCheckBoxItem, CE_CheckBox);
   registerCE(drawRadioItem, CE_RadioButton);
   registerCE(drawCheckLabel, CE_CheckBoxLabel);
   registerCE(drawCheckLabel, CE_RadioButtonLabel);
   // docks.cpp
   registerPE(skip, PE_Q3DockWindowSeparator);
   registerPE(skip, PE_FrameDockWidget);
   registerCE(skip, CE_Q3DockWindowEmptyArea);
   registerCE(drawDockTitle, CE_DockWidgetTitle);
   registerPE(drawDockHandle, PE_IndicatorDockWidgetResizeHandle);
   // frames.cpp
   registerCE(skip, CE_FocusFrame);
   registerPE(skip, PE_FrameStatusBar);
#if QT_VERSION >= 0x040400
   registerPE(skip, PE_FrameStatusBarItem);
#endif
   registerPE(drawFocusFrame, PE_FrameFocusRect);
   registerPE(drawFrame, PE_Frame);
   registerCC(drawGroupBox, CC_GroupBox);
   registerPE(drawGroupBoxFrame, PE_FrameGroupBox);
   // input.cpp
   registerPE(drawLineEditFrame, PE_FrameLineEdit);
   registerPE(drawLineEdit, PE_PanelLineEdit);
   registerCC(drawSpinBox, CC_SpinBox);
   registerCC(drawComboBox, CC_ComboBox);
   registerCE(drawComboBoxLabel, CE_ComboBoxLabel);
   // menus.cpp
   registerPE(drawMenuBarBg, PE_PanelMenuBar);
   registerCE(drawMenuBarBg, CE_MenuBarEmptyArea);
   registerCE(drawMenuBarItem, CE_MenuBarItem);
   registerPE(drawMenuFrame, PE_FrameMenu);
   registerCE(drawMenuItem, CE_MenuItem);
   registerCE(drawMenuScroller, CE_MenuScroller);
   registerCE(skip, CE_MenuEmptyArea);
   registerCE(skip, CE_MenuHMargin);
   registerCE(skip, CE_MenuVMargin);
   // progress.cpp
   registerCE(drawProgressBar, CE_ProgressBar);
   registerCE(drawProgressBarGroove, CE_ProgressBarGroove);
   registerCE(drawProgressBarContents, CE_ProgressBarContents);
   registerCE(drawProgressBarLabel, CE_ProgressBarLabel);
   // scrollareas.cpp
   registerCC(drawScrollBar, CC_ScrollBar);
   registerCE(drawScrollBarAddLine, CE_ScrollBarAddLine);
   registerCE(drawScrollBarSubLine, CE_ScrollBarSubLine);
   registerCE(drawScrollBarGroove, CE_ScrollBarSubPage);
   registerCE(drawScrollBarGroove, CE_ScrollBarAddPage);
   registerCE(drawScrollBarSlider, CE_ScrollBarSlider);
   // shapes.cpp
   registerPE(drawItemCheck, PE_IndicatorViewItemCheck);
   registerPE(drawItemCheck, PE_Q3CheckListIndicator);
   registerPE(drawMenuCheck, PE_IndicatorMenuCheckMark);
   registerPE(drawExclusiveCheck, PE_Q3CheckListExclusiveIndicator);
   registerPE(drawSolidArrowN, PE_IndicatorArrowUp);
   registerPE(drawSolidArrowN, PE_IndicatorSpinUp);
   registerPE(drawSolidArrowN, PE_IndicatorSpinPlus);
   registerPE(drawSolidArrowS, PE_IndicatorArrowDown);
   registerPE(drawSolidArrowS, PE_IndicatorSpinDown);
   registerPE(drawSolidArrowS, PE_IndicatorSpinMinus);
   registerPE(drawSolidArrowS, PE_IndicatorButtonDropDown);
   registerPE(drawSolidArrowE, PE_IndicatorArrowRight);
   registerPE(drawSolidArrowW, PE_IndicatorArrowLeft);
   // slider.cpp
   registerCC(drawSlider, CC_Slider);
   registerCC(drawDial, CC_Dial);
   // tabbing.cpp
   registerPE(drawTabWidget, PE_FrameTabWidget);
   registerPE(drawTabBar, PE_FrameTabBarBase);
   registerCE(drawTab, CE_TabBarTab);
   registerCE(drawTabShape, CE_TabBarTabShape);
   registerCE(drawTabLabel, CE_TabBarTabLabel);
   registerPE(skip, PE_IndicatorTabTear);
   registerCE(drawToolboxTab, CE_ToolBoxTab);
   registerCE(drawToolboxTabShape, CE_ToolBoxTabShape);
   registerCE(drawToolboxTabLabel, CE_ToolBoxTabLabel);
   // toolbars.cpp
   registerCC(drawToolButton, CC_ToolButton);
   registerPE(drawToolButtonShape, PE_PanelButtonTool);
   registerPE(skip, PE_IndicatorToolBarSeparator);
   registerPE(skip, PE_PanelToolBar);
   registerCE(drawToolButtonLabel, CE_ToolButtonLabel);
   registerCE(skip, CE_ToolBar);
   registerPE(skip, PE_FrameButtonTool);
   registerPE(skip, PE_Q3Separator);
   registerPE(drawToolBarHandle, PE_IndicatorToolBarHandle);
   // views.cpp
   registerCE(drawHeader, CE_Header);
   registerCE(drawHeaderSection, CE_HeaderSection);
   registerCE(drawHeaderLabel, CE_HeaderLabel);
   registerPE(drawBranch, PE_IndicatorBranch);
   registerCC(drawTree, CC_Q3ListView);
   registerCE(drawRubberBand, CE_RubberBand);
   registerPE(drawHeaderArrow, PE_IndicatorHeaderArrow);
#if QT_VERSION >= 0x040400
   registerPE(drawItem, PE_PanelItemViewRow);
   registerPE(drawItem, PE_PanelItemViewItem);
#endif
   // window.cpp
   registerPE(drawWindowFrame, PE_FrameWindow);
   registerPE(drawWindowBg, PE_Widget);
   registerPE(drawToolTip, PE_PanelTipLabel);
   registerCC(drawTitleBar, CC_TitleBar);
   registerCE(drawDockHandle, CE_Splitter);
   registerCE(drawSizeGrip, CE_SizeGrip);
}

/**THE STYLE ITSELF*/
BespinStyle::BespinStyle() : QCommonStyle(), mouseButtonPressed_(false),
internalEvent_(false) {
   _scanlines[0] = _scanlines[1] = 0L;
   init();
   registerRoutines();

   //====== TOOLTIP ======================
//    tooltipPalette = qApp->palette();
//    tooltipPalette.setBrush( QPalette::Background, QColor( 255, 255, 220 ) );
//    tooltipPalette.setBrush( QPalette::Foreground, Qt::black );
   //=======================================
}

BespinStyle::~BespinStyle() {
   Gradients::wipe();
}

#include "makros.h"
#undef PAL
#define PAL pal

QColor
BespinStyle::btnBg(const QPalette &pal, bool isEnabled, int hasFocus,
                   int step, bool fullHover, bool reflective) const {

   if (!isEnabled)
      return Colors::mid(Qt::black, FCOLOR(Window),5,100);
   
   QColor c = CCOLOR(btn.std, Bg);
   if (hasFocus)
      c = Colors::mid(FCOLOR(Highlight), c,
                      1, 10 + Colors::contrast(FCOLOR(Highlight), c));

   if (fullHover && step)
      c = Colors::mid(c, CCOLOR(btn.active, Bg),
                      (config.btn.backLightHover ? (80-32*reflective) : 6) - step, step);

   return c;
}

QColor
BespinStyle::btnFg(const QPalette &pal, bool isEnabled, int hover, int step, bool flat) const {
   if (!isEnabled)
      return Colors::mid(FCOLOR(Window), FCOLOR(WindowText), 1, 3);

   QColor fg1 = CCOLOR(btn.std, Fg), fg2 = CCOLOR(btn.active, Fg);
   if (flat) {
      fg1 = FCOLOR(WindowText); fg2 = FCOLOR(Highlight);
   }

   if (config.btn.backLightHover)
      return fg1;
   
   if (hover && !step) step = 6;
   if (step)
      return Colors::mid(fg1, fg2, 6 - step, step);

   return fg1;
}

void
BespinStyle::drawPrimitive ( PrimitiveElement pe, const QStyleOption * option,
                             QPainter * painter, const QWidget * widget) const
{
   Q_ASSERT(option);
   Q_ASSERT(painter);
//    if (pe == PE_IndicatorItemViewItemDrop)
// An indicator that is drawn to show where an item in an item view is about to
// be dropped during a drag-and-drop operation in an item view.
//       qWarning("IndicatorItemViewItemDrop, %d", pe);
   if (pe < N_PE && primitiveRoutine[pe])
      (this->*primitiveRoutine[pe])(option, painter, widget);
   else
      QCommonStyle::drawPrimitive( pe, option, painter, widget );
}

void
BespinStyle::drawControl ( ControlElement element, const QStyleOption * option,
                           QPainter * painter, const QWidget * widget) const
{
   Q_ASSERT(option);
   Q_ASSERT(painter);
   if (element < N_CE && controlRoutine[element])
      (this->*controlRoutine[element])(option, painter, widget);
   else
      QCommonStyle::drawControl( element, option, painter, widget );
}

void
BespinStyle::drawComplexControl ( ComplexControl control,
                                  const QStyleOptionComplex * option,
                                  QPainter * painter,
                                  const QWidget * widget) const
{
   Q_ASSERT(option);
   Q_ASSERT(painter);
   if (control < N_CC && complexRoutine[control])
      (this->*complexRoutine[control])(option, painter, widget);
   else
      QCommonStyle::drawComplexControl( control, option, painter, widget );
}

void
BespinStyle::fillWithMask(QPainter *painter, const QPoint &xy,
                          const QBrush &brush, const QPixmap &mask,
                          QPoint offset) const
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

void
BespinStyle::erase(const QStyleOption *option, QPainter *painter,
                   const QWidget *widget) const
{
   const QWidget *grampa = widget;
   while (!(grampa->isWindow() ||
            (grampa->autoFillBackground() &&
             grampa->objectName() != "qt_scrollarea_viewport")))
      grampa = grampa->parentWidget();

   QPoint tl = widget->mapFrom(const_cast<QWidget*>(grampa), QPoint());
   painter->save();
   painter->setPen(Qt::NoPen);
   painter->setBrush(grampa->palette().brush(grampa->backgroundRole()));
   painter->setBrushOrigin(tl);
   painter->drawRect(option->rect);
   if (grampa->isWindow())  { // means we need to paint the global bg as well
      painter->setClipRect(option->rect, Qt::IntersectClip);
      QStyleOption tmpOpt = *option;
      tmpOpt.rect = QRect(tl, grampa->size());
      tmpOpt.palette = grampa->palette();
      painter->fillRect(option->rect, grampa->palette().brush(QPalette::Window));
      drawWindowBg(&tmpOpt, painter, grampa);
   }
   painter->restore();
}

static void swapPalette(QWidget *widget, BespinStyle *style)
{
   QPalette pal(widget->palette());
   QColor h = pal.color(QPalette::Active, QPalette::WindowText);
   pal.setColor(QPalette::Active, QPalette::WindowText, pal.color(QPalette::Active, QPalette::Window));
   pal.setColor(QPalette::Active, QPalette::Window, h);

//    h = pal.color(QPalette::Active, QPalette::Text);
//    pal.setColor(QPalette::Active, QPalette::Text, pal.color(QPalette::Active, QPalette::Base));
//    pal.setColor(QPalette::Active, QPalette::Base, h);

   h = pal.color(QPalette::Active, QPalette::Button);
   pal.setColor(QPalette::Active, QPalette::Button, pal.color(QPalette::Active, QPalette::ButtonText));
   pal.setColor(QPalette::Active, QPalette::ButtonText, h);
   style->polish(pal);
   widget->setPalette(pal);
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

bool
BespinStyle::eventFilter( QObject *object, QEvent *ev )
{
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
         return false;
      }
      else if (QTabBar *tabBar = qobject_cast<QTabBar*>(object)) {
         if (tabBar->parentWidget() &&
             qobject_cast<QTabWidget*>(tabBar->parentWidget()))
            return false; // no extra tabbar here please... ;)
         QPainter p(tabBar);
         QStyleOptionTabBarBase opt;
         opt.initFrom(tabBar);
         if (QWidget *window = tabBar->window())
            opt.tabBarRect = window->rect();
         drawTabBar(&opt, &p, 0L);
         p.end();
         return false;
      }
      return false;
   }
#if 0
   case QEvent::Resize: {
      if (QMenu *menu = qobject_cast<QMenu*>(object)) {
         if (!menu->isWindow()) return false;
#if SHAPE_POPUP
         QAction *head = menu->actions().at(0);
         QRect r = menu->fontMetrics().boundingRect(menu->actionGeometry(head),
         Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine | Qt::TextExpandTabs | Qt::TextShowMnemonic,
         head->iconText());
         r.adjust(-dpi.f12, -dpi.f3, dpi.f16, dpi.f3);
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
#else
         const int w = menu->width();
         const int h = menu->height();

         QRegion mask(4, 0, w-8, h);
         mask += QRegion(0, 4, w, h-8);
         mask += QRegion(2, 1, w-4, h-2);
         mask += QRegion(1, 2, w-2, h-4);
#endif
         menu->setMask(mask);
         return false;
      }
      return false;
   }
#endif
//    case QEvent::MouseButtonRelease:
//    case QEvent::MouseButtonPress:
//       qWarning("pressed/released");
//       if (object->inherits("QScrollBar")) {
//          qWarning("QScrollBar pressed/released");
//          QWidget *w = static_cast<QWidget*>(object)->parentWidget();
//          if (w && isSpecialFrame(w)) {
//             qWarning("set frame updates to %s",
//                      ev->type() == QEvent::MouseButtonRelease ? "active" : "INactive");
//             w->setUpdatesEnabled(ev->type() == QEvent::MouseButtonRelease);
//          }
//          return false;
//       }
//       return false;
#ifdef MOUSEDEBUG
   case QEvent::MouseButtonPress: {
      QMouseEvent *mev = (QMouseEvent*)ev;
      DEBUG << object;
      return false;
   }
#endif
   case QEvent::Show:
      if (QWidget * widget = qobject_cast<QWidget*>(object)) {
         if (widget->isModal()) {
            if (config.bg.modal.invert) swapPalette(widget, this);
            const QPalette &pal = widget->palette();
            BGMode bgMode = config.bg.mode;
            QColor bg = FCOLOR(Window);
            int gt[2] = { GRAD(kwin)[0], GRAD(kwin)[1] };
            if (config.bg.modal.glassy) {
               bgMode = Plain;
               bg = bg.light(115-Colors::value(bg)/20);
               gt[0] = 0; gt[1] = 3;
            }
            int info = XProperty::encode(bg, FCOLOR(WindowText), bgMode);
            XProperty::set(widget->winId(), XProperty::bgInfo, info);
            info = XProperty::encode(CCOLOR(kwin.active, Bg), CCOLOR(kwin.active, Fg), gt[1]);
            XProperty::set(widget->winId(), XProperty::actInfo, info);
            info = XProperty::encode(CCOLOR(kwin.inactive, Bg), CCOLOR(kwin.inactive, Fg), gt[0]);
            XProperty::set(widget->winId(), XProperty::inactInfo, info);
            widget->setWindowOpacity( config.bg.modal.opacity/100.0 );
            return false;
         }
         if (QMenu * menu = qobject_cast<QMenu*>(widget)) {
            if (menu->parentWidget() &&
               menu->parentWidget()->inherits("QMdiSubWindow")) {
               QPoint pt = menu->parentWidget()->rect().topRight();
               pt += QPoint(-menu->width(), pixelMetric(PM_TitleBarHeight,0,0));
               pt = menu->parentWidget()->mapToGlobal(pt);
               menu->move(pt);
            }
            QMenuBar *bar = bar4popup(menu);
            if (bar)
               menu->move(menu->pos()-QPoint(0,dpi.f2));
#if SHAPE_POPUP
            QMenuBar *bar = bar4popup(menu);
            if (bar) {
               QPoint pos(dpi.f1, 0);
               pos += bar->actionGeometry(menu->menuAction()).topLeft();
               menu->move(bar->mapToGlobal(pos));
               menu->setActiveAction(menu->actions().at(0));
            }
            return false;
#endif
         }
         return false;
      }
   case QEvent::Hide:
      if (QWidget * widget = qobject_cast<QWidget*>(object))
      if (widget->isModal())
      if (config.bg.modal.invert) swapPalette(widget, this);
      return false;
   case QEvent::PaletteChange:
#define LACK_CONTRAST(_C_) Colors::contrast(pal.color(QPalette::Active, QPalette::_C_), pal.color(QPalette::Active, QPalette::_C_##Text)) < 40
#define HARD_CONTRAST(_C_) Colors::value(pal.color(QPalette::Active, QPalette::_C_)) < 128 ? Qt::white : Qt::black
      if (QWidget * widget = qobject_cast<QWidget*>(object))
      if (widget->objectName() == "RenderFormElementWidget") {
         widget->removeEventFilter(this);
         QPalette pal = widget->palette();
         if (LACK_CONTRAST(Window))
            pal.setColor(QPalette::WindowText, HARD_CONTRAST(Window));
         if (LACK_CONTRAST(Button))
            pal.setColor(QPalette::ButtonText, HARD_CONTRAST(Button));
         if (Colors::contrast(pal.color(QPalette::Active, QPalette::Highlight),
                              pal.color(QPalette::Active, QPalette::HighlightedText)) < 40)
            pal.setColor(QPalette::HighlightedText, HARD_CONTRAST(Highlight));
         if (Colors::contrast(pal.color(QPalette::Active, QPalette::Base),
                              pal.color(QPalette::Active, QPalette::Text)) < 40)
            pal.setColor(QPalette::Text, HARD_CONTRAST(Base));
         widget->setPalette(pal);
         widget->installEventFilter(this);
      }
      return false;
   default:
      return false;
   }
}


QPalette
BespinStyle::standardPalette () const
{
   QPalette pal ( QColor(70,70,70), QColor(70,70,70), // windowText, button
                     Qt::white, QColor(211,211,212), QColor(226,226,227), //light, dark, mid
                     Qt::black, Qt::white, //text, bright_text
                     Qt::white, QColor(234,234,236) ); //base, window
   pal.setColor(QPalette::ButtonText, Qt::white);
   pal.setColor(QPalette::Highlight, QColor(97, 147, 207));
   pal.setColor(QPalette::HighlightedText, Qt::white);
   return pal;
}

#undef PAL

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
