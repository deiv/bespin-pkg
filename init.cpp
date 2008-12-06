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

#include <QSettings>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QApplication>
#include <QTimer>

#include <cmath>

#include "animator/tab.h"
#include "colors.h"
#include "bespin.h"
#include "visualframe.h"
#include "makros.h"
#include "config.defaults"

#include <QtDebug>

using namespace Bespin;

static Gradients::Type _progressBase;

static void updatePalette(QPalette &pal, QPalette::ColorGroup group, const QStringList &list)
{
    int max = QPalette::NColorRoles;
    if (max > list.count()) {
        qWarning("The demanded palette seems to be incomplete!");
        max = list.count();
    }
    for (int i = 0; i < max; i++)
        pal.setColor(group, (QPalette::ColorRole) i, list.at(i));
}

static QStringList colors(const QPalette &pal, QPalette::ColorGroup group)
{
    QStringList list;
    for (int i = 0; i < QPalette::NColorRoles; i++)
        list << pal.color(group, (QPalette::ColorRole) i).name();
    return list;
}

#define readInt(_DEF_) iSettings->value(_DEF_).toInt()
#define readBool(_DEF_) iSettings->value(_DEF_).toBool()
#define readRole(_VAR_, _DEF_)\
config._VAR_##_role[0] = (QPalette::ColorRole) iSettings->value(_DEF_).toInt();\
Colors::counterRole(config._VAR_##_role[0], config._VAR_##_role[1])
//, QPalette::_DEF_, Colors::counterRole(QPalette::_DEF_))
#define readGrad(_DEF_) (Gradients::Type) iSettings->value(_DEF_).toInt();

void
Style::removeAppEventFilter()
{
    // for plain qt apps that don't need the palette fix, otherwise we'd keep filtering the app for no reason
    qApp->removeEventFilter(this);
}

void
Style::readSettings(const QSettings* settings)
{
    bool delSettings = false;

    QSettings *iSettings = const_cast<QSettings*>(settings);
    if (!iSettings)
    {
        delSettings = true;
        const char *preset = getenv("BESPIN_PRESET");
        QString qPreset;
        if (preset)
            qPreset = preset;
        else
        {   // maybe there's a preset config'd for this app...
            iSettings = new QSettings("Bespin", "Style");
            iSettings->beginGroup("PresetApps");
            QString cmd = QCoreApplication::applicationName();
            if (appType == KDM)
                cmd = "KDM"; // KDM segfaults on QCoreApplication::arguments()...
            else if (cmd.isEmpty() && !QCoreApplication::arguments().isEmpty())
                cmd = QCoreApplication::arguments().at(0).section('/', -1);
            qPreset = iSettings->value(cmd, QString()).toString();
            if (qPreset.isEmpty() && appType == GTK)
                qPreset = iSettings->value("GTK", QString()).toString();
            iSettings->endGroup();
            iSettings->beginGroup("Style");
        }
        if (!qPreset.isEmpty())
        {
            delete iSettings;
            iSettings = new QSettings("Bespin", "Store");
            if (iSettings->childGroups().contains(qPreset))
            {
                iSettings->beginGroup(qPreset);
                // set custom palette!
                QPalette pal;
                iSettings->beginGroup("QPalette");
                QStringList list = iSettings->value("active", colors(pal, QPalette::Active)).toStringList();
                updatePalette(pal, QPalette::Active, list);
                list = iSettings->value("inactive", colors(pal, QPalette::Inactive)).toStringList();
                updatePalette(pal, QPalette::Inactive, list);
                list = iSettings->value("disabled", colors(pal, QPalette::Disabled)).toStringList();
                updatePalette(pal, QPalette::Disabled, list);
                polish(pal);
                qApp->setPalette(pal);
                iSettings->endGroup();
            }
            else
                { delete iSettings; iSettings = 0L; }

            if (qApp->inherits("KApplication"))
            {   // KDE replaces the palette after styling with an unpolished own version, breaking presets...
                originalPalette = new QPalette(qApp->palette());
            }
            // must be for all apps - KDE does some freaky stuff between the qapp and kapp constructor...
            qApp->installEventFilter(this);
            QTimer::singleShot(10000, this, SLOT(removeAppEventFilter()));
        }
        if (!iSettings)
        {
            iSettings = new QSettings("Bespin", "Style");
            iSettings->beginGroup("Style");
        }
    }
    else
        qWarning("Bespin: WARNING - reading EXTERNAL settings!!!");

    // Background ===========================
    config.bg.minValue = readInt(BG_MINVALUE);
    config.bg.mode = (BGMode) readInt(BG_MODE);
    if (config.bg.mode > BevelH) config.bg.mode = BevelV;
    config.bg.modal.glassy = readBool(BG_MODAL_GLASSY);
    config.bg.modal.opacity = readInt(BG_MODAL_OPACITY);
    config.bg.modal.invert = (appType != KDM) && readBool(BG_MODAL_INVERT);
    config.bg.intensity = CLAMP(100+readInt(BG_INTENSITY),50,150);
    readRole(bg.tooltip, BG_TOOLTIP_ROLE);

#if 0
#ifndef QT_NO_XRENDER
   else if(config.bg.mode == ComplexLights &&
           !QFile::exists(QDir::tempPath() + "bespinPP.lock"))
      QProcess::startDetached(iSettings->
                              value("Bg.Daemon", "bespin pusher").toString());
#else
   if (config.bg.mode == ComplexLights) config.bg.mode = BevelV;
#endif
#endif

    if (config.bg.mode == Scanlines)
        config.bg.structure = readInt(BG_STRUCTURE);

    // Buttons ===========================
    config.btn.checkType = (Check::Type) readInt(BTN_CHECKTYPE);
    config.btn.round = readBool(BTN_ROUND);
    GRAD(btn) = readGrad(BTN_GRADIENT);
    _progressBase = GRAD(btn);
    if (config.btn.layer == 2 && GRAD(btn) == Gradients::Sunken) // NO!
        GRAD(btn) = Gradients::None;

    config.btn.backLightHover = readBool(BTN_BACKLIGHTHOVER);
    config.btn.layer = CLAMP(readInt(BTN_LAYER), 0, 2);
    config.btn.fullHover = config.btn.backLightHover || readBool(BTN_FULLHOVER);

    if (config.btn.layer == 2) config.btn.cushion = true;
    else if (GRAD(btn) ==  Gradients::Sunken) config.btn.cushion = false;
    else config.btn.cushion = readBool(BTN_CUSHION);

    readRole(btn.std, BTN_ROLE);
    readRole(btn.active, BTN_ACTIVEROLE);
    config.btn.ambientLight = readBool(BTN_AMBIENTLIGHT);
    config.btn.bevelEnds = readBool(BTN_BEVEL_ENDS);
   
    // Choosers ===========================
    GRAD(chooser) = readGrad(CHOOSER_GRADIENT);

    config.fadeInactive = readBool(FADE_INACTIVE);

    // Hacks ==================================
    config.hack.messages = readBool(HACK_MESSAGES);
    config.hack.KHTMLView = readBool(HACK_KHTMLVIEW);
    config.hack.krunner= readBool(HACK_KRUNNER);
    config.hack.treeViews = readBool(HACK_TREEVIEWS);
    config.hack.windowMovement = readBool(HACK_WINDOWMOVE);
    config.hack.killThrobber = readBool(HACK_THROBBER);

    // PW Echo Char ===========================
    config.input.pwEchoChar = ushort(iSettings->value(INPUT_PWECHOCHAR).toUInt());

    // kwin - yes i let the style control the deco, iff the deco permits, though :)
    config.kwin.gradient[0] = readGrad(KWIN_INACTIVE_GRADIENT);
    config.kwin.gradient[1] = readGrad(KWIN_ACTIVE_GRADIENT);
    readRole(kwin.inactive, KWIN_INACTIVE_ROLE);
    readRole(kwin.active, KWIN_ACTIVE_ROLE);
    config.kwin.text_role[0] = (QPalette::ColorRole) iSettings->value(KWIN_INACTIVE_TEXT_ROLE).toInt();
    config.kwin.text_role[1] = (QPalette::ColorRole) iSettings->value(KWIN_ACTIVE_TEXT_ROLE).toInt();

    // flanders
    config.leftHanded = readBool(LEFTHANDED) ? Qt::RightToLeft : Qt::LeftToRight;

    // item single vs. double click, wizard appereance
    config.macStyle = readBool(MACSTYLE);

    // Menus ===========================
    // TODO: redundant, kwin and afaik compiz can handle this
    config.menu.opacity = readInt(MENU_OPACITY);
    //--------
    config.menu.round = readBool(MENU_ROUND);
    config.menu.itemGradient = readGrad(MENU_ITEMGRADIENT);
    config.menu.showIcons = readBool(MENU_SHOWICONS);
    config.menu.shadow = readBool(MENU_SHADOW);
    if (appType == GTK)
    {
        config.menu.glassy = false;
        config.menu.std_role[Bg] = QPalette::Window;
        config.menu.std_role[Fg] = QPalette::WindowText;
        config.menu.active_role[Bg] = QPalette::Highlight;
        config.menu.active_role[Fg] = QPalette::HighlightedText;
        // both do not work :-( - also the user has to choose a bg that usually fits window fg
        config.menu.barGradient = Gradients::None;
        config.menu.barSunken = false;
    }
    else
    {
        config.menu.glassy = readBool(MENU_GLASSY);
        readRole(menu.active, MENU_ACTIVEROLE);
        readRole(menu.std, MENU_ROLE);
    }
    if (appType == Plasma)
    {   // that's probably XBar, ...
        // ... and we don't want a bg there
        config.menu.barGradient = Gradients::None;
        config.menu.barSunken = false;
    }
    else
    {
        config.menu.barGradient = readGrad(MENU_BAR_GRADIENT);
        config.menu.barSunken = readBool(MENU_BARSUNKEN);
    }
    readRole(menu.bar, MENU_BARROLE);
    config.menu.boldText = readBool(MENU_BOLDTEXT);
    config.menu.itemSunken = readBool(MENU_ITEM_SUNKEN);
    config.menu.activeItemSunken = config.menu.itemSunken || readBool(MENU_ACTIVEITEMSUNKEN);

    config.newWinBtns = true;

    // Progress ===========================
    GRAD(progress) = readGrad(PROGRESS_GRADIENT);
    config.progress.std_role[Bg] =  (QPalette::ColorRole) readInt(PROGRESS_ROLE_BG);
    config.progress.std_role[Fg] = (QPalette::ColorRole) readInt(PROGRESS_ROLE_FG);

    // ScrollStuff ===========================
    GRAD(scroll) = readGrad(SCROLL_GRADIENT);
    config.scroll.showButtons = readBool(SCROLL_SHOWBUTTONS);
    config.scroll.groove = (Groove::Mode) readInt(SCROLL_GROOVE);

    // Tabs ===========================
    readRole(tab.active, TAB_ACTIVEROLE);
    Animator::Tab::setDuration(CLAMP(iSettings->value(TAB_DURATION).toUInt(), 150, 4000));
    Animator::Tab::setFPS(25);
    GRAD(tab) = readGrad(TAB_GRADIENT);
    readRole(tab.std, TAB_ROLE);
    Animator::Tab::setTransition((Animator::Transition) readInt(TAB_TRANSITION));
    config.tab.activeTabSunken = readBool(TAB_ACTIVETABSUNKEN);
//     GRAD(toolbox) = readGrad(TAB_ACTIVEGRADIENT);

    // Views ===========================
    readRole(view.header, VIEW_HEADERROLE);
    readRole(view.sortingHeader, VIEW_SORTINGHEADERROLE);
    config.view.headerGradient = readGrad(VIEW_HEADERGRADIENT);
    config.view.sortingHeaderGradient = readGrad(VIEW_SORTINGHEADERGRADIENT);

    // General ===========================
    config.shadowIntensity = iSettings->value(SHADOW_INTENSITY).toInt()/100.0;
    // TODO: better make this an env var???
    config.scale = iSettings->value(DEF_SCALE).toDouble();
    if (config.scale != 1.0)
    {
        QFont fnt = qApp->font();
        if (fnt.pointSize() > -1) fnt.setPointSize(fnt.pointSize()*config.scale);
        else fnt.setPixelSize(fnt.pixelSize()*config.scale);
        qApp->setFont(fnt);
    }
    config.sunkenGroups = readBool(SUNKEN_GROUPS);

    //NOTICE gtk-qt fails on several features
    // a key problem seems to be fixed text colors
    // also it will segfault if we hide scrollbar buttons
    // so we adjust some settings here
    if (appType == GTK)
    {
        config.bg.mode = Plain;
        config.bg.modal.glassy = false;
        config.bg.modal.opacity = 100;
        config.bg.modal.invert = false;
        config.bg.intensity = 0;

//       config.btn.std_role[Bg] = QPalette::Window;
//       config.btn.active_role[Bg] = QPalette::Highlight;
//       config.btn.std_role[Fg] = config.btn.active_role[Fg] = QPalette::WindowText;

        config.hack.messages = false;

//         config.progress.std_role[Bg] = config.progress.std_role[Fg] = QPalette::Window;

//       config.tab.std_role[Bg] =  QPalette::Window;
    // gtk fixes the label color... so try to ensure it will be visible
//       if (!Colors::haveContrast(QApplication::palette().color(QPalette::WindowText),
//                                 QApplication::palette().color(config.tab.active_role[Bg])))
//       {
//          config.tab.active_role[Bg] =  QPalette::Window;
//          config.tab.activeTabSunken = true;
//       }

//         config.view.header_role[Bg] = QPalette::Window;
//         config.view.sortingHeader_role[Bg] = QPalette::Window;
    }
   
    if (delSettings)
        delete iSettings;
}

#undef readRole
#undef gradientType

#define SCALE(_N_) lround((_N_)*config.scale)

void Style::initMetrics()
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
   
   dpi.ScrollBarExtent = SCALE(config.scroll.groove > Groove::Groove ? 15 : 17);
   dpi.ScrollBarSliderMin = SCALE(40);
   dpi.SliderThickness = SCALE(20);
   dpi.SliderControl = SCALE(20);
   dpi.Indicator = SCALE(20 - 2*config.btn.layer);
#if 0
   dpi.ExclusiveIndicator = config.btn.layer ? SCALE(16) : SCALE(19);
#else
	dpi.ExclusiveIndicator = SCALE(17);
#endif
}

#undef SCALE

void
Style::init(const QSettings* settings)
{
    QTime time; time.start();
    // various workarounds... ==========================
    if (getenv("OPERA_LD_PRELOAD"))
        appType = Opera;
    else if (!qApp->inherits("KApplication") && getenv("GTK_QT_ENGINE_ACTIVE"))
        { appType = GTK; /*qWarning("BESPIN: Detected GKT+ application");*/ }
    else if (qApp->inherits("GreeterApp"))
        appType = KDM;
    else if (QCoreApplication::applicationName() == "dolphin")
        appType = Dolphin;
    else if (QCoreApplication::applicationName() == "plasma")
        appType = Plasma;
    else if (QCoreApplication::applicationName() == "krunner")
        appType = KRunner;
    else if (QCoreApplication::applicationName() == "kget")
        appType = KGet;
    else if (QCoreApplication::applicationName() == "Designer")
        appType = QtDesigner;
    else
        appType = Unknown;
    // ==========================
    readSettings(settings);
    initMetrics();
    generatePixmaps();
    Gradients::init(config.bg.mode > Scanlines ? (Gradients::BgMode)config.bg.mode : Gradients::BevelV,
                    config.bg.structure, _progressBase, config.bg.intensity, F(8));
    int f2 = F(2), f4 = F(4);
    QRect inner = QRect(0,0,100,100), outer = QRect(0,0,100,100);
    inner.adjust(f4,f4,-f4,-dpi.f1); outer.adjust(0,0,0,dpi.f3);
    VisualFrame::setGeometry(QFrame::Sunken, inner, outer);
    inner = QRect(0,0,100,100); outer = QRect(0,0,100,100);
    inner.adjust(f2,f2,-f2,-f2); outer.adjust(-f2,-f2,f2,f2);
    VisualFrame::setGeometry(QFrame::Plain, inner, outer);
    inner = QRect(0,0,100,100); outer = QRect(0,0,100,100);
    inner.adjust(f2,f2,-f2,0); outer.adjust(-f2,-f2,f2,0);
    VisualFrame::setGeometry(QFrame::Raised, inner, outer);
}
