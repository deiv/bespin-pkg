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
#include <QApplication>
#include <QComboBox>
#include <QDockWidget>
#include <QEvent>
#include <QFrame>
#include <QListView>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPainter>
#include <QStylePlugin>
#include <QScrollBar>
#include <QToolButton>
#include <QTreeView>
#include <QtDBus/QDBusInterface>

/**============= Bespin includes ==========================*/

// #include "debug.h"

#ifdef Q_WS_X11
#include "xproperty.h"
#endif
#include "oxrender.h"
#include "colors.h"
#include "bespin.h"

#include "animator/hover.h"

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
            return new Bespin::Style;
        return 0;
    }
};

Q_EXPORT_PLUGIN2(Bespin, BespinStylePlugin)
/**=========================================================*/

using namespace Bespin;

AppType Style::appType;
Config Style::config;
Dpi Style::dpi;
Style::Lights Style::lights;
Style::Masks Style::masks;
QPalette *Style::originalPalette = 0;
Style::Shadows Style::shadows;


#define N_PE 54
#define N_CE 50
#define N_CC 12
static void
(Style::*primitiveRoutine[N_PE])(const QStyleOption*, QPainter*, const QWidget*) const;
static void
(Style::*controlRoutine[N_CE])(const QStyleOption*, QPainter*, const QWidget*) const;
static void
(Style::*complexRoutine[N_CC])(const QStyleOptionComplex*, QPainter*, const QWidget*) const;

#define registerPE(_FUNC_, _ELEM_) primitiveRoutine[QStyle::_ELEM_] = &Style::_FUNC_
#define registerCE(_FUNC_, _ELEM_) controlRoutine[QStyle::_ELEM_] = &Style::_FUNC_
#define registerCC(_FUNC_, _ELEM_) complexRoutine[QStyle::_ELEM_] = &Style::_FUNC_

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
Style::registerRoutines()
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
#ifdef QT3_SUPPORT
    registerPE(skip, PE_Q3DockWindowSeparator);
    registerCE(skip, CE_Q3DockWindowEmptyArea);
#endif
    if (config.bg.mode == Scanlines)
        registerPE(drawDockBg, PE_FrameDockWidget);
    else
        registerPE(skip, PE_FrameDockWidget);
    registerCE(drawDockTitle, CE_DockWidgetTitle);
    registerCC(drawMDIControls, CC_MdiControls);
    registerPE(drawDockHandle, PE_IndicatorDockWidgetResizeHandle);
    // frames.cpp
    registerCE(skip, CE_FocusFrame);
#if QT_VERSION >= 0x040400
    registerPE(skip, PE_PanelStatusBar);
    registerPE(skip, PE_FrameStatusBarItem);
#else
    registerPE(skip, PE_FrameStatusBar);
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
#ifdef QT3_SUPPORT
    registerPE(drawItemCheck, PE_Q3CheckListIndicator);
    registerPE(drawExclusiveCheck_p, PE_Q3CheckListExclusiveIndicator);
#endif
    registerPE(drawMenuCheck, PE_IndicatorMenuCheckMark);
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
    if (config.bg.mode == Scanlines && config.bg.mode < 5)
        registerCE(drawDockBg, CE_ToolBar);
    else
        registerCE(skip, CE_ToolBar);
    registerPE(skip, PE_FrameButtonTool);
#ifdef QT3_SUPPORT
    registerPE(skip, PE_Q3Separator);
#endif
    registerPE(drawToolBarHandle, PE_IndicatorToolBarHandle);
    // views.cpp
    registerCE(drawHeader, CE_Header);
    registerCE(drawHeaderSection, CE_HeaderSection);
    registerCE(drawHeaderLabel, CE_HeaderLabel);
    registerPE(drawBranch, PE_IndicatorBranch);
#ifdef QT3_SUPPORT
    registerCC(drawTree, CC_Q3ListView);
#endif
    registerCE(drawRubberBand, CE_RubberBand);
    registerPE(drawHeaderArrow, PE_IndicatorHeaderArrow);
#if QT_VERSION >= 0x040400
    registerPE(drawItem, PE_PanelItemViewRow);
    registerPE(drawItem, PE_PanelItemViewItem);
#endif
    // window.cpp
    registerPE(drawWindowFrame, PE_FrameWindow);
//     if (config.menu.shadow)
        registerPE(drawWindowFrame, PE_FrameMenu);
//     else
//         registerPE(skip, PE_FrameMenu);
    registerPE(drawWindowBg, PE_Widget);
    registerPE(drawToolTip, PE_PanelTipLabel);
    registerCC(drawTitleBar, CC_TitleBar);
    registerCE(drawDockHandle, CE_Splitter);
    registerCE(drawSizeGrip, CE_SizeGrip);
}

/**THE STYLE ITSELF*/
#include <QTimer>
Style::Style() : QCommonStyle()
{
    setObjectName(QLatin1String("Bespin"));
    init();
    registerRoutines();
}

Style::~Style()
{
   Gradients::wipe();
}

#include "makros.h"
#undef PAL
#define PAL pal

QColor
Style::btnBg( const QPalette &pal, bool isEnabled, bool hasFocus, int step, bool fullHover,
                    bool reflective) const
{

    if (!isEnabled)
        return Colors::mid(Qt::black, FCOLOR(Window),5,100);

    QColor c = CCOLOR(btn.std, Bg);
    if (hasFocus && config.btn.active_role[Bg] != QPalette::Highlight)
        if (config.btn.layer)
            c = FCOLOR(Highlight);
        else
            c = Colors::mid(FCOLOR(Highlight), c, 1, 10 + Colors::contrast(FCOLOR(Highlight), c));

    if (fullHover && step)
        c = Colors::mid(c, CCOLOR(btn.active, Bg), (config.btn.backLightHover ? (80-32*reflective) : 6) - step, step);

    return c;
}

QColor
Style::btnFg(const QPalette &pal, bool isEnabled, bool hasFocus, int step, bool flat) const
{
    if (!isEnabled)
        return FCOLOR(WindowText); //Colors::mid(FCOLOR(Window), FCOLOR(WindowText), 1, 3);

    if (!config.btn.layer || config.btn.active_role[Bg] == QPalette::Highlight)
        hasFocus = false;
    QColor  fg1 = hasFocus ? FCOLOR(HighlightedText) : CCOLOR(btn.std, Fg),
            fg2 = CCOLOR(btn.active, Fg);
    if (flat)
        { fg1 = FCOLOR(WindowText); fg2 = FCOLOR(Highlight); }

    if (!flat && config.btn.backLightHover)
        return fg1;

    if (step)
        return Colors::mid(fg1, fg2, 6 - step, step);

    return fg1;
}

void
Style::drawItemText(QPainter *painter, const QRect &rect, int alignment, const QPalette &pal,
                          bool enabled, const QString& text, QPalette::ColorRole textRole, QRect *boundingRect) const
{
    if (text.isEmpty())
        return;
    QPen savedPen;
    bool penDirty = false;
    if (textRole != QPalette::NoRole)
    {
        penDirty = true;
        savedPen = painter->pen();
        painter->setPen(QPen(pal.brush(textRole), savedPen.widthF()));
    }
    if (!enabled)
    {   // let's see if we can get some blurrage here =)
        if (!penDirty)
            { savedPen = painter->pen(); penDirty = true; }

        QColor c = painter->pen().color();
        c.setAlpha(c.alpha()/4 + 2);
        painter->setPen(QPen(c, savedPen.widthF()));
        QRect r = rect;
        r.translate(-1,-1);
        painter->drawText(r, alignment, text);
        r.translate(1,2);
        painter->drawText(r, alignment, text);
        r.translate(2,0);
        painter->drawText(r, alignment, text);
        r.translate(-1,-2);
        painter->drawText(r, alignment, text);
    }
    else
        painter->drawText(rect, alignment, text, boundingRect);
    if (penDirty)
        painter->setPen(savedPen);
}

#define X_KdeBase 0xff000000
#define SH_KCustomStyleELement 0xff000001

enum CustomElements { _CE_CapacityBar = 0 /*, ...*/, N_CustomControls };
#if 0
enum SubElements { _SE_AmarokAnalyzerSlider = 0 /*, ...*/, N_CustomSubElements };
#endif

static QStyle::ControlElement primitives[N_CustomControls];
#if 0
static QStyle::SubElement subcontrols[N_CustomSubElements];
#endif

enum ElementType { SH, CE, SE };
static QMap<QString, int> styleElements; // yes. one is enough...
// NOTICE: using QHash instead QMap is probably overhead, there won't be too many items per app
static int counter[3] = { X_KdeBase+3 /*sic!*/, X_KdeBase, X_KdeBase };

void
Style::drawPrimitive ( PrimitiveElement pe, const QStyleOption * option,
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
    {
//         qDebug() << "BESPIN, unsupported primitive:" << pe << widget;
        QCommonStyle::drawPrimitive( pe, option, painter, widget );
    }
}

void
Style::drawControl ( ControlElement element, const QStyleOption * option,
                           QPainter * painter, const QWidget * widget) const
{
    Q_ASSERT(option);
    Q_ASSERT(painter);
    if (element < N_CE && controlRoutine[element])
        (this->*controlRoutine[element])(option, painter, widget);
    else if (element > X_KdeBase)
    {
        if (element == primitives[_CE_CapacityBar])
            drawCapacityBar(option, painter, widget);
        //if (pe == primitives[_PE_WhateverElse])
        // ...
    }
    else
    {
//         qDebug() << "BESPIN, unsupported control:" << element << widget;
        QCommonStyle::drawControl( element, option, painter, widget );
    }
}

void
Style::drawComplexControl ( ComplexControl control, const QStyleOptionComplex * option,
                                  QPainter * painter, const QWidget * widget) const
{
    Q_ASSERT(option);
    Q_ASSERT(painter);
    if (control < N_CC && complexRoutine[control])
        (this->*complexRoutine[control])(option, painter, widget);
    else
    {
//         qDebug() << "BESPIN, unsupported complex control:" << control << widget;
        QCommonStyle::drawComplexControl( control, option, painter, widget );
    }
}

int
Style::elementId(const QString &string) const
{
    int id = styleElements.value(string, 0);
    if (id)
        return id;

    if (string == "CE_CapacityBar")
        primitives[_CE_CapacityBar] = (ControlElement)(id = ++counter[CE]);
#if 0
    else if (string == "amarok.CC_Analyzer")
        complexs[_CC_AmarokAnalyzer] = (ComplexControl)(id = ++counter[CC]);
    // subcontrols (SC_) work muchg different as they're 1. flags and 2. attached to a CC
    else if (string == "amarok.CC_Analyzer:SC_Slider")
    {
        subcontrols[_SC_AmarokAnalyzerSlider] = (SubControl)(id = (1 << scCounter[_CC_AmarokAnalyzer]));
        ++scCounter[_CC_AmarokAnalyzer];
    }
//     else if blablablaba...
#endif
    if (id)
        styleElements.insert(string, id);
    return id;
}

/// ----------------------------------------------------------------------

void
Style::fillWithMask(QPainter *painter, const QPoint &xy,
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
   qPix = FX::applyAlpha(qPix, mask);
   painter->drawPixmap(xy, qPix);
}

void
Style::erase(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QWidget *grampa = widget;
    while ( !(grampa->isWindow() ||
            (grampa->autoFillBackground() && grampa->objectName() != "qt_scrollarea_viewport")))
        grampa = grampa->parentWidget();

    QPoint tl = widget->mapFrom(const_cast<QWidget*>(grampa), QPoint());
    painter->save();
    painter->setPen(Qt::NoPen);
    
    if (!grampa->isWindow())
    {   // we may encounter apps that have semi or *cough* fully *cough* amarok *cough*
        // transparent backgrounds instead of none... ;-)
        painter->setBrush(grampa->window()->palette().color(QPalette::Window));
        painter->drawRect(option->rect);
    } // (semi) catched! (for about 98% of all cases...)
    painter->setBrush(grampa->palette().brush(grampa->backgroundRole()));
    painter->setBrushOrigin(tl);
    painter->drawRect(option->rect);

    if (grampa->isWindow())
    {   // means we need to paint the global bg as well
        painter->setClipRect(option->rect, Qt::IntersectClip);
        QStyleOption tmpOpt = *option;
//         tmpOpt.rect = QRect(tl, widget->size());
        tmpOpt.palette = grampa->palette();
        painter->fillRect(option->rect, grampa->palette().brush(QPalette::Window));
        if (config.bg.mode > Scanlines) painter->translate(tl);
        drawWindowBg(&tmpOpt, painter, grampa);
    }
    painter->restore();
}

// X11 properties for the deco ---------------
void
Style::setupDecoFor(QWidget *widget, const QPalette &palette, int mode, const Gradients::Type (&gt)[2])
{
    // this is important because KDE apps may alter the original palette any time
    const QPalette &pal = originalPalette ? *originalPalette : palette;

    // the title region in the center
    WindowData data;
    if (widget && widget->testAttribute(Qt::WA_MacBrushedMetal))
    {
        data.style = (((Plain & 0xff) << 16) | ((Gradients::None & 0xff) << 8) | (Gradients::None & 0xff));
        QColor bg = pal.color(QPalette::Inactive, QPalette::Window);
        bg = bg.light(115-Colors::value(bg)/20);
        data.inactiveWindow = bg.rgba();
        bg = pal.color(QPalette::Active, QPalette::Window);
        bg = bg.light(115-Colors::value(bg)/20);
        data.activeWindow = bg.rgba();
    }
    else
    {
        data.style = (((mode & 0xff) << 16) | ((gt[0] & 0xff) << 8) | (gt[1] & 0xff));
        data.inactiveWindow = pal.color(QPalette::Inactive, QPalette::Window).rgba();
        data.activeWindow = pal.color(QPalette::Active, QPalette::Window).rgba();
    }
    data.inactiveDeco = CCOLOR(kwin.inactive, Bg).rgba();
    data.activeDeco = CCOLOR(kwin.active, Bg).rgba();
    data.inactiveText = Colors::mid(pal.color(QPalette::Inactive, QPalette::Window), CCOLOR(kwin.text, Bg)).rgba();
    data.activeText = CCOLOR(kwin.text, Fg).rgba();
//     const QColor bg_inact = (gt[0] != Gradients::None && config.kwin.active_role == config.kwin.inactive_role) ?
//     Colors::mid(CCOLOR(kwin.inactive, Bg), CCOLOR(kwin.inactive, Fg), 2, 1) :    ;
    data.inactiveButton = Colors::mid(CCOLOR(kwin.inactive, Bg), CCOLOR(kwin.inactive, Fg), 1, 2).rgba();
    data.activeButton = CCOLOR(kwin.active, Fg).rgba();
    
    // XProperty actually handles the non X11 case, but we avoid overhead ;)
#ifdef Q_WS_X11
    if (widget)
        XProperty::set<uint>(widget->winId(), XProperty::winData, (uint*)&data, XProperty::WORD, 9);
    else
#endif
    {   // dbus solution, currently for gtk
        QByteArray ba(36, 'a');
        uint *ints = (uint*)ba.data();
        ints[0] = data.inactiveWindow;
        ints[1] = data.activeWindow;
        ints[2] = data.inactiveDeco;
        ints[3] = data.activeDeco;
        ints[4] = data.inactiveText;
        ints[5] = data.activeText;
        ints[6] = data.inactiveButton;
        ints[7] = data.activeButton;
        ints[8] = data.style;

        QDBusInterface bespinDeco( "org.kde.kwin", "/BespinDeco", "org.kde.BespinDeco");
#if QT_VERSION < 0x040400
        const qint64 pid = getpid();
#else
        const qint64 pid = QCoreApplication::applicationPid();
#endif
        bespinDeco.call(QDBus::NoBlock, "styleByPid", pid, ba);
    }
}

static const
QPalette::ColorGroup groups[3] = { QPalette::Active, QPalette::Inactive, QPalette::Disabled };

static void
swapPalette(QWidget *widget, Style *style)
{
    // protect our KDE palette fix - in case
//     QPalette *savedPal = originalPalette;
//     originalPalette = 0;
    // looks complex? IS!
    // reason nr. 1: stylesheets. they're nasty and qt operates on the app palette here
    // reason nr. 2: some idiot must have spread the idea that pal.setColor(backgroundRole(), Qt::transparent) is a great
    // idea instead of just using setAutoFillBackground(true), preserving all colors and just not using them.
    // hey, why not call qt to paint some nothing.... *grrrr* i'm angry... again!
    
    QMap<QWidget*, QString> shits;
    QList<QWidget*> kids = widget->findChildren<QWidget*>();
    kids.prepend(widget);

    QPalette pal;
    QPalette::ColorGroup group;
    QWidget *solidBase = 0;
    QColor c1, c2; int a;
    bool fixViewport = false;
    bool hasShit = false;
    foreach (QWidget *kid, kids)
    {
        if (kid->testAttribute(Qt::WA_StyleSheet))
        {   // first get rid of shit
            shits.insert(kid, kid->styleSheet());
            kid->setStyleSheet(QString());
            hasShit = true;
        }
        
        // now swap the palette ----------------
        if (hasShit || kid->testAttribute(Qt::WA_SetPalette) || kid == widget)
        {
            pal = kid->palette();
            solidBase = 0;
            fixViewport = false;
            hasShit = false;
            
            // NOTE: WORKAROUND for dolphin and probably others: see polish.cpp
            if (QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea*>(kid) )
            if (QWidget *vp = area->viewport())
            if (!vp->autoFillBackground() || vp->palette().color(QPalette::Active, vp->backgroundRole()).alpha() == 0)
                fixViewport = true;

            if (fixViewport || kid->palette().color(QPalette::Active, QPalette::Window).alpha() == 0)
            {
                solidBase = kid;
                while ((solidBase = solidBase->parentWidget()))
                {
                    if ((solidBase->autoFillBackground() &&
                        solidBase->palette().color(QPalette::Active, solidBase->backgroundRole()).alpha() != 0) ||
                        solidBase->isWindow())
                        break;
                }
                if (solidBase->palette().brush(solidBase->backgroundRole()).style() > 1)
                    solidBase = 0; // there's some pixmap or so - better do not swap colors atm.
            }
            for (int i = 0; i < 3; ++i)
            {
                group = groups[i];
                
                if (solidBase && !fixViewport) // changing bg color is useless and it's worthless to calculate the fg color
                    pal.setColor(group, QPalette::WindowText, solidBase->palette().color(group, solidBase->foregroundRole()));
                else
                {
                    c1 = pal.color(group, QPalette::Window);
                    a = c1.alpha();
                    c2 = pal.color(group, QPalette::WindowText);
                    c1.setAlpha(c2.alpha());
                    c2.setAlpha(a);
                    pal.setColor(group, QPalette::Window, c2);
                    pal.setColor(group, QPalette::WindowText, c1);
                }

                c1 = pal.color(group, QPalette::Button);
                a = c1.alpha();
                c2 = pal.color(group, QPalette::ButtonText);
                c1.setAlpha(c2.alpha());
                c2.setAlpha(a);
                pal.setColor(group, QPalette::Button, c2);
                pal.setColor(group, QPalette::ButtonText, c1);

                if (solidBase && fixViewport)
                {   // means we have a widget w/o background, don't swap colors, but set colors to solidBase
                    // this is very much a WORKAROUND
                    pal.setColor(group, QPalette::Text, solidBase->palette().color(group, solidBase->foregroundRole()));
                }
            }
            style->polish(pal, false);
            kid->setPalette(pal);

        }
    }

    // this is funny: style shits rely on QApplication::palette() (nice trick, TrottelTech... again)
    // so to apply them with the proper color, we need to change the apps palette to the swapped one,...
    if (!shits.isEmpty())
    {
        QPalette appPal = QApplication::palette();
        // ... reapply the shits...
        QMap<QWidget*, QString>::const_iterator shit = shits.constBegin();
        while (shit != shits.constEnd())
        {
            QApplication::setPalette(shit.key()->palette());
            shit.key()->setStyleSheet(shit.value());
            ++shit;
        }
        // ... and reset the apps palette
        QApplication::setPalette(appPal);
    }
    
//     originalPalette = savedPal;
}

static QMenuBar*
bar4popup(QMenu *menu)
{
    if (!menu->menuAction())
        return NULL;
    if (menu->menuAction()->associatedWidgets().isEmpty())
        return NULL;
    foreach (QWidget *w, menu->menuAction()->associatedWidgets())
        if (qobject_cast<QMenuBar*>(w))
            return static_cast<QMenuBar *>(w);
    return NULL;
}

bool
Style::eventFilter( QObject *object, QEvent *ev )
{
    switch (ev->type())
    {
    case QEvent::MouseMove:
    case QEvent::Timer:
    case QEvent::Move:
        return false; // just for performance - they can occur really often
    case QEvent::Paint:
        if (QFrame *frame = qobject_cast<QFrame*>(object))
        {
            if ((frame->frameShape() == QFrame::HLine || frame->frameShape() == QFrame::VLine) &&
                 frame->isVisible())
            {
                QPainter p(frame);
                Orientation3D o3D = (frame->frameShadow() == QFrame::Sunken) ? Sunken :
                                    (frame->frameShadow() == QFrame::Raised) ? Raised : Relief;
                const bool v = frame->frameShape() == QFrame::VLine;
                shadows.line[v][o3D].render(frame->rect(), &p);
                p.end();
                return true;
            }
            return false;
        }
        else if (QTabBar *tabBar = qobject_cast<QTabBar*>(object))
        {
            if (tabBar->parentWidget() && qobject_cast<QTabWidget*>(tabBar->parentWidget()))
                return false; // no extra tabbar here please... ;)
            QPainter p(tabBar);
            QStyleOptionTabBarBase opt;
            opt.initFrom(tabBar);
            if (QWidget *window = tabBar->window())
            {
                opt.tabBarRect = window->rect();
                opt.tabBarRect.moveTopLeft(tabBar->mapFrom(window, opt.tabBarRect.topLeft()));
            }
            drawTabBar(&opt, &p, NULL);
            p.end();
            return false;
        }
#if 0// doesn't work. sth. weakens the fist sector -> TODO: make KUrlNavigator make use of custom style elements
        else if (object->inherits("KUrlNavigator"))
        {
            QList<QComboBox*> lel = object->findChildren<QComboBox*>();
            if (!lel.isEmpty() && lel.first()->isVisible())
                return false;
            QWidget *w = static_cast<QWidget*>(object);
            QStyleOption opt; opt.initFrom(w);
            QPainter p(w);
            drawLineEditFrame(&opt, &p, w);
            p.end();
            return true;
        }
#endif
        return false;

    case QEvent::Resize:
        
        if (config.menu.round && qobject_cast<QMenu*>(object)
            /*|| qobject_cast<QDockWidget*>(object)*/) // kwin yet cannot. compiz can't even menus...
        {
            QWidget *widget = static_cast<QWidget*>(object);
            // this would be for docks
//             if (!widget->isWindow())
//             {
//                 widget->clearMask();
//                 return false;
//             }
#if 0 // xPerimental code for ribbon like looking menus - not atm.
            QAction *head = menu->actions().at(0);
            QRect r = menu->fontMetrics().boundingRect(menu->actionGeometry(head),
            Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine | Qt::TextExpandTabs | BESPIN_MNEMONIC,
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
            mask -= masks.corner[3].translated(menu->width()-br.width(), menu->height()-br.height()); // br
#endif
            const int w = widget->width();
            const int h = widget->height();
            QRegion mask(4, 0, w-8, h);
            mask += QRegion(0, 4, w, h-8);
            mask += QRegion(2, 1, w-4, h-2);
            mask += QRegion(1, 2, w-2, h-4);
// only top rounded - but looks nasty
//          QRegion mask(0, 0, w, h-4);
//          mask += QRect(1, h-4, w-2, 2);
//          mask += QRect(2, h-2, w-4, 1);
//          mask += QRect(4, h-1, w-8, 1);

            widget->setMask(mask);
            return false;
        }
        return false;

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
    case QEvent::Wheel:
    {
        if (QAbstractSlider* slider = qobject_cast<QAbstractSlider*>(object))
        {
            QWheelEvent *we = static_cast<QWheelEvent*>(ev);
            if ((slider->value() == slider->minimum() && we->delta() > 0) ||
                (slider->value() == slider->maximum() && we->delta() < 0))
                Animator::Hover::Play(slider);
            return false;
        }
    
        if (QListView *list = qobject_cast<QListView*>(object))
        //         if (list->verticalScrollMode() == QAbstractItemView::ScrollPerPixel) // this should be, but semms to be not
        if (list->inherits("KCategorizedView"))
            list->verticalScrollBar()->setSingleStep(list->iconSize().height()/3);
        return false;
    }
#ifdef MOUSEDEBUG
    case QEvent::MouseButtonPress:
    {
        QMouseEvent *mev = (QMouseEvent*)ev;
        qDebug() << "BESPIN:" << object;
        //       DEBUG (object);
        return false;
    }
#endif
    case QEvent::Show:
    {
        QWidget * widget = qobject_cast<QWidget*>(object);
        if (!widget)
            return false;
        
        if (widget->isModal())
        {
            if (config.bg.modal.invert)
                swapPalette(widget, this);
            if (config.bg.modal.glassy)
                widget->setAttribute(Qt::WA_MacBrushedMetal);
#ifdef Q_WS_X11
            setupDecoFor(widget, widget->palette(), config.bg.mode, GRAD(kwin));
#endif
            widget->setWindowOpacity( config.bg.modal.opacity/100.0 );
            return false;
        }
        if (QMenu * menu = qobject_cast<QMenu*>(widget))
        {
            // seems to be necessary, somehow KToolBar context menus manages to take QPalette::Window...?!
            // through title setting?!
            menu->setBackgroundRole ( config.menu.std_role[Bg] );
            menu->setForegroundRole ( config.menu.std_role[Fg] );
            if (menu->parentWidget() && menu->parentWidget()->inherits("QMdiSubWindow"))
            {
                QPoint pt = menu->parentWidget()->rect().topRight();
                pt += QPoint(-menu->width(), pixelMetric(PM_TitleBarHeight,0,0));
                pt = menu->parentWidget()->mapToGlobal(pt);
                menu->move(pt);
            }
            QMenuBar *bar = bar4popup(menu);
            if (bar)
#if 0
            {
                QPoint pos(dpi.f1, 0);
                pos += bar->actionGeometry(menu->menuAction()).topLeft();
                menu->move(bar->mapToGlobal(pos));
                menu->setActiveAction(menu->actions().at(0));
            }
#else
            menu->move(menu->pos()-QPoint(0,dpi.f2));
#endif
            return false;
        }
        return false;
    }
    case QEvent::Hide:
        if (config.bg.modal.invert)
        if (QWidget * widget = qobject_cast<QWidget*>(object))
        if (widget->isModal())
            swapPalette(widget, this);
        return false;
        
    case QEvent::PaletteChange:
    {
        #define LACK_CONTRAST(_ROLE_, _C1_, _C2_) Colors::contrast(pal.color(_ROLE_, _C1_), pal.color(_ROLE_, _C2_)) < 40
        #define HARD_CONTRAST(_ROLE_, _C_) Colors::value(pal.color(_ROLE_, _C_)) < 128 ? Qt::white : Qt::black
        QWidget * widget = qobject_cast<QWidget*>(object);
        if (!widget)
            return false;
        // NOTICE: THIS SUCKS!
        // atm the palettes seem to have enough contrast (they pretend black on white) but end up like white on white
        // i hav no idea what's going on here, but the situation is the same with all styles (i assume khtml assumes
        // window == base == button == white, wtext == text == btext == black)
        // this might also be Qt extra smart not setting equal palettes and mispaint as the widget has no own palette
        // I HAVE NO IDEA :-(
        // so for now i force a hard contrast for all elements - may not be the most fancy, but at least a usable solution
        if (widget->objectName() == "RenderFormElementWidget")
        {
                QPalette pal = widget->palette();
                for (int g = 0; g < 3; ++g)
                {
                    QPalette::ColorGroup group = (QPalette::ColorGroup)g;
                    pal.setColor(group, QPalette::WindowText, HARD_CONTRAST(group, QPalette::Window));
                    pal.setColor(group, QPalette::ButtonText, HARD_CONTRAST(group, QPalette::Button));
                    pal.setColor(group, QPalette::HighlightedText, HARD_CONTRAST(group, QPalette::Highlight));
                    pal.setColor(group, QPalette::Text, HARD_CONTRAST(group, QPalette::Base));
                    pal.setColor(group, widget->foregroundRole(), HARD_CONTRAST(group, widget->backgroundRole()));
                }
                widget->removeEventFilter(this);
                widget->setPalette(pal);
                widget->installEventFilter(this);
                if (QComboBox *box = qobject_cast<QComboBox*>(widget))
                if (box->view())
                {
                    pal = box->view()->palette();
                    for (int g = 0; g < 3; ++g)
                    {
                        QPalette::ColorGroup group = (QPalette::ColorGroup)g;
                        pal.setColor(group, QPalette::Text, HARD_CONTRAST(group, QPalette::Base));
                    }
                    box->view()->setPalette(pal);
                }
        }
        return false;
    }
    case QEvent::ApplicationPaletteChange:
    {
        if (object == qApp && originalPalette)
        {
            // this fixes KApplications
            // "we create the style, then reload the palette from personal settings and reapply it" junk"
            // the order is important or we'll get reloads for sure or eventually!
            object->removeEventFilter(this);
            QPalette *pal = originalPalette;
            originalPalette = 0;
            polish(*pal);
            qApp->setPalette(*pal);
            delete pal;
        }
        return false;
    }
    default:
        return false;
    }
}


QPalette
Style::standardPalette () const
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

