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

#ifndef Q_WS_X11
#define QT_NO_XRENDER #
#endif

#ifndef OXRENDER_H
#define OXRENDER_H

#include <QPixmap>
// #include <QVector>
#ifndef QT_NO_XRENDER

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include "fixx11h.h"

typedef Picture OXPicture;
typedef Pixmap OXPixmap;

#endif

// typedef QVector<double> PointArray;
// typedef QVector<QColor> ColorArray;

namespace FX
{
#ifndef QT_NO_XRENDER
    void freePicture(OXPicture pict);
    void composite( OXPicture src, OXPicture mask, OXPicture dst,
                    int sx, int sy, int mx, int my, int dx, int dy,
                    uint w, uint h, int op = PictOpSrc);
    void composite( OXPicture src, OXPicture mask, const QPixmap &dst,
                    int sx, int sy, int mx, int my, int dx, int dy,
                    uint w, uint h, int op = PictOpSrc);
    void composite( const QPixmap &src, OXPicture mask, const QPixmap &dst,
                    int sx, int sy, int mx, int my, int dx, int dy,
                    uint w, uint h, int op = PictOpSrc);

    void setColor(XRenderColor &xc, double r, double g, double b, double a = 1);
    void setColor(XRenderColor &xc, QColor qc);
#endif

    bool blend(const QPixmap &upper, QPixmap &lower, double opacity = 0.5, int x = 0, int y = 0);
    void desaturate(QImage &img, const QColor &c);
    QPixmap fade(const QPixmap &pix, double percent);
    QPixmap tint(const QPixmap &mask, const QColor &color);
    QPixmap applyAlpha( const QPixmap &toThisPix, const QPixmap &fromThisPix, const QRect &rect = QRect(), const QRect &alphaRect = QRect());
    void expblur(QImage &img, int radius);
//    QPixmap applyAlpha(const QPixmap &toThisPix,
//                       const OXPicture &fromThisPict,
//                       const QRect &rect = QRect(),
//                       const QRect &alphaRect = QRect());
#if 0
    void setAlpha(QPixmap &pix, const OXPicture &mask);
    
// -- couple of XRender versions don't know + others are broken, so we'll leave it for the moment
   void setGradient(XLinearGradient &lg, QPoint p1, QPoint p2);
   void setGradient(XLinearGradient &lg,
                    XFixed x1, XFixed y1,
                    XFixed x2, XFixed y2);
   OXPicture gradient(const QPoint start, const QPoint stop,
                      const ColorArray &colors,
                      const PointArray &stops = PointArray());
   OXPicture gradient(const QPoint c1, int r1, const QPoint c2, int r2,
                      const ColorArray &colors,
                      const PointArray &stops = PointArray());
#endif
}

#define Q2XRenderColor(_XRC_, _QC_) XRenderColor _XRC_; FX::setColor(_XRC_, _QC_)

#endif //OXRENDER_H
