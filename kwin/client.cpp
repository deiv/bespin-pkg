//////////////////////////////////////////////////////////////////////////////
// client.cpp
// -------------------
// Oxygen window decoration for KDE
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

// #include <KGlobal>
#include <QPainter>
#include <QPaintEvent>
#include <QHBoxLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <QX11Info>
#include <QtDebug>

#include <kwindowinfo.h>

#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xatom.h>
#include "../fixx11h.h"

#include "button.h"
#include "../colors.h"
#include "../gradients.h"
#include "../xproperty.h"
#include "client.h"

using namespace Bespin;

PreviewWidget::PreviewWidget(QWidget *p, Qt::WindowFlags f) : QWidget(p,f)
{
   setAutoFillBackground(false);
   setAttribute(Qt::WA_OpaquePaintEvent);
   setAttribute(Qt::WA_PaintOnScreen, false);
}

PreviewWidget::~PreviewWidget(){}

void
PreviewWidget::paintEvent(QPaintEvent *pe)
{
   QPainter p(this);
   p.fillRect(rect(), Qt::red);
   p.end();
}

Client::Client(KDecorationBridge *b, Factory *f) :
KDecoration(b, f), retry(0), bgMode(1), _factory(f), _preview(0) {
}
Client::~Client(){ delete _preview; }

void
Client::addButtons(const QString& s, int &sz)
{
   sz = 4;
   if (!s.length() > 0) return;
   Button::Type type;
   for (int n=0; n < s.length(); ++n) {
      switch (s[n].toAscii()) {
         // i'm no way a friend of cluttering your titlebar with buttons =P
         case 'M': // Menu
         case 'S': // Sticky
         case 'H': // Help
         case 'F': // Keep Above
         case 'B': // Keep Below
         case 'L': // Shade button
            type = Button::Multi; break;
         case 'I': // Minimize
            type = Button::Min; break;
         case 'A': // Maximize
            type = Button::Max; break;
         case 'X': // Close button
            type = Button::Close; break;
         case '_': // Spacer
            titleBar->addSpacing(5);
            sz += 7;
         default:
            continue;
      }
      if (!buttons[type]) { // will be d'played d'abled in case
         buttons[type] = new Button(this, type);
         titleBar->addWidget(buttons[type], 0, Qt::AlignVCenter);
         sz += (buttonSize()+2);
      }
   }
}

void
Client::borders( int& left, int& right, int& top, int& bottom ) const
{
   // KWin seems to call borders() before maximizeChange() in case
   // this may be abug but is annoying at least - TODO: kwin bug report?
   if (maximizeMode() == MaximizeFull) {
      left = right = bottom = options()->moveResizeMaximizedWindows() ? 4 : 0;
      top = _factory->titleSize(true);
   }
   else {
      left = right = bottom =  _factory->borderSize();
      top = _factory->titleSize();
      if (isShade()) bottom = 8;
   }
//    left = right = bottom = borderSize;
//    top = titleSize;
}

void
Client::captionChange()
{
   // TODO caption filtering needs config option
   _caption = trimm(caption());
   widget()->update();
}

QColor
Client::color(ColorType type, bool active) const
{
   if (type < 4)
      return colors[active][type];
   return options()->color(type, active);
}

bool
Client::eventFilter(QObject *o, QEvent *e)
{
   if (o != widget()) return false;
   switch (e->type()) {
      case QEvent::Paint: {
         QPainter p(widget());
         p.setClipRect(static_cast<QPaintEvent*>(e)->rect());
         repaint(p);
         p.end();
         return true;
      }
      case QEvent::MouseButtonDblClick:
         titlebarDblClickOperation();
         return true;
      case QEvent::MouseButtonPress:
         processMousePressEvent(static_cast<QMouseEvent*>(e));
         return true;
      case QEvent::Wheel:
         titlebarMouseWheelOperation(static_cast<QWheelEvent*>(e)->delta());
         return true;
         //       case QEvent::MouseButtonRelease:
      default: return false;
   }
   return false;
}

void
Client::init()
{
   createMainWidget();
   _caption = trimm(caption());
   widget()->setAutoFillBackground(false);
   widget()->setAttribute(Qt::WA_OpaquePaintEvent);
   widget()->setAttribute(Qt::WA_NoSystemBackground);
   widget()->setAttribute(Qt::WA_PaintOnScreen, false);
   widget()->installEventFilter(this);

   titleBar = new QHBoxLayout();
   titleBar->setSpacing(2); titleBar->setContentsMargins(4,0,4,0);
   titleSpacer = new QSpacerItem( 1, titleSize, QSizePolicy::Expanding, QSizePolicy::Fixed);
   QVBoxLayout *layout = new QVBoxLayout(widget());
   layout->setSpacing(0); layout->setContentsMargins(0,0,0,0);
   layout->addLayout(titleBar);
   layout->addStretch(1000);

   for (int i = 0; i < 4; ++i) buttons[i] = 0;
   gType[0] = Gradients::None;
   gType[1] = Gradients::Button;

   if (isPreview()) {
      _preview = new PreviewWidget(widget());
      _preview->setAutoFillBackground(true);
      _preview->setGeometry ( borderSize, titleSize,
                              widget()->width()-2*borderSize,
                              widget()->height()-(borderSize+titleSize));
      _preview->show();
      _preview->raise();
   }

   reset(63);
}

void
Client::maximizeChange()
{
   reset(SettingBorder);
}

KDecorationDefines::Position
Client::mousePosition( const QPoint& p ) const
{
	if (p.y() < 4) { // top
		if (p.x() < 4) return PositionTopLeft; // corner
		if (p.x() > width() - 4) return PositionTopRight; // corner
		return PositionTop;
	}
	if (p.y() > height() - 16) { // bottom
		if (p.x() < 16) return PositionBottomLeft; // corner
		if (p.x() > width() - 16) return PositionBottomRight; // corner
		int off = width()/3;
		if (p.x() > off && p.x() < width() - off) return PositionBottom;
		else return PositionCenter; // not on outer 3rds
	}
	if (p.x() < 4) { // left
		int off = height()/3;
		if (p.y() > off && p.y() < height() - off) return PositionLeft;
		else return PositionCenter; // not on outer 3rds
	}
	if (p.x() > width() - 4) { // right
		int off = height()/3;
		if (p.y() > off && p.y() < height() - off) return PositionRight;
		else return PositionCenter; // not on outer 3rds
	}
   return PositionCenter; // to convince gcc, never reach this anyway
}

QSize
Client::minimumSize() const
{
	return QSize(2*buttonSpace+6*titleSize, titleSize + borderSize);
}

int format, result; unsigned long de; //dead end
uint decoDim = 0;

#define DUMP_PICTURE(_PREF_, _PICT_)\
XRenderComposite(QX11Info::display(), PictOpSrc, _PICT_, 0, _PREF_##Buffer.x11PictureHandle(),\
0, 0, 0, 0, 0, 0, _PREF_##Width, _PREF_##Height);

void
Client::repaint(QPainter &p)
{
   if (!Factory::initialized()) return;

   const QColor bg = color(ColorTitleBlend, isActive());

   if (isShade()) { // only one "big" gradient, as we can't rely on windowId()!!
      const QPixmap &fill =
      Gradients::pix(bg, height(), Qt::Vertical, Gradients::Button);
      p.drawTiledPixmap(0,0,width(),height(), fill);
   }
   else {
   
   const QColor shadow = Colors::mid(bg, Qt::black,8,1);

   // window ================
   p.setBrush(bg); p.setPen(Qt::NoPen);
   switch (bgMode) {
   case 2: // vertical gradient
   case 3: { // horizontal gradient

      uint topTile = 0, btmTile = 0, cnrTile = 0, lCorner = 0, rCorner = 0;
      if (!XProperty::get(windowId(), XProperty::topTile, topTile)) {
         // hmm? paint fallback, wait and try again (10x)
         p.drawRect(left); p.drawRect(right);
         p.drawRect(top); p.drawRect(bottom);
         if (retry < 10)
            QTimer::singleShot(200-16*retry, widget(), SLOT(update()));
         ++retry;
         break;
      }
      XProperty::get(windowId(), XProperty::btmTile, btmTile);
      XProperty::get(windowId(), XProperty::cnrTile, cnrTile);
      XProperty::get(windowId(), XProperty::lCorner, lCorner);
      XProperty::get(windowId(), XProperty::rCorner, rCorner);
      
#define ctWidth 32
#define ctHeight 128

      if (bgMode == 2) {
         p.drawRect(left); p.drawRect(right);
#define tbWidth 32
#define tbHeight 256
#define lrcWidth 128
#define lrcHeight 128
         QPixmap tbBuffer(tbWidth, tbHeight);
         int s1 = tbHeight;
         int s2 = qMin(s1, (height()+1)/2);
         s1 -= s2;
         DUMP_PICTURE(tb, topTile);
         p.drawTiledPixmap( 0, 0, width(), s2, tbBuffer, 0, s1 );
         if (Colors::value(bg) < 245) { // no sense otherwise
            const int w = width()/4 - 128;
            if (w > 0) {
               s2 = 128-s1;
               QPixmap ctBuffer(ctWidth, ctHeight);
               DUMP_PICTURE(ct, cnrTile);
               p.drawTiledPixmap( 0, 0, w, s2, ctBuffer, 0, s1 );
               p.drawTiledPixmap( width()-w, 0, w, s2, ctBuffer, 0, s1 );
            }
            QPixmap lrcBuffer(lrcWidth, lrcHeight);
            DUMP_PICTURE(lrc, lCorner);
            p.drawPixmap(w, 0, lrcBuffer, 0, s1, 128, s2);
            DUMP_PICTURE(lrc, rCorner);
            p.drawPixmap(width()-w-128, 0, lrcBuffer, 0, s1, 128, s2);
         }
         if (!borderSize) break;
         s1 = tbHeight;
         s2 = qMin(s1, height()/2);
         DUMP_PICTURE(tb, btmTile);
         p.drawTiledPixmap( 0, height()-s2, width(), s2, tbBuffer );
#undef tbWidth
#undef tbHeight
#undef lrcWidth
#undef lrcHeight
      }
      else {
#define tbWidth 256
#define tbHeight 32
#define lrcWidth 256
#define lrcHeight 32
         p.drawRect(bottom);
         int s1 = tbWidth;
         int s2 = qMin(s1, (width()+1)/2);
         const int h = qMin(128+32, height()/8);
         QPixmap tbBuffer(tbWidth, tbHeight);
         QPixmap lrcBuffer(lrcWidth, lrcHeight);
         QPixmap ctBuffer(ctWidth, ctHeight);
         DUMP_PICTURE(tb, topTile); // misleading, this is the LEFT column
         p.drawTiledPixmap( 0, h, s2, height()-h, tbBuffer, s1-s2, 0 );
         DUMP_PICTURE(lrc, lCorner); // left bottom shine
         p.drawPixmap(0, h-32, lrcBuffer, s1-s2, 0,0,0);
         DUMP_PICTURE(tb, btmTile); // misleading, this is the RIGHT column
         p.drawTiledPixmap( width() - s2, h, s2, height()-h, tbBuffer );
         DUMP_PICTURE(lrc, rCorner); // right bottom shine
         p.drawPixmap(width() - s2, h-32, lrcBuffer);
         DUMP_PICTURE(ct, cnrTile); // misleading, TOP TILE
         p.drawTiledPixmap( 0, h-(128+32), width(), 128, ctBuffer );
      }
      break;
   }
   case 0: { // plain
      p.setBrush(bg); p.setPen(Qt::NoPen);
      p.drawRect(left); p.drawRect(right);
      p.drawRect(top); p.drawRect(bottom);
      break;
   }
   default:
   case 1: { // scanlines, fallback
      p.drawRect(left); p.drawRect(right);
      p.drawRect(bottom);
      const QPixmap &fill =
      Gradients::pix(bg, titleSize, Qt::Vertical, Gradients::Button);
      p.drawTiledPixmap(top, fill);
      p.setPen(QPen(shadow, 2));
      p.drawLine(0,titleSize-1,width(),titleSize-1);
      break;
   }
   }
   }

   // bar =========================
   if (gType[isActive()]) {
      const QColor bg2 = color(ColorTitleBar, isActive());
      const QPixmap &fill = Gradients::pix(bg2, titleSize, Qt::Vertical,
                                            (Gradients::Type)gType[isActive()]);
      p.setPen(QPen(Colors::mid(bg, Qt::white,1,2), 2)); p.setBrush(fill);
      p.setRenderHint( QPainter::Antialiasing );
      p.drawPath(bar);
      p.setBrush(Qt::NoBrush);
   }

   // title ==============
   if (isActive()) {
      p.setPen(Colors::mid(bg, Qt::black, 2, 1));
      p.drawText ( label.translated(0,-1), Qt::AlignCenter | Qt::TextSingleLine, _caption );
   }
   p.setPen(color(ColorFont, isActive()));
   p.drawText ( label, Qt::AlignCenter | Qt::TextSingleLine, _caption );

   // frame ==============
   if (borderSize) {
      p.setBrush(Qt::NoBrush);
      const QColor border = Colors::mid(bg, color(ColorButtonBg, true));
      p.setPen(border);
      p.drawLine(32+4, 0, width()-(32+5), 0);
      p.drawLine(32+4, height()-1, width()-(32+5), height()-1);
      p.drawLine(0, 32+4, 0, height()-(32+5));
      p.drawLine(width()-1, 32+4, width()-1, height()-(32+5));
      const QPixmap &top = Gradients::borderline(border, Gradients::Top);
      p.drawPixmap(0,4,top); p.drawPixmap(width()-1,4,top);
      const QPixmap &btm = Gradients::borderline(border, Gradients::Bottom);
      p.drawPixmap(0,height()-(32+5),btm); p.drawPixmap(width()-1,height()-(32+5),btm);
      const QPixmap &left = Gradients::borderline(border, Gradients::Left);
      p.drawPixmap(4,0,left); p.drawPixmap(4,height()-1,left);
      const QPixmap &right = Gradients::borderline(border, Gradients::Right);
      p.drawPixmap(width()-(32+5),0,right); p.drawPixmap(width()-(32+5),height()-1,right);
      
      for (int i = 3; i < 16; i += 5)
         p.drawLine(width()-i,height(),width(),height()-i);

#if 0
      p.setPen(Colors::mid(bg, color(ColorButtonBg, isActive()),3,1));
//       p.setRenderHint( QPainter::Antialiasing, true );
      QRect r(0,0,10,10); p.drawArc(r, 90<<4, 90<<4);
      r.moveRight(width()-1); p.drawArc(r, 0, 90<<4);
      r.moveBottom(height()-1); //p.drawArc(r, 270<<4, 90<<4);
      r.moveLeft(0); p.drawArc(r, 180<<4, 90<<4);
#endif
   }

   // TODO BETTER!!! resize indicator
//    p.setBrush(Qt::NoBrush);
//    p.setPen(QPen(Colors::mid(bg, color(ColorButtonBg, isActive()), 4, 1),
//              4, Qt::DotLine, Qt::RoundCap));
//    int off = height()/3;
//    const int bs = (borderSize+1)/2;
//    p.drawLine(bs,off,bs,height()-off);
//    p.drawLine(width()-bs,off,width()-bs,height()-off);
//    off = width()/3;
//    p.drawLine(off,height()-bs,off,height()-bs);
}

void
Client::reset(unsigned long changed)
{
   delete _preview; _preview = 0;
   
   if (changed & SettingFont) {
      titleSize = _factory->titleSize();
      titleSpacer->changeSize( 1, titleSize, QSizePolicy::Expanding, QSizePolicy::Fixed);
   }

   if (changed & SettingDecoration) {
      gType[0] = _factory->gradient(0);
      gType[1] = _factory->gradient(1);
   }

   if (changed & SettingBorder) {
      if (maximizeMode() == MaximizeFull) {
         borderSize = options()->moveResizeMaximizedWindows() ? 4 : 0;
         titleSize = _factory->titleSize(true);
      }
      else {
         borderSize = _factory->borderSize();
         titleSize = _factory->titleSize();
      }

      bottom = QRect(0, height()-borderSize, width(), borderSize);
      const int sideHeight = height() - (titleSize + borderSize);
      left = QRect(0, titleSize, borderSize, sideHeight);
      right = QRect(width()-borderSize, titleSize, borderSize, sideHeight);

      int decoDim =
      ((borderSize & 0xff) << 24) | ((titleSize &0xff) << 16) |
      ((borderSize &0xff) << 8) | (borderSize & 0xff);
      XProperty::set(windowId(), XProperty::decoDim, decoDim);
      titleSpacer->changeSize( 1, titleSize, QSizePolicy::Expanding, QSizePolicy::Fixed);
   }

   if (changed & SettingButtons)
   {
      for (int i = 0; i < 4; ++i) {
         delete buttons[i]; buttons[i] = 0;
      }
      titleBar->removeItem(titleSpacer);
      int sl, sr;
//       addButtons("X_IA", sl); // as long as kwin ignores settings, we ignore kwin :P
      addButtons(options()->titleButtonsLeft(), sl);
      titleBar->addItem(titleSpacer);
//       addButtons("M", sr);
      addButtons(options()->titleButtonsRight(), sr);
      buttonSpace = qMax(sl,sr);
   }

   if (changed & SettingColors) {
      // colors =====================
      for (int a = 0; a < 2; ++a)
         for (int t = 0; t < KDecorationDefines::ColorFrame; ++t)
            colors[a][t] = options()->color((KDecorationDefines::ColorType)t, a);

      if (!(isPreview() || _factory->forceUserColors())) {
      uint info;
      if (XProperty::get(windowId(), XProperty::bgInfo, info)) {
         XProperty::decode(info, colors[0][ColorTitleBlend], colors[0][ColorButtonBg], bgMode);
         colors[0][ColorButtonBg] =
         Colors::mid(colors[0][ColorTitleBlend], colors[0][ColorButtonBg], 2, 1);
         XProperty::decode(info, colors[1][ColorTitleBlend], colors[1][ColorButtonBg], bgMode);
      }
      if (XProperty::get(windowId(), XProperty::inactInfo, info)) {
         XProperty::decode(info, colors[0][ColorTitleBar], colors[0][ColorFont], gType[0]);
         gType[0] = Gradients::fromInfo(gType[0]);
      }
      if (XProperty::get(windowId(), XProperty::actInfo, info)) {
         XProperty::decode(info, colors[1][ColorTitleBar], colors[1][ColorFont], gType[1]);
         gType[1] = Gradients::fromInfo(gType[1]);
      }
      }
   }

   if (changed) widget()->update();
   
}

void
Client::resize( const QSize& s )
{
   widget()->resize(s);

   if (_preview) {
      _preview->setGeometry(borderSize, titleSize,
                             s.width()-2*borderSize,
                             s.height()-(borderSize+titleSize));
   }
   
   top = QRect(0, 0, s.width(), titleSize);
   int x = qMax(s.width()/8, buttonSpace);
   int x2 = width() - x;

   bar = QPainterPath(QPoint(x, 0));
   bar.cubicTo( x+titleSize, 0,
                x+titleSize, titleSize,
                x+3*titleSize, titleSize-1);
   bar.lineTo( x2-3*titleSize, titleSize-1);
   bar.cubicTo( x2-titleSize, titleSize,
                x2-titleSize, 0,
                x2, 0);

   x += 2*titleSize;
   label = QRect(x, 0, width()-2*x, titleSize);

   bottom = QRect(0, height()-borderSize, width(), borderSize);
   const int sideHeight = height() - (titleSize + borderSize);
   left = QRect(0, titleSize, borderSize, sideHeight);
   right = QRect(width()-borderSize, titleSize, borderSize, sideHeight);
   if (maximizeMode() == MaximizeFull) {
      clearMask(); //repaint();
      return;
   }
   int w = width();
   int h = height();
   
   QRegion mask(4, 0, w-8, h);
   mask += QRegion(0, 4, w, h-8);
   mask += QRegion(2, 1, w-4, h-2);
   mask += QRegion(1, 2, w-2, h-4);

   setMask(mask);
   widget()->repaint(); // force! there're painting errors otherwise
}

void
Client::shadeChange()
{
}

void
Client::showWindowMenu(const QRect &r)
{
   showWindowMenu(r.bottomLeft());
}

void
Client::showWindowMenu(const QPoint &p)
{
   QPoint ip = p;
   QPoint gp = widget()->mapToGlobal(QPoint(width()-200, 0));
   if (ip.x() > gp.x()) ip.setX(gp.x());
   KDecoration::showWindowMenu(ip);
}

void
Client::toggleOnAllDesktops()
{
   KDecoration::toggleOnAllDesktops();
   emit stickyChanged(isOnAllDesktops());
}

static bool
isBrowser(const QString &s)
{
   return !s.compare("konqueror", Qt::CaseInsensitive) ||
   !s.compare("opera", Qt::CaseInsensitive) ||
   !s.compare("firefox", Qt::CaseInsensitive) ||
   !s.compare("mozilla", Qt::CaseInsensitive) ||
   !s.compare("safari", Qt::CaseInsensitive); // just in case ;)
}

QString
Client::trimm(const QString &string)
{
   if (!_factory->trimmCaption()) return string;

   /* Ok, *some* apps have really long and nasty window captions
   this looks clutterd, so we allow to crop them a bit and remove
   considered uninteresting informations ======================= */

   QString ret = string;

   /* 1st off: we assume the part beyond the last dash (if any) to be the
   uninteresting one (usually it's the apps name, if there's add info, that's
   more important to the user) ------------------------------------- */

   // ok, here's currently some conflict
   // in e.g. amarok, i'd like to snip "amarok" and preserver "<song> by <artist>"
   // but for e.g. k3b, i'd like to get rid of stupid
   // "the kde application to burn cds and dvds" ...
   if (ret.contains(" - "))
      ret = ret.section(" - ", 0, -2, QString::SectionSkipEmpty );

   /* next, if there're details, cut of by ": ",
   we remove the general part as well ------------------------------*/
   if (ret.contains(": "))
      ret = ret.section(": ", 1, -1, QString::SectionSkipEmpty );

   KWindowInfo info(windowId(), 0, NET::WM2WindowClass);
   QString appName(info.windowClassName());

   /* Browsers set the title to <html><title/></html> - appname
   Now the page titles can be ridiculously looooong, especially on news
   pages, where it consists of article - section - page
   we drop anything, but the page.... ------------------------*/
   if (isBrowser(appName)) {
      if (ret.contains(" - "))
         ret = ret.section(" - ", -1, -1, QString::SectionSkipEmpty);
      /* if this is a url, please get rid of http:// and /file.html ------- */
      if (ret.contains("http://"))
         ret = ret.remove("http://", Qt::CaseInsensitive)/*.section()*/;
   }
   /* last: if the remaining string still contains the app name, please shape
   away additional ifon like compile time, version numbers etc. ------------ */
   else {
      // TODO maybe check with regexp and word bounds?
      int i = ret.indexOf(appName, 0, Qt::CaseInsensitive);
      if (i > -1)
         ret = ret.mid(i, appName.length());
   }

   /* in general, remove leading and ending blanks... */
   return ret.trimmed();
   
//    KWindowInfo info(windowId(), NET::WMVisibleName | NET::WMName, NET::WM2WindowClass);
//    qDebug() << "BESPIN:" << info.windowClassClass() <<  << info.name() << info.visibleName();
//    QByteArray windowClassClass() const;
//    QByteArray windowClassName() const;
//    QString visibleName() const;
}
