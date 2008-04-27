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

#include <QPainter>
#include <QPolygon>
#include <QTimer>

#ifdef Q_WS_X11
#include <QX11Info>
#include <X11/Xlib.h>
#include "fixx11h.h"
#endif

#include "client.h"
#include "resizecorner.h"

using namespace Bespin;

ResizeCorner::ResizeCorner(Client * parent) : QWidget(parent->widget())
{
   if (!(parent->widget() && parent->windowId())) {
      deleteLater();
      return;
   }
   client = parent;
   setCursor(QCursor(Qt::SizeFDiagCursor));
   setFixedSize(16, 16);
   buffer = QPixmap(16,16);
   setAttribute(Qt::WA_OpaquePaintEvent);
   QPolygon triangle(3);
   triangle.putPoints(0, 3, 16,0, 16,16, 0,16);
   setMask ( triangle );
   QTimer::singleShot(0, this, SLOT(hide()));
   QTimer::singleShot(3000, this, SLOT(raise()));
}

void
ResizeCorner::raise()
{
   WId root, daddy = 0;
   WId *kids = 0L;
   uint numKids = 0;
   XQueryTree(QX11Info::display(), client->windowId(), &root, &daddy, &kids, &numKids);
   if (daddy)
      XReparentWindow( QX11Info::display(), winId(), daddy, 0, 0 );
   show();
   move(client->width() - 16, client->height() - 16);
   client->widget()->removeEventFilter(this);
   client->widget()->installEventFilter(this);
}

void
ResizeCorner::setColor(const QColor &c)
{
   bg = c;
   if (bg.value() > 100)
      fg = c.dark(130);
   else
      fg = c.light(120);
   buffer.fill(bg);
   QPainter p(&buffer);
   p.setBrush(fg); p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   QRect r = buffer.rect();
   r.setWidth(7*r.width()/4);
   r.setHeight(7*r.height()/4);
   p.drawPie(r, 90<<4, 90<<4);
   p.end();
}

void
ResizeCorner::move ( int x, int y )
{
   int l,r,t,b;
   client->borders( l, r, t, b );
   QWidget::move(x-(l+r), y-(t+b));
}

bool
ResizeCorner::eventFilter(QObject *obj, QEvent *e)
{
   if ( obj != parent() ) return false;

   if ( e->type() == QEvent::Resize)
      move(client->width() - 16, client->height() - 16);

   return false;
}

void
ResizeCorner::mousePressEvent ( QMouseEvent *mev )
{
   if (mev->button() == Qt::LeftButton)
      client->performWindowOperation(KDecoration::ResizeOp);
   else if (mev->button() == Qt::RightButton) {
      hide(); QTimer::singleShot(5000, this, SLOT(show()));
   }
   else if (mev->button() == Qt::MidButton) {
      hide();
   }
}

void
ResizeCorner::mouseReleaseEvent ( QMouseEvent * )
{
   client->performWindowOperation(KDecoration::NoOp);
}

void
ResizeCorner::paintEvent ( QPaintEvent * )
{
   QPainter p(this);
   p.drawPixmap(0,0,buffer);
   p.end();
}
