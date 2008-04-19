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

#ifndef BESPIN_CONFIG
#define BESPIN_CONFIG

#include "types.h"
#include "gradients.h"

namespace Bespin {

typedef struct Config {
   struct bg {
      BGMode mode;
      int structure, intensity;
      struct {
         bool glassy, invert;
         int opacity;
      } modal;
   } bg;
   
   struct btn {
      int layer;
      Check::Type checkType;
      bool cushion, fullHover, backLightHover, ambientLight, bevelEnds, round;
      Gradients::Type gradient, focusGradient;
      QPalette::ColorRole std_role[2], active_role[2];
   } btn;
   
   struct chooser {
      Gradients::Type gradient;
   } chooser;

   struct hack {
      bool messages, KHTMLView;
   } hack;
   
   struct input {
      ushort pwEchoChar;
   } input;

   struct kwin {
      int gradient[2]; // this is NOT Gradients::Type!!!
      QPalette::ColorRole inactive_role[2], active_role[2];
   } kwin;
   Qt::LayoutDirection leftHanded;

   bool macStyle;
   
   struct menu {
      QPalette::ColorRole std_role[2], active_role[2], bar_role[2];
      Gradients::Type itemGradient, barGradient;
      bool showIcons, shadow, barSunken, boldText, itemSunken, activeItemSunken, glassy;
      int opacity;
   } menu;
   
   struct progress {
      Gradients::Type gradient;
      QPalette::ColorRole std_role[2];
   } progress;
   
   float scale;
   
   struct scroll {
      Gradients::Type gradient;
      Groove::Mode groove;
      bool showButtons;
   } scroll;

   float shadowIntensity;
   
   struct tab {
      QPalette::ColorRole std_role[2], active_role[2];
      Gradients::Type gradient;
      bool activeTabSunken;
   } tab;

   struct toolbox {
      QPalette::ColorRole active_role[2];
      Gradients::Type gradient;
   } toolbox;

   struct view {
      QPalette::ColorRole header_role[2], sortingHeader_role[2];
      Gradients::Type headerGradient, sortingHeaderGradient;
   } view;

} Config;

} // namespace

#endif // BESPIN_CONFIG
