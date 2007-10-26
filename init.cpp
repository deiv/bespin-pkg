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

using namespace Bespin;

extern Config config;
extern Dpi dpi;
extern Gradients ::Type _progressBase;

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
   config.bg.mode = (BGMode) iSettings->value("Bg.Mode", BevelV).toInt();
   
#ifndef QT_NO_XRENDER
   if (config.bg.mode > BevelH)
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
   config.btn.checkType = (Check::Type) iSettings->value("Btn.CheckType", 0).toInt();
   
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
   config.btn.swapFocusHover = iSettings->value("Btn.SwapFocusHover", false).toBool();
   config.btn.ambientLight = iSettings->value("Btn.AmbientLight", true).toBool();
   
   // Choosers ===========================
   GRAD(chooser) = gradientType("Chooser.Gradient", Sunken);
   
   // PW Echo Char ===========================
   config.input.pwEchoChar =
      ushort(iSettings->value("Input.PwEchoChar", 0x26AB).toUInt());
   
   // flanders
   config.leftHanded = Qt::LeftToRight;
   if (iSettings->value("LeftHanded", false).toBool())
      config.leftHanded = Qt::RightToLeft;

   // item single vs. double click, wizard appereance
   config.macStyle = iSettings->value("MacStyle", true).toBool();
   
   // Menus ===========================
   config.menu.itemGradient = gradientType("Menu.ItemGradient", None);
   config.menu.showIcons = iSettings->value("Menu.ShowIcons", false).toBool();
   config.menu.shadow = iSettings->value("Menu.Shadow", false).toBool();
   readRole("Menu.ActiveRole", menu.active, Highlight, HighlightedText);
   readRole("Menu.Role", menu.std, Window, WindowText);
   readRole("Menu.BarRole", menu.bar, Window, WindowText);
   config.menu.barSunken = iSettings->value("Menu.BarSunken", false).toBool();
   config.menu.boldText = iSettings->value("Menu.BoldText", false).toBool();
   config.menu.activeItemSunken = iSettings->value("Menu.ActiveItemSunken", false).toBool();
   
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
   config.tab.activeTabSunken = iSettings->value("Tab.ActiveTabSunken", false).toBool();
   
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
   dpi.Indicator = SCALE(20 - 2*config.btn.layer);
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
