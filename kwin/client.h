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
#ifndef BESPINCLIENT_H
#define BESPINCLIENT_H

#include <QWidget>
#include <kdecoration.h>
#include "factory.h"

#include <X11/extensions/Xrender.h>
#include "../fixx11h.h"

class QBoxLayout;
class QSpacerItem;

namespace Bespin
{

class Button;
class ResizeCorner;

class Bg {
public:
    qint64 hash;
    BgSet *set;
    enum Mode { Plain, Gradient, VerticalGradient, HorizontalGradient, Structure, UNO };
    enum Tile { Top, Bottom, Corner, LeftCorner, RightCorner, Left = Top, Right = Bottom  };
};

class Client : public KDecoration
{
    Q_OBJECT
public:
    Client(KDecorationBridge *b, Factory *f);
    ~Client();
    enum DecoMode { NoDeco, CenterDeco, CornerDeco, ButtonDeco };
    enum Area { Top, Left, Bottom, Right, Label };
    inline void activeChange() { activeChange(true); }
    void activeChange(bool realActiveChange);
    void addButtons(const QString &, int &, bool);
    inline uint backgroundMode() {return myBgMode;}
    void borders( int& left, int& right, int& top, int& bottom ) const;
    int buttonBoxPos(bool active);
    Gradients::Type buttonGradient(bool active);
    void captionChange();
    void desktopChange() {/*TODO what??*/}
    inline Gradients::Type decoGradient() const {return myGradients[isActive()];}
    inline void iconChange() {} // no icon!
    void init();
    bool isSmall() { return iAmSmall; }
    void maximizeChange();
    QSize minimumSize() const;
    KDecorationDefines::Position mousePosition( const QPoint& p ) const;
    void reset(unsigned long changed);
    void resize( const QSize& s );
    void setFullscreen(bool);
    void showDesktopMenu(const QPoint &p);
    void showInfo(const QPoint &p);
    void showWindowList(const QPoint &p);
    void showWindowMenu(const QPoint &p);
    void showWindowMenu(const QRect &r);
    void toggleOnAllDesktops();
    QString trimm(const QString &string);
    void shadeChange();
    inline Gradients::Type unoGradient() const {return myGradients[2];}
public:
    Q_INVOKABLE KDecorationDefines::Position titlebarPosition() { return Factory::verticalTitle() ? PositionLeft : PositionTop; }
public slots:
    void activate();
    void throwOnDesktop();
    void triggerMoveResize();
    void updateStylePixmaps();
    void updateUnoHeight();
    QRegion region(KDecorationDefines::Region r);
signals:
    void maximizeChanged(bool);
    void stickyChanged(bool);
    void shadeChanged(bool);
protected:
    bool eventFilter(QObject *o, QEvent *e);
    void timerEvent(QTimerEvent *te);
protected:
    friend class Button;
    /// works like options()->color(.), but allows per window settings to match the window itself
    QColor color(ColorType type, bool active=true) const;
    DecoMode decoMode() const;
    inline int buttonOpacity() const { return myButtonOpacity; }
    void repaint(QPainter &p, bool paintTitle = true);
    void tileWindow(bool more, bool vertical, bool mirrorGravity);
    void maximumize(Qt::MouseButtons button);
private:
    Q_DISABLE_COPY(Client)
    void fadeButtons();
    void turnGradientToStructure();
    void updateTitleLayout( const QSize& s );
    void updateTitleHeight(int *variable);
    void updateButtonCorner(bool right = false);
    int titleSize() const;
private:
    QColor myColors[2][4]; // [inactive,active][titlebg,buttonbg/border,title,fg(bar,blend,font,btn)]
    Button *myButtons[4];
    int buttonSpace, buttonSpaceLeft, buttonSpaceRight, retry;
    int myButtonOpacity;
    int myActiveChangeTimer;
    Picture myTiles[5];
    uint myBgMode, myUnoHeight;
    Gradients::Type myGradients[3];
    bool iAmSmall;
    QBoxLayout *myTitleBar;
    QSpacerItem *myTitleSpacer;
    QRect myArea[5];
    QPainterPath buttonCorner;
    QString myCaption;
    ResizeCorner *myResizeHandle;
    Bg *myBgSet;
    bool dirty[2];
};


} // namespace

#endif // BESPINCLIENT_H
