//////////////////////////////////////////////////////////////////////////////
// 
// -------------------
// Bespin window decoration for KDE.
// -------------------
// Copyright (c) 2008 Thomas Lübking <baghira-style@gmx.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include <QFontMetrics>
#include <QSettings>
// #include "button.h"
#include "client.h"
#include "factory.h"

#include <QtDebug>

extern "C"
{
KDE_EXPORT KDecorationFactory* create_factory()
{
   return new Bespin::Factory();
}
}

using namespace Bespin;

bool Factory::initialized_ = false;
bool Factory::forceUserColors_ = false;
bool Factory::trimmCaption_ = true;
int Factory::buttonSize_ = -1;
int Factory::borderSize_ = 4;
int Factory::titleSize_[2] = {20,20};
Gradients::Type Factory::gradient_[2] = {Gradients::None, Gradients::Button};
QVector<Button::Type> Factory::multiButton_(0);

Factory::Factory()
{
	readConfig();
   Gradients::init();
	initialized_ = true;
}

Factory::~Factory() { initialized_ = false; }

KDecoration* Factory::createDecoration(KDecorationBridge* b)
{
	return new Client(b, this);
}

//////////////////////////////////////////////////////////////////////////////
// Reset the handler. Returns true if decorations need to be remade, false if
// only a repaint is necessary

bool Factory::reset(unsigned long changed)
{
	initialized_ = false;
	const bool configChanged = readConfig();
	initialized_ = true;

	if (configChanged ||
		(changed & (SettingDecoration | SettingButtons | SettingBorder))) {
		return true;
	}
	else {
		resetDecorations(changed);
		return false;
	}
}

static void
multiVector(const QString & string, QVector<Button::Type> &vector)
{
   Button::Type type; vector.clear();
   for (int i = 0; i < string.length(); ++i) {
      switch (string.at(i).toAscii()) {
         case 'M': type = Button::Menu; break;
         case 'S': type = Button::Stick; break;
         case 'H': type = Button::Help; break;
         case 'F': type = Button::Above; break;
         case 'B': type = Button::Below; break;
//          case 'L': type = Button::Shade; // Shade button
         default: continue;
      }
      vector.append(type);
   }
}

static QString
multiString(const QVector<Button::Type> &vector)
{
   QString string; char c;
   for (int i = 0; i < vector.size(); ++i) {
      switch (vector.at(i)) {
         case Button::Menu: c = 'M'; break;
         case Button::Stick: c = 'S'; break;
         case Button::Help: c = 'H'; break;
         case Button::Above: c = 'F'; break;
         case Button::Below: c = 'B'; break;
         //          case 'L': type = Button::Shade; // Shade button
         default: continue;
      }
      string.append(c);
   }
   return string;
}

bool Factory::readConfig()
{
   bool ret = false;
	QSettings settings("Bespin", "Style");
	settings.beginGroup("Deco");

   bool forcedusercolors = forceUserColors_;
   forceUserColors_ = settings.value("ForceUserColors", false).toBool();
   if (forcedusercolors != forceUserColors_) ret = true;

   bool trimmedCaption = trimmCaption_;
   trimmCaption_ = settings.value("TrimmCaption", true).toBool();
   if (trimmedCaption != trimmCaption_) ret = true;

   Gradients::Type oldgradient = gradient_[0];
   gradient_[0] = (Gradients::Type)settings.value("InactiveGradient",
                                                   Gradients::None).toInt();
   if (oldgradient != gradient_[0]) ret = true;

   oldgradient = gradient_[1];
   gradient_[1] = (Gradients::Type)settings.value("ActiveGradient",
                                                   Gradients::Button).toInt();
   if (oldgradient != gradient_[1]) ret = true;

   QString oldmultiorder = multiString(multiButton_);
   QString newmultiorder = settings.value("MultiButtonOrder", "MHFBS").toString();
   if (oldmultiorder != newmultiorder) {
      ret = true;
      multiVector(newmultiorder, multiButton_);
   }

   int oldbordersize = borderSize_;
   switch (options()->preferredBorderSize(this)) {
      case BorderTiny: borderSize_ = 0; break;
      default:
      case BorderNormal: borderSize_ = 6; break;
      case BorderLarge: borderSize_ = 8; break;
      case BorderVeryLarge: borderSize_ = 11; break;
      case BorderHuge: borderSize_ = 16; break;
      case BorderVeryHuge: borderSize_ = 20; break;
      case BorderOversized: borderSize_ = 30; break;
   }
   if (oldbordersize != borderSize_) ret = true;

   int oldtitlesize = titleSize_[1];
   QFontMetrics fm(options()->font());
   titleSize_[1] = fm.height() + 4;
   if (oldtitlesize != titleSize_[1]) ret = true;
   oldtitlesize = titleSize_[0];
   titleSize_[0] = qMax(titleSize_[1], borderSize_);
   if (oldtitlesize != titleSize_[0]) ret = true;

   if (buttonSize_ != titleSize_[1]) {
      buttonSize_ = titleSize_[1]-8; // for the moment
      Button::init(buttonSize_);
   }

   return ret;
}

bool Factory::supports( Ability ability ) const
{
	switch( ability ) {
	// announce
	case AbilityAnnounceButtons: ///< decoration supports AbilityButton* values (always use)
	case AbilityAnnounceColors: ///< decoration supports AbilityColor* values (always use)
	// buttons
	case AbilityButtonMenu:   ///< decoration supports the menu button
	case AbilityButtonOnAllDesktops: ///< decoration supports the on all desktops button
	case AbilityButtonSpacer: ///< decoration supports inserting spacers between buttons
	case AbilityButtonHelp:   ///< decoration supports what's this help button
	case AbilityButtonMinimize:  ///< decoration supports a minimize button
	case AbilityButtonMaximize: ///< decoration supports a maximize button
	case AbilityButtonClose: ///< decoration supports a close button
	case AbilityButtonAboveOthers: ///< decoration supports an above button
	case AbilityButtonBelowOthers: ///< decoration supports a below button
   
	// colors
	case AbilityColorTitleBack: ///< decoration supports titlebar background color
	case AbilityColorTitleFore: ///< decoration supports titlebar foreground color
	case AbilityColorTitleBlend: ///< decoration supports second titlebar background color
	case AbilityColorButtonBack: ///< decoration supports button background color
		return true;
   case AbilityColorButtonFore: ///< decoration supports button foreground color
   case AbilityColorFrame: ///< decoration supports frame color
   case AbilityButtonShade: ///< decoration supports a shade button
   case AbilityButtonResize: ///< decoration supports a resize button
   case AbilityColorHandle: ///< decoration supports resize handle color
	default:
		return false;
	};
}
