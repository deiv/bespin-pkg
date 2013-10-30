#include "WM.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <QX11Info>

static Atom netMoveResize = XInternAtom(QX11Info::display(), "_NET_WM_MOVERESIZE", False);
#endif

using namespace Bespin;

void WM::triggerMove(WId id, QPoint p)
{
    triggerMoveResize(id, p, Move);
}

void WM::triggerMoveResize(WId id, QPoint p, Direction d)
{
#ifdef Q_WS_X11
    // stolen... errr "adapted!" from QSizeGrip
    QX11Info info;
    XEvent xev;
    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = netMoveResize;
    xev.xclient.display = QX11Info::display();
    xev.xclient.window = id;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = p.x();
    xev.xclient.data.l[1] = p.y();
    xev.xclient.data.l[2] = d;
    xev.xclient.data.l[3] = Button1;
    xev.xclient.data.l[4] = 0;
    XUngrabPointer(QX11Info::display(), QX11Info::appTime());
    XSendEvent(QX11Info::display(), QX11Info::appRootWindow(info.screen()), False,
                SubstructureRedirectMask | SubstructureNotifyMask, &xev);
#endif // Q_WS_X11
}
