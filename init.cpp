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

#include <cmath>

#include "colors.h"
#include "bespin.h"
#include "makros.h"
#include "config.defaults"

using namespace Bespin;

extern Config config;
extern Dpi dpi;
extern Gradients ::Type _progressBase;


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

#define FIX_KAPP_PALETTE 1
#if FIX_KAPP_PALETTE
// this seems to be necessary as KDE somehow sets it's own palette after
// creating the style - god knows why...
static QPalette originalPalette;

void BespinStyle::fixKdePalette()
{
   qApp->setPalette(originalPalette);
}
#endif

#define readInt(_ENTRY_, _DEF_) iSettings->value(_ENTRY_, _DEF_).toInt()
#define readBool(_ENTRY_, _DEF_) iSettings->value(_ENTRY_, _DEF_).toBool()
#define readRole(_ENTRY_, _VAR_, _DEF_)\
config._VAR_##_role[0] = (QPalette::ColorRole) iSettings->value(_ENTRY_, QPalette::_DEF_).toInt();\
Colors::counterRole(config._VAR_##_role[0], config._VAR_##_role[1], QPalette::_DEF_, Colors::counterRole(QPalette::_DEF_))
#define readGrad(_ENTRY_, _DEF_) (Gradients::Type) iSettings->value(_ENTRY_, Gradients::_DEF_).toInt();

void
BespinStyle::readSettings(const QSettings* settings)
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
   config.bg.mode = (BGMode) readInt("Bg.Mode", BG_MODE);
   config.bg.modal.glassy = readBool("Bg.Modal.Glassy", BG_MODAL_GLASSY);
   config.bg.modal.invert = readBool("Bg.Modal.Invert", BG_MODAL_INVERT);
   
#ifndef QT_NO_XRENDER
   if (config.bg.mode > BevelH)
      config.bg.mode = BevelV;
   else if(config.bg.mode == ComplexLights &&
           !QFile::exists(QDir::tempPath() + "bespinPP.lock"))
      QProcess::startDetached(iSettings->
                              value("Bg.Daemon", "bespin pusher").toString());
#else
   if (config.bg.mode == ComplexLights) config.bg.mode = BevelV;
#endif
   
   if (config.bg.mode == Scanlines)
      config.bg.structure = readInt("Bg.Structure", BG_STRUCTURE);
      
   
   // Buttons ===========================
   config.btn.checkType = (Check::Type) readInt("Btn.CheckType", BTN_CHECKTYPE);
   config.btn.round = readBool("Btn.Round", false);
   GRAD(btn) = readGrad("Btn.Gradient", BTN_GRADIENT);
   _progressBase = GRAD(btn);
   if (config.btn.layer == 2 && GRAD(btn) == Gradients::Sunken) // NO!
      GRAD(btn) = Gradients::None;
   
   config.btn.backLightHover = readBool("Btn.BackLightHover", BTN_BACKLIGHTHOVER);

   if (config.btn.backLightHover) {
      config.btn.layer = 0;
      config.btn.fullHover = false;
   } else {
      config.btn.layer = CLAMP(readInt("Btn.Layer", BTN_LAYER), 0, 2);
      config.btn.fullHover = readBool("Btn.FullHover", BTN_FULLHOVER);
   }
   
   if (config.btn.layer == 2) config.btn.cushion = true;
   else if (GRAD(btn) ==  Gradients::Sunken) config.btn.cushion = false;
   else config.btn.cushion = readBool("Btn.Cushion", BTN_CUSHION);
   
   readRole("Btn.Role", btn.std, BTN_ROLE);
   readRole("Btn.ActiveRole", btn.active, BTN_ACTIVEROLE);
   Colors::setButtonRoles(config.btn.std_role[0], config.btn.std_role[1],
                          config.btn.active_role[0], config.btn.active_role[1]);
   config.btn.ambientLight = readBool("Btn.AmbientLight", BTN_AMBIENTLIGHT);
   
   // Choosers ===========================
   GRAD(chooser) = readGrad("Chooser.Gradient", CHOOSER_GRADIENT);
   
   // PW Echo Char ===========================
   config.input.pwEchoChar =
      ushort(iSettings->value("Input.PwEchoChar", INPUT_PWECHOCHAR).toUInt());
   
   // flanders
   config.leftHanded =
      readBool("LeftHanded", LEFTHANDED) ? Qt::RightToLeft : Qt::LeftToRight;

   // item single vs. double click, wizard appereance
   config.macStyle = readBool("MacStyle", MACSTYLE);
   
   // Menus ===========================
   config.menu.glassy = readBool("Menu.Glassy", MENU_GLASSY);
   config.menu.opacity = readInt("Menu.Opacity", MENU_OPACITY);
   config.menu.itemGradient = readGrad("Menu.ItemGradient", MENU_ITEMGRADIENT);
   config.menu.showIcons = readBool("Menu.ShowIcons", MENU_SHOWICONS);
   config.menu.shadow = readBool("Menu.Shadow", MENU_SHADOW);
   readRole("Menu.ActiveRole", menu.active, MENU_ACTIVEROLE);
   readRole("Menu.Role", menu.std, MENU_ROLE);
   readRole("Menu.BarRole", menu.bar, MENU_BARROLE);
   config.menu.barSunken = readBool("Menu.BarSunken", MENU_BARSUNKEN);
   config.menu.boldText = readBool("Menu.BoldText", MENU_BOLDTEXT);
   config.menu.activeItemSunken =
      readBool("Menu.ActiveItemSunken", MENU_ACTIVEITEMSUNKEN);
   
   // Progress ===========================
   GRAD(progress) = readGrad("Progress.Gradient", PROGRESS_GRADIENT);
   readRole("Progress.Role", progress.std, PROGRESS_ROLE);
   
   // ScrollStuff ===========================
   GRAD(scroll) = readGrad("Scroll.Gradient", SCROLL_GRADIENT);
   config.scroll.showButtons = readBool("Scroll.ShowButtons", SCROLL_SHOWBUTTONS);
   config.scroll.sunken = readBool("Scroll.Sunken", SCROLL_SUNKEN);
   config.scroll.groove =
      (!config.scroll.sunken) ? false : readBool("Scroll.Groove", SCROLL_GROOVE);
   
   // Tabs ===========================
   readRole("Tab.ActiveRole", tab.active, TAB_ACTIVEROLE);
   config.tab.animSteps =
      CLAMP(iSettings->value("Tab.AnimSteps", TAB_ANIMSTEPS).toUInt(), 2, 18);
   GRAD(tab) = readGrad("Tab.Gradient", TAB_GRADIENT);
   readRole("Tab.Role", tab.std, TAB_ROLE);
   config.tab.transition =
      (TabAnimInfo::TabTransition) readInt("Tab.Transition", TAB_TRANSITION);
   config.tab.activeTabSunken =
      readBool("Tab.ActiveTabSunken", TAB_ACTIVETABSUNKEN);
   
   // ToolBoxes
   readRole("ToolBox.ActiveRole", toolbox.active, TOOLBOX_ACTIVEROLE);
   GRAD(toolbox) = readGrad("Tab.ActiveGradient", TAB_ACTIVEGRADIENT);
   
   // Views ===========================
   readRole("View.HeaderRole", view.header, VIEW_HEADERROLE);
   readRole("View.SortingHeaderRole", view.sortingHeader, VIEW_SORTINGHEADERROLE);
   config.view.headerGradient = readGrad("View.HeaderGradient", VIEW_HEADERGRADIENT);
   config.view.sortingHeaderGradient =
      readGrad("View.SortingHeaderGradient", VIEW_SORTINGHEADERGRADIENT);
   
   // General ===========================
   config.shadowIntensity = iSettings->value("ShadowIntensity", 100).toInt()/100.0;
   config.scale = iSettings->value("Scale", DEF_SCALE).toDouble();
   if (config.scale != 1.0) {
      QFont fnt = qApp->font();
      if (fnt.pointSize() > -1) fnt.setPointSize(fnt.pointSize()*config.scale);
      else fnt.setPixelSize(fnt.pixelSize()*config.scale);
      qApp->setFont(fnt);
   }
   
   
   if (delSettings)
      delete iSettings;
}

#undef readRole
#undef gradientType

#define SCALE(_N_) lround((_N_)*config.scale)

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
   
   dpi.ScrollBarExtent = SCALE(15);
   dpi.ScrollBarSliderMin = SCALE(40);
   dpi.SliderThickness = SCALE(24);
   dpi.SliderControl = SCALE(19);
   dpi.Indicator = SCALE(20 - 2*config.btn.layer);
   dpi.ExclusiveIndicator = config.btn.layer ? SCALE(16) : SCALE(19);
}

#undef SCALE

void BespinStyle::init(const QSettings* settings) {
   readSettings(settings);
#if FIX_KAPP_PALETTE
   originalPalette = qApp->palette();
#endif
   initMetrics();
   generatePixmaps();
   Gradients::init(config.bg.mode > ComplexLights ?
                   (Gradients::BgMode)config.bg.mode : Gradients::BevelV);
}
