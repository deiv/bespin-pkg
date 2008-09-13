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
#include <netwm.h>
#include <cmath>

#include "../colors.h"
#include "client.h"
#include "factory.h"
#include "button.h"

using namespace Bespin;

QPainterPath Button::shape[NumTypes];
bool Button::fixedColors = false;

Button::Button(Client *parent, Type type, bool isOnTitleBar) : QWidget(parent->widget()),
client(parent), state(0), multiIdx(0), zoomTimer(0), zoomLevel(0)
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setFixedSize(parent->buttonSize(), parent->buttonSize());
    setCursor(Qt::ArrowCursor);
    this->isOnTitleBar = isOnTitleBar;

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
Button::init(int sz, bool leftMenu, bool fColors)
{
    fixedColors = fColors;
    for (int t = 0; t < NumTypes; ++t)
        shape[t] = QPainterPath();
   
    const float s2 = sz/2.0, s3 = sz/3.0, s4 = sz/4.0, s6 = sz/6.0;
#if 1
    shape[Close].addRect(-s2,-s2,sz,sz);
    shape[Close].addRect(-s4,-s4,s2,s2);
    shape[Close].addRect(s3-s2,s3-s2,s3,s3);

    shape[Min].addRect(-s2,-s2,sz,sz);
    shape[Min].addRect(-s4,-s2,sz-s4,sz-s4);
    shape[Min].addRect(s2-s3,-s2,s3,s3);

    shape[Max].addRect(-s2,-s2,sz,sz);
    shape[Max].addRect(-s2,-s4,sz-s4,sz-s4);
    shape[Max].addRect(-s2,s2-s3,s3,s3);

    shape[Restore].addRect(-s2,-s2,sz,sz);
    shape[Restore].addRect(-s2,-s2,sz-s4,sz-s4);
    shape[Restore].addRect(-s2,-s2,s3,s3);

    shape[Stick].addRect(s6-s2,s6-s2,sz-s3,sz-s3);
    shape[Unstick].addRect(s3-s2,s3-s2,s3,s3);
    
    shape[Above].addRect(-s4,-s2,s2,s3);
    shape[Above].addRect(-s2,s2-s3,s3,s3);
    shape[Above].addRect(s2-s3,s2-s3,s3,s3);
    
    shape[Below].addRect(-s4,s2-s3,s2,s3);
    shape[Below].addRect(-s2,-s2,s3,s3);
    shape[Below].addRect(s2-s3,-s2,s3,s3);
    
    shape[UnAboveBelow].addRect(-s2,-s4,s3,s2);
    shape[UnAboveBelow].addRect(s2-s3,-s4,s3,s2);

    shape[Menu].addRect(-s2,-s2,sz,sz);
    shape[Menu].addRect(leftMenu?0:-s2,-s4,s2,sz-s4);

    shape[Help].addRect(-s3,-s2,s3+s4,sz-s3);
    shape[Help].addRect(-s3,-s4,s3,sz-(s3+s4));
    shape[Help].addRect(0,s2-s6,s4,s6);
    
    shape[Shade].addRect(-s2,-s2,sz,s4);
    
    shape[Unshade].addRect(-s2,s4,sz,s4);
    
    shape[Exposee].addRect(-s2,-s2,s3,s3);
    shape[Exposee].addRect(s2-s3,-s2,s3,s3);
    shape[Exposee].addRect(-s2,s2-s3,s3,s3);
    shape[Exposee].addRect(s2-s3,s2-s3,s3,s3);
    
    shape[Info].addRect(-s6,-s2,s4,s4);
    shape[Info].addRect(-s6,-s6,s4,s2+s3);
#else
    QPainterPath rubber;
    shape[Close].addEllipse(-s2,-s2,sz,sz);
    shape[Close].addEllipse(-s4,-s4,s2,s2);
    shape[Close].addEllipse(s3-s2,s3-s2,s3,s3);

    QRect bigRect(-s2, -(sz+s2), 2*sz, 2*sz);
    shape[Min].moveTo(bigRect.center());
    shape[Min].arcTo(bigRect, 180, 90);
    shape[Min].lineTo(bigRect.center());
    bigRect.adjust(s4,s4,-s4,-s4);
    rubber.moveTo(bigRect.center());
    rubber.arcTo(bigRect, 180, 90);
    rubber.lineTo(bigRect.center());
    shape[Min] = shape[Min].subtracted(rubber);
    rubber = QPainterPath();
    shape[Min].addEllipse(s2-s3,-s2,s3,s3);

    bigRect = QRect(-(sz+s2), -s2, 2*sz, 2*sz);
    shape[Max].moveTo(bigRect.center());
    shape[Max].arcTo(bigRect, 0, 90);
    shape[Max].lineTo(bigRect.center());
    bigRect.adjust(s4,s4,-s4,-s4);
    rubber.moveTo(bigRect.center());
    rubber.arcTo(bigRect, 0, 90);
    rubber.lineTo(bigRect.center());
    shape[Max] = shape[Max].subtracted(rubber);
    rubber = QPainterPath();
    shape[Max].addEllipse(-s2,s2-s3,s3,s3);

    bigRect = QRect(-(sz+s2), -(sz+s2), 2*sz, 2*sz);
    shape[Restore].moveTo(bigRect.center());
    shape[Restore].arcTo(bigRect, 0, -90);
    shape[Restore].lineTo(bigRect.center());
    bigRect.adjust(s4,s4,-s4,-s4);
    rubber.moveTo(bigRect.center());
    rubber.arcTo(bigRect, 0, -90);
    rubber.lineTo(bigRect.center());
    shape[Restore] = shape[Restore].subtracted(rubber);
    rubber = QPainterPath();
    shape[Restore].addEllipse(-s2,-s2,s3,s3);

    shape[Stick].addEllipse(s6-s2,s6-s2,sz-s3,sz-s3);
    shape[Unstick].addEllipse(s3-s2,s3-s2,s3,s3);
    
    shape[Above].addEllipse(-s4,-s2,s2,s3);
    shape[Above].addEllipse(-s2,s2-s3,s3,s3);
    shape[Above].addEllipse(s2-s3,s2-s3,s3,s3);
    
    shape[Below].addEllipse(-s4,s2-s3,s2,s3);
    shape[Below].addEllipse(-s2,-s2,s3,s3);
    shape[Below].addEllipse(s2-s3,-s2,s3,s3);
    
    shape[UnAboveBelow].addEllipse(-s2,-s4,s3,s2);
    shape[UnAboveBelow].addEllipse(s2-s3,-s4,s3,s2);

    shape[Menu].addRoundedRect(-s2,-s2,sz,sz,50,50,Qt::RelativeSize);
    rubber.addEllipse(leftMenu?0:-sz,-s4,1.2*sz,1.5*sz);
    shape[Menu] = shape[Menu].subtracted(rubber);
    rubber = QPainterPath();

    shape[Help].addRect(-s3,-s2,s3+s4,sz-s3);
    shape[Help].addRect(-s3,-s4,s3,sz-(s3+s4));
    shape[Help].addRect(0,s2-s6,s4,s6);
    
    shape[Shade].addEllipse(-s2,-s2,sz,s4);
    shape[Unshade].addEllipse(-s2,s4,sz,s4);
    
    shape[Exposee].addEllipse(-s2,-s2,s3,s3);
    shape[Exposee].addEllipse(s2-s3,-s2,s3,s3);
    shape[Exposee].addEllipse(-s2,s2-s3,s3,s3);
    shape[Exposee].addEllipse(s2-s3,s2-s3,s3,s3);
    
    shape[Info].addEllipse(-s6,-s2,s4,s4);
    shape[Info].addEllipse(-s6,-s6,s4,s2+s3);
#endif

//    tip[Close] = i18n("Close");
//    tip[Min] = i18n("Minimize");
//    tip[Max] = i18n("Maximize");
//    tip[Restore] = i18n("Restore");
//    tip[Menu] = i18n("Menu");
//    tip[Help] = i18n("Help");
//    tip[Above] = i18n("Keep above others");
//    tip[Below] = i18n("Keep below others");
//    tip[UnAboveBelow] = i18n("Equal to others");
//    tip[Stick] = i18n("All Desktops");
//    tip[Unstick] = i18n("This Desktops only");
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
                                    bgt = KDecorationDefines::ColorFrame;
    if (isOnTitleBar)
    {
        fgt = KDecorationDefines::ColorFont;
        bgt = KDecorationDefines::ColorTitleBar;
    }
    QColor c = client->color(fgt, client->isActive());
    if (fixedColors && _type < Multi)
        c = Colors::mid(c, QColor(fcolors[_type]), 6-zoomLevel, zoomLevel);
    const QColor bg = client->color(bgt, client->isActive());
    if (isEnabled())
        c = Colors::mid(bg, c, 6-zoomLevel, 6);
    else
        c = Colors::mid(bg, c, 6, 1);
    return c;
}

void
Button::paintEvent(QPaintEvent *)
{
   QPainter p(this);
   p.setRenderHint(QPainter::Antialiasing);
   const float t = height()/2.0;
   p.translate( QPoint(t,t) );
   const float f = (18 + (!(state & Sunken))*zoomLevel)/24.0;
   p.scale ( f, f );
//    p.rotate(60*zoomLevel);
   p.setPen(Qt::NoPen);
   p.setBrush(color());
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
