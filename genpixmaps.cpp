
#include <cmath>
#include <QPainter>
#include <QPixmap>

#include "bespin.h"
#include "makros.h"

using namespace Bespin;

// #define fillRect(_X_,_Y_,_W_,_H_,_B_) setPen(Qt::NoPen); p.setBrush(_B_); p.drawRect(_X_,_Y_,_W_,_H_)
// #define fillRect2(_R_,_B_) setPen(Qt::NoPen); p.setBrush(_B_); p.drawRect(_R_)

#if QT_VERSION >= 0x040400
#define DRAW_ROUND_RECT(_X_,_Y_,_W_,_H_,_RX_,_RY_) drawRoundedRect(_X_, _Y_, _W_, _H_, _RX_, _RY_, Qt::RelativeSize)
#else
#define DRAW_ROUND_RECT(_X_,_Y_,_W_,_H_,_RX_,_RY_) drawRoundRect(_X_, _Y_, _W_, _H_, _RX_, _RY_)
#endif

#define SCALE(_N_) lround(_N_*Style::config.scale)

#define EMPTY_PIX(_W_, _H_) \
QPixmap pix = transSrc->copy(0,0,_W_,_H_);\
QPainter p(&pix); p.setRenderHint(QPainter::Antialiasing);\
p.setPen(Qt::NoPen)

#define SET_ALPHA(_A_) black.setAlpha(_A_); p.setBrush(black)

static QColor black = Qt::black;
#if 0
static void
renderButtonLight(Tile::Set &set)
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

#define WHITE(_A_) QColor(255,255,255, _A_)
#define BLACK(_A_) QColor(0,0,0, _A_)

static QPixmap *transSrc = 0;

static QPixmap
shadow(int size, bool opaque, bool sunken, float factor = 1.0)
{
    EMPTY_PIX(size, size);
    float d = size/2.0;
    QRadialGradient rg(d, d, d);
    const int alpha = (int) (Style::config.shadowIntensity * factor * (sunken ? 70 : (opaque ? 48 : 20)));
    rg.setColorAt(0.7, BLACK(CLAMP(alpha,0,255)));
    rg.setColorAt(1.0, BLACK(0));
    p.fillRect(pix.rect(), rg);
    p.end();
    return pix;
}

static QPixmap
roundMask(int size)
{
    EMPTY_PIX(size, size); p.setBrush(Qt::black);
    p.drawEllipse(pix.rect()); p.end();
    return pix;
}

static QPixmap
roundedMask(int size, int factor)
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

static QPixmap
sunkenShadow(int size, bool enabled)
{
    QImage *tmpImg = new QImage(size,size, QImage::Format_ARGB32);
    tmpImg->fill(Qt::transparent); QPainter p(tmpImg);
    p.setRenderHint(QPainter::Antialiasing); p.setPen(Qt::NoPen);

    int add = enabled*30;
    const int add2 = lround(80./F(4));
    const int rAdd = lround(25./F(4));

    // draw a flat shadow
    SET_ALPHA(sqrt(Style::config.shadowIntensity) * (55 + add));
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

static QPixmap
relief(int size, bool enabled)
{
    const float f = Style::config.shadowIntensity * (enabled ? 1.0 : 0.7);
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

static QPixmap
groupShadow(int size)
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
static void
renderLightLine(Tile::Line &line)
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

void
Style::generatePixmaps()
{
    const int f9 = F(9); const int f11 = SCALE(11);
    const int f13 = SCALE(13); const int f17 = SCALE(17);
    const int f49 = SCALE(49);

    // NOTICE!!! dpi.SliderControl is currently the biggest item using the transpix,
    // increase this in case we need it for bigger things...
    transSrc = new QPixmap(Style::dpi.SliderControl, Style::dpi.SliderControl);
    transSrc->fill(Qt::transparent);

    // MASKS =======================================
    for (int i = 0; i < 2; ++i)
    {
        int s,r;
        if (i)
            { s = f13; r = 99; }
        else
            { s = f9; r = 70; }
        masks.rect[i] = Tile::Set(roundedMask(s, r),s/2,s/2,1,1, r);
    }

    // SHADOWS ===============================
    // sunken
    for (int r = 0; r < 2; ++r)
    {
        int s = r ? f17 : f9;
        for (int i = 0; i < 2; ++i)
        {
            shadows.sunken[r][i] = Tile::Set(sunkenShadow(s, i), s/2,s/2,1,1);
            shadows.sunken[r][i].setDefaultShape(Tile::Ring);
        }
    }

    // relief
    for (int r = 0; r < 2; ++r)
    {
        int s = r ? f17 : f11;
        for (int i = 0; i < 2; ++i)
        {
            shadows.relief[r][i] = Tile::Set(relief(s, i), s/2,s/2,1,1);
            shadows.relief[r][i].setDefaultShape(Tile::Ring);
        }
    }

    // raised
    for (int r = 0; r < 2; ++r)
    {
        int s;  float f = .8;
        s =  r ? f17 : f9;
        for (int i = 0; i < 2; ++i) // opaque?
            for (int j = 0; j < 2; ++j)
            {   // sunken?
                shadows.raised[r][i][j] = Tile::Set(shadow(s,i,j,f), s/2, s/2, 1, 1);
                shadows.raised[r][i][j].setDefaultShape(Tile::Ring);
            }
    }

    
    // fallback ( sunken ) // TODO: raised
    QPixmap tmp = transSrc->copy(0, 0, f9, f9);
    QPainter p;
    p.begin(&tmp);
    p.fillRect(F(1),0,f9-F(2),F(1), BLACK(10));
    p.fillRect(F(2),F(1),f9-F(4),F(1), BLACK(20));
    p.fillRect(F(2),F(2),f9-F(4),F(1), BLACK(40));
    p.fillRect(F(3),F(3),f9-F(6),F(1), BLACK(80));

    p.fillRect(F(1),f9-F(1),f9-F(2),F(1), WHITE(10));
    p.fillRect(F(2),f9-F(2),f9-F(4),F(1), WHITE(20));
    p.fillRect(F(2),f9-F(3),f9-F(4),F(1), WHITE(40));
    p.fillRect(F(3),f9-F(4),f9-F(6),F(1), WHITE(80));

    p.fillRect(0,F(1),F(1),f9-F(2), QColor(128,128,128,10));
    p.fillRect(F(1),F(2),F(1),f9-F(4), QColor(128,128,128,20));
    p.fillRect(F(2),F(2),F(1),f9-F(4), QColor(128,128,128,40));
    p.fillRect(F(3),F(3),F(1),f9-F(6), QColor(128,128,128,80));

    p.fillRect(f9-F(1),F(1),F(1),f9-F(2), QColor(128,128,128,10));
    p.fillRect(f9-F(2),F(2),F(1),f9-F(4), QColor(128,128,128,20));
    p.fillRect(f9-F(3),F(2),F(1),f9-F(4), QColor(128,128,128,40));
    p.fillRect(f9-F(4),F(3),F(1),f9-F(6), QColor(128,128,128,80));

    p.end();
    shadows.fallback = Tile::Set(tmp,f9/2,f9/2,1,1);
    shadows.fallback.setDefaultShape(Tile::Ring);
    // ================================================================

    // LIGHTS ==================================
    for (int r = 0; r < 2; ++r)
    {
        int s = r ? f17 : f11;
        lights.rect[r] = Tile::Set(shadow(s, true, false, 3.0), s/2,s/2,1,1);
        lights.rect[r].setDefaultShape(Tile::Ring);
    }

   // toplight -- UNUSED!
//    renderLightLine(lights.top);
   
    // ================================================================
   
    // SLIDER =====================================
    // shadow
    for (int i = 0; i < 2; ++i)
    {   // opaque?
        shadows.slider[i][false] = shadow(Style::dpi.SliderControl, i,false);
        shadows.slider[i][true] = shadow(Style::dpi.SliderControl-F(2), i,true);
    }
    lights.slider = shadow(Style::dpi.SliderControl, true, false, 3.0);
    masks.slider = roundMask(Style::dpi.SliderControl-F(4));
    // ================================================================

    // RADIOUTTON =====================================
    // shadow
    for (int i = 0; i < 2; ++i)
    {   // opaque?
        shadows.radio[i][false] = shadow(Style::dpi.ExclusiveIndicator, i,false);
        shadows.radio[i][true] = shadow(Style::dpi.ExclusiveIndicator-F(2), i,true);
    }
    // mask
    masks.radio = roundMask(Style::dpi.ExclusiveIndicator-F(4));
    // mask fill
#if 0
    masks.radioIndicator = roundMask(Style::dpi.ExclusiveIndicator - (config.btn.layer ? dpi.f10 : dpi.f12));
#else
    int s = (Style::dpi.ExclusiveIndicator)/4; s *= 2; // cause of int div...
    s += F(2); // cause sunken frame "outer" part covers F(2) pixels
    masks.radioIndicator = roundMask(Style::dpi.ExclusiveIndicator - s);
#endif
    // ================================================================
    // NOTCH =====================================
    masks.notch = roundMask(F(6));
    // ================================================================


// GROUPBOX =====================================
    // shadow
    shadows.group = Tile::Set(groupShadow(f49),F(12),F(12),f49-2*F(12),F(1));
    shadows.group.setDefaultShape(Tile::Ring);
    // ================================================================

    // LINES =============================================
    int f49_2 = (f49-1)/2;
    QLinearGradient lg; QGradientStops stops;
    int w,h,c1,c2;
    for (int i = 0; i < 2; ++i)
    {   // orientarion
        if (i)
        {
            w = F(2); h = f49;
            lg = QLinearGradient(0,0,0,f49);
        }
        else
        {
            w = f49; h = F(2);
            lg = QLinearGradient(0,0,f49,0);
        }
        tmp = QPixmap(w,h);
        for (int j = 0; j < 3; ++j)
        {   // direction
            c1 = (j > 0) ? 255 : 111; c2 = (j > 0) ? 111 : 255;
            tmp.fill(Qt::transparent); p.begin(&tmp);

            stops   << QGradientStop( 0, QColor(c1,c1,c1,0) )
                    << QGradientStop( 0.5, QColor(c1,c1,c1,71) )
                    << QGradientStop( 1, QColor(c1,c1,c1,0) );
            lg.setStops(stops);
            if (i)
                { p.fillRect(0,0,F(1),f49,lg); }
            else
                { p.fillRect(0,0,f49,F(1),lg); }
            stops.clear();

            stops   << QGradientStop( 0, QColor(c2,c2,c2,0) )
                    << QGradientStop( 0.5, QColor(c2,c2,c2,74) )
                    << QGradientStop( 1, QColor(c2,c2,c2,0) );
            lg.setStops(stops);
            if (i)
                { p.fillRect(F(1),0,F(2)-F(1),f49,lg); }
            else
                { p.fillRect(0,F(1),f49,F(2)-F(1),lg); }
            stops.clear();

            p.end();
            shadows.line[i][j] = Tile::Line(tmp, i ? Qt::Vertical : Qt::Horizontal, f49_2, -f49_2);
        }
    }
    // ================================================================
    
    // ================================================================
    // Popup corners - not really pxmaps, though ;) ===================
    // they at least break beryl's popup shadows...
    // see bespin.cpp#Style::eventfilter as well
    int f5 = 4;
    QBitmap bm(2*f5, 2*f5);
    bm.fill(Qt::black);
    p.begin(&bm);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);
    p.drawEllipse(0,0,2*f5,2*f5);
    p.end();
    QRegion circle(bm);
    masks.corner[0] = circle & QRegion(0,0,f5,f5); // tl
    masks.corner[1] = circle & QRegion(f5,0,f5,f5); // tr
    masks.corner[1].translate(-masks.corner[1].boundingRect().left(), 0);
    masks.corner[2] = circle & QRegion(0,f5,f5,f5); // bl
    masks.corner[2].translate(0, -masks.corner[2].boundingRect().top());
    masks.corner[3] = circle & QRegion(f5,f5,f5,f5); // br
    masks.corner[3].translate(-masks.corner[3].boundingRect().topLeft());
    // ================================================================

    delete transSrc;
}
#undef fillRect
