
#include <cmath>
#include <QPainter>
#include <QPixmap>

#include "../makros.h"
#include "dpi.h"
#include "elements.h"

using namespace Bespin;

// #define fillRect(_X_,_Y_,_W_,_H_,_B_) setPen(Qt::NoPen); p.setBrush(_B_); p.drawRect(_X_,_Y_,_W_,_H_)
// #define fillRect2(_R_,_B_) setPen(Qt::NoPen); p.setBrush(_B_); p.drawRect(_R_)

#if QT_VERSION >= 0x040400
#define DRAW_ROUND_RECT(_X_,_Y_,_W_,_H_,_RX_,_RY_) drawRoundedRect(_X_, _Y_, _W_, _H_, _RX_, _RY_, Qt::RelativeSize)
#else
#define DRAW_ROUND_RECT(_X_,_Y_,_W_,_H_,_RX_,_RY_) drawRoundRect(_X_, _Y_, _W_, _H_, _RX_, _RY_)
#endif

#define SCALE(_N_) lround(_N_*ourScale)

#define EMPTY_PIX(_W_, _H_) \
QPixmap pix = transSrc->copy(0,0,_W_,_H_);\
QPainter p(&pix); p.setRenderHint(QPainter::Antialiasing);\
p.setPen(Qt::NoPen)

static QColor black = Qt::black;
#define SET_ALPHA(_A_) black.setAlpha(_A_); p.setBrush(black)
#define WHITE(_A_) QColor(255,255,255, _A_)
#define BLACK(_A_) QColor(0,0,0, _A_)

#if 0
void
Elements::renderButtonLight(Tile::Set &set)
{
   NEW_EMPTY_PIX(f9, f9);
   SET_ALPHA(30);
   p.drawRoundRect(0,0,f9,f9,90,90);
   SET_ALPHA(54);
   p.drawRoundRect(F(1),F(1),f9-2*F(1),f9-2*F(1),80,80);
   SET_ALPHA(64);
   p.drawRoundRect(F(2),F(2),f9-2*F(2),f9-2*F(2),70,70);
   SET_ALPHA(74);
   p.drawRoundRect(F(3),F(3),f9-2*F(3),f9-2*F(3),60,60);
   p.end();
   set = Tile::Set(*pix,f9_2,f9_2,f9-2*f9_2,f9-2*f9_2);
   set.setDefaultShape(Tile::Ring);
   delete pix;
}
#endif

BLIB_EXPORT QPixmap *transSrc = 0;

static float ourShadowIntensity = 1.0;
void
Elements::setShadowIntensity(float intensity) { ourShadowIntensity = intensity; }
static float ourScale = 1.0;
void
Elements::setScale(float scale) { ourScale = scale; }

QPixmap
Elements::shadow(int size, bool opaque, bool sunken, float factor)
{
    EMPTY_PIX(size, size);
    float d = size/2.0;
    QRadialGradient rg(d, d, d);
    const int alpha = (int) (ourShadowIntensity * factor * (sunken ? 70 : (opaque ? 48 : 20)));
    rg.setColorAt(0.7, BLACK(CLAMP(alpha,0,255)));
    rg.setColorAt(1.0, BLACK(0));
    p.fillRect(pix.rect(), rg);
    p.end();
    return pix;
}

QPixmap
Elements::roundMask(int size)
{
    EMPTY_PIX(size, size); p.setBrush(Qt::black);
    p.drawEllipse(pix.rect()); p.end();
    return pix;
}

QPixmap
Elements::roundedMask(int size, int factor)
{
    EMPTY_PIX(size, size); p.setBrush(Qt::black);
#if QT_VERSION >= 0x040400
    p.drawRoundedRect(pix.rect(), factor, factor, Qt::RelativeSize);
#else
    p.drawRoundRect(pix.rect(), factor, factor);
#endif
    p.end();
    return pix;
}

QPixmap
Elements::sunkenShadow(int size, bool enabled)
{
    QImage *tmpImg = new QImage(size,size, QImage::Format_ARGB32);
    tmpImg->fill(Qt::transparent); QPainter p(tmpImg);
    p.setRenderHint(QPainter::Antialiasing); p.setPen(Qt::NoPen);

    int add = enabled*30;
    const int add2 = lround(80./F(4));
    const int rAdd = lround(25./F(4));

    // draw a flat shadow
    SET_ALPHA(sqrt(ourShadowIntensity) * (55 + add));
    p.DRAW_ROUND_RECT(0, 0, size, size-F(2), 80, 80);

    // subtract light
    p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
    add = 100 + 30 - add; int xOff;
    for (int i = 1; i <= F(4); ++i)
    {
        xOff = qMax(i-F(2),0);
        SET_ALPHA(add+i*add2);
        p.DRAW_ROUND_RECT(xOff, i, size-2*xOff, size-(F(2)+i), 75+rAdd, 75+rAdd);
    }

    // add bottom highlight
    p.setCompositionMode( QPainter::CompositionMode_SourceOver );
    p.fillRect(F(3),size-F(2),size-2*F(3),F(1), BLACK(10));
    int w = size/F(3);
    p.fillRect(w,size-F(1),size-2*w,F(1), WHITE(30));

    p.end();

    // create pixmap from image
    QPixmap ret = QPixmap::fromImage(*tmpImg);
    delete tmpImg; return ret;
}

QPixmap
Elements::relief(int size, bool enabled)
{
    const float f = ourShadowIntensity * (enabled ? 1.0 : 0.7);
    EMPTY_PIX(size, size);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(BLACK(int(f*70)), F(1)));
    p.DRAW_ROUND_RECT(0, F(1)/2.0, size, size-.75*F(2), 99, 99);
    p.setPen(QPen(WHITE(int(f*35)), F(1)));
    p.DRAW_ROUND_RECT(0, F(1)/2.0, size, size-F(1), 99, 99);
#if 0
    // the borders cross the pixmap boundings, thus they're too weak, thus we stregth them a bit
    const int d1 = 0.3*size, d2 = 0.7*size;
    p.drawLine(d1, 0, d2, 0); // top
    p.drawLine(0, d1, 0, d2); // left
    p.drawLine(size, d1, size, d2); // right
    p.setPen(QPen(WHITE(int(f*50)), F(1)));
    p.drawLine(d1, size-1, d2, size-1); // bottom
#endif
    p.end(); return pix;
}

#define DRAW_ROUND_ALPHA_RECT(_A_, _X_, _Y_, _W_,_R_)\
p.setBrush(BLACK(_A_)); p.DRAW_ROUND_RECT(_X_, _Y_, _W_, ss, (_R_+1)/2, _R_)

QPixmap
Elements::groupShadow(int size)
{
    const int ss = 2*size;
    QImage *tmpImg = new QImage(size,size, QImage::Format_ARGB32);
    tmpImg->fill(Qt::transparent);
    QPainter p(tmpImg);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);

    DRAW_ROUND_ALPHA_RECT(5, 0, 0, size, 48);
    DRAW_ROUND_ALPHA_RECT(9, F(1), F(1), size-F(2), 32);
    DRAW_ROUND_ALPHA_RECT(11, F(2), F(2), size-F(4), 20);
    DRAW_ROUND_ALPHA_RECT(13, F(3), F(3), size-F(6), 12);
    
    p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
    p.setBrush(BLACK(0)); p.DRAW_ROUND_RECT(F(4), F(2), size-F(8), ss, 6, 11);

    p.setCompositionMode( QPainter::CompositionMode_SourceOver );
    p.setPen(WHITE(60));
    p.setBrush(Qt::NoBrush);
    p.DRAW_ROUND_RECT(F(4), F(2), size-F(8), ss, 6, 11);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
    int f33 = SCALE(33);
    for (int i = 1; i < f33; ++i)
    {
        p.setPen(BLACK(CLAMP(i*lround(255.0/F(32)),0,255)));
        p.drawLine(0, size-i, size, size-i);
    }
    p.end();

    // create pixmap from image
    QPixmap ret = QPixmap::fromImage(*tmpImg);
    delete tmpImg; return ret;
}

#if 0
void
Elements::renderLightLine(Tile::Line &line)
{
   int f49 = SCALE(49);
   int f49_2 = (f49-1)/2;
   NEW_EMPTY_PIX(f49,f49);
   QRadialGradient rg( pix->rect().center(), f49_2 );
   rg.setColorAt ( 0, WHITE(160) ); rg.setColorAt ( 1, WHITE(0) );
   p.fillRect(0,0,f49,f49,rg);
   p.end();
   QPixmap tmp = pix->scaled( f49, dpi.f5, Qt::IgnoreAspectRatio,
                              Qt::SmoothTransformation );
   tmp = tmp.copy(0,F(2),f49,F(3));
   line = Tile::Line(tmp,Qt::Horizontal,f49_2,-f49_2);
   delete pix;
}
#endif

#undef fillRect
