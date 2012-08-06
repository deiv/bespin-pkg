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

#include <QPainter>
#include <QX11Info>
#include <netwm.h>
#include <cmath>

#include "../blib/colors.h"
#include "../blib/FX.h"
#include "../blib/shapes.h"
#include "client.h"
#include "factory.h"
#include "button.h"

using namespace Bespin;

QPainterPath Button::shape[NumTypes];
static QPixmap s_buttonMask[2];
static int s_buttonDepth = -1;
bool Button::fixedColors = false;

Button::Button(Client *parent, Type type, bool left) : QWidget(parent->widget()),
client(parent), state(0), multiIdx(0), hoverTimer(0), hoverLevel(0)
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_Hover, true);

    const int sz = Factory::buttonSize(parent->isSmall());
    setFixedSize(sz, sz);
    setCursor(Qt::ArrowCursor);
    this->left = left;

    myType = type;

    if ( (iAmScrollable = ( myType == Multi )) )
    {
        myType = Factory::multiButtons().at(0);
        connect(client, SIGNAL(keepAboveChanged(bool)), SLOT(clientStateChanged(bool)));
        connect(client, SIGNAL(keepBelowChanged(bool)), SLOT(clientStateChanged(bool)));
        connect(client, SIGNAL(stickyChanged(bool)), SLOT(clientStateChanged(bool)));
        connect(client, SIGNAL(shadeChanged(bool)), SLOT(clientStateChanged(bool)));
        if ( Factory::multiButtons().contains( Button::Max ) )
            connect(client, SIGNAL(maximizeChanged(bool)), SLOT(maximizeChanged(bool)) );
        clientStateChanged(false);
    }
    else if ( myType == Max )
        connect(client, SIGNAL(maximizeChanged(bool)), SLOT(maximizeChanged(bool)) );

// 	setToolTip(tip);
}

void
Button::clientStateChanged(bool state)
{
    if (state)
    {
        switch (myType)
        {
        case Above:
        case Below:
            myType = UnAboveBelow; break;
        case Stick:
            myType = Unstick; break;
        case Shade:
            myType = Unshade; break;
        default:
            return;
        }
    }
    else {
        switch (myType)
        {
        case UnAboveBelow:
            myType = Factory::multiButtons().at(multiIdx); break;
        case Unstick:
            myType = Stick; break;
        case Unshade:
            myType = Shade; break;
        default:
            return;
        }
    }
    repaint();
}

bool
Button::isEnabled() const
{
    if (!QWidget::isEnabled())
        return false;
    switch (myType)
    {
        case Close: return client->isCloseable();
        case Min: return client->isMinimizable();
        case Max: return client->isMaximizable();
        default: break;
    }
    return true;
}

void
Button::init(bool leftMenu, bool fColors, int variant)
{
    const int sz = 99;
    fixedColors = fColors;
    for (int t = 0; t < NumTypes; ++t)
        shape[t] = QPainterPath();
    s_buttonMask[0] = s_buttonMask[1] = QPixmap();

    Shapes::Style style = (Shapes::Style)qMin(qMax(0,variant),NumTypes-1);

    QRectF bound(-sz/2.0, -sz/2.0, sz, sz);
    shape[Close] = Shapes::close(bound, style);
    shape[Min] = Shapes::min(bound, style);
    shape[Max] = Shapes::max(bound, style);
    shape[Restore] = Shapes::restore(bound, style);
    shape[Stick] = Shapes::stick(bound, style);
    shape[Unstick] = Shapes::unstick(bound, style);
    shape[Above] = Shapes::keepAbove(bound, style);
    shape[Below] = Shapes::keepBelow(bound, style);
    shape[UnAboveBelow] = Shapes::unAboveBelow(bound, style);
    shape[Menu] = Shapes::menu(bound, leftMenu, style);
    shape[Help] = Shapes::help(bound, style);
    shape[Shade] = Shapes::shade(bound, style);
    shape[Unshade] = Shapes::unshade(bound, style);
    shape[Exposee] = Shapes::exposee(bound, style);
    shape[Info] = Shapes::info(bound, style);
#if 0
    tip[Close] = i18n("Close");
    tip[Min] = i18n("Minimize");
    tip[Max] = i18n("Maximize");
    tip[Restore] = i18n("Restore");
    tip[Menu] = i18n("Menu");
    tip[Help] = i18n("Help");
    tip[Above] = i18n("Keep above others");
    tip[Below] = i18n("Keep below others");
    tip[UnAboveBelow] = i18n("Equal to others");
    tip[Stick] = i18n("All Desktops");
    tip[Unstick] = i18n("This Desktops only");
#endif
}

void
Button::enterEvent(QEvent *)
{
    if (!isEnabled())
        return;

    state |= Hovered; hoverOut = false;
    hoverLevel += 2;
    if (hoverLevel > 6)
    {
        hoverLevel = 6;
        if (hoverTimer)
            killTimer(hoverTimer);
        hoverTimer = 0;
        return;
    }
    repaint();
    if (!hoverTimer)
        hoverTimer = startTimer ( 50 );
}

void
Button::leaveEvent(QEvent *)
{
    if (!isEnabled())
        return;

    state &= ~Hovered; hoverOut = true;
    --hoverLevel;
    if (hoverLevel < 0)
    {
        hoverLevel = 0;
        if (hoverTimer)
            killTimer(hoverTimer);
        hoverTimer = 0;
        return;
    }
    repaint();
    if (!hoverTimer)
        hoverTimer = startTimer ( 50 );
}

void
Button::maximizeChanged(bool maximized)
{
    myType = maximized ? Restore : Max;
    repaint();
}

void
Button::mousePressEvent ( QMouseEvent * event )
{
    if (!isEnabled()) return;

    if (event->button() == Qt::LeftButton)
    {
        state |= Sunken;
        repaint();
    }
}

void
Button::mouseReleaseEvent ( QMouseEvent *event )
{
    state &= ~Sunken;
    const bool under_mouse = Factory::isCompiz() ? underMouse() : rect().contains(event->pos());
    if (!(isEnabled() && under_mouse))
    {
        repaint();
        return;
    }
    KDecorationFactory* f = client->factory(); // needs to be saved before

    const bool lb = (event->button() == Qt::LeftButton);
    const bool rb = (event->button() == Qt::RightButton);
    switch (myType)
    {
    case Close:
        if (lb && client->isCloseable ())
            client->closeWindow();
        break;
    case Min:
        if (lb && client->isMinimizable ())
            client->minimize();
    //       else if (rb) // TODO get dbus interface or whatever to show the Desktop
    //          client->workspace()->setShowingDesktop( true );
        break;
    case Max:
        if (client->isMaximizable ())
        {
            //TODO support alt/ctrl click?!
    //          KDecorationDefines::MaximizeMode mode;
    //          MaximizeRestore    The window is not maximized in any direction.
    //          MaximizeVertical    The window is maximized vertically.
    //          MaximizeHorizontal    The window is maximized horizontally.
    //          MaximizeFull
            client->maximize(event->button());
//             client->setFullscreen(true);
        }
        break;
    case Restore:
        client->maximize(event->button());
        break;
    case Menu:
        //TODO: prepare menu? -> icon, title, stuff
        client->showWindowMenu (mapToGlobal(rect().bottomLeft()));
        break;
    case Help:
        if (lb) client->showContextHelp(); break;
    case Above:
        if (lb) client->setKeepAbove (!client->keepAbove()); break;
    case Below:
        if (lb) client->setKeepBelow (!client->keepBelow()); break;
    case UnAboveBelow:
        if (lb)
        {
            client->setKeepAbove(false);
            client->setKeepBelow(false);
        }
        break;
    case Stick:
    case Unstick:
        if (lb) client->toggleOnAllDesktops();
        else if (rb) client->showDesktopMenu(mapToGlobal(rect().topLeft())); break;
    case Shade:
    case Unshade:
        if (lb) client->setShade(!client->isSetShade()); break;
    case Exposee:
    //       if (rb) //TODO: toggle KWin composite exposé here on lb if available!
            client->showWindowList(mapToGlobal(rect().topLeft())); break;
    case Info:
        client->showInfo(mapToGlobal(rect().topLeft())); break;
    default:
        return; // invalid type
    }
    if( !f->exists( client )) // destroyed, return immediately
        return;
    repaint();
}

// static uint fcolors[3] = {0x9C3A3A/*0xFFBF0303*/, 0xFFEB55/*0xFFF3C300*/, 0x77B753/*0xFF00892B*/};
static uint fcolors[3] = {0xFFBF0303, 0xFFF3C300, 0xFF00892B};

QColor
Button::color( bool background ) const
{
    KDecorationDefines::ColorType   fgt = KDecorationDefines::ColorButtonBg,
                                    bgt = KDecorationDefines::ColorTitleBlend;
    int bbp = left + client->buttonBoxPos(client->isActive());
    if (bbp < 0 || bbp > 1) // 1 + -1 || 0 + 1 vs. 1 + 1 || 0 + -1
    {
        fgt = KDecorationDefines::ColorFont;
        bgt = KDecorationDefines::ColorTitleBar;
    }
    bool active = client->isActive();
    if (Factory::config()->invertedButtons)
        { KDecorationDefines::ColorType h = fgt; fgt = bgt; bgt = h; active = true || background; }

    if ( background )
        return client->color(bgt, active);

    QColor c = client->color(fgt, active);
    if (fixedColors && myType < Multi)
        c = client->isActive() ? QColor(fcolors[myType]) :
            Colors::mid(c, QColor(fcolors[myType]), 6-hoverLevel, hoverLevel);
    const QColor bg = client->color(bgt, active);
    if (!isEnabled())
        c = Colors::mid(bg, c, 6, 1);
    else if (Factory::buttonnyButton())
        c = Colors::mid(bg, c, 12 - 2*hoverLevel, 4);
    else
        c = Colors::mid(bg, c, 6 + 4*Factory::buttonnyButton() - hoverLevel, 4);
    c.setAlpha(c.alpha()*client->buttonOpacity()/100);
    return c;
}

void
Button::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (!bgPix.isNull())
        p.drawPixmap(0,0, bgPix);

    const int slick = Factory::slickButtons();
    QRectF r(rect());
    if (Factory::buttonnyButton()) {
        { float d = r.height() / 4.0; r.adjust(d,d,-d,-d); }
        if (s_buttonDepth != Factory::config()->buttonDepth || s_buttonMask[0].size() != size()) {
            const int depth = s_buttonDepth = Factory::config()->buttonDepth;
            s_buttonMask[0] = QPixmap(size());
            s_buttonMask[1] = QPixmap(width() - (2*(depth+1)), height() - (2*(depth+1)));
            s_buttonMask[0].fill(Qt::transparent);
            s_buttonMask[1].fill(Qt::transparent);

            QPainter p(&s_buttonMask[0]);
            p.setRenderHint(QPainter::Antialiasing);
            QPoint start(0,0), stop(0,s_buttonMask[0].height());
            QLinearGradient lg(start, stop);
            lg.setColorAt(0, QColor(0,0,0,92));
            lg.setColorAt(1, QColor(255,255,255,92));

            p.setPen(Qt::NoPen);
            p.setBrush(lg);
            if (Factory::config()->roundCorners)
                p.drawEllipse(s_buttonMask[0].rect());
            else
                p.drawRect(s_buttonMask[0].rect());
            if (Factory::buttonGradient() != Gradients::Sunken) {
                stop = QPoint(0,s_buttonMask[0].height()-(2*depth));
                QLinearGradient lg2(start, stop);
                lg2.setColorAt(0, QColor(255,255,255,20));
                lg2.setColorAt(1, QColor(0,0,0,20));
                p.setBrush(lg2);
                if (Factory::config()->roundCorners)
                    p.drawEllipse(s_buttonMask[0].rect().adjusted(depth,depth,-depth,-depth));
                else
                    p.drawRect(s_buttonMask[0].rect().adjusted(depth,depth,-depth,-depth));
            }
            p.end();

            p.begin(&s_buttonMask[1]);
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setBrush(Qt::black);
            if (Factory::config()->roundCorners)
                p.drawEllipse(s_buttonMask[1].rect());
            else
                p.drawRect(s_buttonMask[1].rect());
            p.end();
        }

        p.drawPixmap(0,0, s_buttonMask[0] );


        const bool rFixedColors = fixedColors;
        fixedColors = false;
        QColor c(color(true)); c.setAlpha(255);
        fixedColors = rFixedColors;
        QPixmap texture = Gradients::pix(c, s_buttonMask[1].height(), Qt::Vertical,
                                         (state & Sunken) ? Gradients::Sunken : Factory::buttonGradient());
        if (s_buttonMask[1].width() > 32) { // internal shortcut hack - the cached pixmaps are 32px width
            QPixmap gradient = texture;
            texture = QPixmap(s_buttonMask[1].size());
            QPainter p2(&texture);
            p2.drawTiledPixmap(s_buttonMask[1].rect(), gradient);
            p2.end();
        }

        const int d = Factory::config()->buttonDepth + 1;
        p.drawPixmap(d,d, FX::applyAlpha( texture, s_buttonMask[1], s_buttonMask[1].rect() ) );
    }

    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    const QColor c = color();
    p.setBrush(c);

    float fx, fy;
    const int depth = Factory::buttonnyButton() ? Factory::config()->buttonDepth : 0;
    if ((state & Sunken) && !Factory::buttonnyButton())
        fx = fy = .75;
    else if (slick == 2)
    {
        if (!hoverLevel || depth > 2)
        {
            const float d = r.width()/5.0;
            r.adjust(d,2*d,-d,-2*d);
            p.drawRoundRect(r);
            p.end();
            return;
        }
//             6/(b/a-1) = x
        fx = (9+hoverLevel)/15.0;
        fy = (1.5+hoverLevel)/7.5;
        }
    else if (slick)
    {
        if (!hoverLevel || depth > 2)
        {
            const float d = r.height()/3.0;
            r.adjust(d,d,-d,-d);
            p.drawEllipse(r);
            p.end();
            return;
        }
        fx = fy = (3+hoverLevel)/9.0;
    }
    else if (Factory::buttonnyButton())
        fx = fy = 1.0;
    else
        fx = fy = (18 + hoverLevel)/24.0;
    const float fs = r.height()/99.0;
//     const float t = r.height()/2.0;
//     p.translate( QPoint(t,t) );
    p.translate( r.center() );
    p.scale ( fs*fx, fs*fy );
//    p.rotate(60*hoverLevel); // too annoying, especially for fast zoom in...
    p.drawPath(shape[myType]);
    p.end();
}

void
Button::timerEvent ( QTimerEvent * )
{
    if (hoverOut)
    {
        --hoverLevel;
        if (hoverLevel < 1)
            { killTimer(hoverTimer); hoverTimer = 0; }
    }
    else
    {
        hoverLevel += 2;
        if (hoverLevel > 5)
            { hoverLevel = 6; killTimer(hoverTimer); hoverTimer = 0; }
    }
    repaint();
}

#define CYCLE_ON multiIdx += d;\
if (multiIdx >= mb.size() )\
    multiIdx = 0;\
else if (multiIdx < 0 )\
    multiIdx = mb.size()-1

void
Button::wheelEvent(QWheelEvent *e)
{
    if ( iAmScrollable )
    {
        const QVector<Type> &mb = Factory::multiButtons();
        int d = (e->delta() < 0) ? 1 : -1;

        CYCLE_ON;
        if (mb.at(multiIdx) == Help && !client->providesContextHelp())
            //       || (mb.at(multiIdx) == Shade && !client->isShadeable()))
        {
            CYCLE_ON;
        }

        myType = mb.at(multiIdx);
        if ( (myType == Above && client->keepAbove()) || (myType == Below && client->keepBelow()) )
            myType = UnAboveBelow;
        else if (myType == Stick && client->isOnAllDesktops())
            myType = Unstick;
        else if (myType == Shade && client->isSetShade())
            myType = Unshade;

        //TODO: roll max/vert/hori?!
        repaint();
    }

    else if ( ( myType == Max || myType == Restore ) && isEnabled() )
        client->tileWindow(e->delta() < 0, e->modifiers() & Qt::ControlModifier, !left);
}
