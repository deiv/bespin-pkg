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
#ifndef BESPIN_H
#define BESPIN_H

#include <kdecorationfactory.h>
#include "../gradients.h"

namespace Bespin
{

class Factory: public KDecorationFactory
{
public:
	Factory();
	~Factory();
	KDecoration *createDecoration(KDecorationBridge *b);
	bool reset(unsigned long changed);
	bool supports( Ability ability ) const;
   inline static int buttonSize() {return buttonSize_;}
   inline static int borderSize() {return borderSize_;}
   QList< BorderSize > borderSizes() const {
      return QList< BorderSize >() << BorderTiny << BorderNormal <<
      BorderLarge << BorderVeryLarge << BorderHuge << BorderVeryHuge <<
      BorderOversized << BordersCount;
   }
//    virtual void checkRequirements( KDecorationProvides* provides );
   inline static int titleSize(bool minimal = false) {return titleSize_[minimal];}
	inline static bool initialized() { return initialized_; }
   inline static bool forceUserColors() { return forceUserColors_; }
   inline static bool trimmCaption() { return trimmCaption_; }
   inline static Gradients::Type gradient(bool active) { return gradient_[active]; }
private:
	bool readConfig();
private:
   static bool initialized_, forceUserColors_, trimmCaption_;
   static int buttonSize_, borderSize_, titleSize_[2];
   static Gradients::Type gradient_[2];
};

} //namespace

#endif
