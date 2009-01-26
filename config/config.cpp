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

#include <QApplication>
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QTimer>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QValidator>

#include "../gradients.h"
#include "kdeini.h"
#include "../config.defaults"
#include "config.h"

typedef QMap<QString,QString> StringMap;

/** This function declares the kstyle config plugin, you may need to adjust it
for other plugins or won't need it at all, if you're not interested in a plugin */
extern "C"
{
   Q_DECL_EXPORT QWidget* allocate_kstyle_config(QWidget* parent)
   {
      /**Create our config dialog and reply it as plugin
      This is slightly different from the setup in a standalone dialog at the
      bottom of this file*/
      return new Config(parent);
   }
}

/** Gradient enumeration for the comboboxes, so that i don't have to handle the
integers - not of interest for you*/
enum GradientType {
   GradNone = 0, GradSimple, GradButton, GradSunken, GradGloss,
      GradGlass, GradMetal, GradCloud
};

using namespace Bespin;

static const char* defInfo1 =
"<b>Bespin Style</b><hr>\
<p>\
&copy;&nbsp;2006-2009 by Thomas L&uuml;bking<br>\
Includes Design Ideas by\
<ul type=\"disc\">\
<li>Nuno Pinheiro</li>\
<li>David Vignoni</li>\
<li>Kenneth Wimer</li>\
</ul>\
</p>\
<hr>\
Visit <a href=\"http://cloudcity.sourceforge.net\">CloudCity.SourceForge.Net</a>";

static const char* defInfo2 =
"<div align=\"center\">\
<img src=\":/bespin.png\"/><br>\
<a href=\"http://cloudcity.sourceforge.net\">CloudCity.SourceForge.Net</a>\
</div>";


static const char* defInfo3 =
"<div align=\"center\">\
<img src=\":/bespin.png\"/><br>\
<a href=\"http://cloudcity.sourceforge.net\">CloudCity.SourceForge.Net</a>\
</div>\
<h2>Warning!</h2>\
<p>\
Activating these hacks...\
</p>\
";

/** Intenal class for the PW Char entry - not of interest */

static ushort unicode(const QString &string) {
   if (string.length() == 1)
      return string.at(0).unicode();
   uint n = string.toUShort();
   if (!n)
      n = string.toUShort(0,16);
   if (!n)
      n = string.toUShort(0,8);
   return n;
}

class UniCharValidator : public QValidator {
public:
   UniCharValidator( QObject * parent ) : QValidator(parent){}
   virtual State validate ( QString & input, int & ) const {
      if (input.length() == 0)
         return Intermediate;
      if (input.length() == 1)
         return Acceptable;
      if (input.length() == 2 && input.at(0) == '0' && input.at(1).toLower() == 'x')
         return Intermediate;
      if (unicode(input))
         return Acceptable;
      return Invalid;
   }
};

/** The Constructor - your first job! */
Config::Config(QWidget *parent) : BConfig(parent), loadedPal(0), infoIsManage(false)
{
    /** Setup the UI and geometry */
    ui.setupUi(this);

    /** Some special stuff */
    QEvent event(QEvent::PaletteChange);
    changeEvent(&event);
    ui.info->setOpenExternalLinks( true ); /** i've an internet link here */

    const QPalette::ColorGroup groups[3] = { QPalette::Active, QPalette::Inactive, QPalette::Disabled };
    ui.info->viewport()->setAutoFillBackground(false);
    QPalette pal = ui.info->palette();
    for (int i = 0; i < 3; ++i)
    {
        pal.setColor(groups[i], QPalette::Base, pal.color(groups[i], QPalette::Window));
        pal.setColor(groups[i], QPalette::Text, pal.color(groups[i], QPalette::WindowText));
    }
    ui.info->setPalette(pal);

    ui.logo->setMargin(9);

    ui.sectionSelect->viewport()->setAutoFillBackground(false);
    pal = ui.sectionSelect->palette();
    for (int i = 0; i < 3; ++i)
    {
        pal.setColor(groups[i], QPalette::Base, pal.color(groups[i], QPalette::Window));
        pal.setColor(groups[i], QPalette::Text, pal.color(groups[i], QPalette::WindowText));
    }
    ui.sectionSelect->setPalette(pal);
    
    connect (ui.sectionSelect, SIGNAL(currentRowChanged(int)),
                ui.sections, SLOT(setCurrentIndex(int)));
    connect (ui.sectionSelect, SIGNAL(currentTextChanged(const QString &)),
                this, SLOT(setHeader(const QString &)));

    /** Prepare the settings store, not of interest */
    StringMap presetMap;
    QSettings settings("Bespin", "Store");
    QString preset;
    foreach (preset, settings.childGroups())
        presetMap.insert(preset, "");
    QSettings settings2("Bespin", "Style");
    settings2.beginGroup("PresetApps");
    StringMap::iterator entry;
    foreach (QString appName, settings2.childKeys())
    {
        preset = settings2.value(appName, QString()).toString();
        entry = presetMap.find(preset);
        if (entry != presetMap.end())
        {
            if (!entry.value().isEmpty())
                entry.value() += ", ";
            entry.value() += appName;
        }
    }
    QTreeWidgetItem *item;
    for (entry = presetMap.begin(); entry != presetMap.end(); ++entry)
    {
        item = new QTreeWidgetItem(QStringList() << entry.key() << entry.value());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        ui.store->addTopLevelItem(item);
    }
//    ui.store->addItems( settings.childGroups() );
    ui.store->sortItems(0, Qt::AscendingOrder);
    ui.btnStore->setAutoDefault ( false );
    ui.btnRestore->setAutoDefault ( false );
    ui.btnImport->setAutoDefault ( false );
    ui.btnExport->setAutoDefault ( false );
    ui.btnDelete->setAutoDefault ( false );
    connect (ui.store, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(presetAppsChanged(QTreeWidgetItem *, int)));
    connect (ui.btnStore, SIGNAL(clicked()), this, SLOT(store()));
    connect (ui.btnRestore, SIGNAL(clicked()), this, SLOT(restore()));
    connect (ui.store, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
                this, SLOT(restore(QTreeWidgetItem*, int)));
    connect (ui.btnImport, SIGNAL(clicked()), this, SLOT(import()));
    connect (ui.btnExport, SIGNAL(clicked()), this, SLOT(saveAs()));
    connect (ui.store, SIGNAL(currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem *)),
                this, SLOT(storedSettigSelected(QTreeWidgetItem *)));
    connect (ui.btnDelete, SIGNAL(clicked()), this, SLOT(remove()));
    connect (ui.presetFilter, SIGNAL(textChanged (const QString &)), this, SLOT(filterPresets(const QString &)));
    ui.btnRestore->setEnabled(false);
    ui.btnExport->setEnabled(false);
    ui.btnDelete->setEnabled(false);
    ui.storeLine->hide();
   
    /** fill some comboboxes, not of interest */
    generateColorModes(ui.tooltipRole);
    generateColorModes(ui.crProgressBg);
    generateColorModes(ui.crProgressFg);
    generateColorModes(ui.crTabBar);
    generateColorModes(ui.crTabBarActive);
    generateColorModes(ui.crPopup);
    generateColorModes(ui.crMenuActive);
    generateColorModes(ui.btnRole);
    generateColorModes(ui.btnActiveRole);
    generateColorModes(ui.headerRole);
    generateColorModes(ui.headerSortingRole);
    QList<int> roles; roles << 3 << 4 << 6;
    generateColorModes(ui.viewShadingRole, &roles);
    
    generateColorModes(ui.crMenu);
    generateColorModes(ui.kwinInactiveRole);
    generateColorModes(ui.kwinActiveRole);
    generateColorModes(ui.kwinInactiveText);
    generateColorModes(ui.kwinActiveText);
   
    generateGradientTypes(ui.gradButton);
    generateGradientTypes(ui.gradChoose);
    generateGradientTypes(ui.gradMenuBar);
    generateGradientTypes(ui.gradMenuItem);
    generateGradientTypes(ui.gradProgress);
    generateGradientTypes(ui.gradTab);
    generateGradientTypes(ui.gradScroll);
    generateGradientTypes(ui.headerGradient);
    generateGradientTypes(ui.headerSortingGradient);
    generateGradientTypes(ui.kwinInactiveGrad);
    generateGradientTypes(ui.kwinActiveGrad);
   
    QSettings csettings("Bespin", "Config");
    QStringList strList = csettings.value ( "UserPwChars", QStringList() ).toStringList();
    ushort n;
    foreach (QString str, strList)
    {
        n = str.toUShort(0,16);
        if (n)
            ui.pwEchoChar->addItem(QChar(n), n);
    }
    strList.clear();
    ui.pwEchoChar->addItem(QChar(0x26AB), 0x26AB);
    ui.pwEchoChar->addItem(QChar(0x2022), 0x2022);
    ui.pwEchoChar->addItem(QChar(0x2055), 0x2055);
    ui.pwEchoChar->addItem(QChar(0x220E), 0x220E);
    ui.pwEchoChar->addItem(QChar(0x224E), 0x224E);
    ui.pwEchoChar->addItem(QChar(0x25AA), 0x25AA);
    ui.pwEchoChar->addItem(QChar(0x25AC), 0x25AC);
    ui.pwEchoChar->addItem(QChar(0x25AC), 0x25AC);
    ui.pwEchoChar->addItem(QChar(0x25A0), 0x25A0);
    ui.pwEchoChar->addItem(QChar(0x25CF), 0x25CF);
    ui.pwEchoChar->addItem(QChar(0x2605), 0x2605);
    ui.pwEchoChar->addItem(QChar(0x2613), 0x2613);
    ui.pwEchoChar->addItem(QChar(0x26A1), 0x26A1);
    ui.pwEchoChar->addItem(QChar(0x2717), 0x2717);
    ui.pwEchoChar->addItem(QChar(0x2726), 0x2726);
    ui.pwEchoChar->addItem(QChar(0x2756), 0x2756);
    ui.pwEchoChar->addItem(QChar(0x2756), 0x2756);
    ui.pwEchoChar->addItem(QChar(0x27A4), 0x27A4);
    ui.pwEchoChar->addItem(QChar(0xa4), 0xa4);
    ui.pwEchoChar->addItem("|", '|');
    ui.pwEchoChar->addItem(":", ':');
    ui.pwEchoChar->addItem("*", '*');
    ui.pwEchoChar->addItem("#", '#');
    ui.pwEchoChar->lineEdit()-> setValidator(new UniCharValidator(ui.pwEchoChar->lineEdit()));
    connect (ui.pwEchoChar->lineEdit(), SIGNAL(returnPressed()), this, SLOT (learnPwChar()));
    ui.pwEchoChar->setInsertPolicy(QComboBox::NoInsert);
   
   
    /** connection between the bgmode and the structure combo - not of interest*/
    connect(ui.bgMode, SIGNAL(currentIndexChanged(int)), this, SLOT(handleBgMode(int)));
    connect(ui.sliderGroove, SIGNAL(valueChanged(int)), this, SLOT(handleGrooveMode(int)));
   
    /** 1. name the info browser, you'll need it to show up context help
    Can be any QTextBrowser on your UI form */
    setInfoBrowser(ui.info);
    /** 2. Define a context info that is displayed when no other context help is demanded */
    setDefaultContextInfo(defInfo1);
    
    /** handleSettings(.) tells BConfig to take care (save/load) of a widget
    In this case "ui.bgMode" is the widget on the form,
    "BackgroundMode" specifies the entry in the ini style config file and
    "3" is the default value for this entry*/
    handleSettings(ui.bgMode, BG_MODE);
    handleSettings(ui.bgIntensity, BG_INTENSITY);
    handleSettings(ui.fadeInactive, FADE_INACTIVE);
    handleSettings(ui.structure, BG_STRUCTURE);
    handleSettings(ui.modalGlas, BG_MODAL_GLASSY);
    handleSettings(ui.modalOpacity, BG_MODAL_OPACITY);
    handleSettings(ui.modalInvert, BG_MODAL_INVERT);
    handleSettings(ui.tooltipRole, BG_TOOLTIP_ROLE);
    handleSettings(ui.sunkenGroups, SUNKEN_GROUPS);

    handleSettings(ui.sunkenButtons, "Btn.Layer", 0);
    handleSettings(ui.checkMark, "Btn.CheckType", 0);
    handleSettings(ui.cushion, "Btn.Cushion", true);
    handleSettings(ui.fullButtonHover, "Btn.FullHover", true);
    handleSettings(ui.gradButton, "Btn.Gradient", GradButton);
    handleSettings(ui.btnRole, "Btn.Role", QPalette::Window);
    handleSettings(ui.btnActiveRole, "Btn.ActiveRole", QPalette::Button);
    handleSettings(ui.ambientLight, "Btn.AmbientLight", true);
    handleSettings(ui.backlightHover, "Btn.BacklightHover", false);
    handleSettings(ui.btnRound, "Btn.Round", false);
    handleSettings(ui.btnBevelEnds, BTN_BEVEL_ENDS);
   
    handleSettings(ui.gradChoose, "Chooser.Gradient", GradSunken);

    handleSettings(ui.pwEchoChar, "Input.PwEchoChar", 0x26AB);

    handleSettings(ui.leftHanded, "LeftHanded", false);
    handleSettings(ui.macStyle, "MacStyle", true);

    handleSettings(ui.crMenuActive, MENU_ACTIVEROLE);
    handleSettings(ui.menuRound, MENU_ROUND);
    handleSettings(ui.menuGlas, MENU_GLASSY);
    handleSettings(ui.gradMenuItem, MENU_ITEMGRADIENT);
    handleSettings(ui.showMenuIcons, MENU_SHOWICONS);
    handleSettings(ui.menuShadow, MENU_SHADOW); // false, i have a compmgr running :P
    handleSettings(ui.menuOpacity, MENU_OPACITY);
    handleSettings(ui.crPopup, MENU_ROLE);
    handleSettings(ui.gradMenuBar, MENU_BAR_GRADIENT);
    handleSettings(ui.crMenu, MENU_BARROLE);
    handleSettings(ui.barSunken, MENU_BARSUNKEN);
    handleSettings(ui.menuBoldText, MENU_BOLDTEXT);
    handleSettings(ui.menuActiveItemSunken, MENU_ACTIVEITEMSUNKEN);

    handleSettings(ui.gradProgress, "Progress.Gradient", GradGloss);
    handleSettings(ui.crProgressBg, PROGRESS_ROLE_BG);
    handleSettings(ui.crProgressFg, PROGRESS_ROLE_FG);

    handleSettings(ui.showScrollButtons, "Scroll.ShowButtons", false);
    handleSettings(ui.sliderGroove, "Scroll.Groove", false);
    handleSettings(ui.gradScroll, "Scroll.Gradient", GradButton);
    handleSettings(ui.invertGroove, SCROLL_INVERT_BG);

    handleSettings(ui.shadowIntensity, "ShadowIntensity", 100);
   
    handleSettings(ui.crTabBarActive, "Tab.ActiveRole", QPalette::Highlight);
    handleSettings(ui.tabAnimDuration, TAB_DURATION);
    handleSettings(ui.gradTab, "Tab.Gradient", GradButton);
    handleSettings(ui.crTabBar, "Tab.Role", QPalette::Window);
    handleSettings(ui.tabTransition, TAB_TRANSITION);
    handleSettings(ui.activeTabSunken, "Tab.ActiveTabSunken", false);

    handleSettings(ui.headerRole, VIEW_HEADERROLE);
    handleSettings(ui.headerSortingRole, VIEW_SORTINGHEADERROLE);
    handleSettings(ui.headerGradient, VIEW_HEADERGRADIENT);
    handleSettings(ui.headerSortingGradient, VIEW_SORTINGHEADERGRADIENT);
    handleSettings(ui.viewShadingRole, VIEW_SHADE_ROLE);
    handleSettings(ui.viewShadeLevel, VIEW_SHADE_LEVEL);

    handleSettings(ui.kwinActiveGrad, KWIN_ACTIVE_GRADIENT);
    handleSettings(ui.kwinInactiveGrad, KWIN_INACTIVE_GRADIENT);
    handleSettings(ui.kwinActiveRole, KWIN_ACTIVE_ROLE);
    handleSettings(ui.kwinInactiveRole, KWIN_INACTIVE_ROLE);
    handleSettings(ui.kwinActiveText, KWIN_ACTIVE_TEXT_ROLE);
    handleSettings(ui.kwinInactiveText, KWIN_INACTIVE_TEXT_ROLE);

    handleSettings(ui.hackMessages, HACK_MESSAGES);
    setContextHelp(ui.btnRole, "<b>Messageboxes</b><hr>\
    Overwrites the painting routines of QMessageBoxes for a custom appereance.<br>\
    Also removes the Window decoration but allows you to drag around the window by\
    clicking anywhere.");

    handleSettings(ui.hackKrunner, HACK_KRUNNER);
    setContextHelp(ui.btnRole, "<b>KRunner</b><hr>\
    You'll get a window colored glass look and flat buttons.<br>");

    handleSettings(ui.hackKHtml, HACK_KHTMLVIEW);
    setContextHelp(ui.hackKHtml, "<b>Konqueror HTML window</b><hr>\
    By default, Konquerors HTML view has no frame around, but you may force a sunken\
    frame here.<br>\
    Notice that this may have a bad impact on scroll performance, especially if you\
    lack HW alpha blending.");

    handleSettings(ui.hackWindowMove, HACK_WINDOWMOVE);
    setContextHelp(ui.hackWindowMove, "<b>Easy Window Draging</b><hr>\
    Usually you'll have to hit the titlebar in order to drag around a window.<br>\
    This one allows you to drag the window by clicking many empty spaces.<br>\
    Currently supported items:<br>\
    - Dialogs<br>\
    - Menubars<br>\
    - Toolbars (including disabled buttons)<br>\
    - Docks<br>\
    - Groupboxers<br>\
    - Mainwindows<br>\
    - Statusbars<br>\
    - SMPlayer/DragonPlayer Video areas<br>");

    handleSettings(ui.killThrobber, HACK_THROBBER);
    setContextHelp(ui.killThrobber, "<b>Kill JarJar, err... Throbber</b><hr>\
    You see the nasty rotating thing in the top right of konqueror, now even oversizing the menubar?\
    Click here and you won't anymore >-P" );
    
    handleSettings(ui.hackTreeViews, HACK_TREEVIEWS);
    setContextHelp(ui.hackTreeViews, "<b>Animate TreeViwes</b><hr>\
    This is a plain vanilla Qt feature, but must be activated by developers for each\
    treeview in order to be used - can be cute, can be annoying: choose by yourself<br>\
    This way it's activated globally." );

    handleSettings(ui.hackAmarokContext, HACK_AMAROK_CONTEXT);
    setContextHelp(ui.hackAmarokContext, "<b>Hide Amarok's Context</b><hr>\
    Did i mention that i don't get a resonable internet connection ;-)\
    Anyway. You should be able to toggle this as an Amarok feature (QDockWidget instead QSplitter\
    would imho be a good idea) but it seems as if this layout is gonna sty hardcoded for a while.\
    Atm it is <b><i>not</i></b> possible to change this dynamically while amarok is running, but\
    you can toggle it here and the next time you startup Amarok, the Context will be gone.\
    (I.e. to reshow, you must quit Amarok, toggle it off and restart Amarok afterwards - i'll look\
    for a smarter solution)" );

    handleSettings(ui.hackAmarokFrames, HACK_AMAROK_FRAMES);
    setContextHelp(ui.hackAmarokFrames, "<b>Unframe Amarok</b><hr>\
    Amarok uses lists with a window background. Therefore it might be resonable to not use sunken\
    frames on them. Choose yourself." );

    handleSettings(ui.hackAmarokDisplay, HACK_AMAROK_DISPLAY);
    setContextHelp(ui.hackAmarokDisplay, "<b>Bespinification ;-P</b><hr>\
    You get a system frame, inverted colors, system sliders and the current track above the position\
    slider. Also a button to toggle the context view (plasma thing in the middle).<hr>\
    <b>RANT:</b><br>\
    Amarok uses overheaded svg theming, but (though possible) custom theming isn't wanted, ending up\
    with the UI halfwise themed by the system style and a static svg theme.<br>\
    Allthough many ppl. mourned you shall not be allowed to redesign the UI (simple using QDockWIdget\
    - like dolphis does) or even toggle elements (many ppl. seem to have low interest in a permanent\
    plasma center and handling collapsed splitters isn't too much fun either).<br>\
    I like Amarok alot. I don't even mind the debatable new playlist look - especially as it's sayd to\
    be configurable, pretty much like KMail currently is.<br>\
    BUT THIS SUCKS!<br>\
    I do not want an overheaded alien application where i'm told what UI elements i need where - Sorry :(" );

    /** setContextHelp(.) attaches a context help string to a widget on your form */
    setContextHelp(ui.btnRole, "<b>Button Colors</b><hr>\
    The default and the hovered color of a button.<br>\
    <b>Notice:</b> It's strongly suggested to select \"Button\" to\
    (at least and best excatly) one of the states!");

    setContextHelp(ui.btnActiveRole, "<b>Button Colors</b><hr>\
    The default and the hovered color of a button.<br>\
    <b>Notice:</b> It's strongly suggested to select \"Button\" to\
    (at least and best excatly) one of the states!");

    strList <<
        "<b>Plain (color)</b><hr>Select if you have a really lousy \
        machine or just hate structured backgrounds." <<

        "<b>Scanlines</b><hr>Wanna Aqua feeling?" <<

        "<b>Vertical Top/Bottom Gradient</b><hr>Simple gradient that brightens \
        on the upper and darkens on the lower end<br>(cheap, fallback suggestion 1)" <<

        "<b>Horizontal Left/Right Gradient</b><hr>Simple gradient that darkens \
        on left and right side." <<

        "<b>Vertical Center Gradient</b><hr>The window vertically brightens \
        to the center" <<

        "<b>Horizontal Center Gradient</b><hr>The window horizontally brightens \
        to the center (similar to Apples Brushed Metal, less cheap, \
        fallback suggestion 2)";
   
    /** if you call setContextHelp(.) with a combobox and pass a stringlist,
    the strings are attached to the combo entries and shown on select/hover */
    setContextHelp(ui.bgMode, strList);
    strList.clear();

    strList <<
        "<b>Jump</b><hr>No transition at all - fastest but looks stupid" <<

        "<b>ScanlineBlend</b><hr>Similar to CrossFade, but won't require \
        Hardware acceleration." <<

        "<b>SlideIn</b><hr>The new tab falls down from top" <<

        "<b>SlideOut</b><hr>The new tab rises from bottom" <<

        "<b>RollIn</b><hr>The new tab appears from Top/Bottom to center" <<

        "<b>RollOut</b><hr>The new tab appears from Center to Top/Bottom" <<

        "<b>OpenVertically</b><hr>The <b>old</b> Tab slides <b>out</b> \
        to Top and Bottom" <<

        "<b>CloseVertically</b><hr>The <b>new</b> Tab slides <b>in</b> \
        from Top and Bottom" <<

        "<b>OpenHorizontally</b><hr>The <b>old</b> Tab slides <b>out</b> \
        to Left and Right" <<

        "<b>CloseHorizontally</b><hr>The <b>new</b> Tab slides <b>in</b> \
        from Left and Right" <<

        "<b>CrossFade</b><hr>What you would expect - one fades out while the \
        other fades in.<br>\
        This is CPU hungry - better have GPU Hardware acceleration.";

    setContextHelp(ui.tabTransition, strList);

    setContextHelp(ui.store, "<b>Settings management</b><hr>\
                    <p>\
                    You can save your current settings (including colors from qconfig!) and\
                    restore them later here.\
                    </p><p>\
                    It's also possible to im and export settings from external files and share\
                    them with others.\
                    </p><p>\
                    You can also call the config dialog with the paramater \"demo\"\
                    <p><b>\
                    bespin demo [some style]\
                    </b></p>\
                    to test your settings on various widgets.\
                    </p><p>\
                    If you want to test settings before importing them, call\
                    <p><b>\
                    bespin try &lt;some_settings.conf&gt;\
                    </b></p>\
                    Installed presets can be referred by the BESPIN_PRESET environment variable\
                    <p><b>\
                    BESPIN_PRESET=\"Bos Taurus\" bespin demo\
                    </b></p>\
                    will run the Bespin demo widget with the \"Bos Taurus\" preset\
                    (this works - of course - with every Qt4 application,\
                    not only the bespin executable)\
                    </p>");
   
    setContextHelp(ui.pwEchoChar, "<b>Pasword Echo Character</b><hr>\
                    The character that is displayed instead of the real text when\
                    you enter e.g. a password.<br>\
                    You can enter any char or unicode number here.\
                    <b>Notice:</b> That not all fontsets may provide all unicode characters!");

    setContextHelp(ui.tabAnimDuration, "<b>Tab Transition Duration</b><hr>\
                    How long the transition lasts.");


    setContextHelp(ui.cushion, "<b>Cushion Mode</b><hr>\
                    By default, the buttons are kinda solid and will move towards\
                    the background when pressed.<br>\
                    If you check this, you'll get a more cushion kind look, i.e.\
                    the Button will be \"pressed in\"");

    setContextHelp(ui.ambientLight, "<b>Ambient Lightning</b><hr>\
                    Whether to paint a light reflex on the bottom right corner of\
                    Pushbuttons.<br>\
                    You can turn this off for artistic reasons or on bright color\
                    settings (to save some CPU cycles)");

    setContextHelp(ui.showScrollButtons, "<b>Show Scrollbar buttons</b><hr>\
                    Seriously, honestly: when do you ever use the buttons to move\
                    a scrollbar slider? (ok, notebooks don't have a mousewheel...)");

    setContextHelp(ui.crPopup, "<b>Popup Menu Role</b><hr>\
                    Choose anything you like (hint: saturated colors annoy me :)<br>\
                    The Text color is chosen automatically<br>\
                    Selected items are like selected menubar items if you choose \"Window\" here,\
                    otherwise they appear inverted");

    setContextHelp(ui.crMenuActive, "<b>Selected Menubar Item Role</b><hr>\
                    You may choose any role here<br>\
                    Select \"WindowText\" if you want inversion.<br>\
                    <b>Warning!</b><br>If you select \"Window\" here and \"None\" \
                    below, the hovering is hardly indicated!");

    setContextHelp(ui.menuBoldText, "<b>Rounded popup corners</b><hr>\
                    Round popup corners are nifty, but lame (Buhhh ;-P) compiz won't draw shadows\
                    then. So you can check them off here. (The proper solution to your problem was\
                    of course to use KWin... You GNOME guy! We hates thy GNOME guys. Really much =D");

    setContextHelp(ui.menuBoldText, "<b>Bold Menu Text</b><hr>\
                    Depending on your font this can be a good choice especially \
                    for bright text on dark backgrounds.");

    setContextHelp(ui.crTabBar, "<b>Tabbar Role</b><hr>\
                    The color of the tabbar background<br>\
                    The Text color is chosen automatically");

    setContextHelp(ui.crTabBarActive, "<b>Tabbar Active Item Role</b><hr>\
                    The color of the hovered or selected tab<br>\
                    The Text color is chosen automatically");

    setContextHelp(ui.fullButtonHover, "<b>Fully filled hovered buttons</b><hr>\
                    This is especially a good idea if the contrast between the\
                    button and Window color is low and also looks ok with Glass/Gloss\
                    gradient settings - but may be toggled whenever you want");

    setContextHelp(ui.leftHanded, "<b>Are you a Flanders?</b><hr>\
                    Some people (\"Lefties\") prefer a reverse orientation of eg.\
                    Comboboxes or Spinboxes.<br>\
                    If you are a Flanders, check this - maybe you like it.<br>\
                    (Not exported from presets)");

    setContextHelp(ui.macStyle, "<b>Mac Style Behaviour</b><hr>\
                    This currently affects the appereance of Wizzards and allows\
                    you to activate items with a SINGLE mouseclick, rather than\
                    the M$ DOUBLEclick thing ;)<br>\
                    (Not exported from presets)");

   
    /** setQSetting(.) tells BConfig to store values at
    "Company, Application, Group" - these strings are passed to QSettings */
    setQSetting("Bespin", "Style", "Style");

    /** you can call loadSettings() whenever you want, but (obviously)
    only items that have been propagated with handleSettings(.) are handled !!*/
    loadSettings();

    /** ===========================================
    You're pretty much done here - simple eh? ;)
        The following code reimplemets some BConfig functions
    (mainly to manage Qt color setttings)

    if you want a standalone app you may want to check the main() funtion
    at the end of this file as well - but there's nothing special about it...
        =========================================== */

    ui.sections->setCurrentIndex(0);
}

void
Config::changeEvent(QEvent *event)
{
    if (event->type() != QEvent::PaletteChange)
        return;
    
    const int s = 32;
    QPixmap logo(s*4,s*4);
    QPainterPath path;
    path.moveTo(logo.rect().center());
    path.arcTo(logo.rect(), 90, 270);
    path.lineTo(logo.rect().right(), logo.rect().y()+4*s/3);
    path.lineTo(logo.rect().right()-s, logo.rect().y()+4*s/3);
    path.lineTo(logo.rect().center().x() + s/2, logo.rect().center().y());
    path.lineTo(logo.rect().center());
    path.closeSubpath();
    path.addEllipse(logo.rect().right()-3*s/2, logo.rect().y(), s, s);
    logo.fill(Qt::transparent);
    QColor c = palette().color(foregroundRole());
    c.setAlpha(180);
    QPainter p(&logo);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(c);
    p.setPen(Qt::NoPen);
    p.drawPath(path);
    p.end();
    ui.logo->setPixmap(logo);
}


QStringList
Config::colors(const QPalette &pal, QPalette::ColorGroup group)
{
    QStringList list;
    for (int i = 0; i < QPalette::NColorRoles; i++)
        list << pal.color(group, (QPalette::ColorRole) i).name();
    return list;
}

void
Config::updatePalette(QPalette &pal, QPalette::ColorGroup group, const QStringList &list)
{
    int max = QPalette::NColorRoles;
    if (max > list.count())
    {
        qWarning("The demanded palette seems to be incomplete!");
        max = list.count();
    }
    for (int i = 0; i < max; i++)
        pal.setColor(group, (QPalette::ColorRole) i, list.at(i));
}

bool
Config::load(const QString &preset)
{
    QSettings store("Bespin", "Store");
    if (!store.childGroups().contains(preset))
        return false;
    store.beginGroup(preset);

    QSettings system("Bespin", "Style");
    system.beginGroup("Style");
    foreach (QString key, store.allKeys())
        if (key != "QPalette")
            system.setValue(key, store.value(key));
    system.endGroup();

    //NOTICE: we pass the loaded palette through ::updatePalette() to ensure
    // we'll get a complete palette for this Qt version as otherwise Qt will
    // fallback to the default palette... ===================================
    QPalette pal;
    store.beginGroup("QPalette");
    updatePalette(pal, QPalette::Active, store.value("active").toStringList());
    updatePalette(pal, QPalette::Inactive, store.value("inactive").toStringList());
    updatePalette(pal, QPalette::Disabled, store.value("disabled").toStringList());
    store.endGroup(); store.endGroup();
    savePalette(pal);
    return true;
}

static bool
blackListed(QString &key)
{
    return
        key.startsWith("Hack.") || // don't im/export hacks
        key == "FadeInactive" || // or dimmed inactive wins
        key == "Tab.Duration" || key == "Tab.Transition" || // or tab trans settings
        key == "MacStyle" || // or macfeeling
        key == "Menu.Opacity" || // or menu opacity (interferes with e.g. kwin/compiz)
        key == "LeftHanded" || // or flanders mode
        key == "Scroll.ShowButtons" || // or the scrollbar look
        key == "StoreName"; // finally don't im/export storename, are forcewise written
}

QString
Config::sImport(const QString &filename)
{
    if (!QFile::exists(filename))
        return QString();

    QSettings file(filename, QSettings::IniFormat);

    if (!file.childGroups().contains("BespinStyle"))
        return QString();

    file.beginGroup("BespinStyle");

    QString demandedName;
    QString storeName = demandedName = file.value("StoreName", "Imported").toString();

    QSettings store("Bespin", "Store");

    int i = 2;
    QStringList entries = store.childGroups();
    while (entries.contains(storeName))
        storeName = demandedName + '#' + QString::number(i++);

    store.beginGroup(storeName);
    foreach (QString key, file.allKeys())
    {
        if (!blackListed(key))
            store.setValue(key, file.value(key));
    }

    store.endGroup();
    file.endGroup();
    return storeName;
}

bool
Config::sExport(const QString &preset, const QString &filename)
{
    QSettings store("Bespin", "Store");
    if (!store.childGroups().contains(preset))
        return false;
    store.beginGroup(preset);

    QSettings file(filename, QSettings::IniFormat);
    file.beginGroup("BespinStyle");

    file.setValue("StoreName", preset);
    foreach (QString key, store.allKeys())
    {
        if (!blackListed(key))
            file.setValue(key, store.value(key));
    }
    store.endGroup();
    file.endGroup();
    return true;
}

/** reimplemented - i just want to extract the data from the store */
static QString lastPath = QDir::home().path();

void
Config::saveAs()
{
    if (!ui.store->currentItem())
        return;

    QString filename = QFileDialog::getSaveFileName(parentWidget(), tr("Save Configuration"),
                                                    lastPath, tr("Config Files (*.bespin *.conf *.ini)"));
    sExport(ui.store->currentItem()->text(0), filename);
}

/** reimplemented - i just want to merge the data into the store */
void
Config::import()
{
    QStringList filenames = QFileDialog::getOpenFileNames(parentWidget(),
        tr("Import Configuration"), lastPath, tr("Config Files (*.bespin *.conf *.ini)"));

    QTreeWidgetItem *item;
    foreach(QString filename, filenames)
    {
        QString storeName = sImport(filename);
        if (!storeName.isNull())
        {
            item = new QTreeWidgetItem(QStringList(storeName));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            ui.store->addTopLevelItem(item);
            ui.store->sortItems(0, Qt::AscendingOrder);
        }
    }
}

void
Config::presetAppsChanged(QTreeWidgetItem *changingItem, int idx)
{
    if (!changingItem || idx != 1)
        return;
    QStringList newApps = changingItem->text(1).split(',', QString::SkipEmptyParts);
    for (int i = 0; i < newApps.count(); ++i)
        newApps[i] = newApps[i].simplified();

    QTreeWidgetItem *item;
    const int cnt = ui.store->topLevelItemCount();
    for (int i = 0; i < cnt; ++i)
    {
        item = ui.store->topLevelItem(i);
        if (item == changingItem || item->text(1).isEmpty())
            continue;
        QStringList apps = item->text(1).split(',', QString::SkipEmptyParts);
        for (int i = 0; i < apps.count(); ++i)
            apps[i] = apps[i].simplified();
        foreach (QString newApp, newApps)
            apps.removeAll(newApp);
        item->setText(1, apps.join(", "));
    }
    emit changed(true);
    emit changed();
}

/** addition to the import functionality
1. we won't present a file dialog, but a listview
2. we wanna im/export the current palette as well
*/
void
Config::restore()
{
    restore(ui.store->currentItem(), 0);
}

void
Config::restore(QTreeWidgetItem *item, int col)
{
    if (col == 1)
    {
        ui.store->editItem( item, col );
        return;
    }
    setQSetting("Bespin", "Store", item->text(0));
    loadSettings(0, false, true);
    setQSetting("Bespin", "Style", "Style");

    /** import the color settings as well */
    if (!loadedPal)
        loadedPal = new QPalette;
    else
        emit changed(true); // we must update cause we loded probably different colors before

    QStringList list;
    const QPalette &pal = QApplication::palette();
    QSettings settings("Bespin", "Store");
    settings.beginGroup(item->text(0));
    settings.beginGroup("QPalette");

    list = settings.value ( "active", colors(pal, QPalette::Active) ).toStringList();
    updatePalette(*loadedPal, QPalette::Active, list);
    list = settings.value ( "inactive", colors(pal, QPalette::Inactive) ).toStringList();
    updatePalette(*loadedPal, QPalette::Inactive, list);
    list = settings.value ( "disabled", colors(pal, QPalette::Disabled) ).toStringList();
    updatePalette(*loadedPal, QPalette::Disabled, list);

    settings.endGroup();
    settings.endGroup();
}

void
Config::save()
{
    BConfig::save();
    /** store app presets */
    QSettings settings("Bespin", "Style");
    settings.beginGroup("PresetApps");
    settings.remove("");
    QTreeWidgetItem *item;
    const int cnt = ui.store->topLevelItemCount();
    for (int i = 0; i < cnt; ++i)
    {
        item = ui.store->topLevelItem(i);
        if (item->text(1).isEmpty())
            continue;
        QStringList apps = item->text(1).split(',', QString::SkipEmptyParts);
        foreach (QString app, apps)
            settings.setValue(app.simplified(), item->text(0));
    }
    settings.endGroup();
    /** save the palette loaded from store to qt configuration */
    if (loadedPal)
        savePalette(*loadedPal);
}

static QColor mid(const QColor &c1, const QColor &c2, int w1 = 1, int w2 = 1)
{
    int sum = (w1+w2);
    return QColor((w1*c1.red() + w2*c2.red())/sum,
                    (w1*c1.green() + w2*c2.green())/sum,
                    (w1*c1.blue() + w2*c2.blue())/sum,
                    (w1*c1.alpha() + w2*c2.alpha())/sum);
}


void
Config::savePalette(const QPalette &pal)
{

    // for Qt =====================================
    QSettings settings("Trolltech");
    settings.beginGroup("Qt");
    settings.beginGroup("Palette");

    settings.setValue ( "active", colors(pal, QPalette::Active) );
    settings.setValue ( "inactive", colors(pal, QPalette::Inactive) );
    settings.setValue ( "disabled", colors(pal, QPalette::Disabled) );

    settings.endGroup(); settings.endGroup();

    // and KDE ==== I'm now gonna mourn a bit and not allways be prudent...:
    //
    // yeah - 5000 new extra colors for a style that relies on very restricted
    // color assumptions (kstyle, plastik and thus oxygen...) - sure...
    // [ UPDATE:this is meanwhile fixed... ]
    // and please don't sync the Qt palette, ppl. will certainly be happy to
    // make color setting in KDE first and then in qtconfig for QApplication...
    // [ End update ]
    //
    // Ok, KDE supports extra DecorationFocus and DecorationHover
    //                        --------------      ---------------
    // -- we don't so we won't sync
    //
    // next, there's ForegroundLink and ForegroundVisited in any section
    //               -------------      ----------------
    // -- we just map them to QPalette::Link & QPalette::LinkVisited
    //
    // third, there're alternate backgroundcolors for all sections - sure:
    //                 ------------------------        ---------
    // my alternate button color: 1st button blue, second red, third blue...
    // -- we'll' do what we do for normal alternate row colors and slightly shift
    // to the foreground....
    //
    // there's a ForegroundActive color that is by default... PINK???
    //          -----------------
    // what a decent taste for asthetics... &-}
    // -- we just leave it as it is, cause i've no idea what it shall be good for
    // (active palette text - that's gonna be fun ;-)
    //
    // last there're ForegroundNegative, ForegroundNeutral and ForegroundPositive
    //               ------------------  ----------------      -----------------
    // what basically means: shifted to red, yellow and green...
    // who exactly is resposible for this fucking ridiculous nonsense?
    //
    // oh, and of course there NEEDS to be support for speciacl chars in the
    // KDE ini files - plenty. who could ever life without keys w/o ':' or '$'
    // so we cannot simply use QSettings on a KDE ini file, thus we'll use our
    // own very... slim... ini parser, ok, we just read the file group it by
    // ^[.* entries, replace the color things and than flush the whole thing back
    // on disk

    KdeIni *kdeglobals = KdeIni::open("kdeglobals");
    if (!kdeglobals)
    {
        qWarning("Warning: kde4-config not found or \"--path config\" flag does not work\nWarning: No KDE support!");
        return;
    }
    const QString prefix("Colors:");
#if QT_VERSION >= 0x040400
    const int numItems = 5;
#else
    const int numItems = 4;
#endif
    static const char *items[numItems] =
    {
        "Button", "Selection", "View", "Window"
#if QT_VERSION >= 0x040400
        , "Tooltip"
#endif
    };
    static const QPalette::ColorRole roles[numItems][2] =
    {
        {QPalette::Button, QPalette::ButtonText},
        {QPalette::Highlight, QPalette::HighlightedText},
        {QPalette::Base, QPalette::Text},
        {QPalette::Window, QPalette::WindowText}
#if QT_VERSION >= 0x040400
        , {QPalette::ToolTipBase, QPalette::ToolTipText}
#endif
    };
    for (int i = 0; i < numItems; ++i)
    {
        kdeglobals->setGroup(prefix + items[i]);
        kdeglobals->setValue("BackgroundAlternate", mid(pal.color(QPalette::Active, roles[i][0]),
                                                        pal.color(QPalette::Active, roles[i][1]), 15, 1));
        kdeglobals->setValue("BackgroundNormal", pal.color(QPalette::Active, roles[i][0]));
        kdeglobals->setValue("ForegroundInactive", pal.color(QPalette::Disabled, roles[i][1]));
        kdeglobals->setValue("ForegroundLink", pal.color(QPalette::Active, QPalette::Link));
        kdeglobals->setValue("ForegroundNegative", mid(pal.color(QPalette::Active, roles[i][1]), Qt::red));
        kdeglobals->setValue("ForegroundNeutral", mid(pal.color(QPalette::Active, roles[i][1]), Qt::yellow));
        kdeglobals->setValue("ForegroundNormal", pal.color(QPalette::Active, roles[i][1]));
        kdeglobals->setValue("ForegroundPositive", mid(pal.color(QPalette::Active, roles[i][1]), Qt::green));
        kdeglobals->setValue("ForegroundVisited", pal.color(QPalette::Active, QPalette::LinkVisited));
    }
    kdeglobals->close();
    delete kdeglobals; kdeglobals = 0;

}

/** see above, we'll present a name input dialog here */
void
Config::store()
{
    ui.presetLabel->hide();
    ui.presetFilter->hide();
    ui.storeLine->setText("Enter a name or select an item above");
    ui.storeLine->selectAll();
    ui.storeLine->show();
    ui.storeLine->setFocus();
    connect (ui.storeLine, SIGNAL(returnPressed()), this, SLOT(store2a()));
    connect (ui.store, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(store2b(QTreeWidgetItem *)));
}

void
Config::store2a()
{
    if (sender() != ui.storeLine)
        return;
    const QString &string = ui.storeLine->text();
    if (string.isEmpty()) {
        ui.storeLine->setText("Valid names have some chars...");
        return;
    }
    if (!ui.store->findItems ( string, Qt::MatchExactly ).isEmpty()) {
        ui.storeLine->setText("Item allready exists, please click it to replace it!");
        return;
    }
    disconnect (ui.storeLine, SIGNAL(returnPressed()), this, SLOT(store2a()));
    disconnect (ui.store, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(store2b(QTreeWidgetItem *)));
    ui.storeLine->hide();
    ui.presetLabel->show();
    ui.presetFilter->show();
    store3( string, true );
}

void
Config::store2b(QTreeWidgetItem* item)
{
    disconnect (ui.storeLine, SIGNAL(returnPressed()), this, SLOT(store2a()));
    disconnect (ui.store, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(store2b(QTreeWidgetItem *)));
    ui.storeLine->hide();
    ui.presetLabel->show();
    ui.presetFilter->show();
    store3( item->text(0), false );
}

/** real action */
void
Config::store3( const QString &string, bool addItem )
{
    if (addItem)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(string));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        ui.store->addTopLevelItem(item);
        ui.store->sortItems(0, Qt::AscendingOrder);
    }
    setQSetting("Bespin", "Store", string);
    save();
    setQSetting("Bespin", "Style", "Style");

    QSettings settings("Bespin", "Store");
    settings.beginGroup(string);
    /** Clear unwanted keys*/
    settings.remove("LeftHanded");
    settings.remove("MacStyle");
    settings.remove("Scroll.ShowButtons");
    settings.remove("Tab.Duration");
    settings.remove("Tab.Transition");
    /** Now let's save colors as well */
    settings.beginGroup("QPalette");

    const QPalette &pal = QApplication::palette();
    settings.setValue ( "active", colors(pal, QPalette::Active) );
    settings.setValue ( "inactive", colors(pal, QPalette::Inactive) );
    settings.setValue ( "disabled", colors(pal, QPalette::Disabled) );

    settings.endGroup();
    settings.endGroup();
}


void
Config::remove()
{
    QTreeWidgetItem *item = ui.store->currentItem();
    if (!item) return;

    QSettings store("Bespin", "Store");
    store.beginGroup(item->text(0));
    store.remove("");
    store.endGroup();
    delete item;
}

void
Config::storedSettigSelected(QTreeWidgetItem *item)
{
    ui.btnRestore->setEnabled(item);
    ui.btnExport->setEnabled(item);
    ui.btnDelete->setEnabled(item);
}

void
Config::handleBgMode(int idx)
{
    ui.structure->setEnabled(idx == 1);
}

static const char *grooveModes[4] = {"Line", "Groove", "Inlay", "Sunken"};

void
Config::handleGrooveMode(int v)
{
    if (v > 3 || v < 0)
        ui.grooveLabel->setText("INVALID");
    else
        ui.grooveLabel->setText(grooveModes[v]);
}

void
Config::learnPwChar()
{
    ushort n = unicode(ui.pwEchoChar->lineEdit()->text());
    if (ui.pwEchoChar->findData(n) != -1)
        return;
    ui.pwEchoChar->insertItem(0, QString(QChar(n)), QVariant(n));
    QSettings settings("Bespin", "Config");
    QStringList list = settings.value ( "UserPwChars", QStringList() ).toStringList();
    list << QString::number( n, 16 );
    settings.setValue("UserPwChars", list);
}

void
Config::filterPresets(const QString & string)
{
    QTreeWidgetItem *item;
    const int cnt = ui.store->topLevelItemCount();
    for (int i = 0; i < cnt; ++i)
    {
        item = ui.store->topLevelItem(i);
        item->setHidden(!item->text(0).contains(string, Qt::CaseInsensitive));
    }
}

static bool haveIcons = false;
static QIcon icons[8];
static const QPalette::ColorRole roles[] =
{
   QPalette::Window, QPalette::WindowText,
   QPalette::Base, QPalette::Text,
   QPalette::Button, QPalette::ButtonText,
   QPalette::Highlight, QPalette::HighlightedText
};
static const char* roleStrings[] =
{
   "Window", "Window Text",
   "Base (text editor)", "Text (text editor)",
   "Button", "Button Text",
   "Highlight", "Highlighted Text"
};
static void
ensureIcons()
{
    if (haveIcons)
        return;
    QPixmap pix(16,16);
    pix.fill(Qt::white);
    QPainter p(&pix);
    p.fillRect(1,1,14,14, Qt::black);
    p.end();
    QPalette *pal = /*loadedPal ? loadedPal :*/ &qApp->palette();
    for (int i = 0; i < 8; ++i)
    {
        p.begin(&pix);
        p.fillRect(2,2,12,12, pal->color(roles[i]));
        p.end();
        icons[i] = QIcon(pix);
    }
}

/** The combobox filler you've read of several times before ;) */
void
Config::generateColorModes(QComboBox *box, QList<int> *wantedRoles)
{
    ensureIcons();
    box->clear();
    box->setIconSize ( QSize(16,16) );
    if (wantedRoles)
    {
        foreach (int i, *wantedRoles)
            if (i < 8)
                box->addItem(icons[i], roleStrings[i], roles[i]);
    }
    else
    {
        for (int i = 0; i < 8; ++i)
            box->addItem(icons[i], roleStrings[i], roles[i]);
    }
}

void
Config::generateGradientTypes(QComboBox *box)
{
    box->clear();
    box->addItem("None");
    box->addItem("Simple");
    box->addItem("Button");
    box->addItem("Sunken");
    box->addItem("Gloss");
    box->addItem("Glass");
    box->addItem("Metal");
    box->addItem("Cloudy");
}


void
Config::setHeader(const QString &title)
{
//     setDefaultContextInfo("<qt><center><h1>" + title + "</h1></center></qt>");
    ui.header->setText("<qt><center><h1>" + title + "</h1></center></qt>"); // must force
//     resetInfo();
}
