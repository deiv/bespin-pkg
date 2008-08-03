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
#include <QLayout>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPen>
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

#define IS_HTML_WIDGET (widget->objectName() == "RenderFormElementWidget")

using namespace Bespin;

extern Config config;
extern Dpi dpi;

static bool isQtDesigner = false;

static inline void
setBoldFont(QWidget *w, bool bold = true)
{
    QFont fnt = w->font();
    fnt.setBold(bold);
    w->setFont(fnt);
}

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
    isQtDesigner = isQtDesigner || app->applicationName() == "Designer";
}

#define _SHIFTCOLOR_(clr) clr = QColor(CLAMP(clr.red()-10,0,255),CLAMP(clr.green()-10,0,255),CLAMP(clr.blue()-10,0,255))


void BespinStyle::polish( QPalette &pal )
{
    QColor c = pal.color(QPalette::Active, QPalette::Background);
    if (config.bg.mode > Scanlines)
    {
        int h,s,v;
        c.getHsv(&h,&s,&v);
        if (v < 70) // very dark colors won't make nice backgrounds ;)
            c.setHsv(h,s,70);
        pal.setColor( QPalette::Window, c );
    }
    if (_scanlines[0]) delete _scanlines[0]; _scanlines[0] = 0;
    if (_scanlines[1]) delete _scanlines[1]; _scanlines[1] = 0;
    QLinearGradient lg; QPainter p;
    if (config.bg.mode == Scanlines)
    {
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

    // dark, light & etc are tinted... no good:
    pal.setColor(QPalette::Dark, QColor(70,70,70));
    pal.setColor(QPalette::Mid, QColor(100,100,100));
    pal.setColor(QPalette::Midlight, QColor(220,220,220));
    pal.setColor(QPalette::Light, QColor(240,240,240));


#if QT_VERSION >= 0x040400
    // tooltip (NOTICE not configurable by qtconfig, kde can, let's see what we're gonna do on this...)
    pal.setColor(QPalette::ToolTipBase, pal.color(QPalette::Active, QPalette::WindowText));
    pal.setColor(QPalette::ToolTipText, pal.color(QPalette::Active, QPalette::Window));
#endif

    // inactive palette
    if (config.fadeInactive)
    { // fade out inactive foreground and highlight colors...
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

#undef PAL
#define PAL pal

inline static void
polishGTK(QWidget * widget)
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

void
BespinStyle::polish( QWidget * widget )
{
    // GTK-Qt gets a special handling - see above
    if (isGTK)
    {
        polishGTK(widget);
        return;
    }

    // !!! protect against polishing /QObject/ attempts! (this REALLY happens from time to time...)
    if (!widget)
        return;

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

        // talk to kwin about colors, gradients, etc.
        setupDecoFor(widget);

        if (config.bg.mode > Scanlines)
            widget->setAttribute(Qt::WA_StyledBackground);

        //BEGIN Popup menu handling                                                                -
        if (QMenu *menu = qobject_cast<QMenu *>(widget))
        { // glass mode popups
            if (config.menu.glassy)
            {
                if (config.bg.mode == Scanlines)
                {
                    QPalette pal = widget->palette();
                    pal.setBrush(QPalette::Background, pal.color(QPalette::Active, QPalette::Background));
                    menu->setPalette(pal);
                }
                menu->setAttribute(Qt::WA_MacBrushedMetal);
                menu->setAttribute(Qt::WA_StyledBackground);
            }
            // apple style popups
            if (config.bg.mode == Scanlines)
            {
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
            if (config.menu.boldText)
                setBoldFont(menu);
            
            // eventfiltering to reposition MDI windows and correct distance to menubars
            menu->installEventFilter(this);
#if 0
            /// NOTE this was intended to be for some menu mock from nuno where the menu
            // reaches kinda ribbon-like into the bar
            // i'll keep it to remind myself and in case i get it to work one day ;-)
            if (bar4popup(menu))
            {
                QAction *action = new QAction( menu->menuAction()->iconText(), menu );
                connect (action, SIGNAL(triggered(bool)), menu, SLOT(hide()));
                menu->insertAction(menu->actions().at(0), action);
            }
#endif
        }
        //END Popup menu handling                                                                  -

        /// WORKAROUND Qt color bug, uses daddies palette and FGrole, but TooltipBase as background
        else if (widget->inherits("QWhatsThat"))
            widget->setPalette(QToolTip::palette()); // so this is Qt bug WORKAROUND
    
        /// modal dialogs
        else if (config.bg.modal.invert || config.bg.modal.glassy || config.bg.modal.opacity < 100)
            // the modality isn't necessarily set yet, so we catch it on QEvent::Show
            widget->installEventFilter(this);
            

    }
    //END Window handling                                                                          -

    //BEGIN Frames                                                                                 -
    else if (QFrame *frame = qobject_cast<QFrame *>(widget)) // sic!!! no window frames!
    {
        // sunken looks soo much nicer ;)
        if (frame->parentWidget() && frame->parentWidget()->inherits("KTitleWidget"))
            frame->setFrameShadow(QFrame::Sunken);

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
            if (QAbstractItemView *itemView = qobject_cast<QAbstractItemView*>(widget) )
            {
                if (QTreeView* tv = qobject_cast<QTreeView*>(frame))
                {   // allow all treeviews to be animated!
                    if (config.hack.treeViews)
                        tv->setAnimated ( true );
                }
                else // treeview hovering sucks, as the "tree" doesn't get an update
                { // Enable hover effects in listview, all itemviews like in kde is pretty annoying
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

    /// PUSHBUTTONS - hovering/animation ===========================================================
    else if (widget->inherits("QAbstractButton"))
    {
        if (widget->inherits("QToolBoxButton") || IS_HTML_WIDGET )
            widget->setAttribute(Qt::WA_Hover); // KHtml
        else
            Animator::Hover::manage(widget);

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
    
    /// COMBOBOXES - hovering/animation ============================================================
    else if (widget->inherits("QComboBox"))
    {
        if (IS_HTML_WIDGET)
            widget->setAttribute(Qt::WA_Hover);
        else
            Animator::Hover::manage(widget);
    }
    /// SLIDERS / SCROLLBARS / SCROLLAREAS - hovering/animation ====================================
    else if (qobject_cast<QAbstractSlider*>(widget))
    {
        widget->setAttribute(Qt::WA_Hover);
        // NOTICE
        // QAbstractSlider::setAttribute(Qt::WA_OpaquePaintEvent) saves surprisinlgy little CPU
        // so that'd just gonna add more complexity for literally nothing...
        if (widget->inherits("QScrollBar"))
        {   // ...as the slider is usually not bound to e.g. a "scrollarea"
            QWidget *dad = widget;
            if (!widget->parentWidget())
            {   // this catches e.g. plasma used QGraphicsProxyWidgets...
                qWarning("Bespin, transparent scrollbar!");
                widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
            }
            while ((dad = dad->parentWidget()))
            {   // digg for a potential KHTMLView ancestor, making this a html input scroller
                if (dad->inherits("KHTMLView"))
                {   // NOTICE this slows down things as it triggers a repaint of the frame
                    widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
                    // ...but this re-enbales speed and currently does the job
                    // TODO how's css/khtml policy on applying colors?
                    widget->setAutoFillBackground ( true );
                    widget->setBackgroundRole ( QPalette::Base ); // QPalette::Window looks wrong
                    break;
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

    /// PROGRESSBARS - hover/animation and bold font ===============================
    else if (widget->inherits("QProgressBar"))
    {
        widget->setAttribute(Qt::WA_Hover);
        setBoldFont(widget);
        Animator::Progress::manage(widget);
    }

    /// Tab animation, painting override
    else if (qobject_cast<QTabBar *>(widget))
    {
        widget->setAttribute(Qt::WA_Hover);
        // the eventfilter overtakes the widget painting to allow tabs ABOVE the tabbar
        widget->installEventFilter(this);
    }

    /// Menubars and toolbar default to QPalette::Button - looks crap and leads to flicker...?!
    QMenuBar *mbar = qobject_cast<QMenuBar *>(widget);
    if (mbar && !(isQtDesigner && mbar->inherits("QDesignerMenuBar")))
        MacMenu::manage(mbar);

    bool isTopContainer = (mbar || qobject_cast<QToolBar *>(widget));
#ifdef QT3_SUPPORT
    isTopContainer = isTopContainer || widget->inherits("Q3ToolBar");
#endif
    if (isTopContainer || qobject_cast<QToolBar *>(widget->parent()))
    {
        widget->setBackgroundRole(QPalette::Window);
        widget->setForegroundRole(QPalette::WindowText);
        if (isTopContainer && config.bg.mode == Scanlines)
        {
            widget->setAutoFillBackground ( true );
            QPalette pal = widget->palette();
            QColor c = pal.color(QPalette::Active, QPalette::Window);

            if (!_scanlines[1])
            makeStructure(&_scanlines[1], c, true);
            QBrush brush( c, *_scanlines[1] );
            pal.setBrush( QPalette::Window, brush );
            widget->setPalette(pal);
        }
        else if (widget->inherits("QToolBarHandle"))
            widget->setAttribute(Qt::WA_Hover);
    }

    /// hover some leftover widgets
    if (widget->inherits("QAbstractSpinBox") || widget->inherits("QSplitterHandle") ||
        widget->inherits("QWorkspaceTitleBar") || widget->inherits("Q3DockWindowResizeHandle"))
        widget->setAttribute(Qt::WA_Hover);

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

    /// KHtml css colors can easily get messed up, either because i'm unsure about what colors
    // are set or KHtml does wrong OR (mainly) by html "designers"
    if (IS_HTML_WIDGET)
    {   // the eventfilter watches palette changes and ensures contrasted foregrounds...
        widget->installEventFilter(this);
        QEvent ev(QEvent::PaletteChange);
        eventFilter(widget, &ev);
    }
   
}
#undef PAL

void
BespinStyle::unPolish( QApplication *app )
{
    app->setPalette(QPalette());
}

void
BespinStyle::unPolish( QWidget *widget )
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
