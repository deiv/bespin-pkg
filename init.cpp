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
#include <QPainter>
#include <QProcess>
#include <QApplication>
#include <QTimer>

#include <cmath>

#include "animator/tab.h"
#include "blib/colors.h"
#include "blib/elements.h"
#include "bespin.h"
#include "hacks.h"
#include "visualframe.h"
#include "makros.h"
#include "config.defaults"

#include <QtDebug>

using namespace Bespin;

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

static int clamp(int x, int l, int u)
{
    return CLAMP(x,l,u);
}

static QStringList colors(const QPalette &pal, QPalette::ColorGroup group)
{
    QStringList list;
    for (int i = 0; i < QPalette::NColorRoles; i++)
        list << pal.color(group, (QPalette::ColorRole) i).name();
    return list;
}

#if 0
static int fontOffset()
{
    QString string = "qtipdfghjklöäyxbQWRTZIPÜSDFGHJKLÖÄYXVBN!()?ß\"";
    QFont font; QFontMetrics metrics(font);
    QImage img(metrics.size(0, string), QImage::Format_ARGB32);
    img.fill(0xffffffff);
    QPainter p(&img); p.setPen(Qt::black); p.drawText(img.rect(), Qt::AlignCenter, string); p.end();
    int result = 0;
    for (int y = 0; y < img.height(); ++y)
    {
        QRgb *scanLine = (QRgb*)img.scanLine(y);
        for (int x = 0; x < img.width(); ++x)
        {
            if (qRed(*scanLine++) < 128)
                { result = y << 16; goto descent; }
        }
        if ( result ) break;
    }
descent:
    for (int y = img.height(); y > 0; --y)
    {
        QRgb *scanLine = (QRgb*)img.scanLine(y-1);
        for (int x = 0; x < img.width(); ++x)
        {
            if (qRed(*scanLine++) < 128)
                return ( result | ( (metrics.height() - (y + 1) ) & 0xffff) );
        }
    }
    return result;
}
#endif

#define readInt(_DEF_) iSettings->value(_DEF_).toInt()
#define readBool(_DEF_) iSettings->value(_DEF_).toBool()
#define readRole(_VAR_, _DEF_)\
config._VAR_##_role[0] = (QPalette::ColorRole) iSettings->value(_DEF_).toInt();\
Colors::counterRole(config._VAR_##_role[0], config._VAR_##_role[1])
//, QPalette::_DEF_, Colors::counterRole(QPalette::_DEF_))
#define readGrad(_DEF_) (Gradients::Type) clamp(iSettings->value(_DEF_).toInt(), 0, Gradients::TypeAmount-1);

void
Style::removeAppEventFilter()
{
    // for plain qt apps that don't need the palette fix, otherwise we'd keep filtering the app for no reason
    qApp->removeEventFilter(this);
}

void
Style::readSettings(const QSettings* settings, QString appName)
{
    bool delSettings = false;

    QSettings *iSettings = const_cast<QSettings*>(settings);
    if (!iSettings)
    {
        delSettings = true;
        QString qPreset;
        
        iSettings = new QSettings("Bespin", "Style");
        iSettings->beginGroup("Style");
        //BEGIN read some user personal settings (i.e. not preset related)
        // flanders
        config.leftHanded = readBool(LEFTHANDED) ? Qt::RightToLeft : Qt::LeftToRight;
        if ((config.showOff = readBool(SHOW_OFF)))
            { ori[0] = Qt::Vertical; ori[1] = Qt::Horizontal; }
        else
            { ori[0] = Qt::Horizontal; ori[1] = Qt::Vertical; }
        config.shadowTitlebar = readBool(SHADOW_TITLEBAR);
        // item single vs. double click, wizard appereance
        config.macStyle = readBool(MACSTYLE);
        config.fadeInactive = readBool(FADE_INACTIVE);
        // Hacks ==================================
        Hacks::config.messages = readBool(HACK_MESSAGES);
        Hacks::config.KHTMLView = readBool(HACK_KHTMLVIEW);
        Hacks::config.krunner= readBool(HACK_KRUNNER);
        Hacks::config.treeViews = readBool(HACK_TREEVIEWS);
        Hacks::config.windowMovement = readBool(HACK_WINDOWMOVE);
        Hacks::config.killThrobber = readBool(HACK_THROBBER);
        Hacks::config.opaqueDolphinViews = appType == Dolphin && readBool(HACK_DOLPHIN_VIEWS);
        // PW Echo Char ===========================
        config.input.pwEchoChar = ushort(iSettings->value(INPUT_PWECHOCHAR).toUInt());
#if BESPIN_ARGB_WINDOWS
        config.menu.opacity = clamp(readInt(MENU_OPACITY), 0, 0xff);
#else
        config.menu.opacity = 0xff;
#endif
        config.menu.glassy = readBool(MENU_GLASSY);
        config.menu.showIcons = appType == Opera || readBool(MENU_SHOWICONS);
        config.menu.shadow = readBool(MENU_SHADOW);

        config.winBtnStyle = 2; // this is a kwin deco setting, TODO: read from there?
        
        Animator::Tab::setTransition((Animator::Transition) readInt(TAB_TRANSITION));
        Animator::Tab::setDuration(clamp(iSettings->value(TAB_DURATION).toUInt(), 150, 4000));
        //END personal settings
        //NOTICE we do not end group here, but below. this way it's open if we don't make use of presets
        
        // check for environment override
        const char *preset = getenv("BESPIN_PRESET");
        if (preset)
            qPreset = preset;
        else // but maybe there's a preset config'd for this app...
        {
            iSettings->endGroup();
            iSettings->beginGroup("PresetApps");
            QString cmd = QCoreApplication::applicationName();
            if (appType == KDM)
                cmd = "KDM"; // KDM segfaults on QCoreApplication::arguments()...
            else if (cmd.isEmpty() && !QCoreApplication::arguments().isEmpty())
                cmd = QCoreApplication::arguments().at(0).section('/', -1);
            if (!cmd.isEmpty())
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
            // must be for all apps - KDE does some freaky stuff between the qApp and KApp constructor...
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
    if (appType == Opera && config.bg.mode > Scanlines)
        config.bg.mode = Plain; // it doesn't work - at least atm - and breaks kwin appereance...
    else if (config.bg.mode > BevelH)
        config.bg.mode = BevelV;

    config.bg.intensity = clamp(100+readInt(BG_INTENSITY), 30, 300);
#if BESPIN_ARGB_WINDOWS
    if (appType == KWin || appType == Plasma)
        config.bg.opacity = 0xff;
    else
        config.bg.opacity = clamp(readInt(BG_OPACITY), 0, 0xff);
    if (config.bg.opacity != 0xff)
    {
        QStringList blacklist = iSettings->value(ARGB_BLACKLIST).toString().split(',', QString::SkipEmptyParts);
        if (blacklist.contains(appName))
            config.bg.opacity = 0xff;
        Animator::Tab::setTransition(Animator::Jump);
    }
#else
    config.bg.opacity = 0xff;
#endif
    if ((config.bg.glassy = readBool(ARGB_GLASSY)))
        config.bg.mode = Plain;

    config.bg.docks.invert = readBool(BG_DOCKS_INVERT);
    config.bg.docks.shape = config.bg.docks.invert ? readBool(BG_DOCKS_SHAPE) : false;

    config.bg.modal.glassy = readBool(BG_MODAL_GLASSY);
    config.bg.modal.opacity = readInt(BG_MODAL_OPACITY);
    if (config.bg.opacity)
        config.bg.modal.opacity = 255*config.bg.modal.opacity/config.bg.opacity;
    config.bg.modal.invert = (appType != KDM) && readBool(BG_MODAL_INVERT);

    config.bg.ringOverlay = readBool(BG_RING_OVERLAY);

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
    if (config.btn.layer == 2 && GRAD(btn) == Gradients::Sunken) // NO!
        GRAD(btn) = Gradients::None;

    config.btn.backLightHover = readBool(BTN_BACKLIGHTHOVER);
    config.btn.layer = clamp(readInt(BTN_LAYER), 0, 2);
    config.btn.fullHover = config.btn.backLightHover || readBool(BTN_FULLHOVER);
    config.btn.minHeight = readInt(BTN_MIN_HEIGHT);

    if (config.btn.layer == 2) config.btn.cushion = true;
    else if (GRAD(btn) ==  Gradients::Sunken) config.btn.cushion = false;
    else config.btn.cushion = readBool(BTN_CUSHION);

    readRole(btn.std, BTN_ROLE);
    readRole(btn.active, BTN_ACTIVEROLE);
    config.btn.ambientLight = readBool(BTN_AMBIENTLIGHT);
    config.btn.bevelEnds = readBool(BTN_BEVEL_ENDS);

    // .Tool
    config.btn.tool.connected = readBool(BTN_CONNECTED_TOOLS);
    config.btn.tool.disabledStyle = readInt(BTN_DISABLED_TOOLS);
    if (config.btn.tool.connected)
    {
        config.btn.tool.sunken = readBool(BTN_SUNKEN_TOOLS);
        readRole(btn.tool.std, BTN_TOOL_ROLE);
        readRole(btn.tool.active, BTN_TOOL_ACTIVEROLE);
        GRAD(btn.tool) = readGrad(BTN_TOOL_GRADIENT);
    }
    else
    {
        config.btn.tool.sunken = false;
        config.btn.tool.std_role[Bg] = config.btn.tool.active_role[Bg] = QPalette::Window;
        config.btn.tool.std_role[Fg] = config.btn.tool.active_role[Fg] = QPalette::WindowText;
    }
   
    // Choosers ===========================
    GRAD(chooser) = readGrad(CHOOSER_GRADIENT);


    // kwin - yes i let the style control the deco, iff the deco permits, though :)
    config.kwin.gradient[0] = readGrad(KWIN_INACTIVE_GRADIENT);
    config.kwin.gradient[1] = readGrad(KWIN_ACTIVE_GRADIENT);
    readRole(kwin.inactive, KWIN_INACTIVE_ROLE);
    readRole(kwin.active, KWIN_ACTIVE_ROLE);
    config.kwin.text_role[0] = (QPalette::ColorRole) iSettings->value(KWIN_INACTIVE_TEXT_ROLE).toInt();
    config.kwin.text_role[1] = (QPalette::ColorRole) iSettings->value(KWIN_ACTIVE_TEXT_ROLE).toInt();

    // Menus ===========================
    //--------
    config.menu.round = readBool(MENU_ROUND);
    config.menu.itemGradient = readGrad(MENU_ITEMGRADIENT);
    config.menu.roundSelect = iSettings->value("Menu.RoundSelect",
                                              !(config.menu.itemGradient == Gradients::Glass ||
                                              config.menu.itemGradient == Gradients::Gloss)).toBool();
    
    if (appType == GTK)
    {
        config.menu.glassy = false;
        config.menu.std_role[Bg] = QPalette::Window;
        config.menu.std_role[Fg] = QPalette::WindowText;
        config.menu.active_role[Bg] = QPalette::Highlight;
        config.menu.active_role[Fg] = QPalette::HighlightedText;
        // both do not work :-( - also the user has to choose a bg that usually fits window fg
        config.UNO.used = false;
        config.UNO.sunken = false;
    }
    else
    {
        config.UNO.used = readBool(UNO_UNO);
        readRole(menu.active, MENU_ACTIVEROLE);
        readRole(menu.std, MENU_ROLE);
    }

    if (appType == Plasma || appType == BEshell)
        config.UNO.used = false; // that's probably XBar, ...
    
    if (config.UNO.used)
    {
        config.UNO.sunken = readBool(UNO_SUNKEN);
        config.UNO.toolbar = readBool(UNO_TOOLBAR);
        config.UNO.title = readBool(UNO_TITLE);
        config.UNO.gradient = readGrad(UNO_GRADIENT);
        readRole(UNO._, UNO_ROLE);
    }
    else
    {
        config.UNO.sunken = false;
        config.UNO.toolbar = false;
        config.UNO.title = false;
        config.UNO.gradient = Gradients::None;
        config.UNO.__role[Bg] = QPalette::Window;
        config.UNO.__role[Fg] = QPalette::WindowText;
    }

    config.menu.boldText = readBool(MENU_BOLDTEXT);
    config.menu.itemSunken = readBool(MENU_ITEM_SUNKEN);
    config.menu.activeItemSunken = config.menu.itemSunken || readBool(MENU_ACTIVEITEMSUNKEN);

    if (readBool(SHOW_MNEMONIC))
        config.mnemonic = Qt::TextShowMnemonic;
    else
        config.mnemonic = Qt::TextHideMnemonic;

    // Progress ===========================
    GRAD(progress) = readGrad(PROGRESS_GRADIENT);
    config.progress.__role[Bg] =  (QPalette::ColorRole) readInt(PROGRESS_ROLE_BG);
    config.progress.__role[Fg] = (QPalette::ColorRole) readInt(PROGRESS_ROLE_FG);

    // ScrollStuff ===========================
    GRAD(scroll) = readGrad(SCROLL_GRADIENT);
    config.scroll.showButtons = readBool(SCROLL_SHOWBUTTONS);
    config.scroll.fatSlider = !readBool(SCROLL_SLIM_SLIDER);
    config.scroll.groove = (Groove::Mode) readInt(SCROLL_GROOVE);
    config.scroll.invertBg = readBool(SCROLL_INVERT_BG);
    // this MUST happen after reading the button role, which it will default to!
    config.scroll.__role[Bg] = (QPalette::ColorRole) readInt(SCROLL_ROLE);
    config.scroll.__role[Fg] = (QPalette::ColorRole) readInt(SCROLL_ACTIVEROLE);
//     config.scroll.std_role[Bg] = config.btn.std_role[Bg];
//     config.scroll.std_role[Fg] = config.btn.std_role[Fg];
//     config.scroll.active_role[Bg] = config.btn.active_role[Bg];
//     config.scroll.active_role[Fg] = config.btn.active_role[Fg];

    // Tabs ===========================
    readRole(tab.active, TAB_ACTIVEROLE);
    Animator::Tab::setFPS(25);
    GRAD(tab) = readGrad(TAB_GRADIENT);
    readRole(tab.std, TAB_ROLE);
    config.tab.activeTabSunken = readBool(TAB_ACTIVETABSUNKEN);
//     GRAD(toolbox) = readGrad(TAB_ACTIVEGRADIENT);

    // Views ===========================
    readRole(view.header, VIEW_HEADERROLE);
    readRole(view.sortingHeader, VIEW_SORTINGHEADERROLE);
    config.view.headerGradient = readGrad(VIEW_HEADERGRADIENT);
    config.view.sortingHeaderGradient = readGrad(VIEW_SORTINGHEADERGRADIENT);
    config.view.shadeLevel = readInt(VIEW_SHADE_LEVEL);
    config.view.shadeRole = (QPalette::ColorRole) iSettings->value(VIEW_SHADE_ROLE).toInt();

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
    config.groupBoxMode = readInt(GROUP_BOX_MODE);

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

        Hacks::config.messages = false;

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
   Dpi::target.f1 = SCALE(1); Dpi::target.f2 = SCALE(2);
   Dpi::target.f3 = SCALE(3); Dpi::target.f4 = SCALE(4);
   Dpi::target.f5 = SCALE(5); Dpi::target.f6 = SCALE(6);
   Dpi::target.f7 = SCALE(7); Dpi::target.f8 = SCALE(8);
   Dpi::target.f9 = SCALE(9); Dpi::target.f10 =SCALE(10);
   
   Dpi::target.f12 = SCALE(12); Dpi::target.f13 = SCALE(13);
   Dpi::target.f16 = SCALE(16); Dpi::target.f18 = SCALE(18);
   Dpi::target.f20 = SCALE(20); Dpi::target.f32 = SCALE(32);
   Dpi::target.f80 = SCALE(80);
   
   Dpi::target.ScrollBarExtent = SCALE((config.scroll.groove > Groove::Groove ? 15 : 17) - config.btn.fullHover - 2*!config.scroll.fatSlider);
   Dpi::target.ScrollBarSliderMin = SCALE(40);
   Dpi::target.SliderThickness = SCALE(20);
   Dpi::target.SliderControl = SCALE(20);
   Dpi::target.Indicator = SCALE(20 - 2*config.btn.layer);
#if 0
   Dpi::target.ExclusiveIndicator = config.btn.layer ? SCALE(16) : SCALE(19);
#else
	Dpi::target.ExclusiveIndicator = SCALE(17);
#endif
}

#undef SCALE

void
Style::init(const QSettings* settings)
{
    QTime time; time.start();
    // various workarounds... ==========================
    appType = Unknown;
    QString appName;
    if (getenv("OPERA_LD_PRELOAD"))
        appType = Opera;
    else if (!qApp->inherits("KApplication") && getenv("GTK_QT_ENGINE_ACTIVE"))
        { appType = GTK; /*qWarning("BESPIN: Detected GKT+ application");*/ }
    else if (qApp->inherits("GreeterApp"))
        appType = KDM;
    else
    {
        appName = QCoreApplication::applicationName();
        if (appName == "dolphin")
            appType = Dolphin;
        if (appName == "konversation")
            appType = Konversation;
        else if (appName == "be.shell")
            appType = BEshell;
        else if (appName == "plasma" || appName.startsWith("plasma-"))
            appType = Plasma;
        else if (appName == "krunner")
            appType = KRunner;
        else if (appName == "kget")
            appType = KGet;
        else if (appName == "ktorrent")
            appType = KTorrent;
        else if (appName == "Designer")
            appType = QtDesigner;
        else if (appName == "kdevelop")
            appType = KDevelop;
        else if (appName == "kwin")
            appType = KWin;
        else if (appName == "amarok")
            appType = Amarok;
        else if (appName.isEmpty() && !QCoreApplication::arguments().isEmpty())
        {
            appName = QCoreApplication::arguments().at(0).section('/', -1);
            if (appName == "designer")
                appType = QtDesigner;
//             if (appName == "arora")
//                 appType = Arora;
        }
    }
    // ==========================
    readSettings(settings, appName);
    initMetrics();
    Elements::setScale(config.scale);
    generatePixmaps();
    Gradients::init(config.bg.mode > Scanlines ? (Gradients::BgMode)config.bg.mode : Gradients::BevelV,
                    config.bg.structure, config.bg.intensity, F(8), false, config.groupBoxMode == 2);
    int f1 = F(1), f3 = F(3), f4 = F(4);
    QRect inner = QRect(0,0,100,100), outer = QRect(0,0,100,100);
    inner.adjust(f4,f4,-f4,-f1); outer.adjust(0,0,0,F(2));
    VisualFrame::setGeometry(QFrame::Sunken, inner, outer);
    inner.setRect(0,0,100,100); outer.setRect(0,0,100,100);
    inner.adjust(1,1,-1,-1); outer.adjust(-1,-1,1,1);
    VisualFrame::setGeometry(QFrame::Plain, inner, outer);
    inner.setRect(0,0,100,100); outer.setRect(0,0,100,100);
    inner.adjust(f1,f1,-f1,0); outer.adjust(-f3,-f3,f3,0);
    VisualFrame::setGeometry(QFrame::Raised, inner, outer);
}
