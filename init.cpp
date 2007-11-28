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
static Gradients::Type _progressBase;


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

#define readInt(_DEF_) iSettings->value(_DEF_).toInt()
#define readBool(_DEF_) iSettings->value(_DEF_).toBool()
#define readRole(_VAR_, _DEF_)\
config._VAR_##_role[0] = (QPalette::ColorRole) iSettings->value(_DEF_).toInt();\
Colors::counterRole(config._VAR_##_role[0], config._VAR_##_role[1])
//, QPalette::_DEF_, Colors::counterRole(QPalette::_DEF_))
#define readGrad(_DEF_) (Gradients::Type) iSettings->value(_DEF_).toInt();
#include <QtDebug>
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
      }
      if (!iSettings) {
         iSettings = new QSettings("Bespin", "Style");
         iSettings->beginGroup("Style");
      }
   }
   else
      qWarning("Bespin: WARNING - reading EXTERNAL settings!!!");
   
   // Background ===========================
   config.bg.mode = (BGMode) readInt(BG_MODE);
   config.bg.modal.glassy = readBool(BG_MODAL_GLASSY);
   config.bg.modal.opacity = readInt(BG_MODAL_OPACITY);
   config.bg.modal.invert = readBool(BG_MODAL_INVERT);
   config.bg.intensity = CLAMP(100+readInt(BG_INTENSITY),50,150);
   
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
      config.bg.structure = readInt(BG_STRUCTURE);
      
   // Buttons ===========================
   config.btn.checkType = (Check::Type) readInt(BTN_CHECKTYPE);
   config.btn.round = readBool(BTN_ROUND);
   GRAD(btn) = readGrad(BTN_GRADIENT);
   _progressBase = GRAD(btn);
   if (config.btn.layer == 2 && GRAD(btn) == Gradients::Sunken) // NO!
      GRAD(btn) = Gradients::None;
   
   config.btn.backLightHover = readBool(BTN_BACKLIGHTHOVER);
   config.btn.layer = config.btn.backLightHover ? 0 : CLAMP(readInt(BTN_LAYER), 0, 2);
   config.btn.fullHover = readBool(BTN_FULLHOVER);
   
   if (config.btn.layer == 2) config.btn.cushion = true;
   else if (GRAD(btn) ==  Gradients::Sunken) config.btn.cushion = false;
   else config.btn.cushion = readBool(BTN_CUSHION);
   
   readRole(btn.std, BTN_ROLE);
   readRole(btn.active, BTN_ACTIVEROLE);
   config.btn.ambientLight = readBool(BTN_AMBIENTLIGHT);
   config.btn.bevelEnds = readBool(BTN_BEVEL_ENDS);
   
   // Choosers ===========================
   GRAD(chooser) = readGrad(CHOOSER_GRADIENT);
   
   // PW Echo Char ===========================
   config.input.pwEchoChar =
      ushort(iSettings->value(INPUT_PWECHOCHAR).toUInt());

   // flanders
   config.leftHanded =
      readBool(LEFTHANDED) ? Qt::RightToLeft : Qt::LeftToRight;

   // item single vs. double click, wizard appereance
   config.macStyle = readBool(MACSTYLE);
   
   // Menus ===========================
   config.menu.glassy = readBool(MENU_GLASSY);
   config.menu.opacity = readInt(MENU_OPACITY);
   config.menu.itemGradient = readGrad(MENU_ITEMGRADIENT);
   config.menu.showIcons = readBool(MENU_SHOWICONS);
   config.menu.shadow = readBool(MENU_SHADOW);
   readRole(menu.active, MENU_ACTIVEROLE);
   readRole(menu.std, MENU_ROLE);
   readRole(menu.bar, MENU_BARROLE);
   config.menu.barSunken = readBool(MENU_BARSUNKEN);
   config.menu.boldText = readBool(MENU_BOLDTEXT);
   config.menu.activeItemSunken =
      readBool(MENU_ACTIVEITEMSUNKEN);
   
   // Progress ===========================
   GRAD(progress) = readGrad(PROGRESS_GRADIENT);
   config.progress.std_role[Bg] = 
      (QPalette::ColorRole) iSettings->value(PROGRESS_ROLE_BG).toInt();
   config.progress.std_role[Fg] =
      (QPalette::ColorRole) iSettings->value(PROGRESS_ROLE_FG).toInt();

   // ScrollStuff ===========================
   GRAD(scroll) = readGrad(SCROLL_GRADIENT);
   config.scroll.showButtons = readBool(SCROLL_SHOWBUTTONS);
   config.scroll.sunken = readBool(SCROLL_SUNKEN);
   config.scroll.groove =
      (!config.scroll.sunken) ? false : readBool(SCROLL_GROOVE);
   
   // Tabs ===========================
   readRole(tab.active, TAB_ACTIVEROLE);
   config.tab.animSteps =
      CLAMP(iSettings->value(TAB_ANIMSTEPS).toUInt(), 2, 18);
   GRAD(tab) = readGrad(TAB_GRADIENT);
   readRole(tab.std, TAB_ROLE);
   config.tab.transition =
      (TabAnimInfo::TabTransition) readInt(TAB_TRANSITION);
   config.tab.activeTabSunken =
      readBool(TAB_ACTIVETABSUNKEN);
   
   // ToolBoxes
   readRole(toolbox.active, TOOLBOX_ACTIVEROLE);
   GRAD(toolbox) = readGrad(TAB_ACTIVEGRADIENT);
   
   // Views ===========================
   readRole(view.header, VIEW_HEADERROLE);
   readRole(view.sortingHeader, VIEW_SORTINGHEADERROLE);
   config.view.headerGradient = readGrad(VIEW_HEADERGRADIENT);
   config.view.sortingHeaderGradient =
      readGrad(VIEW_SORTINGHEADERGRADIENT);

   // General ===========================
   config.shadowIntensity = iSettings->value(SHADOW_INTENSITY).toInt()/100.0;
   config.scale = iSettings->value(DEF_SCALE).toDouble();
   if (config.scale != 1.0) {
      QFont fnt = qApp->font();
      if (fnt.pointSize() > -1) fnt.setPointSize(fnt.pointSize()*config.scale);
      else fnt.setPixelSize(fnt.pixelSize()*config.scale);
      qApp->setFont(fnt);
   }
   
   if (delSettings) delete iSettings;
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
   
   dpi.ScrollBarExtent = SCALE(config.btn.fullHover ? 15 : 17);
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
                   (Gradients::BgMode)config.bg.mode :
                   Gradients::BevelV, _progressBase, config.bg.intensity, dpi.f8);
}
