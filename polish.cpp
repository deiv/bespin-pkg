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

#include <QAbstractItemView>
// #include <QAbstractScrollArea>
#include <QAbstractSlider>
#include <QApplication>
#include <QComboBox>
#include <QtDebug>
#include <QLabel>
#include <QLayout>
#include <QLCDNumber>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QScrollArea>
#include <QToolBar>
#include <QToolTip>
#include <QTreeView>

#include "colors.h"

#include <unistd.h>
#include <cmath>

#ifdef Q_WS_X11
#include "macmenu.h"
#include "xproperty.h"
#endif

#include "visualframe.h"
#include "bespin.h"
#include "hacks.h"

#include "animator/hover.h"
#include "animator/aprogress.h"
#include "animator/tab.h"

#include "makros.h"
#undef CCOLOR
#undef FCOLOR
#define CCOLOR(_TYPE_, _FG_) PAL.color(QPalette::Active, Style::config._TYPE_##_role[_FG_])
#define FCOLOR(_TYPE_) PAL.color(QPalette::Active, QPalette::_TYPE_)

class EventKiller : public QObject
{
//     Q_OBJECT
public:
    bool eventFilter( QObject *, QEvent *)
    { return true; }
};

static EventKiller eventKiller;

using namespace Bespin;

Hacks::Config Hacks::config;

static inline void
setBoldFont(QWidget *w, bool bold = true)
{
    if (w->font().pointSize() < 1)
        return;
    QFont fnt = w->font();
    fnt.setBold(bold);
    w->setFont(fnt);
}

void Style::polish ( QApplication * app )
{
    VisualFrame::setStyle(this);
    QPalette pal = app->palette();
    polish(pal);
    QPalette *opal = originalPalette;
    originalPalette = 0; // so our eventfilter won't react on this... ;-P
    app->setPalette(pal);
    originalPalette = opal;
}

#define _SHIFTCOLOR_(clr) clr = QColor(CLAMP(clr.red()-10,0,255),CLAMP(clr.green()-10,0,255),CLAMP(clr.blue()-10,0,255))

#undef PAL
#define PAL pal

void Style::polish( QPalette &pal, bool onInit )
{
    QColor c = pal.color(QPalette::Active, QPalette::Window);

    if (config.bg.mode > Plain)
    {
        int h,s,v,a;
        c.getHsv(&h,&s,&v,&a);
        if (v < config.bg.minValue) // very dark colors won't make nice backgrounds ;)
            c.setHsv(h, s, config.bg.minValue, a);
// #if BESPIN_ARGB_WINDOWS
//         c.setAlpha(config.bg.opacity);
// #endif
    }
    pal.setColor( QPalette::Window, c );

    // AlternateBase
    pal.setColor(QPalette::AlternateBase, Colors::mid(pal.color(QPalette::Active, QPalette::Base),
                                                      pal.color(QPalette::Active, config.view.shadeRole),
                                                      100,config.view.shadeLevel));
    // highlight colors
    const int highlightGray = qGray(pal.color(QPalette::Active, QPalette::Highlight).rgb());
    const QColor grey(highlightGray,highlightGray,highlightGray);
    pal.setColor(QPalette::Disabled, QPalette::Highlight, grey);

    // Link colors can not be set through qtconfig - and the colors suck
    QColor link = pal.color(QPalette::Active, QPalette::Highlight);
    const int vwt = Colors::value(pal.color(QPalette::Active, QPalette::Window));
    const int vt = Colors::value(pal.color(QPalette::Active, QPalette::Base));
    int h,s,v; link.getHsv(&h,&s,&v);
    s = sqrt(s/255.0)*255.0;
    
    if (vwt > 128 && vt > 128)
        v = 3*v/4;
    else if (vwt < 128 && vt < 128)
        v = qMin(255, 7*v/6);
    link.setHsv(h, s, v);
    
    pal.setColor(QPalette::Link, link);
    
    link = Colors::mid(link, Colors::mid(pal.color(QPalette::Active, QPalette::Text),
                                         pal.color(QPalette::Active, QPalette::WindowText)), 4, 1);
                                         pal.setColor(QPalette::LinkVisited, link);

    if (onInit)
    {
        // dark, light & etc are tinted... no good:
        pal.setColor(QPalette::Dark, QColor(70,70,70));
        pal.setColor(QPalette::Mid, QColor(100,100,100));
        pal.setColor(QPalette::Midlight, QColor(220,220,220));
        pal.setColor(QPalette::Light, QColor(240,240,240));

#if QT_VERSION >= 0x040400
        // tooltip (NOTICE not configurable by qtconfig, kde can, let's see what we're gonna do on this...)
        pal.setColor(QPalette::ToolTipBase, pal.color(QPalette::Active, config.bg.tooltip_role[Bg]));
        pal.setColor(QPalette::ToolTipText, pal.color(QPalette::Active, config.bg.tooltip_role[Fg]));
#endif
    }

    // inactive palette
    if (config.fadeInactive)
    { // fade out inactive foreground and highlight colors...
        pal.setColor(QPalette::Inactive, QPalette::Highlight,
                     Colors::mid(pal.color(QPalette::Active, QPalette::Highlight), grey, 2,1));
        pal.setColor(QPalette::Inactive, QPalette::WindowText,
                     Colors::mid(pal.color(QPalette::Active, QPalette::Window), pal.color(QPalette::Active, QPalette::WindowText), 1,4));
        pal.setColor(QPalette::Inactive, QPalette::ButtonText,
                     Colors::mid(pal.color(QPalette::Active, QPalette::Button), pal.color(QPalette::Active, QPalette::ButtonText), 1,4));
        pal.setColor(QPalette::Inactive, QPalette::Text,
                     Colors::mid(pal.color(QPalette::Active, QPalette::Base), pal.color(QPalette::Active, QPalette::Text), 1,4));
    }

    // fade disabled palette
    pal.setColor(QPalette::Disabled, QPalette::WindowText,
                 Colors::mid(pal.color(QPalette::Active, QPalette::Window), pal.color(QPalette::Active, QPalette::WindowText),2,1));
    pal.setColor(QPalette::Disabled, QPalette::Base,
                 Colors::mid(pal.color(QPalette::Active, QPalette::Window), pal.color(QPalette::Active, QPalette::Base),1,2));
    pal.setColor(QPalette::Disabled, QPalette::Text,
                 Colors::mid(pal.color(QPalette::Active, QPalette::Base), pal.color(QPalette::Active, QPalette::Text)));
    pal.setColor(QPalette::Disabled, QPalette::AlternateBase,
                 Colors::mid(pal.color(QPalette::Disabled, QPalette::Base), pal.color(QPalette::Disabled, QPalette::Text),15,1));

    // more on tooltips... (we force some colors...)
    if (!onInit)
        return;

    QPalette toolPal = QToolTip::palette();
    const QColor bg = pal.color(config.bg.tooltip_role[Bg]);
    const QColor fg = pal.color(config.bg.tooltip_role[Fg]);
    toolPal.setColor(QPalette::Window, bg);
    toolPal.setColor(QPalette::WindowText, fg);
    toolPal.setColor(QPalette::Base, bg);
    toolPal.setColor(QPalette::Text, fg);
    toolPal.setColor(QPalette::Button, bg);
    toolPal.setColor(QPalette::ButtonText, fg);
    toolPal.setColor(QPalette::Highlight, fg); // sic!
    toolPal.setColor(QPalette::HighlightedText, bg); // sic!
#if QT_VERSION >= 0x040400
    toolPal.setColor(QPalette::ToolTipBase, bg);
    toolPal.setColor(QPalette::ToolTipText, fg);
#endif
    QToolTip::setPalette(toolPal);


#ifdef Q_WS_X11
    if (appType == GTK)
        setupDecoFor(NULL, pal, config.bg.mode, GRAD(kwin));
#endif
}


#if 0
static QMenuBar *
bar4popup(QMenu *menu)
{
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


inline static void
polishGTK(QWidget * widget, const Config &config)
{
    enum MyRole{Bg = Style::Bg, Fg = Style::Fg};
    QColor c1, c2, c3, c4;
    if (widget->objectName() == "QPushButton" ||
        widget->objectName() == "QComboBox" ||
        widget->objectName() == "QCheckBox" ||
        widget->objectName() == "QRadioButton" )
    {
        QPalette pal = widget->palette();
        c1 = CCOLOR(btn.std, Bg);
        c2 = CCOLOR(btn.active, Bg);
        c3 = CCOLOR(btn.std, Fg);
        c4 = CCOLOR(btn.active, Fg);
        
        pal.setColor(QPalette::Disabled, QPalette::Button, Colors::mid(Qt::black, FCOLOR(Window),5,100));
        pal.setColor(QPalette::Disabled, QPalette::ButtonText, Colors::mid(FCOLOR(Window), FCOLOR(WindowText),3,1));
        
        pal.setColor(QPalette::Inactive, QPalette::Button, c1);
        pal.setColor(QPalette::Active, QPalette::Button, c2);
        pal.setColor(QPalette::Inactive, QPalette::ButtonText, c3);
        pal.setColor(QPalette::Active, QPalette::ButtonText, config.btn.backLightHover ? c3 : c4);

        widget->setPalette(pal);
    }

    if (widget->objectName() == "QTabWidget" ||
        widget->objectName() == "QTabBar")
    {
        QPalette pal = widget->palette();
        c1 = CCOLOR(tab.std, Bg);
        c2 = CCOLOR(tab.active, Bg);
        c3 = CCOLOR(tab.std, Fg);
        c4 = CCOLOR(tab.active, Fg);

        pal.setColor(QPalette::Disabled, QPalette::WindowText, Colors::mid(c1, c3, 3, 1));
        pal.setColor(QPalette::Inactive, QPalette::Window, c1);
        pal.setColor(QPalette::Active, QPalette::Window, c2);
        pal.setColor(QPalette::Inactive, QPalette::WindowText, c3);
        pal.setColor(QPalette::Active, QPalette::WindowText, c4);
        widget->setPalette(pal);
    }

    if (widget->objectName() == "QMenuBar" )
    {
        QPalette pal = widget->palette();
        c1 = Colors::mid(FCOLOR(Window), CCOLOR(menu.bar, Bg),1,6);
        c2 = CCOLOR(menu.active, Bg);
        c3 = CCOLOR(menu.bar, Fg);
        c4 = CCOLOR(menu.active, Fg);
        
        pal.setColor(QPalette::Inactive, QPalette::Window, c1);
        pal.setColor(QPalette::Active, QPalette::Window, c2);
        pal.setColor(QPalette::Inactive, QPalette::WindowText, c3);
        pal.setColor(QPalette::Active, QPalette::WindowText, c4);
        widget->setPalette(pal);
    }

    if (widget->objectName() == "QMenu" )
    {
        QPalette pal = widget->palette();
        c1 = CCOLOR(menu.std, Bg);
        c2 = CCOLOR(menu.active, Bg);
        c3 = CCOLOR(menu.std, Fg);
        c4 = CCOLOR(menu.active, Fg);
        
        pal.setColor(QPalette::Inactive, QPalette::Window, c1);
        pal.setColor(QPalette::Active, QPalette::Window, c2);
        pal.setColor(QPalette::Inactive, QPalette::WindowText, c3);
        pal.setColor(QPalette::Active, QPalette::WindowText, c4);
        widget->setPalette(pal);
    }
}

void
Style::polish( QWidget * widget )
{
    // GTK-Qt gets a special handling - see above
    if (appType == GTK)
    {
        polishGTK(widget, config);
        return;
    }

    // !!! protect against polishing /QObject/ attempts! (this REALLY happens from time to time...)
    if (!widget)
        return;

//     if (widget->inherits("QGraphicsView"))
//         qDebug() << "BESPIN" << widget;

#ifdef MOUSEDEBUG
    widget->removeEventFilter(this);
    widget->installEventFilter(this);
#endif

    // NONONONONO!!!!! ;)
    if (qobject_cast<VisualFramePart*>(widget))
        return;

    // apply any user selected hacks
    Hacks::add(widget);

    //BEGIN Window handling                                                                        -
    if (widget->isWindow() && !widget->inherits("QTipLabel"))
    {
        QPalette pal = widget->palette();

        /// this is dangerous! e.g. applying to QDesktopWidget leads to infinite recursion...
        /// also doesn't work bgs get transparent and applying this to everythign causes funny sideeffects...
#if BESPIN_ARGB_WINDOWS
        if (!(  config.bg.opacity == 0xff || // opaque
                widget->windowType() == Qt::Desktop || // makes no sense + QDesktopWidget is often misused
                widget->testAttribute(Qt::WA_X11NetWmWindowTypeDesktop) || // makes no sense
                widget->testAttribute(Qt::WA_TranslucentBackground)))
        {
            widget->setAttribute(Qt::WA_TranslucentBackground);
            // WORKAROUND: somehow the window gets repositioned to <1,<1 and thus always appears in the upper left corner
            // we just move it faaaaar away so kwin will take back control and apply smart placement or whatever
            widget->move(10000,10000);
        }
        if (config.bg.glassy)
            widget->setAttribute(Qt::WA_MacBrushedMetal);
#endif
        if (config.bg.mode > Plain || config.bg.opacity != 0xff || config.bg.ringOverlay)
        {
            if (config.bg.opacity != 0xff)
            {
                widget->removeEventFilter(this);
                widget->installEventFilter(this);
            }
            widget->setAttribute(Qt::WA_StyledBackground);
        }

        //BEGIN Popup menu handling                                                                -
        if (QMenu *menu = qobject_cast<QMenu *>(widget))
        {
            if (config.menu.glassy)
            {   // glass mode popups
                menu->setAttribute(Qt::WA_MacBrushedMetal);
                menu->setAttribute(Qt::WA_StyledBackground);
            }
            // opacity
            menu->setWindowOpacity( config.menu.opacity/100.0 );
            // color swapping
            menu->setAutoFillBackground(true);
            menu->setBackgroundRole ( config.menu.std_role[Bg] );
            menu->setForegroundRole ( config.menu.std_role[Fg] );
            if (config.menu.boldText)
                setBoldFont(menu);
            
            // eventfiltering to reposition MDI windows and correct distance to menubars
            menu->removeEventFilter(this);
            menu->installEventFilter(this);
#if 0
            /// NOTE this was intended to be for some menu mock from nuno where the menu
            /// reaches kinda ribbon-like into the bar
            /// i'll keep it to remind myself and in case i get it to work one day ;-)
            if (bar4popup(menu))
            {
                QAction *action = new QAction( menu->menuAction()->iconText(), menu );
                connect (action, SIGNAL(triggered(bool)), menu, SLOT(hide()));
                menu->insertAction(menu->actions().at(0), action);
            }
#endif
        }
        //END Popup menu handling                                                                  -
# if 0
        /// WORKAROUND for krunner's white flicker showup bg...
        else if (appType == KRunner && widget->inherits("Interface"))
            widget->setAttribute(Qt::WA_NoSystemBackground);
#endif
        /// WORKAROUND Qt color bug, uses daddies palette and FGrole, but TooltipBase as background
        else if (widget->inherits("QWhatsThat"))
            widget->setPalette(QToolTip::palette()); // so this is Qt bug WORKAROUND
#if 0 // until kwin provides better shadows
        else if (widget->inherits("QDockWidget"))
            widget->installEventFilter(this); // shape corners... repeated below!
#endif
        else
        {
            // talk to kwin about colors, gradients, etc.
            Qt::WindowFlags ignore =    Qt::Sheet | Qt::Drawer | Qt::Popup | Qt::ToolTip |
                                        Qt::SplashScreen | Qt::Desktop |
                                        Qt::X11BypassWindowManagerHint;// | Qt::FramelessWindowHint; <- could easily change mind...?!
            ignore &= ~Qt::Dialog; // erase dialog, it's in drawer et al. but takes away window as well
            // this can be expensive, so avoid for popups, combodrops etc.
            if (!(widget->windowFlags() & ignore))
            {
                if (widget->isVisible())
                    setupDecoFor(widget, widget->palette(), config.bg.mode, GRAD(kwin));
                widget->removeEventFilter(this);
                widget->installEventFilter(this); // catch show event and palette changes for deco
            }
        }
    }
    //END Window handling                                                                          -

    //BEGIN Frames                                                                                 -
    else if (QFrame *frame = qobject_cast<QFrame *>(widget)) // sic! for "else" - no window frames!
    {
        // just saw they're niftier in skulpture -> had to do sth. ;-P
        if (QLCDNumber *lcd = qobject_cast<QLCDNumber*>(frame))
        {
            if (lcd->frameShape() != QFrame::NoFrame)
                lcd->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
            lcd->setSegmentStyle(QLCDNumber::Flat);
            lcd->setAutoFillBackground(true);
        }
#if 0 // i want them centered, but titlewidget fights back, and it's not worth the eventfilter monitor
        else if (QLabel *label = qobject_cast<QLabel*>(frame))
        {   // i want them center aligned
            if (label->parentWidget() && label->parentWidget()->parentWidget() &&
                label->parentWidget()->parentWidget()->inherits("KTitleWidget"))
                label->setAlignment(Qt::AlignCenter);
        }
#endif
        // sunken looks soo much nicer ;)
        else if (frame->parentWidget() && frame->parentWidget()->inherits("KTitleWidget"))
        {
            if (config.bg.mode == Scanlines)
                frame->setFrameShadow(QFrame::Sunken);
            else
            {
                frame->setAutoFillBackground(false);
                frame->setBackgroundRole(QPalette::Window);
                frame->setForegroundRole(QPalette::WindowText);
            }
        }
        else if (frame->frameShape() != QFrame::NoFrame )
        {
            // Kill ugly line look (we paint our styled v and h lines instead ;)
            if (frame->frameShape() == QFrame::HLine || frame->frameShape() == QFrame::VLine)
            {
                widget->removeEventFilter(this);
                widget->installEventFilter(this);
            }

            // Kill ugly winblows frames... (qShadeBlablabla stuff)
            else if (frame->frameShape() != QFrame::StyledPanel)
            {
                frame->setFrameShape(QFrame::StyledPanel);
                if ( frame->frameShape() == QFrame::Box )
                    frame->setFrameShadow( QFrame::Plain );
            }
        }

        // scrollarea hovering
        if (qobject_cast<QAbstractScrollArea*>(frame)
#ifdef QT3_SUPPORT
            || frame->inherits("Q3ScrollView")
#endif
            )
        {
            Animator::Hover::manage(frame);
            if (QAbstractItemView *itemView = qobject_cast<QAbstractItemView*>(frame) )
            {
                /// NOTE: WORKAROUND for (no more) dolphin but amarok and probably others:
                // if the viewport ist not autofilled, it's roles need to be adjusted (like QPalette::Window/Text)
                // force this here, hoping it won't cause to many problems - and make a bug report
                if (QWidget *vp = itemView->viewport())
                {
                    if (!vp->autoFillBackground() || vp->palette().color(QPalette::Active, vp->backgroundRole()).alpha() < 180)
                    {
//                         qDebug() << "BESPIN works around a visual problem in" << itemView << ", please contact thomas.luebking 'at' web.de";
                        QPalette pal = itemView->palette();
                        if (!vp->autoFillBackground() || vp->palette().color(QPalette::Active, vp->backgroundRole()).alpha() < 25)
                        {
                            pal.setColor(QPalette::Active, QPalette::Base, pal.color(QPalette::Active, QPalette::Window));
                            pal.setColor(QPalette::Inactive, QPalette::Base, pal.color(QPalette::Inactive, QPalette::Window));
                            pal.setColor(QPalette::Disabled, QPalette::Base, pal.color(QPalette::Disabled, QPalette::Window));
                            vp->setAutoFillBackground(false);
                        }
                        pal.setColor(QPalette::Active, QPalette::Text, pal.color(QPalette::Active, QPalette::WindowText));
                        pal.setColor(QPalette::Inactive, QPalette::Text, pal.color(QPalette::Inactive, QPalette::WindowText));
                        pal.setColor(QPalette::Disabled, QPalette::Text, pal.color(QPalette::Disabled, QPalette::WindowText));
                        itemView->setPalette(pal);
                    }
                    if (!vp->autoFillBackground())
                    {
                        QPalette pal = itemView->palette();
//                         Colors::mid(pal.color(_S_, QPalette::Window), pal.color(_S_, QPalette::Base),6,1)
                        #define ALT_BASE(_S_) Colors::mid(pal.color(_S_, QPalette::Window), pal.color(QPalette::_S_, QPalette::AlternateBase),\
                        Colors::contrast(pal.color(_S_, QPalette::Window), pal.color(_S_, QPalette::AlternateBase)), 10)
                        pal.setColor(QPalette::Active, QPalette::AlternateBase, ALT_BASE(QPalette::Active));
                        pal.setColor(QPalette::Inactive, QPalette::AlternateBase, ALT_BASE(QPalette::Inactive));
                        pal.setColor(QPalette::Disabled, QPalette::AlternateBase, ALT_BASE(QPalette::Disabled));
                        itemView->setPalette(pal);
                        #undef ALT_BASE
                    }
                }

                if (itemView->inherits("KCategorizedView"))
                {
                    itemView->removeEventFilter(this);
                    itemView->installEventFilter(this); // scrolldistance...
//                     itemView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
                }

#if QT_VERSION >= 0x040500
                itemView->viewport()->setAttribute(Qt::WA_Hover);
#endif
                if (QTreeView* tv = qobject_cast<QTreeView*>(itemView))
                {   // allow all treeviews to be animated! NOTICE: animation causes visual errors on non autofilling views...
                    if (Hacks::config.treeViews &&
                        tv->viewport()->autoFillBackground() &&
                        tv->viewport()->palette().color(tv->viewport()->backgroundRole()).alpha() > 200) // 255 would be perfect, though
                    tv->setAnimated(true);
                }
                else
                {   // Enable hover effects in listview, treeview hovering sucks, as the "tree" doesn't get an update
                    itemView->viewport()->setAttribute(Qt::WA_Hover);
                    if (widget->inherits("QHeaderView"))
                        widget->setAttribute(Qt::WA_Hover);
                }
            }
        }

        /// Tab Transition animation,
        if (widget->inherits("QStackedWidget"))
            // NOTICE do NOT(!) apply this on tabs explicitly, as they contain a stack!
            Animator::Tab::manage(widget);

        /// QToolBox handling - a shame they look that crap by default!
        if (widget->inherits("QToolBox"))
        {   // get rid of QPalette::Button
            widget->setBackgroundRole(QPalette::Window);
            widget->setForegroundRole(QPalette::WindowText);
            if (widget->layout())
            {   // get rid of nasty indention
                widget->layout()->setMargin ( 0 );
                widget->layout()->setSpacing ( 0 );
            }
        }

        /// "Frame above Content" look, but ...
        else if (isSpecialFrame(widget))
        {   // ...QTextEdit etc. can be handled more efficiently
            if (frame->lineWidth() == 1)
                frame->setLineWidth(F(4)); // but must have enough indention
        }
        else if (!widget->inherits("KPIM::OverlayWidget"))
            VisualFrame::manage(frame);
    }
    //END FRAMES                                                                                   -

    //BEGIN PUSHBUTTONS - hovering/animation                                                       -
    else if (qobject_cast<QAbstractButton*>(widget))
    {
//         widget->setBackgroundRole(config.btn.std_role[Bg]);
//         widget->setForegroundRole(config.btn.std_role[Fg]);
        widget->setAttribute(Qt::WA_Hover, false); // KHtml
        if (widget->inherits("QToolBoxButton") || IS_HTML_WIDGET )
            widget->setAttribute(Qt::WA_Hover); // KHtml
        else
        {
            if (QPushButton *pbtn = qobject_cast<QPushButton*>(widget))
            {
                if (widget->parentWidget() &&
                    widget->parentWidget()->inherits("KPIM::StatusbarProgressWidget"))
                    pbtn->setFlat(true);

                // HACK around "weird" original appearance ;-P
                // also see eventFilter
                if (pbtn->inherits("KUrlButton") || pbtn->inherits("BreadcrumbItemButton"))
                {
                    pbtn->setBackgroundRole(QPalette::Window);
                    pbtn->setForegroundRole(QPalette::Link);
                    QPalette pal = pbtn->palette();
                    pal.setColor(QPalette::HighlightedText, pal.color(QPalette::Active, QPalette::Window));
                    pbtn->setPalette(pal);
                    pbtn->setCursor(Qt::PointingHandCursor);
                    pbtn->removeEventFilter(this);
                    pbtn->installEventFilter(this);
                    widget->setAttribute(Qt::WA_Hover);
                }
            }
            else if (widget->inherits("QToolButton") &&
                // of course plasma needs - again - a WORKAROUND, we seem to be unable to use bg/fg-role, are we?
                !(appType == Plasma && widget->inherits("ToolButton")))
            {
                QPalette::ColorRole bg = QPalette::Window, fg = QPalette::WindowText;
                if (QWidget *dad = widget->parentWidget())
                {
                    bg = dad->backgroundRole();
                    fg = dad->foregroundRole();

                    if (QMenuBar *mbar = qobject_cast<QMenuBar*>(dad))
                    if (Hacks::config.killThrobber && widget->inherits("KAnimatedButton"))
                    {   // this is konquerors throbber...
                        widget->hide();
                        widget->setParent(mbar->parentWidget());
                        mbar->setCornerWidget(NULL);
                    }
                }
                widget->setBackgroundRole(bg);
                widget->setForegroundRole(fg);
            }
            if (!widget->testAttribute(Qt::WA_Hover))
                Animator::Hover::manage(widget);
        }

        // NOTICE WORKAROUND - this widget uses the style to paint the bg, but hardcodes the fg...
        // TODO: inform Joseph Wenninger <jowenn@kde.org> and really fix this
        // (fails all styles w/ Windowcolored ToolBtn and QPalette::ButtonText != QPalette::WindowText settings)
        if (widget->inherits("KMultiTabBarTab"))
        {
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
    
    //BEGIN COMBOBOXES - hovering/animation                                                        -
    else if (QComboBox *cb = qobject_cast<QComboBox*>(widget))
    {
        if (cb->view())
            cb->view()->setTextElideMode( Qt::ElideMiddle);

        if (cb->parentWidget() && cb->parentWidget()->inherits("KUrlNavigator"))
            cb->setIconSize(QSize(0,0));

        if (IS_HTML_WIDGET)
            widget->setAttribute(Qt::WA_Hover);
        else
            Animator::Hover::manage(widget);
    }
    //BEGIN SLIDERS / SCROLLBARS / SCROLLAREAS - hovering/animation                                -
    else if (qobject_cast<QAbstractSlider*>(widget))
    {
        widget->removeEventFilter(this);
        widget->installEventFilter(this); // finish animation
        
        widget->setAttribute(Qt::WA_Hover);
        // NOTICE
        // QAbstractSlider::setAttribute(Qt::WA_OpaquePaintEvent) saves surprisinlgy little CPU
        // so that'd just gonna add more complexity for literally nothing...
        // ...as the slider is usually not bound to e.g. a "scrollarea"
        if (widget->inherits("QScrollBar"))
        {
            // TODO: find a general catch for the plasma problem
            if (appType == Plasma) // yes - i currently don't know how to detect those things otherwise
                widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
            else
            {
                QWidget *dad = widget;
                while ((dad = dad->parentWidget()))
                {   // digg for a potential KHTMLView ancestor, making this a html input scroller
                    if (dad->inherits("KHTMLView"))
                    {   // NOTICE this slows down things as it triggers a repaint of the frame
                        widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
                        // ...but this would re-enbale speed - just: how to get the proper palette
                        // what if there's a bg image?
                        // TODO how's css/khtml policy on applying colors?
    //                     widget->setAutoFillBackground ( true );
    //                     widget->setBackgroundRole ( QPalette::Base ); // QPalette::Window looks wrong
    //                     widget->setForegroundRole ( QPalette::Text );
                        break;
                    }
                }
            }

            /// Scrollarea hovering - yes, this is /NOT/ redundant to the one above!
            if (QWidget *area = widget->parentWidget())
            {
                if ((area = area->parentWidget())) // sic!
                {
                    if (qobject_cast<QAbstractScrollArea*>(area))
                        area = 0; // this is handled for QAbstractScrollArea, but...
                    else // Konsole, Kate, etc. need a special handling!
                        area = widget->parentWidget();
                }
                if (area)
                    Animator::Hover::manage(area, true);
            }
        }
    }

    //BEGIN PROGRESSBARS - hover/animation and bold font                                           -
    else if (widget->inherits("QProgressBar"))
    {
        widget->setAttribute(Qt::WA_Hover);
        setBoldFont(widget);
        Animator::Progress::manage(widget);
    }

#if QT_VERSION >= 0x040500
        else if ( widget->inherits( "QTabWidget" ) )
        {
            widget->removeEventFilter(this);
            widget->installEventFilter( this );
        }
#endif

    //BEGIN Tab animation, painting override                                                       -
    else if (QTabBar *bar = qobject_cast<QTabBar *>(widget))
    {
        widget->setAttribute(Qt::WA_Hover);
        if (bar->drawBase())
        {
            widget->setBackgroundRole(config.tab.std_role[0]);
            widget->setForegroundRole(config.tab.std_role[1]);
        }
        // the eventfilter overtakes the widget painting to allow tabs ABOVE the tabbar
        widget->removeEventFilter(this);
        widget->installEventFilter(this);
    }
    else if (config.bg.docks.invert && widget->inherits("QDockWidget"))
    {
        QPalette pal = widget->palette();
        QColor c = pal.color(QPalette::Window);
        pal.setColor(QPalette::Window, pal.color(QPalette::WindowText));
        pal.setColor(QPalette::WindowText, c);
        widget->setPalette(pal);
        widget->setAutoFillBackground(true);
//         if (config.bg.docks.shape)
//         {
//             widget->removeEventFilter(this);
//             widget->installEventFilter(this); // shape corners... but kwin will refuse shadows then...
//         }
    }
    else if (widget->inherits("KFadeWidgetEffect"))
    {   // interfers with our animation, is slower and cannot handle non plain backgrounds
        // (unfortunately i cannot avoid the widget grabbing)
        // maybe ask ereslibre to query a stylehint for this?
        widget->hide();
        widget->installEventFilter(&eventKiller);
    }
    /// hover some leftover widgets
    else if (widget->inherits("QAbstractSpinBox") || widget->inherits("QSplitterHandle") ||
        /*widget->inherits("QDockWidget") ||*/ widget->inherits("QWorkspaceTitleBar") ||
        widget->inherits("Q3DockWindowResizeHandle"))
        widget->setAttribute(Qt::WA_Hover);

    /// Menubars and toolbar default to QPalette::Button - looks crap and leads to flicker...?!
    if (QMenuBar *mbar = qobject_cast<QMenuBar *>(widget))
    {
        widget->setBackgroundRole(config.menu.bar_role[Bg]);
        widget->setForegroundRole(config.menu.bar_role[Fg]);
#ifdef Q_WS_X11
        if (!((appType == KDevelop) || (appType == QtDesigner) && mbar->inherits("QDesignerMenuBar")))
            MacMenu::manage(mbar);
#endif
    }   

    bool isTopContainer = qobject_cast<QToolBar *>(widget);
#ifdef QT3_SUPPORT
    isTopContainer = isTopContainer || widget->inherits("Q3ToolBar");
#endif
    if (isTopContainer || qobject_cast<QToolBar *>(widget->parent()))
    {
        widget->setBackgroundRole(QPalette::Window);
        widget->setForegroundRole(QPalette::WindowText);
        if (!isTopContainer && widget->inherits("QToolBarHandle"))
            widget->setAttribute(Qt::WA_Hover);
    }

    /// this is for QToolBox kids - they're autofilled by default - what looks crap
    if (widget->autoFillBackground() && widget->parentWidget() &&
        ( widget->parentWidget()->objectName() == "qt_scrollarea_viewport" ) &&
        widget->parentWidget()->parentWidget() && //grampa
        qobject_cast<QAbstractScrollArea*>(widget->parentWidget()->parentWidget()) &&
        widget->parentWidget()->parentWidget()->parentWidget() && // grangrampa
        widget->parentWidget()->parentWidget()->parentWidget()->inherits("QToolBox") )
    {
        widget->parentWidget()->setAutoFillBackground(false);
        widget->setAutoFillBackground(false);
    }

    // this is a WORKAROUND for amarok filebrowser, see above on itemviews...
    if (widget->inherits("KDirOperator") && widget->parentWidget() && widget->parentWidget()->inherits("FileBrowser"))
    {
        QPalette pal = widget->palette();
        pal.setColor(QPalette::Active, QPalette::Text, pal.color(QPalette::Active, QPalette::WindowText));
        pal.setColor(QPalette::Inactive, QPalette::Text, pal.color(QPalette::Inactive, QPalette::WindowText));
        pal.setColor(QPalette::Disabled, QPalette::Text, pal.color(QPalette::Disabled, QPalette::WindowText));
        widget->setPalette(pal);
    }
#if 1
    /// to update the scrollbars
    if (widget->inherits("QWebView"))
        widget->setAttribute(Qt::WA_Hover);
#endif
#if 1
    /// KHtml css colors can easily get messed up, either because i'm unsure about what colors
    /// are set or KHtml does wrong OR (mainly) by html "designers"
    if (IS_HTML_WIDGET)
    {   // the eventfilter watches palette changes and ensures contrasted foregrounds...
        widget->removeEventFilter(this);
        widget->installEventFilter(this);
        QEvent ev(QEvent::PaletteChange);
        eventFilter(widget, &ev);
    }
#endif
}
#undef PAL

void
Style::unpolish( QApplication *app )
{
    VisualFrame::setStyle(0L);
    Hacks::releaseApp();
    Gradients::wipe();
    app->setPalette(QPalette());
}

void
Style::unpolish( QWidget *widget )
{
    if (!widget)
        return;

    if (widget->isWindow())
    {
#ifdef Q_WS_X11
        XProperty::remove(widget->winId(), XProperty::winData);
        XProperty::remove(widget->winId(), XProperty::bgPics);
#endif
        if (qobject_cast<QMenu *>(widget))
            widget->clearMask();
    }

    if (qobject_cast<QAbstractButton*>(widget) || qobject_cast<QToolBar*>(widget) ||
        qobject_cast<QMenuBar*>(widget) || qobject_cast<QMenu*>(widget) ||
        widget->inherits("QToolBox"))
    {
        widget->setBackgroundRole(QPalette::Button);
        widget->setForegroundRole(QPalette::ButtonText);
    }
    if (QFrame *frame = qobject_cast<QFrame *>(widget))
        VisualFrame::release(frame);
#ifdef Q_WS_X11
    if (QMenuBar *mbar = qobject_cast<QMenuBar *>(widget))
        MacMenu::release(mbar);
#endif

    Animator::Hover::release(widget);
    Animator::Progress::release(widget);
    Animator::Tab::release(widget);
    Hacks::remove(widget);

    widget->removeEventFilter(this);
}
#undef CCOLOR
#undef FCOLOR
