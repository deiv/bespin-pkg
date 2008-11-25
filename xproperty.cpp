//////////////////////////////////////////////////////////////////////////////
//
// -------------------
// Bespin style & window decoration for KDE
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

#include "xproperty.h"


using namespace Bespin;

#include <X11/Xatom.h>
#include <QX11Info>

Atom XProperty::winData = XInternAtom(QX11Info::display(), "BESPIN_WIN_DATA", False);
Atom XProperty::bgPics = XInternAtom(QX11Info::display(), "BESPIN_BG_PICS", False);
Atom XProperty::decoDim = XInternAtom(QX11Info::display(), "BESPIN_DECO_DIM", False);
Atom XProperty::pid = XInternAtom(QX11Info::display(), "_NET_WM_PID", False);

#if 1

void
XProperty::handleProperty(WId window, Atom atom, uchar **data, Type type, unsigned long n)
{
    if (*data) // this is ok, internally used only
    {
        XChangeProperty(QX11Info::display(), window, atom, XA_CARDINAL, type, PropModeReplace, *data, n );
        return;
    }
    int result, de; //dead end
    unsigned long nn, de2;
    result = XGetWindowProperty(QX11Info::display(), window, atom, 0L, n, False, XA_CARDINAL, &de2, &de, &nn, &de2, data);
    if (result != Success || *data == X::None || n != nn)
        *data = NULL; // superflous?!?
}

#else

unsigned char *
XProperty::get(WId window, Atom atom, unsigned long n)
{
    unsigned char *chardata = 0;
    int result, de; //dead end
    unsigned long nn, de2;
    result = XGetWindowProperty(QX11Info::display(), window, atom, 0L, n, False,
                                    XA_CARDINAL, &de2, &de, &nn, &de2, &chardata);

    if (result != Success || chardata == X::None || n != nn)
        return NULL;

    return chardata;

//     memcpy (&data, chardata, sizeof (int));
//     return true;
}

bool
XProperty::get(WId window, Atom atom, uint& data)
{
    uchar *chardata = 0;
    int result, de; //dead end
    unsigned long de2;
    result = XGetWindowProperty(QX11Info::display(), window, atom, 0L, sizeof(uint), False,
                                    XA_CARDINAL, &de2, &de, &de2, &de2, &chardata);

    if (result != Success || chardata == X::None)
        return false;

    memcpy (&data, chardata, sizeof (int));
    return true;
}

void
XProperty::set(WId window, Atom atom, uint data)
{
    XChangeProperty(QX11Info::display(), window, atom, XA_CARDINAL, 8, PropModeReplace, (const uchar*)&data, sizeof(uint));
}

void
XProperty::set(WId window, Atom atom, const uchar *data, unsigned long n)
{
   XChangeProperty(QX11Info::display(), window, atom, XA_CARDINAL, 8, PropModeReplace, (const uchar*)data, n);
}
#endif
#if 0

/* The below functions mangle 2 rbg (24bit) colors and a 2 bit hint into
a 32bit integer to be set as X11 property
Of course this is convulsive, but doesn't hurt for our purposes
::encode() is a bit trickier as it needs to decide whether the color values
should be rounded up or down like
x = qMin(lround(x/8.0),31) IS WRONG! as it would impact the hue and while
value manipulations are acceptable, hue values are NOT (this is a 8v stepping
per channel and as we're gonna create gradients out of the colors, black could
turn some kind of very dark red...)
Just trust and don't touch ;) (Yes future Thomas, this means YOU!)
======================================================================*/

#include <QtDebug>
uint
XProperty::encode(const QColor &bg, const QColor &fg, uint hint)
{
    int r,g,b; bg.getRgb(&r,&g,&b);
    int d = r%8 + g%8 + b%8;
    if (d > 10)
    {
        r = qMin(r+8, 255);
        g = qMin(g+8, 255);
        b = qMin(b+8, 255);
    }
    uint info = (((r >> 3) & 0x1f) << 27) | (((g >> 3) & 0x1f) << 22) | (((b >> 3) & 0x1f) << 17);

    fg.getRgb(&r,&g,&b);
    d = r%8 + g%8 + b%8;
    if (d > 10)
    {
        r = qMin(r+8, 255);
        g = qMin(g+8, 255);
        b = qMin(b+8, 255);
    }
    info |= (((r >> 3) & 0x1f) << 12) | (((g >> 3) & 0x1f) << 7) | (((b >> 3) & 0x1f) << 2) | hint & 3;
    return info;
}

void
XProperty::decode(uint info, QColor &bg, QColor &fg, uint &hint)
{
    bg.setRgb(  ((info >> 27) & 0x1f) << 3,
                ((info >> 22) & 0x1f) << 3,
                ((info >> 17) & 0x1f) << 3 );
    fg.setRgb(  ((info >> 12) & 0x1f) << 3,
                ((info >> 7) & 0x1f) << 3,
                ((info >> 2) & 0x1f) << 3 );
    hint = info & 3;
}
#endif
