
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QX11Info>

#include <QtDebug>

#include "xproperty.h"
#include "shadows.h"

using namespace Bespin;

static QPixmap (*pixmaps[2])[8];
static unsigned long globalShadowData[2][12];

static unsigned long*
shadowData(Shadows::Type t, bool storeToRoot)
{
    unsigned long *data = XProperty::get<unsigned long>(QX11Info::appRootWindow(), XProperty::bespinShadow[t-1], XProperty::LONG, 12);
    if (!data)
    {
        const int sz = t == Shadows::Large ? 32 : 20;

        globalShadowData[t-1][8] = (sz-4)/2;
        globalShadowData[t-1][9] = 2*(sz-4)/3;
        globalShadowData[t-1][10] = sz-4;
        globalShadowData[t-1][11] = 2*(sz-4)/3;

        if (!pixmaps[t-1])
        {
            QPixmap *store = new QPixmap[8];
            pixmaps[t-1] = (QPixmap (*)[8])store;

            // radial gradient requires the raster engine anyway and we need *working* ... -> QImage
            QImage shadow(2*sz+1, 2*sz+1, QImage::Format_ARGB32);
            shadow.fill(Qt::transparent);

            QRadialGradient rg(QPoint(sz+1,sz+1),sz);

            QPainter p(&shadow);
            p.setPen(Qt::NoPen);

            rg.setColorAt(0, QColor(0,0,0,112-sz)); rg.setColorAt(0.98, QColor(0,0,0,0));
            p.setBrush(rg);
            p.drawRect(shadow.rect());

            rg.setStops(QGradientStops());

            rg.setColorAt(0, QColor(0,0,0,96-sz)); rg.setColorAt(0.80, QColor(0,0,0,0));
            p.setBrush(rg);
            p.drawRect(shadow.rect());

            rg.setStops(QGradientStops());

            rg.setColorAt(0, QColor(0,0,0,72-sz)); rg.setColorAt(0.66, QColor(0,0,0,0));
            p.setBrush(rg);
            p.drawRect(shadow.rect());

            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            p.setRenderHint(QPainter::Antialiasing);
            p.setBrush(Qt::transparent);
            p.drawRoundedRect(shadow.rect().adjusted(globalShadowData[t-1][9], globalShadowData[t-1][8],
                                                     -globalShadowData[t-1][11], -globalShadowData[t-1][10]), 4,4);
            p.end();

            QPixmap shadowPix = QPixmap::fromImage(shadow);

            store[0] = shadowPix.copy(sz, 0, 1, sz); // topcenter -> clockwise
            store[1] = shadowPix.copy(sz+1, 0, sz, sz);
            store[2] = shadowPix.copy(sz+1, sz, sz, 1);
            store[3] = shadowPix.copy(sz+1, sz+1, sz, sz);
            store[4] = shadowPix.copy(sz, sz+1, 1, sz);
            store[5] = shadowPix.copy(0, sz+1, sz, sz);
            store[6] = shadowPix.copy(0, sz, sz, 1);
            store[7] = shadowPix.copy(0, 0, sz, sz); // topleft
        }
        for (int i = 0; i < 8; ++i)
            globalShadowData[t-1][i] = (*pixmaps[t-1])[i].handle();

        data = &globalShadowData[t-1][0];
        if (storeToRoot)
            XProperty::set(QX11Info::appRootWindow(), XProperty::bespinShadow[t-1], data, XProperty::LONG, 12);
    }
    return data;
}


BLIB_EXPORT void
Shadows::set(WId id, Shadows::Type t, bool storeToRoot)
{
    if (id == QX11Info::appRootWindow()) {
        qWarning("BESPIN WARNING! Setting shadow to ROOT window is NOT supported");
        return;
    }
    switch(t)
    {
    case Shadows::None:
        XProperty::remove(id, XProperty::kwinShadow);
        break;
    case Shadows::Large:
    case Shadows::Small:
        XProperty::set(id, XProperty::kwinShadow, shadowData(t, storeToRoot), XProperty::LONG, 12);
    default:
        break;
    }
}

