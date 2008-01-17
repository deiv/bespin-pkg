#ifndef BESPIN_CONFIG
#define BESPIN_CONFIG

#include "types.h"
#include "gradients.h"
#include "styleanimator.h"

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
   
   struct input {
      ushort pwEchoChar;
   } input;

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
      bool groove, showButtons, sunken;
   } scroll;

   float shadowIntensity;
   
   struct tab {
      QPalette::ColorRole std_role[2], active_role[2];
      Gradients::Type gradient;
      int animSteps;
      bool activeTabSunken;
      TabAnimInfo::TabTransition transition;
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
