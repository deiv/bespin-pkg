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

#include <QMap>
#include <QVector>
#include <kdecorationfactory.h>
#include "../gradients.h"
#include "button.h"

class QMenu;
class QTextBrowser;

namespace Bespin
{

class Client;

typedef struct
{
    bool forceUserColors, trimmCaption, resizeCorner;
} Config;

class Factory : public QObject, public KDecorationFactory
{
public:
    Factory();
    ~Factory();
    KDecoration *createDecoration(KDecorationBridge *b);
    bool reset(unsigned long changed);
    bool supports( Ability ability ) const;
    inline static int buttonSize() {return buttonSize_;}
    inline static int borderSize() {return borderSize_;}
    inline static int initialized() {return initialized_;}
    QList< BorderSize > borderSizes() const
    {
        return QList< BorderSize >() << BorderTiny << BorderNormal <<
        BorderLarge << BorderVeryLarge << BorderHuge << BorderVeryHuge <<
        BorderOversized;
    }
//    virtual void checkRequirements( KDecorationProvides* provides );
    inline static int titleSize(bool minimal = false) {return titleSize_[minimal];}
    inline static const Config *config() { return &_config; }
    inline static Gradients::Type gradient(bool active) { return gradient_[active]; }
    bool hasDecoInfo(qint64 pid, uint &bgColors, uint &activeColors, uint &inactiveColors);
    inline static const QVector<Button::Type> &multiButtons() { return multiButton_; }
    void showDesktopMenu(const QPoint &p, Client *client);
    void showInfo(const QPoint &p, WId id);
    void showWindowList(const QPoint &p, Client *client);
protected:
    friend class BespinDecoAdaptor;
    void learn(qint64 pid, uint bgColors, uint activeColors, uint inactiveColors);
    void forget(qint64 pid);
private:
    bool readConfig();
private:
    typedef struct
    {
        uint bgColors, activeColors, inactiveColors;
    } DecoInfo;
    QMap<qint64, DecoInfo*> _decoInfos;
    static bool initialized_;
    static int buttonSize_, borderSize_, titleSize_[2];
    static Gradients::Type gradient_[2];
    static QVector<Button::Type> multiButton_;
    static Config _config;
    static QMenu *desktopMenu_, *_windowList;
    static QTextBrowser *_windowInfo;
};

} //namespace

#endif
