/*
 *   Bespin window decoration for KWin
 *   Copyright 2008-2012 by Thomas Lübking <thomas.luebking@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef BESPIN_H
#define BESPIN_H

#include <QHash>
#include <QObject>
#include <QPixmap>
#include <QVector>
#include <kdecorationfactory.h>
#include "../blib/gradients.h"
#include "../blib/xproperty.h"
#include "button.h"

class QMenu;
class QTextBrowser;

namespace Bespin
{

class Client;

typedef struct _Preset
{
    WindowData data;
    QStringList classes;
    QList<NET::WindowType>types;
} Preset;

typedef struct
{
    bool    forceUserColors, trimmCaption, resizeCorner, roundCorners, hideInactiveButtons,
            verticalTitle, variableShadowSizes, buttonnyButton, forceBorderLines, invertedButtons;
    int slickButtons, titleAlign, buttonDepth;
    Gradients::Type gradient[2][2], buttonGradient;
    QStringList smallTitleClasses;
} Config;

class Factory : public QObject, public KDecorationFactory
{
    friend class BespinDecoAdaptor;
    friend class Client;
    Q_OBJECT
public:
    Factory();
    ~Factory();
    KDecoration *createDecoration(KDecorationBridge *b);
    bool reset(unsigned long changed);
    bool supports( Ability ability ) const;

    inline static int baseSize() { return ourBorderSize[0] ? ourBorderSize[0] : !weAreComposited; }
    inline static Gradients::Type buttonGradient() {return ourConfig.buttonGradient;}
    inline static int buttonSize(bool small) {return ourButtonSize[small];}
    inline static bool buttonnyButton() {return ourConfig.buttonnyButton;}
    inline static const Config *config() { return &ourConfig; }
    inline static int edgeSize() { return ourBorderSize[1] ? ourBorderSize[1] : !weAreComposited; }
    inline static Qt::KeyboardModifier commandKey() { return ourCommandKey; }
    inline static bool compositingActive() { return weAreComposited; }
    static WindowData *decoInfo(qint64 pid);
    static WindowData *decoInfo(QString WMclass, NET::WindowType type, bool strict);
    static int defaultBgMode() { return ourBgMode; }
    inline static int initialized() { return weAreInitialized; }
    inline static bool isCompiz() { return weAreCompiz; }
    inline static const QVector<Button::Type> &multiButtons() { return ourMultiButton; }
    inline static bool roundCorners() { return ourConfig.roundCorners; }
    inline static int slickButtons() { return ourConfig.slickButtons; }
    inline static float smallFactor() { return 0.75; }
    static void showDesktopMenu(const QPoint &p, Client *client);
    static void showInfo(const QPoint &p, WId id);
    static void showWindowList(const QPoint &p, Client *client);
    inline static int titleSize(bool minimal = false) {return ourTitleSize[minimal];}
    inline static bool variableShadowSizes() { return ourConfig.variableShadowSizes; }
    inline static bool verticalTitle() { return ourConfig.verticalTitle; }
protected:
    static BgSet *bgSet(const QColor &c, bool vertical, int intensity, qint64 *hash = 0);
    static void forget(qint64 pid);
    static void kickBgSet(qint64 hash);
    static void learn(qint64 pid, QByteArray data);
    static QPixmap mask;
    void updateDeco(WId id);
    void setNetbookMode(bool on);
private:
    bool readConfig();
private slots:
    void cleanUp();
    void postInit();
    void updateCompositingState(bool);
private:
    static Qt::KeyboardModifier ourCommandKey;
    static QHash<qint64, WindowData*> ourDecoInfos;
    static QHash<qint64, BgSet*> ourBgSets;
    static QList<Preset*> ourPresets;
    static bool weAreInitialized, weAreComposited, weAreCompiz, weForceBorderlines;
    static int ourButtonSize[2], ourTitleSize[2], ourBgMode, ourBorderSize[2];
    static QVector<Button::Type> ourMultiButton;
    static Config ourConfig;
    static QMenu *ourDesktopMenu, *ourWindowList;
    static QTextBrowser *ourWindowInfo;
};

} //namespace

#endif
