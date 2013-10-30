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

// #include <KGlobal>
#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QHBoxLayout>
#include <QTimer>
#include <QVBoxLayout>
#include <QX11Info>
#include <QtDebug>

#include <kdeversion.h>
#include <klocale.h>
#include <kwindowinfo.h>
#include <kwindowsystem.h>
#include <netwm.h>

#include <cmath>

#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <X11/Xatom.h>
#include "../fixx11h.h"

#include "../blib/colors.h"
#include "../blib/FX.h"
#include "../blib/gradients.h"
#include "../blib/xproperty.h"
#include "../blib/shadows.h"
#include "../blib/shapes.h"
#include "../blib/WM.h"
#include "button.h"
#include "resizecorner.h"
#include "client.h"

using namespace Bespin;
static QStringList kwin_seps;

Client::Client(KDecorationBridge *b, Factory *f) :
KDecoration(b, f), retry(0), myButtonOpacity(0), myActiveChangeTimer(0),
myBgMode(Factory::defaultBgMode()), myResizeHandle(0), myBgSet(0)
{
    for (int i = 0; i < 5; ++i)
        myTiles[i] = 0;
    setParent( f );
}

Client::~Client()
{
    if (myActiveChangeTimer) {
        killTimer(myActiveChangeTimer);
        myActiveChangeTimer = 0;
    }
    if (myBgSet)
    {
        if (! --myBgSet->set->clients)
            Factory::kickBgSet(myBgSet->hash);
        delete myBgSet;
    }
#if KDE_IS_VERSION(4,7,0)
    if (!Factory::initialized()) // factory being deconstructed
        Bespin::Shadows::set(windowId(), Bespin::Shadows::None, false);
#endif
//    delete myResizeHandle;
//    delete [] myButtons;
//    delete myTitleBar;
//    delete myTitleSpacer;
}


void
Client::updateStylePixmaps()
{
    for (int i = 0; i < 5; ++i)
        myTiles[i] = 0;

    if (Factory::config()->forceUserColors)
        return; // "no"

    unsigned long _5 = 5;
    if (WindowPics *pics = (WindowPics*)XProperty::get<Picture>(windowId(), XProperty::bgPics, XProperty::LONG, &_5)) {
        if (FX::usesXRender() && (myTiles[Bg::Top] = pics->topTile)) {
            if (myBgSet) {
                if (! --myBgSet->set->clients)
                    Factory::kickBgSet(myBgSet->hash);
                delete myBgSet;
                myBgSet = 0;
            }
            myTiles[Bg::Bottom]         = pics->btmTile;
            myTiles[Bg::Corner]         = pics->cnrTile;
            myTiles[Bg::LeftCorner]     = pics->lCorner;
            myTiles[Bg::RightCorner]    = pics->rCorner;
        // no XRender means we cannot share pictures
        // let's hope the clients doesn't try to send some
        } else if (myBgMode == Bg::Structure) {
            /// NOTICE encoding the bg structure & intensity in the btmTile Pic!!
            myTiles[Bg::Bottom] = pics->btmTile;
            /// NOTICE encoding the bg dims in the cnrTile Pic!!
            myTiles[Bg::Corner] = pics->cnrTile;
            /// -------------------------
            myTiles[Bg::Top] = myTiles[Bg::LeftCorner] = myTiles[Bg::RightCorner]  = 0;
        }
        else if (myBgMode > Bg::Gradient) {
            qint64 hash = 0;
            /// NOTICE style encodes the intensity in pics->btmTile!
            BgSet *set = Factory::bgSet(myColors[isActive()][0], myBgMode == Bg::VerticalGradient, pics->btmTile, &hash);
            if (!myBgSet) {
                ++set->clients;
                myBgSet = new Bg;
                myBgSet->hash = hash;
                myBgSet->set = set;
            }
            else if (myBgSet->hash != hash) {
                if (! --myBgSet->set->clients)
                    Factory::kickBgSet(myBgSet->hash);
                ++set->clients;
                myBgSet->set = set;
                myBgSet->hash = hash;
            }
            if (FX::usesXRender()) {
                myTiles[Bg::Top]            = set->topTile.x11PictureHandle();
                myTiles[Bg::Bottom]         = set->btmTile.x11PictureHandle();
                myTiles[Bg::Corner]         = set->cornerTile.x11PictureHandle();
                myTiles[Bg::LeftCorner]     = set->lCorner.x11PictureHandle();
                myTiles[Bg::RightCorner]    = set->rCorner.x11PictureHandle();
            }
            else
                for (int i = 0; i < 5; ++i)
                    myTiles[i] = -1;
        }
        XFree(pics);
    }
    if (myTiles[Bg::Top] || myTiles[Bg::Bottom])
        widget()->update();
    else {
        if ((!retry || sender()) && retry < 50) {
            QTimer::singleShot(100+10*retry, this, SLOT(updateStylePixmaps()));
            ++retry;
        }
    }
}

void
Client::updateUnoHeight()
{
#if 0
    // NOTICE: this should be superflous!
    WindowData *data = (WindowData*)XProperty::get<uint>(windowId(), XProperty::winData, XProperty::WORD, 9);
    if (data)
    {
        myUnoHeight = ((data->style >> 24) & 0xff);
        widget()->update();
        XFree(data);
    }
#endif
}

void
Client::activeChange(bool realActiveChange)
{
    if (myGradients[0] != myGradients[1])
        updateTitleLayout(widget()->size());
    if (myBgMode > Bg::Plain)
        updateStylePixmaps();
    if (realActiveChange) {
        fadeButtons();
#if KDE_IS_VERSION(4,7,0)
        if (Factory::variableShadowSizes())
            Bespin::Shadows::set(windowId(), isActive() ? Bespin::Shadows::Large : Bespin::Shadows::Small, true);
#endif
    }
//     if (myUnoHeight)
//         updateUnoHeight();
    if (myResizeHandle) {
        myResizeHandle->setColor(color(ColorTitleBar, isActive()));
        myResizeHandle->update();
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
            if (Factory::multiButtons().isEmpty())
                continue; // no button to insert
            type = Button::Multi; break;
        case 'I': // Minimize
            if (!(isMinimizable() || isMaximizable()))
                continue;
            type = Button::Min; break;
        case 'A': // Maximize
            if (!(isMinimizable() || isMaximizable()))
                continue;
            type = Button::Max; break;
        case 'X': // Close button
            type = Button::Close; break;
        case '_': // Spacer
            myTitleBar->addSpacing(5);
            sz += 7;
        default:
            continue;
        }
        if (!myButtons[type])
        {   // will be d'played d'abled in case
            myButtons[type] = new Button(this, type, left);
            // optimizes, but breaks with recent KDE/X/nvidia? combinations...
            if (!isPreview())
            {
//                 myButtons[type]->setAutoFillBackground(true);
//                 myButtons[type]->setAttribute(Qt::WA_OpaquePaintEvent);
//                 myButtons[type]->setAttribute(Qt::WA_PaintOnScreen);
                myButtons[type]->setAttribute(Qt::WA_NoSystemBackground);
            }
            myTitleBar->addWidget(myButtons[type], 0, Qt::AlignCenter);
            sz += (Factory::buttonSize(iAmSmall) + 2);
        }
    }
}

void
Client::borders( int& left, int& right, int& top, int& bottom ) const
{
    const MaximizeMode maximized = maximizeMode();
#if KDE_IS_VERSION(4,11,0)
    QuickTileMode quickTiled = quickTileMode();
    if (quickTiled == QuickTileMaximize) {
        quickTiled = QuickTileNone;
    }
#else
    const int quickTiled(0), QuickTileLeft(1), QuickTileRight(1), QuickTileTop(1), QuickTileBottom(1);
#endif
    const bool vertical = Factory::verticalTitle();
    if ((maximized & MaximizeHorizontal) || (quickTiled & QuickTileLeft))
        left = vertical ? Factory::buttonSize(iAmSmall) + 5 : 0;
    else
        left = vertical ? titleSize() : Factory::edgeSize();
    if ((maximized & MaximizeHorizontal) || (quickTiled & QuickTileRight))
        right = 0;
    else
        right = vertical ? Factory::baseSize() : Factory::edgeSize();
    if ((maximized & MaximizeVertical) || (quickTiled && (quickTiled & (QuickTileTop|QuickTileBottom)) != QuickTileBottom))
        top = vertical ? 0 : Factory::buttonSize(iAmSmall) + 5;
    else
        top = vertical ? Factory::edgeSize() : titleSize();
    if ((maximized & MaximizeVertical) || (quickTiled && (quickTiled & (QuickTileTop|QuickTileBottom)) != QuickTileTop))
        bottom = 0;
    else
        bottom = vertical ? Factory::edgeSize() : Factory::baseSize();

    if (isShade()) {
        vertical ? (right = left-4) : (bottom = top-4); // titlebar looses 2ps to splitter
    }

    if (myArea[Left].width() == left && myArea[Left].top() == top &&
        myArea[Right].width() == right && myArea[Bottom].height() == bottom)
        return;

    Client *that = const_cast<Client*>(this);
    that->myArea[Left].setWidth(left);
    that->myArea[Left].moveTop(top);
    that->myArea[Right].setWidth(right);
    that->myArea[Right].moveTop(top);
    that->myArea[Top].setHeight(top);
    that->myArea[Bottom].setHeight(bottom);
    if (vertical) {
        that->myTitleSpacer->changeSize( left, 1, QSizePolicy::Fixed, QSizePolicy::Expanding);
    } else {
        that->myTitleSpacer->changeSize( 1, top, QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    that->myTitleBar->invalidate();
    uint decoDim = ((left & 0xff) << 24) | ((top & 0xff) << 16) | ((right & 0xff) << 8) | (bottom & 0xff);
    XProperty::set<uint>(that->windowId(), XProperty::decoDim, &decoDim, XProperty::LONG);

#if 0
    int *title, *border, *counter;
    if (Factory::verticalTitle())
    {
        if (isShade())
        {
            top = widget()->height() - 2*myEdgeSize;
            left = Factory::titleSize(iAmSmall) + 8;
            right = Factory::titleSize(iAmSmall) + 8 - widget()->width();
            bottom = 0;
            return;
        }
        title = &left;
        border = &top;
        counter = &right;
    }
    else
        { title = &top; border = &left; counter = &bottom; }

    if (maximizeMode() == MaximizeFull)
    {
        if ( options()->moveResizeMaximizedWindows() )
        {
            *border = right = qMin(4, Factory::edgeSize());
            *counter =  qMin(4, Factory::baseSize());
        }
        else
            *border = right = *counter = 0;
    }
    else
    {
        *border = right = Factory::edgeSize();
        *counter = isShade() ? 14 : Factory::baseSize();
    }
    Client *that = const_cast<Client*>(this);
    that->updateTitleHeight(title);
    that->myTitleSize = *title;
    if (Factory::verticalTitle()) {
        that->myTitleSpacer->changeSize( myTitleSize, 1, QSizePolicy::Fixed, QSizePolicy::Expanding);
        that->myArea[Left].setWidth(myTitleSize);
    } else {
        that->myTitleSpacer->changeSize( 1, myTitleSize, QSizePolicy::Expanding, QSizePolicy::Fixed);
        that->myArea[Top].setHeight(myTitleSize);
    }
    that->myTitleBar->invalidate();
#endif
}

QRegion Client::region(KDecorationDefines::Region r)
{
    if (r != KDecorationDefines::ExtendedBorderRegion) // whatever that will be
        return QRegion();
    if (maximizeMode() == MaximizeFull || (Factory::baseSize() && Factory::edgeSize())) // not want || need input window
        return QRegion();
    if (!isResizable()) // nope
        return QRegion();
//     if (myResizeHandle) // TODO later - first we show everyone that this is now possible
//         return QRegion();
    const int edge = Factory::edgeSize() ? 0 : 3;
    const int base = Factory::baseSize() ? 0 : 3;
    if (Factory::verticalTitle()) // rotated titlebar, edge, base
        return QRegion(widget()->rect().adjusted(0,-edge,base,edge)) - widget()->rect();
    return QRegion(widget()->rect().adjusted(-edge,0,edge,base)) - widget()->rect();
}

int
Client::buttonBoxPos(bool active)
{
    DecoMode dm(NoDeco);
    if (Factory::buttonnyButton())
        dm = ButtonDeco;
    if (myGradients[active])
        dm = (myBgMode == Bg::Gradient && !myUnoHeight) ? CenterDeco : CornerDeco;

    if (dm != CornerDeco)
        return dm == CenterDeco ? 0 : 2;
    if (buttonSpaceLeft < buttonSpaceRight)
        return 1;
    return -1;
}

Gradients::Type
Client::buttonGradient(bool active)
{
    return myGradients[active] ? myGradients[active] : Factory::buttonGradient();
}

void
Client::captionChange()
{
    myCaption = trimm(caption());
    myCaption.replace(i18n("[modified]"), "*");
//     if (Factory::verticalTitle() && myButtons[Button::Multi])
//         myButtons[Button::Multi]->setToolTip(myCaption);
//     else
        widget()->update();
}

QColor
Client::color(ColorType type, bool active) const
{
    if (type < 4)
        return myColors[active][type];
    return options()->color(type, active);
}

Client::DecoMode
Client::decoMode() const
{
    if (Factory::buttonnyButton())
        return ButtonDeco;
    if (decoGradient())
        return (myBgMode == Bg::Gradient && !myUnoHeight) ? CenterDeco : CornerDeco;
    return NoDeco;
}

WM::Direction direction(const QRect &r, const QPoint &p)
{
    WM::Direction d = WM::Move;
    if (p.x() < r.width()/3) {
        d = WM::Left;
    } else if (p.x() > 2*r.width()/3) {
        d = WM::Right;
    }
    if (p.y() < r.height()/3) {
        d = (d == WM::Left) ? WM::TopLeft : ((d == WM::Right) ? WM::TopRight : WM::Top);
    } else if (p.y() > 2*r.height()/3) {
        d = (d == WM::Left) ? WM::BottomLeft : ((d == WM::Right) ? WM::BottomRight : WM::Bottom);
    }
    return d;
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
        const bool realWindow = !isPreview();
        QRegion clip = static_cast<QPaintEvent*>(e)->region();

#if 0
        QPaintDevice *dev = QPainter::redirected(widget());
        if (dev && dev->devType() == QInternal::Widget)
        qDebug() << static_cast<QWidget*>(dev) << static_cast<QWidget*>(dev)->geometry();
        else if (dev && dev->devType() == QInternal::Pixmap)
        qDebug() << "Pixmap" << static_cast<QPixmap*>(dev)->size();
#endif

        QPainter p(widget());
        p.setClipRegion(clip);
        // WORKAROUND a bug in QPaintEngine + QPainter::setRedirected
        if (dirty[isActive()])
        {
            dirty[isActive()] = false;
            repaint(p, decoMode() == CenterDeco);
        }
        repaint(p);

        if ( Factory::roundCorners() && realWindow && maximizeMode() != MaximizeFull && Factory::compositingActive() )
        {
            const bool full = isShade() || Factory::baseSize() > 3;
            const int sw = Factory::mask.width() / 2 + 1;
            const int sh = Factory::mask.height() / 2 + 1;
            const int sx = Factory::mask.width() - sw;
            const int sy = Factory::mask.height() - sh;
            const int x2 = widget()->rect().width() - sw;
            const int y2 = widget()->rect().height() - sh;
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);

            p.drawPixmap( 0,0, Factory::mask, 0,0, sw,sh ); // topLeft
            if ( full || !Factory::verticalTitle() )
                p.drawPixmap( x2,0, Factory::mask, sx,0, sw,sh ); // topRight
            if ( full || Factory::verticalTitle() )
                p.drawPixmap( 0,y2, Factory::mask, 0,sy, sw,sh ); // btmLeft
            if ( full )
                p.drawPixmap( x2,y2, Factory::mask, sx,sy, sw,sh ); // btmRight
        }

        p.end();

        if (realWindow)
        {
            QPixmap *buffer = 0;
            QPixmap dBuffer;
            QPoint off(0,0);

            if (color(ColorTitleBar, isActive()).alpha() == 0xff)
            {
                // try to copy from redirection
                if (FX::usesXRender())
                if ( QPaintDevice *device = QPainter::redirected(widget(), &off) )
                {
                    if (device->devType() == QInternal::Pixmap)
                        buffer = static_cast<QPixmap*>(device);
                }
                if (!buffer)
                {
                    // nope, repaint tp buffer
                    dBuffer = QPixmap(myTitleBar->geometry().size());
                    buffer = &dBuffer;
                    p.begin(buffer);
//                     p.setClipping(false);
                    p.setClipRegion(myTitleBar->geometry());
                    repaint(p, false);
                    p.end();
                }
            }

            for (int i = 0; i < 4; ++i)
                if (myButtons[i])
                {   // dump button BGs unless ARGB
                    myButtons[i]->setBg(buffer ? buffer->copy(myButtons[i]->geometry().translated(-off)) : dBuffer);
                    // enforce repaint, button thinks it's independend
                    myButtons[i]->repaint();
                }

            if (myResizeHandle)
                myResizeHandle->repaint();
        }
        return true;
    }
    case QEvent::Enter:
    case QEvent::Leave:
        if (!isActive())
            fadeButtons();
        return false;
    case QEvent::MouseMove:
        if (widget()->mouseGrabber() == widget()) {
            QMouseEvent *me = static_cast<QMouseEvent*>(e);
            Qt::CursorShape shape = Qt::ForbiddenCursor;
            if (widget()->rect().contains(me->pos())) {
                switch (direction(widget()->rect(), me->pos())) {
                case WM::TopLeft:
                case WM::BottomRight:
                    shape = Qt::SizeFDiagCursor; break;
                case WM::Top:
                case WM::Bottom:
                    shape = Qt::SizeVerCursor; break;
                case WM::TopRight:
                case WM::BottomLeft:
                    shape = Qt::SizeBDiagCursor; break;
                case WM::Right:
                case WM::Left:
                    shape = Qt::SizeHorCursor; break;
                default:
                case WM::Move:
                    shape = Qt::SizeAllCursor; break;
                }
            }
            if (!QApplication::overrideCursor() || shape != QApplication::overrideCursor()->shape()) {
                if (QApplication::overrideCursor())
                    QApplication::restoreOverrideCursor();
                QApplication::setOverrideCursor(shape);
            }
        }
        return false;
    case QEvent::MouseButtonDblClick:
        titlebarDblClickOperation();
        return true;
    case QEvent::MouseButtonPress: {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        if (widget()->mouseGrabber() == widget()) {
            QApplication::restoreOverrideCursor();
            widget()->releaseMouse();
            if (widget()->rect().contains(me->pos()))
                WM::triggerMoveResize(windowId(), me->globalPos(), direction(widget()->rect(), me->pos()));
        } else {
            processMousePressEvent(me);
        }
        return true;
    }
    case QEvent::Wheel:
        titlebarMouseWheelOperation(static_cast<QWheelEvent*>(e)->delta());
        return true;
        //       case QEvent::MouseButtonRelease:
    default:
        return false;
    }
    return false;
}


void
Client::fadeButtons()
{
    if (Factory::config()->hideInactiveButtons && !myActiveChangeTimer)
    {
        myActiveChangeTimer = startTimer(40);
        QTimerEvent te(myActiveChangeTimer);
        timerEvent(&te);
    }
}

static const unsigned long
supported_types =   NET::NormalMask | NET::DesktopMask | NET::DockMask | NET::ToolbarMask |
                    NET::MenuMask | NET::DialogMask | NET::OverrideMask | NET::TopMenuMask |
                    NET::UtilityMask | NET::SplashMask;

void
Client::init()
{
    createMainWidget();
#if KDE_IS_VERSION(4,7,0)
    Bespin::Shadows::set(windowId(), Bespin::Shadows::Large, true);
#endif
    dirty[0] = dirty[1] = true;

    KWindowInfo info(windowId(), NET::WMWindowType, NET::WM2WindowClass);
    NET::WindowType type = info.windowType( supported_types );
    iAmSmall = (type == NET::Utility || type == NET::Menu || type == NET::Toolbar) ||
                Factory::config()->smallTitleClasses.contains(info.windowClassName(), Qt::CaseInsensitive) ||
                Factory::config()->smallTitleClasses.contains(info.windowClassClass(), Qt::CaseInsensitive);

    if (isPreview())
        myCaption =  "Bespin";
    else if (!Factory::verticalTitle())
        myCaption = trimm(caption());
    widget()->setAutoFillBackground(false); // !isPreview());
    widget()->setAttribute(Qt::WA_OpaquePaintEvent, !isPreview());
    widget()->setAttribute(Qt::WA_NoSystemBackground, isPreview());
    widget()->setAttribute(Qt::WA_PaintOnScreen, !isPreview());
    widget()->setAttribute(Qt::WA_StyledBackground, false);
    widget()->installEventFilter(this);

    QBoxLayout *layout;
    if (Factory::verticalTitle())
    {
        myTitleBar = new QVBoxLayout();
        myTitleBar->setContentsMargins(0,4,0,4);
        layout = new QHBoxLayout(widget());
        myTitleSpacer = new QSpacerItem( myArea[Left].width(), 1, QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
    else
    {
        myTitleBar = new QHBoxLayout();
        myTitleBar->setContentsMargins(4,0,4,0);
        layout = new QVBoxLayout(widget());
        myTitleSpacer = new QSpacerItem( 1, myArea[Top].height(), QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    myTitleBar->setSpacing(2);

    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    layout->addLayout(myTitleBar);
    layout->addStretch(1000);

    for (int i = 0; i < 4; ++i)
        myButtons[i] = 0;
    myGradients[0] = Gradients::None;
    myGradients[1] = Gradients::Button;

    if (!isPreview() && Factory::config()->resizeCorner && isResizable())
        myResizeHandle = new ResizeCorner(this);
    reset(63);

    if (Factory::verticalTitle())
        captionChange();
}

void
Client::maximizeChange()
{
    emit maximizeChanged(maximizeMode() == MaximizeFull);
}

#define PARTIAL_MOVE 0

KDecorationDefines::Position
Client::mousePosition( const QPoint& p ) const
{
    if (isShade() || !isResizable())
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
    if (Factory::verticalTitle())
        return QSize(myArea[Left].width() + Factory::baseSize(), buttonSpaceLeft + buttonSpaceRight + 2*Factory::edgeSize());
    else
        return QSize(buttonSpaceLeft + buttonSpaceRight + 2*Factory::edgeSize(), myArea[Top].height() + Factory::baseSize());
}

#define DUMP_PICTURE(_PREF_, _PICT_, _SET_PICT_)\
if (FX::usesXRender()) { \
if (bg.alpha() != 0xff){\
    _PREF_##Buffer.detach();\
    _PREF_##Buffer.fill(Qt::transparent);\
    XRenderComposite(QX11Info::display(), PictOpOver, myTiles[_PICT_], 0, _PREF_##Buffer.x11PictureHandle(),\
    0, 0, 0, 0, 0, 0, _PREF_##Width, _PREF_##Height);\
}\
else {\
    _PREF_##Buffer.fill(bg);\
    XRenderComposite(QX11Info::display(), PictOpSrc, myTiles[_PICT_], 0, _PREF_##Buffer.x11PictureHandle(),\
    0, 0, 0, 0, 0, 0, _PREF_##Width, _PREF_##Height);\
}}\
else if (this->myBgSet && this->myBgSet->set) { \
    _PREF_##Buffer = this->myBgSet->set->_SET_PICT_; \
}

inline static void shrink(QFont &fnt, float factor)
{
    if (fnt.pointSize() > -1)
        fnt.setPointSize(qRound(fnt.pointSize()*factor));
    else
        fnt.setPixelSize(qRound(fnt.pixelSize()*factor));
}

static Gradients::Type s_previewGradient = Gradients::TypeAmount;
void
Client::repaint(QPainter &p, bool paintTitle)
{
    if (!Factory::initialized())
        return;

    QColor bg = color(ColorTitleBar, isActive());

    // preview widget =============================================================================
    if (isPreview())
    {
        if (s_previewGradient == Gradients::TypeAmount)
            s_previewGradient = Gradients::Type(rand() % Gradients::TypeAmount);
        QRect r = widget()->rect();
        bool hadAntiAliasing = p.testRenderHint(QPainter::Antialiasing);
        p.setRenderHint( QPainter::Antialiasing );
        // the shadow - this is rather expensive, but hey - who cares... ;-P
        p.setPen(Qt::NoPen);
        for (int i = 0; i < 8; ++i) {
            p.setBrush(QColor(0,0,0,4+2*i));
            int rd = 12-i/2;
            p.drawRoundedRect(r, rd, rd);
            r.adjust(i%2,!(i%3),-i%2,-1);
        }

        // the window
        p.setBrush(Gradients::pix(bg, r.height(), Qt::Vertical, s_previewGradient));
        p.setBrushOrigin(QPoint(0,r.y()));
        QColor c = color(Client::ColorFont, isActive());
        c.setAlpha(80);
        p.setPen(c);
        p.drawRoundedRect(r, 6, 6);

        // logo
        if (isActive())
        {
            const int s = qMin(r.width(), r.height())/2;
            QRect logo(0,0,s,s);
            logo.moveCenter(r.center());

            c.setAlpha(180); p.setBrush(c); p.setPen(Qt::NoPen);
            p.drawPath(Shapes::logo(logo));
        }
        p.setRenderHint( QPainter::Antialiasing, hadAntiAliasing );
    }

    // only one "big" gradient, as we can't rely on windowId()! ====================================
    else if (isShade())
    {
        const Qt::Orientation o = Factory::verticalTitle() ? Qt::Horizontal : Qt::Vertical;
        const QPixmap &fill = Gradients::pix(bg, width(), o, Gradients::Button);
        p.drawTiledPixmap(0,0,width(),height(), fill);
    }

    // window ===================================================================================
    else
    {
        p.setBrush(bg); p.setPen(Qt::NoPen);
        switch (myBgMode) {
        case Bg::VerticalGradient: {
#define ctWidth 32
#define ctHeight 128
#define tbWidth 32
#define tbHeight 256
#define lrcWidth 128
#define lrcHeight 128

            if (!myTiles[Bg::Top]) { // hmm? paint fallback
                for (int i = 0; i < 4; ++i)
                    p.drawRect(myArea[i]);
                // and wait for pixmaps
                updateStylePixmaps();
                break;
            }

            p.drawRect(myArea[Left]); p.drawRect(myArea[Right]);
            if (bg.alpha() != 0xff)
                { p.drawRect(myArea[Top]); p.drawRect(myArea[Bottom]); }

            QPixmap tbBuffer(tbWidth, tbHeight);
            int s1 = tbHeight;
            int s2 = qMin(s1, (height()+1)/2);
            s1 -= s2;
            DUMP_PICTURE(tb, Bg::Top, topTile);
            p.drawTiledPixmap( 0, 0, width(), s2, tbBuffer, 0, s1 );
            if (Colors::value(bg) < 245 && bg.alpha() == 0xff)
            {   // makes no sense otherwise
                const int w = width()/4 - 128;
                if (w > 0)
                {
                    s2 = 128-s1;
                    QPixmap ctBuffer(ctWidth, ctHeight);
                    DUMP_PICTURE(ct, Bg::Corner, cornerTile);
                    p.drawTiledPixmap( 0, 0, w, s2, ctBuffer, 0, s1 );
                    p.drawTiledPixmap( width()-w, 0, w, s2, ctBuffer, 0, s1 );
                }
                QPixmap lrcBuffer(lrcWidth, lrcHeight);
                DUMP_PICTURE(lrc, Bg::LeftCorner, lCorner);
                p.drawPixmap(w, 0, lrcBuffer, 0, s1, 128, s2);
                DUMP_PICTURE(lrc, Bg::RightCorner, rCorner);
                p.drawPixmap(width()-w-128, 0, lrcBuffer, 0, s1, 128, s2);
            }
            if ( !( Factory::baseSize() || Factory::verticalTitle() ) )
                break;
            s1 = tbHeight;
            s2 = qMin(s1, height()/2);
            DUMP_PICTURE(tb, Bg::Bottom, btmTile);
            p.drawTiledPixmap( 0, height()-s2, width(), s2, tbBuffer );
            break;
        }
#undef tbWidth
#undef tbHeight
#undef lrcWidth
#undef lrcHeight

        case Bg::HorizontalGradient:
        {
#define tbWidth 256
#define tbHeight 32
#define lrcWidth 256
#define lrcHeight 32

            p.drawRect(myArea[Top]); // can be necessary for flat windows
            p.drawRect(myArea[Bottom]);
            if (bg.alpha() != 0xff)
                { p.drawRect(myArea[Left]); p.drawRect(myArea[Right]); }
            int s1 = tbWidth;
            int s2 = qMin(s1, (width()+1)/2);
            const int h = qMin(128+32, height()/8);
            QPixmap tbBuffer(tbWidth, tbHeight);
            QPixmap lrcBuffer(lrcWidth, lrcHeight);
            QPixmap ctBuffer(ctWidth, ctHeight);
            DUMP_PICTURE(tb, Bg::Left, topTile);
            p.drawTiledPixmap( 0, h, s2, height()-h, tbBuffer, s1-s2, 0 );
            DUMP_PICTURE(lrc, Bg::LeftCorner, lCorner); // left bottom shine
            p.drawPixmap(0, h-32, lrcBuffer, s1-s2, 0,0,0);
            DUMP_PICTURE(tb, Bg::Right, btmTile);
            p.drawTiledPixmap( width() - s2, h, s2, height()-h, tbBuffer );
            DUMP_PICTURE(lrc, Bg::RightCorner, rCorner); // right bottom shine
            p.drawPixmap(width() - s2, h-32, lrcBuffer);
            DUMP_PICTURE(ct, Bg::Corner, cornerTile); // misleading, TOP TILE
            p.drawTiledPixmap( 0, h-(128+32), width(), 128, ctBuffer );
            break;
        }
        case Bg::Plain:
            p.setBrush(bg); p.setPen(Qt::NoPen);
            for (int i = 0; i < 4; ++i)
                p.drawRect(myArea[i]);
            break;
        case Bg::Structure: {
            QPixmap texture;
            if (FX::usesXRender() && myTiles[Bg::Top]) {
                const int width = ((myTiles[Bg::Corner] >> 16) & 0xffff);
                const int height = (myTiles[Bg::Corner] & 0xffff);
                texture = QPixmap(width, height);
                if (bg.alpha() != 0xff) {
                    texture.fill(Qt::transparent);
                    XRenderComposite(QX11Info::display(), PictOpOver,
                                        myTiles[Bg::Top], 0, texture.x11PictureHandle(),
                                        0, 0, 0, 0, 0, 0, width, height);
                } else {
                    texture.fill(bg);
                    XRenderComposite(QX11Info::display(), PictOpSrc,
                                        myTiles[Bg::Top], 0, texture.x11PictureHandle(),
                                        0, 0, 0, 0, 0, 0, width, height);
                }
            } else if (myTiles[Bg::Bottom]) {
                texture = Factory::structure(bg, (myTiles[Bg::Bottom] & 0xff), ((myTiles[Bg::Bottom] >> 8) & 0xff));
            }
            if (texture.isNull())
                p.setBrush(bg);
            else {
                p.setBrush(texture);
                p.setBrushOrigin(QPoint(myArea[Left].width()-texture.width(), myArea[Top].height()-texture.height()));
            }
            for (int i = 0; i < 4; ++i)
                p.drawRect(myArea[i]);
            break;
        }
        default:
        case Bg::Gradient: {
            Qt::Orientation o = Qt::Vertical;
            p.setPen(Qt::NoPen);
            const Gradients::Type gradient = Factory::config()->gradient[0][isActive()];
            QRect ttBar = myArea[Top];
            int ttSize = ttBar.height();
            QLine borderline;
            if (Factory::verticalTitle()) {
                o = Qt::Horizontal;
                ttBar = myArea[Left];
                ttSize = ttBar.width();
                p.drawRect(myArea[Top]);
                borderline.setLine(myArea[Left].right(), myArea[Top].bottom(), myArea[Left].right(), myArea[Bottom].y());
            } else {
                p.drawRect(myArea[Left]);
                borderline.setLine(myArea[Left].right(), myArea[Top].bottom(), myArea[Right].x(), myArea[Top].bottom());
            }
            p.drawRect(myArea[Right]);
            p.drawRect(myArea[Bottom]);
            if (gradient == Gradients::None)
                p.drawRect(ttBar);
            else {
                p.fillRect(ttBar, Gradients::brush(bg, ttSize, o, gradient));
                p.setPen(Colors::mid(bg, Qt::black,6,1));
                p.drawLine(borderline);
                /* unused counter border 
                p.setPen(Colors::mid(bg, Qt::white,6,1));
                p.drawLine(0,myTitleSize-1,width(),myTitleSize-1); */
            }
            break;
        }
        } // switch

        if (decoMode() == CenterDeco) {
            if (paintTitle && decoGradient() && myArea[Label].width()) { // nice deco
                Qt::Orientation o = Qt::Vertical;
                int rnd[2];
                int ttSize = myArea[Top].height();
                p.setRenderHint( QPainter::Antialiasing );

                p.setBrushOrigin(myArea[Label].topLeft());
                p.setPen(Qt::NoPen);
                if (Factory::verticalTitle()) {
                    o = Qt::Horizontal;
                    rnd[0] = 99;
                    ttSize = myArea[Left].width();
                    rnd[1] = ttSize*99/myArea[Label].height();
                } else {
                    rnd[0] = ttSize*99/myArea[Label].width();
                    rnd[1] = 99;
                }

                p.setBrush(Gradients::pix(bg, ttSize, o, Gradients::Sunken));
                p.drawRoundRect(myArea[Label].adjusted(2,2,-2,-2), rnd[0], rnd[1]);
                bg = color(ColorTitleBlend, isActive());
                p.setBrush(Gradients::pix(bg, ttSize, o, decoGradient()));
                p.drawRoundRect(myArea[Label].adjusted(3,3,-3,-3), rnd[0], rnd[1]);

                p.setRenderHint( QPainter::Antialiasing, false );
            }
        }
        if (myUnoHeight && !Factory::verticalTitle() && unoGradient() != Gradients::None)
            p.fillRect(myArea[Top], Gradients::brush(bg, myUnoHeight, Qt::Vertical, unoGradient()));
    }

    if (paintTitle && isShade())
    {   // splitter
//         QColor bg2 = color(ColorTitleBlend, isActive());
        p.setPen(Colors::mid(bg, Qt::black, 3, 1));
        if (Factory::verticalTitle())
        {
            int x = myArea[Left].right() - 1;
            p.drawLine(x, 8, x, height()-8);
            ++x;
            p.setPen(Colors::mid(bg, Qt::white, 2, 1));
            p.drawLine(x, 8, x, height()-8);
        }
        else
        {
            int y = myArea[Top].bottom() - 1;
            p.drawLine(8, y, width()-8, y);
            ++y;
            p.setPen(Colors::mid(bg, Qt::white, 2, 1));
            p.drawLine(8, y, width()-8, y);
        }
    }

//     paintTitle = paintTitle && !Factory::verticalTitle();
    const int sz = Factory::verticalTitle() ? myArea[Label].height() : myArea[Label].width();
    // title ==============
    if (paintTitle && sz > 0)
    {
        const QColor titleColor = color((isShade() && decoMode() == CenterDeco) ? ColorButtonBg : ColorFont, isActive());
        int tf = Factory::config()->titleAlign | Qt::AlignVCenter | Qt::TextSingleLine;

        // FONT ==========================================
        QFont fnt = options()->font();
        if (iAmSmall) {
            shrink(fnt, Factory::smallFactor());
            tf |= Qt::AlignBottom;
        }
        else
            tf |= Qt::AlignVCenter;
        float tw = 1.07*QFontMetrics(fnt).size(tf, myCaption).width();
        if (tw > sz)
        {
            float f = sz/tw;
            if (f > 0.9)
                fnt.setStretch(qRound(fnt.stretch()*f));
            else
                shrink(fnt, qMax(f,0.75f));
        }
        p.setFont(fnt);
        // ===============
        if ( Factory::verticalTitle() )
        {
            p.translate(myArea[Label].center());
            p.rotate(-90.0);
            p.translate(-myArea[Label].center());
            const int d = (myArea[Label].height() - myArea[Label].width()) / 2;
            myArea[Label].setRect( myArea[Label].x() - d, myArea[Label].y() + d, myArea[Label].height(), myArea[Label].width() );
        }

        if (myBgMode == Bg::Gradient)
            myArea[Label].adjust(8,0,-8,0);

        if (Factory::config()->embossTitle) {
            int d = 0;
            const int bgv = Colors::value(bg), fgv = Colors::value(titleColor);
//             if (bgv < fgv) // dark bg -> dark top borderline
//                 { p.setPen(Colors::mid(bg, Qt::black, bgv, 160)); d = -1; }
//             else // bright bg -> bright bottom borderline
//                 { p.setPen(Colors::mid(bg, Qt::white, 16, bgv)); d = 1; }

            if (bgv < fgv) // dark bg -> dark top borderline
                { p.setPen(QColor(0,0,0,255-bgv)); d = -1; }
            else  // bright bg -> bright bottom borderline
            {
                int s = 255 - bgv + qMin(bg.red(), qMin(bg.green(), bg.blue()));
                // s^12/255^11 in integer compatible way ;-)
                s = s*s/255;
                s = s*s/255;
                s = s*s/255;
                s = s*s/255;
                s = s*s/255;
                s = s*s/255;
                s = s - 128;
                s = 64 + s*s*s / 20000;
                p.setPen(QColor(255,255,255,s));
                d = 1;
            }

            QPoint off(0,0); Factory::verticalTitle() ? off.setX(-d) : off.setY(d);
            p.drawText ( myArea[Label].translated(off), tf, myCaption);
        }

        QRect tr;
        p.setPen(titleColor);
        p.drawText ( myArea[Label], tf, myCaption, &tr );

        bool wantBorderLines = !Factory::verticalTitle() && tr.left() - 37 > myArea[Label].left() && tr.right() + 37 < myArea[Label].right();
        if (wantBorderLines) { // otherwise painting looks crap
            if (!Factory::config()->forceBorderLines) { // not forced, check whether inactive looks like active
                if (Factory::config()->hideInactiveButtons && !Factory::config()->buttonnyButton)
                    wantBorderLines = false; // perfectly hinted
                else {
                    wantBorderLines =   color(ColorTitleBar, 0) == color(ColorTitleBar, 1) &&
                                        myGradients[0] == myGradients[1] &&
                                        color(ColorTitleBlend, 0) == color(ColorTitleBlend, 1);
                }
            }
        }
        if (wantBorderLines)
        {   // inactive window looks like active one...
            int y = myArea[Label].center().y();
            if ( !(tf & Qt::AlignLeft) )
                p.drawPixmap(tr.x() - 38, y, Gradients::borderline(titleColor, Gradients::Left));
            if ( !(tf & Qt::AlignRight) )
                p.drawPixmap(tr.right() + 6, y, Gradients::borderline(titleColor, Gradients::Right));
        }

        if (myBgMode == Bg::Gradient)
            myArea[Label].adjust(-8,0,8,0);
        if ( Factory::verticalTitle() )
        {
            p.translate(myArea[Label].center());
            p.rotate(90.0);
            p.translate(-myArea[Label].center());
            const int d = (myArea[Label].width() - myArea[Label].height()) / 2;
            myArea[Label].setRect( myArea[Label].x() + d, myArea[Label].y() - d, myArea[Label].height(), myArea[Label].width() );
        }
    }

    const QColor bg2 = color(ColorTitleBlend, isActive());
    QColor shadow = Colors::mid(bg2, Qt::black,4,1);
    const bool outline = (bg2.rgb() != bg.rgb()) && (decoMode() != ButtonDeco);
    if (outline || !Factory::compositingActive())
    {
        // outline
        p.setBrush(Qt::NoBrush);
        p.setRenderHint( QPainter::Antialiasing );
        int x,y,w,h;
        widget()->rect().getRect(&x,&y,&w,&h);
        if (outline) {
            p.setPen(QPen(bg2, 3));
            ++x;++y;w-=2;h-=2;
        } else {
            p.setPen(shadow);
        }
        char lines[4] = {0,0,0,0};
        if (Factory::roundCorners() && myArea[Left].width() > 3)
            lines[0] = 2;
        else if (myArea[Left].width())
            lines[0] = 1;
        if (Factory::roundCorners() && myArea[Right].width() > 3)
            lines[1] = 2;
        else if (myArea[Right].width())
            lines[1] = 1;
        if (Factory::roundCorners() && myArea[Top].height() > 3)
            lines[2] = 2;
        else if (myArea[Top].height())
            lines[2] = 1;
        if (Factory::roundCorners() && myArea[Bottom].height() > 3)
            lines[3] = 2;
        else if (myArea[Bottom].height())
            lines[3] = 1;

        QPainterPath path;
        const int cornerSize = 13;

        if (lines[0] + lines[2] == 4) {
            path.arcMoveTo(x, y, cornerSize, cornerSize, 90);
            path.arcTo(x, y, cornerSize, cornerSize, 90, 90);
        } else if (lines[0]) {
            path.moveTo(x,y);
        }

        if (lines[0] + lines[3] == 4) {
            path.arcTo(x, y+h-cornerSize, cornerSize, cornerSize, 2*90, 90);
        } else if (lines[0]) {
            path.lineTo(x, y+h);
        } else {
            path.moveTo(x, y+h);
        }

        if (lines[3] + lines[1] == 4) {
            path.arcTo(x+w-cornerSize, y+h-cornerSize, cornerSize, cornerSize, 3*90, 90);
        } else if (lines[3]) {
            path.lineTo(x+w, y+h);
        } else {
            path.moveTo(x+w, y+h);
        }

        if (lines[2] + lines[1] == 4) {
            path.arcTo(x+w-cornerSize, y, cornerSize, cornerSize, 0, 90);
        } else if (lines[1]) {
            path.lineTo(x+w, y);
        } else {
            path.moveTo(x+w, y);
        }

        if (lines[2])
            path.lineTo(x, y);

        p.drawPath(path);
    }

    // bar =========================
    if (decoMode() == CornerDeco)
    {
        p.setPen(shadow);
        p.setBrushOrigin(0,0);
        if (decoGradient())
        {   // button corner
            const QPixmap &fill = Factory::verticalTitle() ? Gradients::pix(bg2, myArea[Left].width(), Qt::Horizontal, decoGradient()) : 
                                                             Gradients::pix(bg2, myArea[Top].height(), Qt::Vertical, decoGradient());
            p.setBrush(fill);
            p.setRenderHint( QPainter::Antialiasing );
            p.drawPath(buttonCorner);
        }
    }
}

void
Client::reset(unsigned long changed)
{
    if (changed & (SettingFont|SettingBorder)) {
        int dummy[4];
        borders(dummy[0],dummy[1],dummy[2],dummy[3]);
    }

    if (changed & SettingDecoration)
    {
        myGradients[0] = Factory::config()->gradient[1][0];
        myGradients[1] = Factory::config()->gradient[1][1];
        changed |= SettingColors;
    }

    if (changed & SettingFont)
    {
        if (Factory::verticalTitle())
            myTitleSpacer->changeSize( myArea[Left].width(), 1, QSizePolicy::Fixed, QSizePolicy::Expanding);
        else
            myTitleSpacer->changeSize( 1, myArea[Top].height(), QSizePolicy::Expanding, QSizePolicy::Fixed);
        myTitleBar->invalidate();
    }

    if (changed & SettingButtons)
    {
        myButtonOpacity = (isActive() || !Factory::config()->hideInactiveButtons) ? 100 : 0;
        QLayoutItem *item;
        while ((item = myTitleBar->takeAt(0)))
            if (item != myTitleSpacer) {
                if (item->widget())
                    delete item->widget();
                else if (item->spacerItem())
                    delete item->spacerItem();
                else
                    delete item;
            }
        for (int i = 0; i < 4; ++i)
            myButtons[i] = 0;
        addButtons(options()->titleButtonsLeft(), buttonSpaceLeft, true);
        myTitleBar->addItem(myTitleSpacer);
        addButtons(options()->titleButtonsRight(), buttonSpaceRight, false);
        buttonSpace = qMax(buttonSpaceLeft, buttonSpaceRight);
    }

    if ((changed & (SettingButtons | SettingFont)) && (buttonSpaceLeft >= buttonSpaceRight))
        updateButtonCorner(false);

    if (changed & SettingColors)
    {   // colors =====================
        for (int i = 0; i < 5; ++i)
            myTiles[i] = 0;

        for (int a = 0; a < 2; ++a)
        for (int t = 0; t < 4; ++t)
            myColors[a][t] = options()->color((ColorType)t, a);

        bool def = true;
        if (isPreview())
        {
            def = false;
            myBgMode = Bg::Plain;
            myGradients[0] = myGradients[1] = myGradients[2] = Gradients::None;

            myColors[0][ColorTitleBlend] = myColors[1][ColorTitleBlend] =
            myColors[0][ColorTitleBar] = myColors[1][ColorTitleBar] =
            widget()->palette().color(QPalette::Active, QPalette::Window);

            myColors[1][ColorButtonBg] = myColors[1][ColorFont] =
            widget()->palette().color(QPalette::Active, QPalette::WindowText);

            myColors[0][ColorButtonBg] = myColors[0][ColorFont] =
            Colors::mid(myColors[0][ColorTitleBar], myColors[1][ColorFont]);
        }
        else if (!Factory::config()->forceUserColors)
        {
            unsigned long _9 = 9;
            WindowData *data = 0;
            bool needFree = false;

             // check for strict match data from preset ==========================================
            KWindowInfo info(windowId(), NET::WMWindowType, NET::WM2WindowClass);
            data = Factory::decoInfo(info.windowClassClass(), info.windowType(supported_types), true);
            if (data && (((data->style >> 16) & 0xff) > 1))
            {
                WindowPics pics;
                pics.topTile = pics.cnrTile = pics.lCorner = pics.rCorner = 0;
                /// NOTICE encoding the bg gradient intensity in the btmTile Pic!!
                pics.btmTile = 150;
                XProperty::set<Picture>(windowId(), XProperty::bgPics, (Picture*)&pics, XProperty::LONG, 5);
            }

            // check for data from style ====================================================
            if (!data) {
                data = (WindowData*)XProperty::get<uint>(windowId(), XProperty::winData, XProperty::WORD, &_9);
                if (data)
                    needFree = true;  // X provides a deep copy
            }

            // check for weak match data from preset ==========================================
            if (!data) {
                data = Factory::decoInfo(info.windowClassClass(), info.windowType(supported_types), false);
                if (data && (((data->style >> 16) & 0xff) > 1))
                {
                    WindowPics pics;
                    pics.topTile = pics.cnrTile = pics.lCorner = pics.rCorner = 0;
                    /// NOTICE encoding the bg gradient intensity in the btmTile Pic!!
                    pics.btmTile = 150;
                    XProperty::set<Picture>(windowId(), XProperty::bgPics, (Picture*)&pics, XProperty::LONG, 5);
                }
            }

            // check for data from dbus =========================================================
            if (!data)
            {
                long int *pid = XProperty::get<long int>(windowId(), XProperty::pid, XProperty::LONG);
                if (pid)
                {
                    if ((data = Factory::decoInfo(*pid)))
                        XProperty::set<uint>(windowId(), XProperty::winData, (uint*)data, XProperty::WORD, 9);
                    XFree(pid);
                }
            }

            // read data ========================================================================
            if (data)
            {
                def = false;
                myColors[0][ColorTitleBar].setRgba(data->inactiveWindow);
                myColors[1][ColorTitleBar].setRgba(data->activeWindow);
                myColors[0][ColorTitleBlend].setRgba(data->inactiveDeco);
                myColors[1][ColorTitleBlend].setRgba(data->activeDeco);
                myColors[0][ColorFont].setRgba(data->inactiveText);
                myColors[1][ColorFont].setRgba(data->activeText);
                myColors[0][ColorButtonBg].setRgba(data->inactiveButton);
                myColors[1][ColorButtonBg].setRgba(data->activeButton);
                myBgMode = ((data->style >> 16) & 0xff);
                if (myBgMode > 127) {
                    myBgMode = Bg::Structure;
                }
                myUnoHeight = ((data->style >> 24) & 0xff); // if (..)
//                     QTimer::singleShot(2500, this, SLOT(updateUnoHeight()));
                myGradients[0] = (Gradients::Type)((data->style >> 11) & 0x1f);
                myGradients[1] = (Gradients::Type)((data->style >> 6) & 0x1f);
                myGradients[2] = (Gradients::Type)((data->style >> 1) & 0x1f);
            }

            // free X11 data ======================================================================
            if (needFree)
                XFree(data);
        }

        if (def)
        {   // the fallback solution
            myUnoHeight = 0;
            if ((myBgMode = Factory::defaultBgMode()) > Bg::Gradient)
            {
                WindowPics pics;
                pics.topTile = pics.cnrTile = pics.lCorner = pics.rCorner = 0;
                /// NOTICE encoding the bg gradient intensity in the btmTile Pic!!
                pics.btmTile = 150;
                XProperty::set<Picture>(windowId(), XProperty::bgPics, (Picture*)&pics, XProperty::LONG, 5);
            }
            for (int i = 0; i <  2; ++i)
            {
                if (!myGradients[i])
                {   //buttoncolor MUST be = titlefont
                    myColors[i][ColorTitleBlend] = myColors[i][ColorTitleBar];
                    myColors[i][ColorButtonBg] = myColors[i][ColorFont];
                }
            }
        }
        else if (myBgMode == Bg::Gradient) {
            // if the user set a colormode from the style, but no gradient, we use the color on
            // the default gradient and NOT the nonexisting accessoire
            for (int i = 0; i <  2; ++i) {
                if (myGradients[i] == Gradients::None) {
                    int alpha = myColors[i][ColorTitleBar].alpha();
                    myColors[i][ColorTitleBar] = myColors[i][ColorTitleBlend];
                    myColors[i][ColorTitleBar].setAlpha(alpha);
                    myColors[i][ColorFont] = myColors[i][ColorButtonBg];
                } else if (!Factory::buttonnyButton()) {   // needs titlefont and button bg swapped...
                    QColor h = myColors[i][ColorButtonBg];
                    myColors[i][ColorButtonBg] = myColors[i][ColorFont];
                    myColors[i][ColorFont] = h;
                }
            }
            // last, clamp ColorTitleBlend to v >= 80
            int h,s,v;
            for (int i = 0; i <  2; ++i) {
                v = Colors::value(myColors[i][ColorTitleBlend]);
                if (v < 70) {
                    myColors[i][ColorTitleBlend].getHsv(&h,&s,&v);
                    myColors[i][ColorTitleBlend].setHsv(h,s,70);
                }
            }
        }
        QPalette pal = widget()->palette();
        pal.setColor(QPalette::Active, widget()->backgroundRole(), color(ColorTitleBar, true));
        pal.setColor(QPalette::Inactive, widget()->backgroundRole(), color(ColorTitleBar, false));
        pal.setColor(QPalette::Disabled, widget()->backgroundRole(), color(ColorTitleBar, false));
        widget()->setPalette(pal);
    }


    // WORKAROUND a bug apparently in QPaintEngine which seems to
    // fail to paint IMAGE_RGB -> QPixmap -> device while device is redirected
    dirty[0] = dirty[1] = color(ColorTitleBar, isActive()).alpha() == 0xff;
    if (changed)
        activeChange(false); // handles bg pixmaps in case and triggers update

}

void
Client::updateButtonCorner(bool right)
{
    int ts = myArea[Top].bottom();
    if (Factory::verticalTitle())
    {
        ts = myArea[Left].right();
        if (right) // bottom
        {
            const int h = height();
            const int dr = buttonSpaceRight + ts;
            const int bs = buttonSpaceRight;
            buttonCorner = QPainterPath(QPoint(-1, h));
            buttonCorner.lineTo(ts, h);
            buttonCorner.lineTo(ts, h - bs + ts/2); // straight to bl offset
            buttonCorner.cubicTo(ts+2, h - dr + ts/2,  0, h-bs,  -1, h-dr); // curve to tr offset
        }
        else
        {
            const int dl = buttonSpaceLeft + ts;
            const int bs = buttonSpaceLeft;
            buttonCorner = QPainterPath(QPoint(-1, 0)); //tl corner
            buttonCorner.lineTo(-1, dl); // straight to tl end
            buttonCorner.cubicTo(0, bs,  ts+2, dl - ts/2,  ts, bs - ts/2); // curve to bl offset
            buttonCorner.lineTo(ts, 0); // straight to bl end
        }
    }
    else if (right)
    {
        const int w = width();
        const int dr = buttonSpaceRight + ts;
        const int bs = buttonSpaceRight;
        buttonCorner = QPainterPath(QPoint(w, -1)); //tl corner
        buttonCorner.lineTo(w, ts); // straight to br corner
        buttonCorner.lineTo(w - bs + ts/2, ts); // straight to bl offset
        buttonCorner.cubicTo(w - dr + ts/2, ts+2,   w-bs, 0,   w-dr, -1); // curve to tr offset
    }
    else
    {
        const int dl = buttonSpaceLeft + ts;
        const int bs = buttonSpaceLeft;
        buttonCorner = QPainterPath(QPoint(0, -1)); //tl corner
        buttonCorner.lineTo(dl, -1); // straight to tl end
        buttonCorner.cubicTo(bs, 0,   dl - ts/2, ts+2,   bs - ts/2, ts); // curve to bl offset
        buttonCorner.lineTo(0, ts); // straight to bl end
    }
}


void
Client::updateTitleLayout( const QSize& )
{
    int dl = buttonSpaceLeft, dr = buttonSpaceRight;
    const int ttSize = Factory::verticalTitle() ? myArea[Left].width() : myArea[Top].height();
    if (Factory::config()->titleAlign == Qt::AlignHCenter)
        dl = dr = buttonSpace;
    if (decoMode() == CornerDeco) {
        if (buttonSpaceLeft <= buttonSpaceRight) {
            updateButtonCorner(true);
            dr += ttSize;
        }
        else
            dl += ttSize;

    }
    else
        { dl += 8; dr += 8; }

    if (Factory::verticalTitle())
        myArea[Label].setRect(0, dl, ttSize, height()-(dl+dr));
    else
        myArea[Label].setRect(dl, 0, width()-(dl+dr), ttSize);

    if (!myArea[Label].isValid())
        myArea[Label] = QRect();
}

int 
Client::titleSize() const
{
    int ret = Factory::titleSize(iAmSmall);
    if (Factory::config()->buttonnyButton) // need more padding to look good
    if (!myUnoHeight || Factory::verticalTitle()) // is not effectively UNO
    if (myBgMode == Bg::Gradient)
        ret += 6;
    return ret;
}

void
Client::resize( const QSize& s )
{
    widget()->resize(s);
    int w = s.width(), h = s.height();

    myArea[Top].setWidth( w );
    myArea[Left].setHeight( h - (myArea[Top].height() + myArea[Bottom].height()) );
    myArea[Bottom].setWidth( w );
    myArea[Bottom].moveBottom( h-1 );
    myArea[Right].setHeight( h - (myArea[Top].height() + myArea[Bottom].height()) );
    myArea[Right].moveRight( w-1 );
    updateTitleLayout( s );
#if KDE_IS_VERSION(4,7,0)
    if ( Factory::roundCorners() && !Factory::compositingActive() )
#else
    if ( Factory::roundCorners() )
#endif
    {
        if (maximizeMode() == MaximizeFull)
            { clearMask(); widget()->update(); return; }

        int d = (isShade() || Factory::baseSize() > 3) ? 8 : 4;
        QRegion mask;
        if (Factory::verticalTitle())
        {
            mask = QRegion(0, 4, w, h-8);
            mask += QRegion(4, 0, w-d, h);
            mask += QRegion(1, 2, w-d/4, h-4);
            mask += QRegion(2, 1, w-d/2, h-2);
        }
        else
        {
            mask = QRegion(4, 0, w-8, h);
            mask += QRegion(0, 4, w, h-d);
            mask += QRegion(2, 1, w-4, h-d/4);
            mask += QRegion(1, 2, w-2, h-d/2);
        }
        setMask(mask);
    }
    else
        clearMask();

    widget()->update(); // force! there're painting errors otherwise
}

void
Client::setFullscreen(bool on)
{
    unsigned long state = KWindowInfo(windowId(), 0, NET::WMState).state();
    if (on)
        state |= NET::FullScreen;
    else
        state &= ~NET::FullScreen;
    KWindowSystem::setState(windowId(), state);
}

void
Client::shadeChange()
{
    emit shadeChanged(isSetShade());
}

void
Client::showDesktopMenu(const QPoint &p)
{
   QPoint ip = p;
   QPoint gp = widget()->mapToGlobal(QPoint(width()-60, 0));
   if (ip.x() > gp.x()) ip.setX(gp.x());
   Factory::showDesktopMenu(ip, this);
}

void
Client::showWindowList(const QPoint &p)
{
    QPoint ip = p;
    QPoint gp = widget()->mapToGlobal(QPoint(width()-200, 0));
    if (ip.x() > gp.x())
        ip.setX(gp.x());
    Factory::showWindowList(ip, this);
}

void
Client::showInfo(const QPoint &p)
{
    QPoint ip = p;
    QPoint gp = widget()->mapToGlobal(QPoint(width()-320, 0));
    if (ip.x() > gp.x())
        ip.setX(gp.x());
    Factory::showInfo(ip, windowId());
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
    if (ip.x() > gp.x())
        ip.setX(gp.x());
    KDecoration::showWindowMenu(ip);
}

void
Client::tileWindow(bool more, bool vertical, bool mirrorGravity)
{
    const int tiles = 6;
    int state = 0, sz = 0, flags = (mirrorGravity ? 3 : 1) | (1<<14);
    bool change = false;
    /*
    The low byte of data.l[0] contains the gravity to use; it may contain any value allowed
    for the WM_SIZE_HINTS.win_gravity property: NorthWest (1), North (2), NorthEast (3),
        West (4), Center (5), East (6), SouthWest (7), South (8), SouthEast (9) and Static (10).
        A gravity of 0 indicates that the Window Manager should use the gravity specified in
        WM_SIZE_HINTS.win_gravity. The bits 8 to 11 indicate the presence of x, y, width and height.
        The bits 12 to 15 indicate the source (see the section called Source indication in requests),
        so 0001 indicates the application and 0010 indicates a Pager or a Taskbar.
        The remaining bits should be set to zero.
        */

    const QRect area(QApplication::desktop()->availableGeometry(KWindowInfo(windowId(), NET::WMGeometry).geometry().center()));

    if (vertical) {
        if (!(sz = area.height()))
            return;
        state = qRound((double)tiles*height()/sz);
        change = (qAbs(height()-state*sz/tiles) < 0.05*sz);
        flags |= 1<<11;
    } else {
        if (!(sz = area.width()))
            return;
        state = qRound((double)tiles*width()/sz);
        change = (qAbs(width()-state*sz/tiles) < 0.05*sz);
        flags |= 1<<10;
    }

    if (change) {
        if (!more)
            ++state;
        else if (state > 1)
            --state;
    }

    if (!state || state > tiles)
        return;

    sz = state*sz/tiles;

    NETRootInfo rootinfo(QX11Info::display(), NET::WMMoveResize );
    rootinfo.moveResizeWindowRequest(windowId(), flags, 0, 0,
                                     sz - (myArea[Left].width() + myArea[Right].width()),
                                     sz - (myArea[Top].height() + myArea[Bottom].height()));
}

void Client::maximumize(Qt::MouseButtons button)
{
    int flags(0);
    WindowOperation op = options()->operationMaxButtonClick(button);
    if (op == MaximizeOp) {
        flags = 1<<8|1<<9|1<<10|1<<11;
    } else if (op == HMaximizeOp) {
        flags = 1<<8|1<<10;
    } else if (op == VMaximizeOp) {
        flags = 1<<9|1<<11;
    } else {
        return; // invalid op, should not happen
    }
    const QRect r(QApplication::desktop()->availableGeometry(KWindowInfo(windowId(), NET::WMGeometry).geometry().center()));
    NETRootInfo rootinfo(QX11Info::display(), NET::WMMoveResize );
    rootinfo.moveResizeWindowRequest(windowId(), flags, r.x(), r.y(),
                                     r.width() - (myArea[Left].width() + myArea[Right].width()),
                                     r.height() - (myArea[Top].height() + myArea[Bottom].height()));
}


void
Client::timerEvent(QTimerEvent *te)
{
    if (te->timerId() != myActiveChangeTimer)
    {
        KDecoration::timerEvent(te);
        return;
    }
    if (isActive() || widget()->underMouse())
    {
        myButtonOpacity += 25;
        if (myButtonOpacity > 99)
        {
            killTimer(myActiveChangeTimer);
            myActiveChangeTimer = 0;
            myButtonOpacity = 100;
        }
    }
    else
    {
        myButtonOpacity -= 10;
        if (myButtonOpacity < 1)
        {
            killTimer(myActiveChangeTimer);
            myActiveChangeTimer = 0;
            myButtonOpacity = 0;
        }
    }
    for (int i = 0; i < 4; ++i)
        if (myButtons[i])
            myButtons[i]->repaint();
}

void
Client::activate()
{
    QAction *act = qobject_cast<QAction*>(sender());
    if (!act)
        return;

    bool ok;
    int id = act->data().toUInt(&ok);
    if (ok)
    {
        KWindowSystem::activateWindow( id );
        return;
    }
    KWindowSystem::activateWindow( windowId() );
}

void
Client::throwOnDesktop()
{
    QAction *act = qobject_cast<QAction*>(sender());
    if (!act)
        return;

    bool ok;
    int desktop = act->data().toInt(&ok);
    if (ok)
        setDesktop(desktop);
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
    return  !s.compare("konqueror", Qt::CaseInsensitive) ||
            !s.compare("rekonq", Qt::CaseInsensitive) ||
            !s.compare("qupzilla", Qt::CaseInsensitive) ||
            !s.compare("chromium", Qt::CaseInsensitive) ||
            !s.compare("firefox", Qt::CaseInsensitive) ||
            !s.compare("opera", Qt::CaseInsensitive) ||
            !s.compare("arora", Qt::CaseInsensitive) ||
            !s.compare("leechcraft", Qt::CaseInsensitive) ||
            !s.compare("mozilla", Qt::CaseInsensitive) ||
            !s.compare("safari", Qt::CaseInsensitive); // just in case ;)
}

void Client::triggerMoveResize()
{
    widget()->grabMouse();
}

QString
Client::trimm(const QString &string)
{
    if (!Factory::config()->trimmCaption)
        return string;

    KWindowInfo info(windowId(), 0, NET::WM2WindowClass);
    QString appName(info.windowClassName());

    // ---------------------------------------------------
    // Amarok shortcut
    // Amarok displays the Current Track Info in the title
    if (!appName.compare("amarok", Qt::CaseInsensitive))
        return "Amarok";
    // ---------------------------------------------------

    if (kwin_seps.isEmpty()) {
        kwin_seps << QString(" %1 ").arg(QChar(0x2013)) << // recently used utf-8 dash
                     QString(" %1 ").arg(QChar(0x2014)) <<  // trojitá uses an em dash ...
                     " - ";
    }
    /* Ok, *some* apps have really long and nasty window captions
    this looks clutterd, so we allow to crop them a bit and remove
    considered to be uninteresting informations ==================== */

    QString ret = string;

    /* 1st off: we assume the part beyond the last dash (if any) to be the
    uninteresting one (usually it's the apps name, if there's add info, that's
    more important to the user) ------------------------------------- */

    // Amarok 2 is however different...
//     if (ret.contains(" :: "))
//         ret = ret.section(" :: ", 0, -2, QString::SectionSkipEmpty );

    // ok, here's currently some conflict
    // in e.g. amarok, i'd like to snip "amarok" and preserve "<song> by <artist>"
    // but for e.g. k3b, i'd like to get rid of stupid
    // "the kde application to burn cds and dvds" ...
    foreach (const QString &s, kwin_seps) {
        if (ret.contains(s)) {
            ret = ret.section(s, 0, -2, QString::SectionSkipEmpty );
            break;
        }
    }
    
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
    else
    {   // TODO maybe check with regexp and word bounds?

        QRegExp rgxp( "\\b" + appName + "\\b" );
        rgxp.setCaseSensitivity(Qt::CaseInsensitive);
        int i = ret.indexOf( rgxp );
        if (i > -1)
            ret = ret.mid(i, appName.length());
        else
        {
            appName = info.windowClassClass();
            rgxp.setPattern( "\\b" + appName + "\\b" );
            i = ret.indexOf( rgxp );
            if (i > -1)
                ret = ret.mid(i, appName.length());
        }
    }

    // in general, remove leading [and trailing] blanks and special chars
    ret = ret.remove(QRegExp("^\\W*"));
    ret = ret.trimmed();

    if (ret.isEmpty())
        ret = string; // ...

    return ret;

//    KWindowInfo info(windowId(), NET::WMVisibleName | NET::WMName, NET::WM2WindowClass);
//    qDebug() << "BESPIN:" << info.windowClassClass() <<  << info.name() << info.visibleName();
//    QByteArray windowClassClass() const;
//    QByteArray windowClassName() const;
//    QString visibleName() const;
}
