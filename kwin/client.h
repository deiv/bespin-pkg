//////////////////////////////////////////////////////////////////////////////
// 
// -------------------
// Bespin window decoration for KDE
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

#ifndef BESPINCLIENT_H
#define BESPINCLIENT_H

#include <QWidget>
#include <kdecoration.h>
#include "factory.h"

#include <X11/extensions/Xrender.h>
#include "../fixx11h.h"

class QHBoxLayout;
class QSpacerItem;

namespace Bespin
{

class Button;
class ResizeCorner;

class Client : public KDecoration
{
    Q_OBJECT
public:
    Client(KDecorationBridge *b, Factory *f);
    ~Client();
    void activeChange();
    void addButtons(const QString &, int &, bool);
    inline uint backgroundMode() {return bgMode;}
    void borders( int& left, int& right, int& top, int& bottom ) const;
    int buttonBoxPos(bool active);
    void captionChange();
    inline const Config* config() const { return _factory->config(); }
    void desktopChange() {/*TODO what??*/}
    inline Gradients::Type gradient() {return gType[isActive()];}
    inline void iconChange() {} // no icon!
    void init();
    void maximizeChange();
    QSize minimumSize() const;
    KDecorationDefines::Position mousePosition( const QPoint& p ) const;
    void reset(unsigned long changed);
    void resize( const QSize& s );
    void showDesktopMenu(const QPoint &p);
    void showInfo(const QPoint &p);
    void showWindowList(const QPoint &p);
    void showWindowMenu(const QPoint &p);
    void showWindowMenu(const QRect &r);
    void toggleOnAllDesktops();
    QString trimm(const QString &string);
    void shadeChange();
    inline Factory *factory() {return _factory;}
public slots:
    void updateStylePixmaps();
    void activate();
    void throwOnDesktop();
signals:
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
    inline int buttonSize() const { return _factory->buttonSize(); }
    void repaint(QPainter &p);
    inline int slickButtons() const { return _factory->slickButtons(); }
    void tileWindow(bool more, bool vertical, bool mirrorGravity);
private:
    Q_DISABLE_COPY(Client)
    void fadeButtons();
    void updateTitleLayout( const QSize& s );
    void updateButtonCorner(bool right = false);
    
    QColor colors[2][4]; // [inactive,active][titlebg,buttonbg/border,title,fg(bar,blend,font,btn)]
    Button *buttons[4];
    int borderSize, titleSize, buttonSpace, buttonSpaceLeft, buttonSpaceRight, retry;
    int myButtonOpacity;
    int myActiveChangeTimer;
    Picture topTile, btmTile, cnrTile, lCorner, rCorner;
    uint bgMode;
    Gradients::Type gType[2];
    bool _small;
    Factory *_factory;
    QHBoxLayout *titleBar;
    QSpacerItem *titleSpacer;
    QRect top, bottom, left, right, label;
    QPainterPath buttonCorner;
    QString _caption;
    ResizeCorner *corner;
};


} // namespace

#endif // BESPINCLIENT_H
