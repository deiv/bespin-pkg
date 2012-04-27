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
 *   You should have received a copy of the GNU Library General Public
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
};

class Client : public KDecoration
{
    Q_OBJECT
public:
    Client(KDecorationBridge *b, Factory *f);
    ~Client();
    inline void activeChange() { activeChange(true); }
    void activeChange(bool realActiveChange);
    void addButtons(const QString &, int &, bool);
    inline uint backgroundMode() {return bgMode;}
    void borders( int& left, int& right, int& top, int& bottom ) const;
    int buttonBoxPos(bool active);
    void captionChange();
    void desktopChange() {/*TODO what??*/}
    inline Gradients::Type gradient() {return gType[isActive()];}
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
    inline QPoint titleOffset() const
        { return Factory::verticalTitle() ? QPoint(myTitleSize, 0) : QPoint(0, myTitleSize); }
    void toggleOnAllDesktops();
    QString trimm(const QString &string);
    void shadeChange();
public slots:
    void activate();
    void throwOnDesktop();
    void updateStylePixmaps();
    void updateUnoHeight();
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
    inline int buttonOpacity() const { return myButtonOpacity; }
    void repaint(QPainter &p, bool paintTitle = true);
    void tileWindow(bool more, bool vertical, bool mirrorGravity);
private:
    Q_DISABLE_COPY(Client)
    void fadeButtons();
    void updateTitleLayout( const QSize& s );
    void updateButtonCorner(bool right = false);
private:
    QColor colors[2][4]; // [inactive,active][titlebg,buttonbg/border,title,fg(bar,blend,font,btn)]
    Button *buttons[4];
    int myBaseSize, myEdgeSize, myTitleSize, buttonSpace, buttonSpaceLeft, buttonSpaceRight, retry;
    int myButtonOpacity;
    int myActiveChangeTimer;
    Picture topTile, btmTile, cornerTile, lCorner, rCorner;
    uint bgMode, unoHeight;
    Gradients::Type gType[2];
    bool iAmSmall;
    QBoxLayout *myTitleBar;
    QSpacerItem *myTitleSpacer;
    QRect top, bottom, left, right, label;
    QPainterPath buttonCorner;
    QString myCaption;
    ResizeCorner *corner;
    Bg *bg;
    bool dirty[2];
};


} // namespace

#endif // BESPINCLIENT_H
