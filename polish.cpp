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
// #include <QAbstractSlider>
#include <QApplication>
#include <QtDebug>
#include <QLabel>
#include <QLayout>
#include <QLCDNumber>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPen>
#include <QScrollArea>
#include <QToolBar>
#include <QToolTip>
#include <QTreeView>
#include <QtDBus/QDBusInterface>

#include "colors.h"

#ifdef Q_WS_X11
#if QT_VERSION < 0x040400
#include <unistd.h>
#endif
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

static inline void
setBoldFont(QWidget *w, bool bold = true)
{
    QFont fnt = w->font();
    fnt.setBold(bold);
    w->setFont(fnt);
}

void Style::polish ( QApplication * app )
{
    QPalette pal = app->palette();
    polish(pal);
    QPalette *opal = originalPalette;
    originalPalette = 0; // so our eventfilter won't react on this... ;-P
    app->setPalette(pal);
    originalPalette = opal;
}

#define _SHIFTCOLOR_(clr) clr = QColor(CLAMP(clr.red()-10,0,255),CLAMP(clr.green()-10,0,255),CLAMP(clr.blue()-10,0,255))

static QColor
mid(const QColor &c1, const QColor &c2, int w1 = 1, int w2 = 1)
{
    int sum = (w1+w2);
    return QColor(((w1*c1.red() + w2*c2.red())/sum) & 0xff,
                 ((w1*c1.green() + w2*c2.green())/sum) & 0xff,
                 ((w1*c1.blue() + w2*c2.blue())/sum) & 0xff,
                 ((w1*c1.alpha() + w2*c2.alpha())/sum) & 0xff);
}

static QDBusInterface bespinDeco( "org.kde.kwin", "/BespinDeco", "org.kde.BespinDeco");

#undef PAL
#define PAL pal

void Style::polish( QPalette &pal, bool onInit )
{
    QColor c = pal.color(QPalette::Active, QPalette::Background);
    if (config.bg.mode > Plain)
    {
        int h,s,v,a;
        c.getHsv(&h,&s,&v,&a);
        if (v < config.bg.minValue) // very dark colors won't make nice backgrounds ;)
            c.setHsv(h, s, config.bg.minValue, a);
//         c.setAlpha(128);
        pal.setColor( QPalette::Window, c );
    }

    // AlternateBase
    pal.setColor(QPalette::AlternateBase, mid(pal.color(QPalette::Active, QPalette::Base),
                                              pal.color(QPalette::Active, QPalette::Text),15,1));
    // highlight colors
    const int highlightGray = qGray(pal.color(QPalette::Active, QPalette::Highlight).rgb());
    const QColor grey(highlightGray,highlightGray,highlightGray);
    pal.setColor(QPalette::Disabled, QPalette::Highlight, grey);

    if (onInit)
    {
        // dark, light & etc are tinted... no good:
        pal.setColor(QPalette::Dark, QColor(70,70,70));
        pal.setColor(QPalette::Mid, QColor(100,100,100));
        pal.setColor(QPalette::Midlight, QColor(220,220,220));
        pal.setColor(QPalette::Light, QColor(240,240,240));

        // Link colors can not be set through qtconfig - and the colors suck
        pal.setColor(QPalette::Link, mid(pal.color(QPalette::Active, QPalette::Text),
                                         pal.color(QPalette::Active, QPalette::Highlight), 1, 8));
        pal.setColor(QPalette::LinkVisited, mid(pal.color(QPalette::Active, QPalette::Text),
                                                pal.color(QPalette::Active, QPalette::Highlight), 1, 4));

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
                    mid(pal.color(QPalette::Active, QPalette::Highlight), grey, 2,1));
        pal.setColor(QPalette::Inactive, QPalette::WindowText,
                    mid(pal.color(QPalette::Active, QPalette::Window), pal.color(QPalette::Active, QPalette::WindowText), 1,4));
        pal.setColor(QPalette::Inactive, QPalette::ButtonText,
                    mid(pal.color(QPalette::Active, QPalette::Button), pal.color(QPalette::Active, QPalette::ButtonText), 1,4));
        pal.setColor(QPalette::Inactive, QPalette::Text,
                    mid(pal.color(QPalette::Active, QPalette::Base), pal.color(QPalette::Active, QPalette::Text), 1,4));
    }

    // fade disabled palette
    pal.setColor(QPalette::Disabled, QPalette::WindowText,
                 mid(pal.color(QPalette::Active, QPalette::Window), pal.color(QPalette::Active, QPalette::WindowText),2,1));
    pal.setColor(QPalette::Disabled, QPalette::Base,
                 mid(pal.color(QPalette::Active, QPalette::Window), pal.color(QPalette::Active, QPalette::Base),1,2));
    pal.setColor(QPalette::Disabled, QPalette::Text,
                 mid(pal.color(QPalette::Active, QPalette::Base), pal.color(QPalette::Active, QPalette::Text)));
    pal.setColor(QPalette::Disabled, QPalette::AlternateBase,
                 mid(pal.color(QPalette::Disabled, QPalette::Base), pal.color(QPalette::Disabled, QPalette::Text),15,1));

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
    {
        bespinDeco.call(QDBus::NoBlock, "styleByPid",
#if QT_VERSION < 0x040400
                        getpid(),
#else
                        QCoreApplication::applicationPid(),
#endif
                        XProperty::encode(FCOLOR(Window), FCOLOR(WindowText), config.bg.mode),
                        XProperty::encode(CCOLOR(kwin.active, Bg), CCOLOR(kwin.active, Fg), GRAD(kwin)[1]),
                        XProperty::encode(CCOLOR(kwin.inactive, Bg), fg, GRAD(kwin)[0]));
    }
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
polishGTK(QWidget * widget)
{
    enum MyRole{Bg = Style::Bg, Fg = Style::Fg};
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

void
Style::polish( QWidget * widget )
{
    // GTK-Qt gets a special handling - see above
    if (appType == GTK)
    {
        polishGTK(widget);
        return;
    }

    // !!! protect against polishing /QObject/ attempts! (this REALLY happens from time to time...)
    if (!widget)
        return;

//     if (widget->inherits("QGraphicsView"))
//         qDebug() << "BESPIN" << widget;

#ifdef MOUSEDEBUG
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

        if (config.bg.mode > Plain)
            widget->setAttribute(Qt::WA_StyledBackground);

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

        /// WORKAROUND for krunner's white flicker showup bg...
        else if (appType == KRunner && widget->inherits("Interface"))
            widget->setAttribute(Qt::WA_NoSystemBackground);

        /// WORKAROUND Qt color bug, uses daddies palette and FGrole, but TooltipBase as background
        else if (widget->inherits("QWhatsThat"))
            widget->setPalette(QToolTip::palette()); // so this is Qt bug WORKAROUND
#if 0 // until kwin provides better shadows
        else if (widget->inherits("QDockWidget"))
            widget->installEventFilter(this); // shape corners... repeated below!
#endif
        else
        {   /// modal dialogs
            if (config.bg.modal.invert || config.bg.modal.glassy || config.bg.modal.opacity < 100)
            // the modality isn't necessarily set yet, so we catch it on QEvent::Show
                widget->installEventFilter(this);

            // talk to kwin about colors, gradients, etc.
            Qt::WindowFlags ignore =    Qt::Sheet | Qt::Drawer | Qt::Popup | Qt::ToolTip |
                                        Qt::SplashScreen | Qt::Desktop |
                                        Qt::X11BypassWindowManagerHint;// | Qt::FramelessWindowHint; <- could easily change mind...?!
            ignore &= ~Qt::Dialog; // erase dialog, it's in drawer et al. - takes away window as well
            if (!(widget->windowFlags() & ignore))
                setupDecoFor(widget); // this can be expensive, so avoid for popups, combodrops etc.
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

        // Kill ugly winblows frames... (qShadeBlablabla stuff)
        else if (   frame->frameShape() == QFrame::Box ||
                    frame->frameShape() == QFrame::Panel ||
                    frame->frameShape() == QFrame::WinPanel)
            frame->setFrameShape(QFrame::StyledPanel);

        // Kill ugly line look (we paint our styled v and h lines instead ;)
        else if (   frame->frameShape() == QFrame::HLine ||
                    frame->frameShape() == QFrame::VLine)
            widget->installEventFilter(this);

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
                QWidget *vp = itemView->viewport();
                if (vp && vp->palette().color(QPalette::Active, vp->backgroundRole()).alpha() < 180)
                {
                    qDebug() << "BESPIN works around a visual problem in" << itemView << ", please contact thomas.luebking 'at' web.de";
                    QPalette pal = itemView->palette();
                    if (vp->palette().color(QPalette::Active, vp->backgroundRole()).alpha() < 25)
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

//                 if (itemView->inherits("KCategorizedView"))

                if (!qobject_cast<QTreeView*>(itemView))
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
                frame->setLineWidth(dpi.f4); // but must have enough indention
        }
        else
            VisualFrame::manage(frame);
    }
    //END FRAMES                                                                                   -

    //BEGIN PUSHBUTTONS - hovering/animation                                                       -
    else if (widget->inherits("QAbstractButton"))
    {
        if (widget->inherits("QToolBoxButton") || IS_HTML_WIDGET )
            widget->setAttribute(Qt::WA_Hover); // KHtml
        else
        {
            if (widget->inherits("QToolButton") &&
                // of course plasma needs - again - a WORKAROUND, we seem to be unable to use bg/fg-role, are we?
                !(appType == Plasma && widget->inherits("ToolButton")))
            {
                if (config.hack.killThrobber && widget->inherits("KAnimatedButton") && widget->parentWidget())
                if (QMenuBar *mbar = qobject_cast<QMenuBar*>(widget->parentWidget()))
                {   // this is konquerors throbber...
                    widget->hide();
                    widget->setParent(mbar->parentWidget());
                    mbar->setCornerWidget(NULL);
                }

                widget->setBackgroundRole(QPalette::Window);
                widget->setForegroundRole(QPalette::WindowText);
            }
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
        // NOTICE WORKAROUND - this widget paints no bg, uses foregroundcolor() to paint the text...
        // and has - of course - foregroundRole() == QPalette::ButtonText
        // TODO: inform Peter Penz <peter.penz@gmx.at> or *cough* Aaron J. Seigo <aseigo@kde.org> and really fix this
        if (widget->inherits("KUrlButton"))
        {
            widget->setBackgroundRole(QPalette::Window);
            widget->setForegroundRole(QPalette::WindowText);
        }
    }
    
    //BEGIN COMBOBOXES - hovering/animation                                                        -
    else if (widget->inherits("QComboBox"))
    {
        if (IS_HTML_WIDGET)
            widget->setAttribute(Qt::WA_Hover);
        else
            Animator::Hover::manage(widget);
    }
    //BEGIN SLIDERS / SCROLLBARS / SCROLLAREAS - hovering/animation                                -
    else if (qobject_cast<QAbstractSlider*>(widget))
    {
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

    //BEGIN Tab animation, painting override                                                       -
    else if (qobject_cast<QTabBar *>(widget))
    {
        widget->setAttribute(Qt::WA_Hover);
        // the eventfilter overtakes the widget painting to allow tabs ABOVE the tabbar
        widget->installEventFilter(this);
    }
#if 0 // until kwin provides better shaodws
    else if (widget->inherits("QDockWidget"))
        widget->installEventFilter(this); // shape corners... repeated above!
#endif
    else if (widget->inherits("KFadeWidgetEffect"))
    {   // interfers with our animation, is slower and cannot handle non plain backgrounds
        // (unfortunately i cannot avoid the widget grabbing)
        // maybe ask ereslibre to query a stylehint for this?
        widget->hide();
        widget->installEventFilter(&eventKiller);
    }
    /// hover some leftover widgets
    else if (widget->inherits("QAbstractSpinBox") || widget->inherits("QSplitterHandle") ||
        widget->inherits("QDockWidget") || widget->inherits("QWorkspaceTitleBar") ||
        widget->inherits("Q3DockWindowResizeHandle"))
        widget->setAttribute(Qt::WA_Hover);
#if 0 // does not work for all plasma versions...
    else if (appType == KRunner && widget->inherits("KLineEdit") &&
             widget->parentWidget() && widget->parentWidget()->inherits("KHistoryComboBox"))
    {
        // KRunner needs a little help...
        // 1. normal painting seems buggy
        // 2. normal painting looks debatable, as the styled lineedit can easily break the plasma theme look...
        QWidget *window = widget->window();
        QList<QLabel*> lables = window->findChildren<QLabel*>();
        if (!lables.isEmpty())
        {   // this is my ticket to hell...
            // i want the lineedit use the plasma theme fg color
            // a) we have no access to plasma themes from here (... as i do no way intend to link plasma)
            // b) krunner only plasmafies some lables, but not our lineedit
            // c) krunner sets QPalette::WindowText, but lineedits hardcode QPalette::Text
            // => i just look for the lables, take their palette,
            // map the WindowText color to the Text color and put the palette on the lineedit
            // (font adjustment is just for fun - and looks better + is more readable, especially on
            // default "white-on-black")
            // DRAWBACK: currently this will NOT survice plasma theme changes - i'd have to
            // monitor the label for palette changes.. we'll see (the proper fixes would be in
            // qlineedit - don't hardcode fg - and krunner - plasmafy all widgets)
            QPalette pal = lables.at(0)->palette();
            pal.setColor(QPalette::Base, pal.color(QPalette::Active, QPalette::Window));
            pal.setColor(QPalette::Text, pal.color(QPalette::Active, QPalette::WindowText));
            pal.setColor(QPalette::HighlightedText, pal.color(QPalette::Active, QPalette::Window));
            pal.setColor(QPalette::Highlight, pal.color(QPalette::Active, QPalette::WindowText));
            widget->setPalette(pal);
            QFont fnt = widget->font(); fnt.setBold(true); widget->setFont(fnt);
        }
    }
#endif

    /// Menubars and toolbar default to QPalette::Button - looks crap and leads to flicker...?!
    QMenuBar *mbar = qobject_cast<QMenuBar *>(widget);
    if (mbar && !((appType == QtDesigner) && mbar->inherits("QDesignerMenuBar")))
        MacMenu::manage(mbar);

    bool isTopContainer = (mbar || qobject_cast<QToolBar *>(widget));
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

    /// KHtml css colors can easily get messed up, either because i'm unsure about what colors
    /// are set or KHtml does wrong OR (mainly) by html "designers"
    if (IS_HTML_WIDGET)
    {   // the eventfilter watches palette changes and ensures contrasted foregrounds...
        widget->installEventFilter(this);
        QEvent ev(QEvent::PaletteChange);
        eventFilter(widget, &ev);
    }
}
#undef PAL

void
Style::unPolish( QApplication *app )
{
    app->setPalette(QPalette());
}

void
Style::unPolish( QWidget *widget )
{
    if (QFrame *frame = qobject_cast<QFrame *>(widget))
        VisualFrame::release(frame);
    if (QMenuBar *mbar = qobject_cast<QMenuBar *>(widget))
        MacMenu::release(mbar);

    Animator::Hover::release(widget);
    Animator::Progress::release(widget);
    Animator::Tab::release(widget);
    Hacks::remove(widget);

    widget->removeEventFilter(this);
}
#undef CCOLOR
#undef FCOLOR
