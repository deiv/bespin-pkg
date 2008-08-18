/* Bespin widget style for Qt4
   Copyright (C) 2007 Thomas Luebking <thomas.luebking@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "draw.h"
#include "animator/aprogress.h"

static int step = -1;

void
BespinStyle::drawProgressBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(pb, ProgressBar);
    OPT_HOVER

    // groove + contents ======
    step = Animator::Progress::step(widget);
    drawProgressBarGroove(pb, painter, widget);
    drawProgressBarContents(pb, painter, widget);
    // label? =========
    if (hover && pb->textVisible)
        drawProgressBarLabel(pb, painter, widget);
    // reset step!
    step = -1;
}

// a nice round embedded chunk
static inline void
drawShape(QPainter *p, int s, int x = 0, int y = 0, bool outline = true)
{
    s -= 2;
    p->setPen(QPen(QColor(0,0,0,70),2));
    p->drawEllipse(x+1,y+2,s,s);
    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(QColor(255,255,255,outline ? 40 : 20),1));
    p->drawEllipse(x,y+1,s+2,s);
}

void
BespinStyle::drawProgressBarGC(const QStyleOption * option, QPainter * painter,
                               const QWidget * widget, bool content) const
{
    if (appType == GTK && !content)
        return; // looks really crap

    ASSURE_OPTION(pb, ProgressBarV2);

    bool reverse = option->direction == Qt::RightToLeft;
    if (pb->invertedAppearance)
        reverse = !reverse;
    const bool vertical = pb->orientation == Qt::Vertical;
    const bool busy = pb->maximum == 0 && pb->minimum == 0;
    int x,y,l,t;
    RECT.getRect(&x,&y,&l,&t);
    
    if (vertical) // swap width & height...
        { int h = x; x = y; y = h; l = RECT.height(); t = RECT.width(); }

    double val = 0.0;
    if (busy)
    {   // progress with undefined duration / stepamount
        if (step < 0)
            step = Animator::Progress::step(widget);
        val = -3.0 * step / l;
    }
    else
        val = pb->progress / double(pb->maximum - pb->minimum);

    // maybe there's nothing to do for us
    if (content)
        { if (val == 0.0) return; }
    else if (val == 1.0)
        return;

    // calculate chunk dimensions - you don't wanna know and i forgot... ;-P
    int s = qMin(qMax(l / 10, dpi.f16), t /*16*t/10*/);
    int ss = (10*s)/16;
    int n = l/s;
    if (vertical || reverse)
    {
        x = vertical ? RECT.bottom() : RECT.right();
        x -= ((l - n*s) + (s - ss))/2 + ss;
        s = -s;
    }
    else
        x += (l - n*s + s - ss)/2;
    y += (t-ss)/2;
    --x; --y;

    // cause most chunks will look the same we render ONE into a buffer and then just dump that multiple times...
    QPixmap renderPix(ss+2,ss+2);
    renderPix.fill(Qt::transparent);
    QPainter p(&renderPix);
    p.setRenderHint(QPainter::Antialiasing);

    // draw a chunk
    int nn = (val < 0) ? 0 : int(n*val);
    if (content)
        p.setBrush(Gradients::pix(CCOLOR(progress.std, Fg), ss, Qt::Vertical, GRAD(progress) ));
    else
    {   // this is the "not-yet-done" part - in case we're currently painting it...
        if (busy)
            nn = n;
        else
            { x += nn*s; nn = n - nn; }
        const QColor c = CCOLOR(progress.std, Bg);
        p.setBrush(Gradients::pix(c, ss, Qt::Vertical, GRAD(progress) ));
    }
    p.setBrushOrigin(0,1);
    drawShape(&p, ss);
    p.end();

    if (vertical) // x is in fact y!
        for (int i = 0; i < nn; ++i)
            { painter->drawPixmap(y,x, renderPix); x+=s; }
    else // x is as expected... - gee my math teacher was actually right: "always label the axis!"
        for (int i = 0; i < nn; ++i)
            { painter->drawPixmap(x,y, renderPix); x+=s; }

    // if we're painting the actual progress, ONE chunk may be "semifinished" - that's done below
    if (content)
    {
        bool b = (nn < n);
//       x+=2; y+=2; ss-=2;
        if (busy)
        {   // the busy indicator has always a semifinished item, but we need to calculate which first
            b = true;
            val = -val; nn = int(n*val); x += nn*s;
            double o = n*val - nn;
            if (o < .5)
                val += o/n;
            else
                val += (1.0-2*o)/n;
        }
        if (b)
        {
            int q = int((10*n)*val) - 10*nn;
            if (q)
            {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);

                const QColor c = Colors::mid(CCOLOR(progress.std, Bg), CCOLOR(progress.std, Fg), 10-q, q);
                painter->setBrush(Gradients::pix(c, ss, Qt::Vertical, GRAD(progress) ));

                if (vertical) // swap again, we abuse 'q' from above
                    { q = x; x = y; y = q; }

                painter->setBrushOrigin(0, y);
                drawShape(painter, ss, x, y, false);
                painter->restore();
            }
        }
    }
}

void
BespinStyle::drawProgressBarLabel(const QStyleOption *option, QPainter *painter, const QWidget*) const
{
    ASSURE_OPTION(progress, ProgressBarV2);
   
    painter->save();
    QRect rect = RECT;
    if (progress->orientation == Qt::Vertical)
    {   // vertical progresses have text rotated by 90° or 270°
        QMatrix m;
        int h = rect.height(); rect.setHeight(rect.width()); rect.setHeight(h);
        if (progress->bottomToTop)
            { m.translate(0.0, RECT.height()); m.rotate(-90); }
        else
            { m.translate(RECT.width(), 0.0); m.rotate(90); }
        painter->setMatrix(m);
    }
    painter->setPen(FCOLOR(Window));
    int flags = Qt::AlignCenter | Qt::TextSingleLine;
    // "burred" background contrast
    rect.translate(-1,-1);
    painter->drawText(rect, flags, progress->text);
    rect.translate(0,2);
    painter->drawText(rect, flags, progress->text);
    rect.translate(2,0);
    painter->drawText(rect, flags, progress->text);
    rect.translate(0,-2);
    painter->drawText(rect, flags, progress->text);
    rect.translate(-1,1);
    // text
    painter->setPen(FCOLOR(WindowText));
    painter->drawText(rect, flags, progress->text);
    painter->restore();
}

//    case PE_IndicatorProgressChunk: // Section of a progress bar indicator; see also QProgressBar.
