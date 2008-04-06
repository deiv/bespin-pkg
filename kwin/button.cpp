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
#include <cmath>

#include "../colors.h"
#include "factory.h"
#include "client.h"
#include "button.h"

using namespace Bespin;

Button::Button(Client *parent, Type type) : QWidget(parent->widget()),
client(parent), zoomTimer(0), zoomLevel(0)
{
   if (type == Multi)
      this->type = Menu;
   else
      this->type = type;
	setAutoFillBackground(false);
	setAttribute(Qt::WA_OpaquePaintEvent, false);
   setFixedSize(parent->buttonSize(), parent->buttonSize());
	setCursor(Qt::ArrowCursor);
// 	setToolTip(tip);
}

bool Button::isEnabled() const {
   if (!QWidget::isEnabled())
      return false;
   switch (type) {
      case Close: return client->isCloseable();
      case Min: return client->isMinimizable();
      case Max: return client->isMaximizable();
      default: break;
   }
   return true;
}

QPainterPath Button::shape[NumTypes];

void Button::init(int sz)
{
   for (int t = 0; t < NumTypes; ++t)
      shape[t] = QPainterPath();
   
   const float s2 = sz/2.0, s3 = sz/3.0, s4 = sz/4.0, s6 = sz/6.0;

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
   shape[Menu].addRect(-s2,-s4,s2,sz-s4);
   
   shape[Help].addRect(-s3,-s2,s3+s4,sz-s3);
   shape[Help].addRect(-s3,-s4,s3,sz-(s3+s4));
   shape[Help].addRect(0,s2-s6,s4,s6);


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
#include <QtDebug>
void Button::enterEvent(QEvent *)
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

void Button::leaveEvent(QEvent *)
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

void Button::mousePressEvent ( QMouseEvent * event )
{
   if (!isEnabled()) return;
   
   if (event->button() == Qt::LeftButton)
      state |= Sunken; repaint();
}

void Button::mouseReleaseEvent ( QMouseEvent * event )
{
   if (!isEnabled() || !underMouse()) return;
   
   KDecorationFactory* f = client->factory(); // needs to be saved before
	state &= ~Sunken;
   const bool lb = (event->button() == Qt::LeftButton);
   switch (type) {
   case Close:
      if (lb && client->isCloseable ())
         client->closeWindow();
      break;
   case Min:
      if (lb && client->isMinimizable ())
         client->minimize();
      // TODO: rgt click to lower?!
      break;
   case Max:
      if (client->isMaximizable ()) {
         type = Restore;
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
      type = Max;
      client->maximize(event->button());
      break;
   case Menu:
      //TODO: prepare menu? -> icon, title, stuff
      client->showWindowMenu (mapToGlobal(QPoint(0,height())));
      break;
   case Help:
      if (lb) client->showContextHelp(); break;
   case Above:
      if (lb) {
         type = UnAboveBelow;
         client->setKeepAbove (!client->keepAbove());
      }
      break;
   case Below:
      if (lb) {
         type = UnAboveBelow;
         client->setKeepBelow (!client->keepBelow());
      }
      break;
   case UnAboveBelow:
      if (lb) {
         client->setKeepAbove(false);
         client->setKeepBelow(false);
         type = Above;
      }
      break;
   case Stick:
   case Unstick:
      if (lb) {
         client->toggleOnAllDesktops();
         if (client->isOnAllDesktops())
            type = Stick;
         else
            type = Unstick;
      }
      break;
   default:
      return; // invalid type
   }
   if( !f->exists( client )) // destroyed, return immediately
      return;
   repaint();
}

QColor Button::color() const
{
   QColor c =
   client->color(KDecorationDefines::ColorButtonBg, client->isActive());
   const QColor bg =
   client->color(KDecorationDefines::ColorTitleBlend, client->isActive());
   if (isEnabled())
      c = Colors::mid(bg, c, 6-zoomLevel, 6);
   else
      c = Colors::mid(bg, c, 4, 1);
   return c;
}

void Button::paintEvent(QPaintEvent *)
{
   QPainter p(this);
   p.setRenderHint(QPainter::Antialiasing);
   const float t = height()/2.0; p.translate( QPoint(t,t) );
   const float f = (18 + zoomLevel)/24.0; p.scale ( f, f );
   p.setPen(Qt::NoPen);
   p.setBrush(color());
   p.drawPath(shape[type]);
   p.end();
}

void Button::timerEvent ( QTimerEvent * )
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

void Button::wheelEvent(QWheelEvent *e)
{
//    if (!isEnabled()) return; // NOTICE remember Obama: "Yes we can!" ;-)
   int d = (e->delta() < 0) ? 1 : -1;
   if (type > Multi) {
      type = (Type)(type + d);
      if (type == Help && !client->providesContextHelp())
//          || type == Shade && !client->isShadeable()
         type = (Type)(type + d);
      if (type >= Special)
         type = (Type)(Multi + 1);
      else if (type <= Multi)
         type = (Type)(Special - 1);
   }
   //TODO: roll max/vert/hori?!
   repaint();
}
