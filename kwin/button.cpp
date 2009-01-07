//////////////////////////////////////////////////////////////////////////////
// button.cpp
// -------------------
// Bespin window decoration for KDE
// -------------------
// Copyright (c) 2008 Thomas Lübking <baghira-style@web.de>
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

#include <QPainter>
#include <QX11Info>
#include <netwm.h>
#include <cmath>

#include "../colors.h"
#include "../paths.h"
#include "client.h"
#include "factory.h"
#include "button.h"

using namespace Bespin;

QPainterPath Button::shape[NumTypes];
bool Button::fixedColors = false;

Button::Button(Client *parent, Type type, bool left) : QWidget(parent->widget()),
client(parent), state(0), multiIdx(0), zoomTimer(0), zoomLevel(0)
{
    setAutoFillBackground(false);
    setFixedSize(parent->buttonSize(), parent->buttonSize());
    setCursor(Qt::ArrowCursor);
    this->left = left;

    if (type == Multi)
    {
        _type = client->factory()->multiButtons().at(0);
        connect (client, SIGNAL (keepAboveChanged(bool)),
                    this, SLOT (clientStateChanged(bool)));
        connect (client, SIGNAL (keepBelowChanged(bool)),
                    this, SLOT (clientStateChanged(bool)));
        connect (client, SIGNAL (stickyChanged(bool)),
                    this, SLOT (clientStateChanged(bool)));
        connect (client, SIGNAL (shadeChanged(bool)),
                    this, SLOT (clientStateChanged(bool)));
        clientStateChanged(false);
    }
    else
        _type = type;

// 	setToolTip(tip);
}

void
Button::clientStateChanged(bool state)
{
    if (state)
    {
        switch (_type)
        {
        case Above:
        case Below:
            _type = UnAboveBelow; break;
        case Stick:
            _type = Unstick; break;
        case Shade:
            _type = Unshade; break;
        default:
            return;
        }
    }
    else {
        switch (_type)
        {
        case UnAboveBelow:
            _type = client->factory()->multiButtons().at(multiIdx); break;
        case Unstick:
            _type = Stick; break;
        case Unshade:
            _type = Shade; break;
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
    switch (_type)
    {
        case Close: return client->isCloseable();
        case Min: return client->isMinimizable();
        case Max: return client->isMaximizable();
        default: break;
    }
    return true;
}

void
Button::init(int sz, bool leftMenu, bool fColors, bool round)
{
    fixedColors = fColors;
    for (int t = 0; t < NumTypes; ++t)
        shape[t] = QPainterPath();

    QRectF bound(-sz/2.0, -sz/2.0, sz, sz);
    shape[Close] = Shapes::close(bound, round);
    shape[Min] = Shapes::min(bound, round);
    shape[Max] = Shapes::max(bound, round);
    shape[Restore] = Shapes::restore(bound, round);
    shape[Stick] = Shapes::stick(bound, round);
    shape[Unstick] = Shapes::unstick(bound, round);
    shape[Above] = Shapes::keepAbove(bound, round);
    shape[Below] = Shapes::keepBelow(bound, round);
    shape[UnAboveBelow] = Shapes::unAboveBelow(bound, round);
    shape[Menu] = Shapes::menu(bound, leftMenu, round);
    shape[Help] = Shapes::help(bound, round);
    shape[Shade] = Shapes::shade(bound, round);
    shape[Unshade] = Shapes::unshade(bound, round);
    shape[Exposee] = Shapes::exposee(bound, round);
    shape[Info] = Shapes::info(bound, round);
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
   if (!isEnabled()) return;
   
	state |= Hovered; zoomOut = false;
	zoomLevel += 2;
	if (zoomLevel > 6) {
      zoomLevel = 6;
		if (zoomTimer) killTimer(zoomTimer);
		zoomTimer = 0;
		return;
	}
   repaint();
	if (!zoomTimer) zoomTimer = startTimer ( 50 );
}

void
Button::leaveEvent(QEvent *)
{
   if (!isEnabled()) return;
   
	state &= ~Hovered; zoomOut = true;
	--zoomLevel;
	if (zoomLevel < 0) {
      zoomLevel = 0;
		if (zoomTimer) killTimer(zoomTimer);
		zoomTimer = 0;
		return;
	}
   repaint();
	if (!zoomTimer) zoomTimer = startTimer ( 50 );
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
Button::mouseReleaseEvent ( QMouseEvent * event )
{
    state &= ~Sunken;
    if (!isEnabled() || !QRect(mapToGlobal(QPoint(0,0)), size()).contains(QCursor::pos()))
    {
        repaint();
        return;
    }
   
    KDecorationFactory* f = client->factory(); // needs to be saved before

    const bool lb = (event->button() == Qt::LeftButton);
    const bool rb = (event->button() == Qt::RightButton);
    switch (_type)
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
            _type = Restore;
            //TODO support alt/ctrl click?!
    //          KDecorationDefines::MaximizeMode mode;
    //          MaximizeRestore    The window is not maximized in any direction.
    //          MaximizeVertical    The window is maximized vertically.
    //          MaximizeHorizontal    The window is maximized horizontally.
    //          MaximizeFull
            client->maximize(event->button());
        }
        break;
    case Restore:
        _type = Max;
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

static uint fcolors[3] = {0xFFBF0303, 0xFFF3C300, 0xFF00892B};

QColor
Button::color() const
{
    KDecorationDefines::ColorType   fgt = KDecorationDefines::ColorButtonBg,
                                    bgt = KDecorationDefines::ColorTitleBlend;
    int bbp = left + client->buttonBoxPos(client->isActive());
    if (bbp < 0 || bbp > 1) // 1 + -1 || 0 + 1 vs. 1 + 1 || 0 + -1
    {
        fgt = KDecorationDefines::ColorFont;
        bgt = KDecorationDefines::ColorTitleBar;
    }
    QColor c = client->color(fgt, client->isActive());
    if (fixedColors && _type < Multi)
        c = client->isActive() ? QColor(fcolors[_type]) :
            Colors::mid(c, QColor(fcolors[_type]), 6-zoomLevel, zoomLevel);
    const QColor bg = client->color(bgt, client->isActive());
    if (isEnabled())
        c = Colors::mid(bg, c, 6-zoomLevel, 4);
    else
        c = Colors::mid(bg, c, 6, 1);
    return c;
}

void
Button::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (!bgPix.isNull())
        p.drawPixmap(0,0, bgPix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(color());
    const int slick = client->slickButtons();
    
    float fx, fy;
    if (state & Sunken)
        fx = fy = .75;
    else if (slick)
    {
        if (slick == 2)
        {
            if (!zoomLevel)
            {
                const float s = width()/5.0;
                p.drawRoundRect(s, 2*s, 3*s, s);
                p.end(); return;
            }
//             6/(b/a-1) = x
            fx = (9+zoomLevel)/15.0;
            fy = (1.5+zoomLevel)/7.5;
        }
        else
        {
            if (!zoomLevel)
            {
                const float s = height()/3.0;
                p.drawEllipse(QRectF(s, s, s, s));
                p.end(); return;
            }
            fx = fy = (3+zoomLevel)/9.0;
        }
    }
    else
        fx = fy = (18 + zoomLevel)/24.0;
    const float t = height()/2.0;
    p.translate( QPoint(t,t) );
    p.scale ( fx, fy );
//    p.rotate(60*zoomLevel); // too annoying, especially for fast zoom in...
    p.drawPath(shape[_type]);
    p.end();
}

void
Button::timerEvent ( QTimerEvent * )
{
	if (zoomOut) {
		--zoomLevel;
		if (zoomLevel < 1) {
			killTimer(zoomTimer); zoomTimer = 0;
		}
	}
	else {
		zoomLevel += 2;
		if (zoomLevel > 5) {
			killTimer(zoomTimer); zoomTimer = 0;
		}
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
//    if (!isEnabled()) return; // NOTICE remember Obama: "Yes we can!" ;-)
    if (_type < Multi) return;

    const QVector<Type> &mb = client->factory()->multiButtons();
    int d = (e->delta() < 0) ? 1 : -1;

    CYCLE_ON;
    if (mb.at(multiIdx) == Help && !client->providesContextHelp())
//       || (mb.at(multiIdx) == Shade && !client->isShadeable()))
    {
        CYCLE_ON;
    }

   _type = mb.at(multiIdx);
   if ((_type == Above && client->keepAbove()) ||
      (_type == Below && client->keepBelow()))
      _type = UnAboveBelow;
   else if (_type == Stick && client->isOnAllDesktops())
      _type = Unstick;
   else if (_type == Shade && client->isSetShade())
      _type = Unshade;

   //TODO: roll max/vert/hori?!
   repaint();
}
