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
#include <QAction>
#include <QPainter>
#include <QPaintEvent>
#include <QHBoxLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <QX11Info>
#include <QtDebug>

#include <kwindowinfo.h>
#include <kwindowsystem.h>

#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xatom.h>
#include "../fixx11h.h"

#include "button.h"
#include "../colors.h"
#include "../gradients.h"
#include "../xproperty.h"
#include "resizecorner.h"
#include "client.h"

using namespace Bespin;

Client::Client(KDecorationBridge *b, Factory *f) :
KDecoration(b, f), retry(0),
topTile(0), btmTile(0), cnrTile(0), lCorner(0), rCorner(0),
bgMode(1), _factory(f), corner(0) { }

Client::~Client(){
//    delete corner;
//    delete [] buttons;
//    delete titleBar;
//    delete titleSpacer;
}

void
Client::updateStylePixmaps()
{
    if (XProperty::get(windowId(), XProperty::topTile, topTile))
    {
        XProperty::get(windowId(), XProperty::btmTile, btmTile);
        XProperty::get(windowId(), XProperty::cnrTile, cnrTile);
        XProperty::get(windowId(), XProperty::lCorner, lCorner);
        XProperty::get(windowId(), XProperty::rCorner, rCorner);
        widget()->update();
    }
    else
    {
        topTile = btmTile = cnrTile = lCorner = rCorner = 0;
        if ((!retry || sender()) && retry < 100)
        {
            QTimer::singleShot(100, this, SLOT(updateStylePixmaps()));
            ++retry;
        }
    }
}

void
Client::activeChange()
{
    if (gType[0] != gType[1])
        updateTitleLayout(widget()->size());
    if (bgMode > 1)
    {
        updateStylePixmaps();
    }
    if (corner)
    {
        corner->setColor(color(ColorTitleBar, isActive()));
        corner->update();
    }
    widget()->update();
}

void
Client::addButtons(const QString& s, int &sz, bool left)
{
    sz = 4;
    if (!s.length() > 0) return;
    Button::Type type;
    for (int n=0; n < s.length(); ++n)
    {
        switch (s[n].toAscii())
        {
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
        if (!buttons[type])
        {   // will be d'played d'abled in case
            buttons[type] = new Button(this, type, left);
            titleBar->addWidget(buttons[type], 0, Qt::AlignVCenter);
            sz += (buttonSize()+2);
        }
    }
}

void
Client::borders( int& left, int& right, int& top, int& bottom ) const
{
    // KWin seems to call borders() before maximizeChange() in case
    // this may be a bug but is annoying at least - TODO: kwin bug report?
    if (maximizeMode() == MaximizeFull)
    {
        left = right = bottom = options()->moveResizeMaximizedWindows() ? 4 : 0;
        top = _factory->titleSize(true);
    }
    else
    {
        left = right = bottom =  _factory->borderSize();
        top = _factory->titleSize(_small);
        if (isShade())
            bottom = 8;
    }
    //    left = right = bottom = borderSize;
    //    top = titleSize;
}

void
Client::captionChange()
{
    _caption = trimm(caption());
    _caption.replace("[modified]", "*");
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
    if (o != widget())
        return false;
    switch (e->type())
    {
    case QEvent::Paint:
    {
        QPainter p(widget());
        p.setClipRegion(static_cast<QPaintEvent*>(e)->region());
        p.setFont(options()->font());
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
    default:
        return false;
    }
    return false;
}

static const unsigned long
supported_types =   NET::NormalMask | NET::DesktopMask | NET::DockMask | NET::ToolbarMask |
                    NET::MenuMask | NET::DialogMask | NET::OverrideMask | NET::TopMenuMask |
                    NET::UtilityMask | NET::SplashMask;

void
Client::init()
{
    createMainWidget();
    NET::WindowType type = windowType( supported_types );
    _small = type == NET::Utility || type == NET::Menu || type == NET::Toolbar;

    _caption = isPreview() ? (isActive() ? "Active Window" : "Inactive Window") : trimm(caption());
    widget()->setAutoFillBackground(false);
    widget()->setAttribute(Qt::WA_OpaquePaintEvent, !isPreview());
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

    for (int i = 0; i < 4; ++i)
        buttons[i] = 0;
    gType[0] = Gradients::None;
    gType[1] = Gradients::Button;

    if (config()->resizeCorner && isResizable())
        corner = new ResizeCorner(this);
    reset(63);
}

void
Client::maximizeChange()
{
    reset(SettingBorder);
}

#define PARTIAL_MOVE 0

KDecorationDefines::Position
Client::mousePosition( const QPoint& p ) const
{
    if (!isResizable())
        return PositionCenter;
   
    if (p.y() < 4)
    {   // top
        if (p.x() < 4) return PositionTopLeft; // corner
        if (p.x() > width() - 4) return PositionTopRight; // corner
        return PositionTop;
    }
    if (p.y() > height() - 16)
    {   // bottom
        if (p.x() < 16) return PositionBottomLeft; // corner
        if (p.x() > width() - 16) return PositionBottomRight; // corner
#if PARTIAL_MOVE
        int off = width()/3;
        if (p.x() > off && p.x() < width() - off) return PositionBottom;
        return PositionCenter; // not on outer 3rds
#else
        return PositionBottom;
#endif
    }
    if (p.x() < 4)
    {   // left
#if PARTIAL_MOVE
        int off = height()/3;
        if (p.y() > off && p.y() < height() - off) return PositionLeft;
        return PositionCenter; // not on outer 3rds
#else
        return PositionLeft;
#endif
    }
    if (p.x() > width() - 4)
    {   // right
#if PARTIAL_MOVE
        int off = height()/3;
        if (p.y() > off && p.y() < height() - off) return PositionRight;
        return PositionCenter; // not on outer 3rds
#else
        return PositionRight;
#endif
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
    if (!Factory::initialized())
        return;

    QColor bg = color(ColorTitleBar, isActive());
    if (isPreview())
    {
        QRect r = widget()->rect();
        p.setRenderHint( QPainter::Antialiasing );
        // the shadow - this is rather expensive, but hey - who cares... ;-P
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0,0,0,40));
        p.drawRoundedRect(r, 8, 8);
        r.adjust(1,0,-1,-1);
        p.drawRoundedRect(r, 7, 7);
        r.adjust(1,0,-1,-1);
        p.setBrush(QColor(0,0,0,20));
        p.drawRoundedRect(r, 6, 6);
        r.adjust(0,0,0,-1);
        p.drawRoundedRect(r, 6, 6);
        r.adjust(0,0,0,-1);
        
        // the window
        p.setBrush(Gradients::pix(bg, r.height(), Qt::Vertical, Gradients::Button));
        QColor c = color(ColorFont, isActive());
        c.setAlpha(80);
        p.setPen(c);
        p.drawRoundedRect(r, 6, 6);
        
        // logo
        if (isActive())
        {
            c.setAlpha(180);
            p.setBrush(c);
            p.setPen(Qt::NoPen);
            int s = qMin(r.width(), r.height())/2;
            QRect logo(0,0,s,s);
            s /= 4;
            logo.moveCenter(r.center());
            QPainterPath path;
            path.moveTo(logo.center());
            path.arcTo(logo, 90, 270);
            path.lineTo(logo.right(), logo.y()+4*s/3);
            path.lineTo(logo.right()-s, logo.y()+4*s/3);
            path.lineTo(logo.center().x() + s/2, logo.center().y());
            path.lineTo(logo.center());
            path.closeSubpath();
            path.addEllipse(logo.right()-3*s/2, logo.y(), s, s);
            p.drawPath(path);
        }
        c.setAlpha(255);
        p.setPen(c);
        p.setBrush(Qt::NoBrush);
        p.drawText(r, Qt::AlignHCenter | Qt::TextSingleLine | Qt::AlignTop, _caption);
    }
    else if (isShade())
    { // only one "big" gradient, as we can't rely on windowId()!!
        const QPixmap &fill = Gradients::pix(bg, height(), Qt::Vertical, Gradients::Button);
        p.drawTiledPixmap(0,0,width(),height(), fill);
    }
    else
    {   // window ================
        p.setBrush(bg); p.setPen(Qt::NoPen);
        switch (bgMode)
        {
        case 2: // vertical gradient
        case 3:
        {   // horizontal gradient
            if (!topTile)
            {   // hmm? paint fallback
                p.drawRect(left); p.drawRect(right); p.drawRect(top); p.drawRect(bottom);
                // and wait for pixmaps
                updateStylePixmaps();
                break;
            }
            
#define ctWidth 32
#define ctHeight 128
            if (bgMode == 2)
            {
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
                if (Colors::value(bg) < 245)
                {   // no sense otherwise
                    const int w = width()/4 - 128;
                    if (w > 0)
                    {
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
            else
            {
#define tbWidth 256
#define tbHeight 32
#define lrcWidth 256
#define lrcHeight 32
                p.drawRect(top); // can be necessary for flat windows
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
//       p.setPen(bg);
//       p.drawLine(width()/4, titleSize-1, 3*width()/4, titleSize-1);
            break;
        }
        case 0:
        {   // plain
            p.setBrush(bg); p.setPen(Qt::NoPen);
            p.drawRect(left); p.drawRect(right);
            p.drawRect(top); p.drawRect(bottom);
            break;
        }
        default:
        case 1:
        {   // scanlines, fallback
            p.drawRect(left); p.drawRect(right); p.drawRect(bottom);
            const QPixmap &fill = Gradients::pix(bg, titleSize, Qt::Vertical, config()->gradient[0][isActive()]);
            p.drawTiledPixmap(top, fill);
            p.setPen(Colors::mid(bg, Qt::black,6,1));
            p.drawLine(0,titleSize-1,width(),titleSize-1);
//       p.setPen(Colors::mid(bg, Qt::white,6,1));
//       p.drawLine(0,titleSize-1,width(),titleSize-1);

            Gradients::Type titleGradient = (Gradients::Type)gType[isActive()];
            if (titleGradient && label.width())
            {   // nice deco
                p.setRenderHint( QPainter::Antialiasing );
                bg = color(ColorTitleBlend, isActive());
                const QPixmap &fill = Gradients::pix(bg, titleSize, Qt::Vertical, titleGradient);
                const QColor shadow = Colors::mid(bg, Qt::black,6,1);
                p.setPen(QPen(shadow, 2)); p.setBrush(fill);
                p.drawRoundRect(label.adjusted(0,4,0,-4),titleSize*99/label.width(),99);
                p.setRenderHint( QPainter::Antialiasing, false );
            }
            break;
        }
        }
    }

    
    if (isShade())
    {   // splitter
        QColor bg2 = color(ColorTitleBlend, isActive());
        p.setPen(Colors::mid(bg, Qt::black, 3, 1));
        int y = titleSize-2;
        p.drawLine(8, y, width()-8, y);
        ++y;
        p.setPen(Colors::mid(bg, Qt::white, 2, 1));
        p.drawLine(8, y, width()-8, y);
    }

    // title ==============
    if (isActive())
    {   // emboss?!
        int d = 0;
        if (Colors::value(bg) < 110) // dark bg -> dark top borderline
            { p.setPen(Colors::mid(bg, Qt::black, 2, 1)); d = -1; }
        else // bright bg -> bright bottom borderline
            { p.setPen(Colors::mid(bg, Qt::white)); d = 1; }
        p.drawText ( label.translated(0,d), Qt::AlignCenter | Qt::TextSingleLine, _caption );
    }
    p.setPen(color((isShade() && bgMode == 1) ? ColorButtonBg : ColorFont, isActive()));
    p.drawText ( label, Qt::AlignCenter | Qt::TextSingleLine, _caption );

    // bar =========================
    if (bgMode != 1)
    {
        const QColor bg2 = color(ColorTitleBlend, isActive());

        if (gType[isActive()])
        {   // button corner
            QColor shadow = Colors::mid(bg2, Qt::black,4,1);
            const QPixmap &fill = Gradients::pix(bg2, titleSize, Qt::Vertical, (Gradients::Type)gType[isActive()]);
            p.setPen(shadow);
            p.setBrush(fill);
            p.setRenderHint( QPainter::Antialiasing );
            p.drawPath(buttonCorner);
        }
        
        if ((bg2 != bg) && (borderSize || gType[isActive()]))
        {   // outline
            p.setBrush(Qt::NoBrush);
            p.setRenderHint( QPainter::Antialiasing );
            p.setPen(QPen(bg2,3));
            if (borderSize)
                p.drawRect(1,1,width()-2,height()-2);
            else
                p.drawLine(0,1,width(),1);
        }
        else if (borderSize)
        {   // static bool KWindowSystem::compositingActive();
            // frame ==============
            const QColor border = Colors::mid(bg, color(ColorButtonBg, true),2,1);
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
        }
    }
}

void
Client::reset(unsigned long changed)
{
    if (changed & SettingFont)
    {
        titleSize = _factory->titleSize(_small);
        titleSpacer->changeSize( 1, titleSize, QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    if (changed & SettingDecoration)
    {
        gType[0] = config()->gradient[1][0];
        gType[1] = config()->gradient[1][1];
        changed |= SettingColors;
    }

    if (changed & SettingBorder)
    {
        if (maximizeMode() == MaximizeFull)
        {
            if (options()->moveResizeMaximizedWindows())
                borderSize = 4;
            else
            {
                borderSize = 0;
                if (corner)
                    corner->hide();
            }
            titleSize = _factory->titleSize(true);
        }
        else
        {
            borderSize = _factory->borderSize();
            titleSize = _factory->titleSize(_small);
            if (corner)
                corner->show();
        }

        bottom = QRect(0, height()-borderSize, width(), borderSize);
        const int sideHeight = height() - (titleSize + borderSize);
        left = QRect(0, titleSize, borderSize, sideHeight);
        right = QRect(width()-borderSize, titleSize, borderSize, sideHeight);

        int decoDim =   ((borderSize & 0xff) << 24) | ((titleSize &0xff) << 16) |
                        ((borderSize &0xff) << 8) | (borderSize & 0xff);
        XProperty::set(windowId(), XProperty::decoDim, decoDim);
        titleSpacer->changeSize( 1, titleSize, QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    if (changed & SettingButtons)
    {
        for (int i = 0; i < 4; ++i)
            { delete buttons[i]; buttons[i] = 0; }
        titleBar->removeItem(titleSpacer);
        addButtons(options()->titleButtonsLeft(), buttonSpaceLeft, true);
        titleBar->addItem(titleSpacer);
        addButtons(options()->titleButtonsRight(), buttonSpaceRight, false);
        buttonSpace = qMax(buttonSpaceLeft, buttonSpaceRight);
    }

    if ((changed & (SettingButtons | SettingFont)) && (buttonSpaceLeft >= buttonSpaceRight))
        updateButtonCorner(false);

    if (changed & SettingColors)
    {   // colors =====================
        for (int a = 0; a < 2; ++a)
        for (int t = 0; t < 4; ++t)
            colors[a][t] = options()->color((ColorType)t, a);

        bool def = (bgMode == 1);
        if (isPreview())
        {
            def = false;
            bgMode = 0;
            gType[0] = gType[1] = Gradients::None;
            colors[0][ColorTitleBlend] = colors[1][ColorTitleBlend] =
                                         colors[0][ColorTitleBar] = colors[1][ColorTitleBar] =
                                         widget()->palette().color(QPalette::Active, QPalette::Window);
            colors[1][ColorButtonBg] = colors[1][ColorFont] =
                                       widget()->palette().color(QPalette::Active, QPalette::WindowText);
            colors[0][ColorButtonBg] = colors[0][ColorFont] =
                                     Colors::mid(colors[0][ColorTitleBar], colors[1][ColorFont]);
        }
        else if (!config()->forceUserColors)
        {
            KWindowInfo info(windowId(), 0, NET::WM2WindowClass);
            if (info.windowClassClass() == "Wine")
            {   // this is a Wine window, make ungly win grey (everything 'bout win is ugly... ;-)
                QColor winGrey(192,192,192);
                colors[0][ColorTitleBlend] = colors[0][ColorTitleBar] = winGrey;
                colors[1][ColorTitleBlend] = colors[1][ColorTitleBar] = Qt::black;
                colors[0][ColorButtonBg] = colors[0][ColorFont] = Colors::mid(winGrey, QColor(0,0,0));
                colors[1][ColorButtonBg] = colors[1][ColorFont] = Qt::white;
                bgMode = 1;
            }
            else
            {   // nope, but maybe stylecontrolled
                uint bgColors, actColors, inactColors, pid;
                bool b1 = false, b2 = false, b3 = false;
                if (XProperty::get(windowId(), XProperty::pid, pid) &&
                    factory()->hasDecoInfo(pid, bgColors, actColors, inactColors))
                    { b1 = b2 = b3 = true; }
                else
                {
                    b1 = XProperty::get(windowId(), XProperty::bgInfo, bgColors);
                    b2 = XProperty::get(windowId(), XProperty::actInfo, actColors);
                    b3 = XProperty::get(windowId(), XProperty::inactInfo, inactColors);
                }
                if (b1)
                {
                    def = false;
                    XProperty::decode(bgColors, colors[1][ColorTitleBar], colors[1][ColorFont], bgMode);
                    colors[0][ColorTitleBar] = colors[1][ColorTitleBar];
                    colors[0][ColorFont] = Colors::mid(colors[1][ColorTitleBar], colors[1][ColorFont],2,1);
                }
                if (b2)
                {
                    XProperty::decode(actColors, colors[1][ColorTitleBlend], colors[1][ColorButtonBg], gType[1]);
                    gType[1] = Gradients::fromInfo(gType[1]);
                }
                if (b3)
                {
                    XProperty::decode(inactColors, colors[0][ColorTitleBlend], colors[0][ColorButtonBg], gType[0]);
                    gType[0] = Gradients::fromInfo(gType[0]);
                }
            }
        }
        if (def)
        {   // the fallback solution
            for (int i = 0; i <  2; ++i)
            {
                if (!gType[i])
                {   //buttoncolor MUST be = titlefont
                    colors[i][ColorTitleBlend] = colors[i][ColorTitleBar];
                    colors[i][ColorButtonBg] = colors[i][ColorFont];
                }
                // usually the window is titlebar colored and the titleblend gradient painted upon - in case
                // but the fallback shall be fully titleblend with a titlebar color section behind the title
                // to not have to handle this during the painting, we just swap the colors here
                else
                {
                    QColor h = colors[i][ColorTitleBlend];
                    colors[i][ColorTitleBlend] = colors[i][ColorTitleBar];
                    colors[i][ColorTitleBar] = h;
                }
            }
        }
        else if (bgMode == 1)
        {
            // iff the user set a colormode from the style, but no gradient, we use the color on 
            // the default gradient and NOT the nonexisting accessoire
            for (int i = 0; i <  2; ++i)
            {
                if (gType[i] == Gradients::None)
                {
                    colors[i][ColorTitleBar] = colors[i][ColorTitleBlend];
                    colors[i][ColorFont] = colors[i][ColorButtonBg];
                }
                else
                {   // needs titlefont and button bg swapped...
                    QColor h = colors[i][ColorButtonBg];
                    colors[i][ColorButtonBg] = colors[i][ColorFont];
                    colors[i][ColorFont] = h;
                }
            }
        }
        // last, clamp ColorTitleBlend to v >= 80
        int h,s,v;
        for (int i = 0; i <  2; ++i)
        {
            v = Colors::value(colors[i][ColorTitleBlend]);
            if (v < 80)
            {
                colors[i][ColorTitleBlend].getHsv(&h,&s,&v);
                colors[i][ColorTitleBlend].setHsv(h,s,80);
            }
        }
    }

    if (changed & (SettingColors | SettingButtons))
    {
        // buttons[i]->isOnTitleBar att indicates a left side button
        for (int i = 0; i < 4; ++i)
            if (bgMode == 1)
                buttons[i]->isOnTitleBar = !gType[true];
            else
                buttons[i]->isOnTitleBar = !gType[true] ||
                                           (!buttons[i]->isOnTitleBar && buttonSpaceLeft >= buttonSpaceRight) ||
                                           (buttons[i]->isOnTitleBar && buttonSpaceLeft < buttonSpaceRight);
    }

    if (changed)
        activeChange(); // handles bg pixmaps in case and triggers update
   
}

void
Client::updateButtonCorner(bool right)
{
    const int ts = titleSize;
    if (right)
    {
        const int w = width();
        const int dr = buttonSpaceRight + titleSize;
        const int bs = buttonSpaceRight;
        buttonCorner = QPainterPath(QPoint(w, -1)); //tl corner
        buttonCorner.lineTo(w, ts); // straight to br corner
        buttonCorner.lineTo(w - bs + ts/2, ts); // straight to bl offset
        buttonCorner.cubicTo(w - dr + ts/2, ts+2,   w-bs, 0,   w-dr, -1); // curve to tr offset
    }
    else
    {
        const int dl = buttonSpaceLeft + titleSize;
        const int bs = buttonSpaceLeft;
        buttonCorner = QPainterPath(QPoint(0, -1)); //tl corner
        buttonCorner.lineTo(dl, -1); // straight to tl end
        buttonCorner.cubicTo(bs, 0,   dl - titleSize/2, ts+2,   bs - ts/2, ts); // curve to bl offset
        buttonCorner.lineTo(0, ts); // straight to bl end
    }
}


void
Client::updateTitleLayout( const QSize& )
{
    if (bgMode != 1 && (gType[0] || gType[1]))
    {
        if (buttonSpaceLeft <= buttonSpaceRight)
            updateButtonCorner(true);
        label.setRect(buttonSpaceLeft + titleSize, 0, width()-(buttonSpace+2*titleSize), titleSize);
    }
    else
    {
        int d = buttonSpace + 8;
        label = QRect(d, 0, width()-2*d, titleSize);
    }
}

void
Client::resize( const QSize& s )
{
    widget()->resize(s);
    int w = s.width(), h = s.height();

    updateTitleLayout( s );

    top = QRect(0, 0, w, titleSize);
    bottom = QRect(0, h-borderSize, w, borderSize);
    const int t2 = 2*titleSize/3;
    const int sideHeight = h - borderSize;
    left = QRect(0, t2, borderSize, sideHeight);
    right = QRect(w-borderSize, t2, borderSize, sideHeight);

    if (maximizeMode() == MaximizeFull)
        { clearMask(); /*repaint();*/ return; }

    int d = (isShade() || borderSize > 5) ? 8 : 4;
    QRegion mask(4, 0, w-8, h);
    mask += QRegion(0, 4, w, h-d);
    mask += QRegion(2, 1, w-4, h-d/4);
    mask += QRegion(1, 2, w-2, h-d/2);

    setMask(mask);
    widget()->repaint(); // force! there're painting errors otherwise
}

void
Client::shadeChange()
{
   KDecoration::toggleOnAllDesktops();
   emit shadeChanged(isSetShade());
}

void
Client::showDesktopMenu(const QPoint &p)
{
   QPoint ip = p;
   QPoint gp = widget()->mapToGlobal(QPoint(width()-60, 0));
   if (ip.x() > gp.x()) ip.setX(gp.x());
   _factory->showDesktopMenu(ip, this);
}

void
Client::showWindowList(const QPoint &p)
{
   QPoint ip = p;
   QPoint gp = widget()->mapToGlobal(QPoint(width()-200, 0));
   if (ip.x() > gp.x()) ip.setX(gp.x());
   _factory->showWindowList(ip, this);
}

void
Client::showInfo(const QPoint &p)
{
   QPoint ip = p;
   QPoint gp = widget()->mapToGlobal(QPoint(width()-320, 0));
   if (ip.x() > gp.x()) ip.setX(gp.x());
   _factory->showInfo(ip, windowId());
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
Client::activate() {
   if (QAction *act = qobject_cast<QAction*>(sender())) {
      bool ok; int id = act->data().toUInt(&ok);
      if (ok) {
         KWindowSystem::activateWindow( id );
         return;
      }
   }
   KWindowSystem::activateWindow( windowId() );
}

void
Client::throwOnDesktop() {
   if (QAction *act = qobject_cast<QAction*>(sender())) {
      bool ok;
      int desktop = act->data().toInt(&ok);
      if (ok) setDesktop(desktop);
   }
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
    if (!config()->trimmCaption) return string;

    /* Ok, *some* apps have really long and nasty window captions
    this looks clutterd, so we allow to crop them a bit and remove
    considered to be uninteresting informations ==================== */

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

    KWindowInfo info(windowId(), 0, NET::WM2WindowClass);
    QString appName(info.windowClassName());

    /* Browsers set the title to <html><title/></html> - appname
    Now the page titles can be ridiculously looooong, especially on news
    pages --------------------------------- */
    if (isBrowser(appName))
    {
        int n = qMin(2, ret.count(" - "));
        if (n--) // select last two if 3 or more sects, prelast otherwise
            ret = ret.section(" - ", -2, n-2, QString::SectionSkipEmpty);
    }

    /* next, if there're details, cut of by ": ",
    we remove the general part as well ------------------------------*/
    if (ret.contains(": "))
        ret = ret.section(": ", 1, -1, QString::SectionSkipEmpty );

    /* if this is a url, please get rid of http:// and /file.html ------- */
    if (ret.contains("http://"))
        ret = ret.remove("http://", Qt::CaseInsensitive)/*.section()*/;

    /* last: if the remaining string still contains the app name, please shape
    away additional info like compile time, version numbers etc. ------------ */
    else {
        // TODO maybe check with regexp and word bounds?
        int i = ret.indexOf(appName, 0, Qt::CaseInsensitive);
        if (i > -1)
            ret = ret.mid(i, appName.length());
    }

    ret = ret.trimmed();
    if (ret.isEmpty())
        ret = string; // ...

    /* in general, remove leading and ending blanks... */
    return ret;
   
//    KWindowInfo info(windowId(), NET::WMVisibleName | NET::WMName, NET::WM2WindowClass);
//    qDebug() << "BESPIN:" << info.windowClassClass() <<  << info.name() << info.visibleName();
//    QByteArray windowClassClass() const;
//    QByteArray windowClassName() const;
//    QString visibleName() const;
}
