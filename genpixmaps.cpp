
#include <cmath>
#include <QPainter>
#include <QPixmap>

#include "bespin.h"
#include "makros.h"

using namespace Bespin;

extern Dpi dpi;
extern Config config;

// #define fillRect(_X_,_Y_,_W_,_H_,_B_) setPen(Qt::NoPen); p.setBrush(_B_); p.drawRect(_X_,_Y_,_W_,_H_)
// #define fillRect2(_R_,_B_) setPen(Qt::NoPen); p.setBrush(_B_); p.drawRect(_R_)

#ifdef QT_NO_XRENDER
// simply sets the pixmaps alpha value to all rgb (i.e. grey) channels
// TODO: maybe adjust rgb to psychovisual values? (qGrey() inversion)
static QPixmap rgbFromAlpha(const QPixmap &pix) {
   QImage img = pix.toImage();
   unsigned int *data = ( unsigned int * ) img.bits();
   int total = img.width() * img.height(), alpha;
   for ( int i = 0 ; i < total ; ++i ) {
      alpha = qAlpha(data[i]);
      data[i] = qRgba( alpha, alpha, alpha, 255 );
   }
   return QPixmap::fromImage(img);
}
#define UPDATE_COLORS(_PIX_) _PIX_ = rgbFromAlpha(_PIX_);
#else
#define UPDATE_COLORS(_PIX_) //
#endif

#define SCALE(_N_) lround(_N_*config.scale)

#define NEW_EMPTY_PIX(_W_, _H_) \
QPixmap *pix = new QPixmap(_W_, _H_);\
pix->fill(Qt::transparent);\
QPainter p(pix);\
p.setRenderHint(QPainter::Antialiasing);\
p.setPen(Qt::NoPen)

#define EMPTY_PIX(_W_, _H_) \
QPixmap pix(size, size); pix.fill(Qt::transparent);\
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
   p.drawRoundRect(f1,f1,f9-2*f1,f9-2*f1,80,80);
   SET_ALPHA(64);
   p.drawRoundRect(f2,f2,f9-2*f2,f9-2*f2,70,70);
   SET_ALPHA(74);
   p.drawRoundRect(f3,f3,f9-2*f3,f9-2*f3,60,60);
   p.end();
   set = Tile::Set(*pix,f9_2,f9_2,f9-2*f9_2,f9-2*f9_2);
   set.setClipOffsets(f3, f3, f3, f3);
   set.setDefaultShape(Tile::Ring);
   delete pix;
}
#endif

#define WHITE(_A_) QColor(255,255,255, _A_)
#define BLACK(_A_) QColor(0,0,0, _A_)

static int f1, f2, f3, f4;

static QPixmap
shadow(int size, bool opaque, bool sunken, float factor = 1.0)
{
   EMPTY_PIX(size, size);
   float d = size/2.0;
   QRadialGradient rg(d, d, d);
   const int alpha =
      (int) (config.shadowIntensity * factor * (sunken ? 70 : (opaque ? 48 : 20)));
   rg.setColorAt(0.7, BLACK(CLAMP(alpha,0,255)));
   rg.setColorAt(1.0, BLACK(0));
   p.fillRect(pix.rect(), rg); p.end();
   return pix;
}

static QPixmap
roundMask(int size)
{
   EMPTY_PIX(size, size); p.setBrush(Qt::black);
   p.drawEllipse(pix.rect()); p.end();
   UPDATE_COLORS(pix);
   return pix;
}

static QPixmap
roundedMask(int size, int factor)
{
   EMPTY_PIX(size, size); p.setBrush(Qt::black);
   p.drawRoundRect(pix.rect(),factor,factor); p.end();
   UPDATE_COLORS(pix);
   return pix;
}

static QPixmap
sunkenShadow(int size, bool enabled)
{
   QImage *tmpImg = new QImage(size,size, QImage::Format_ARGB32);
   tmpImg->fill(Qt::transparent); QPainter p(tmpImg);
   p.setRenderHint(QPainter::Antialiasing); p.setPen(Qt::NoPen);

   int add = enabled*30;
   const int add2 = lround(80./f4);
   const int rAdd = lround(25./f4);

   // draw a flat shadow
   SET_ALPHA(55+add);
   p.drawRoundRect(0,0,size,size-f2,80,80);

   // subtract light
   p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
   add = 100 + 30 - add; int xOff;
   for (int i = 1; i <= f4; ++i) {
      xOff = qMax(i-f2,0); SET_ALPHA(add+i*add2);
      p.drawRoundRect(xOff,i,size-2*xOff,size-(f2+i), 75+rAdd, 75+rAdd);
   }

   // add bottom highlight
   p.setCompositionMode( QPainter::CompositionMode_SourceOver );
   p.fillRect(f3,size-f2,size-2*f3,f1, BLACK(10));
   int w = size/f3;
   p.fillRect(w,size-f1,size-2*w,f1, WHITE(30));
   
   p.end();

   // create pixmap from image
   QPixmap ret = QPixmap::fromImage(*tmpImg);
   delete tmpImg; return ret;
}

static QPixmap
relief(int size, bool enabled)
{
   const float f = enabled ? 1.0 : 0.7;
   EMPTY_PIX(size, size);
   p.setBrush(Qt::NoBrush);
   p.setPen(QPen(WHITE(int(f*70)), f1));
   p.drawRoundRect(0,0,size,size,80,80);
   p.setPen(QPen(BLACK(int(f*70)), f1));
   p.drawRoundRect(f1,f1,size-f2,size-f2,80,80);
   p.end(); return pix;
}

static QPixmap
groupShadow(int size)
{
   QImage *tmpImg = new QImage(size,size, QImage::Format_ARGB32);
   tmpImg->fill(Qt::transparent); QPainter p(tmpImg);
   p.setRenderHint(QPainter::Antialiasing); p.setPen(Qt::NoPen);

   int ss = 2*size;

   p.setBrush(QColor(0,0,0,5)); p.drawRoundRect(0,0,size,ss,14,7);
   p.setBrush(QColor(0,0,0,9)); p.drawRoundRect(f1,f1,size-f2,ss,13,7);
   p.setBrush(QColor(0,0,0,11)); p.drawRoundRect(f2,f2,size-f4,ss,12,6);
   p.setBrush(QColor(0,0,0,13)); p.drawRoundRect(f3,f3,size-dpi.f6,ss,48,24);
   p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
   p.setBrush(QColor(0,0,0,0)); p.drawRoundRect(f4,f2,size-dpi.f8,ss,11,6);
//    p.setCompositionMode( QPainter::CompositionMode_SourceOver );
//    p.setPen(QColor(255,255,255,200)); p.setBrush(Qt::NoBrush);
//    p.drawRoundRect(dpi.f4,dpi.f2,f49-dpi.f8,2*f49,11,6);
   p.setRenderHint(QPainter::Antialiasing, false);
//    p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
   int f33 = SCALE(33);
   for (int i = 1; i < f33; ++i) {
      p.setPen(QColor(0,0,0,CLAMP(i*lround(255.0/dpi.f32),0,255)));
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
   tmp = tmp.copy(0,f2,f49,dpi.f3);
   line = Tile::Line(tmp,Qt::Horizontal,f49_2,-f49_2);
   delete pix;
}
#endif
void BespinStyle::generatePixmaps()
{

   f1 = dpi.f1; f2 = dpi.f2; f3 = dpi.f3; f4 = dpi.f4;
   const int f9 = dpi.f9; const int f11 = SCALE(11);
   const int f13 = SCALE(13); const int f17 = SCALE(17);
   const int f49 = SCALE(49);

   // MASKS =======================================
   for (int i = 0; i < 2; ++i) {
      int s,r;
      if (i) {s = f13; r = 99;} else {s = f9; r = 70;}
      masks.rect[i] = Tile::Set(roundedMask(s, r),s/2,s/2,1,1, r);
      masks.rect[i].setClipOffsets(0,0,0,0);
   }
   
   // SHADOWS ===============================
   // sunken
   for (int r = 0; r < 2; ++r) {
      int s = r ? f17 : f9;
      for (int i = 0; i < 2; ++i) {
         shadows.sunken[r][i] = Tile::Set(sunkenShadow(s, i), s/2,s/2,1,1);
         shadows.sunken[r][i].setDefaultShape(Tile::Ring);
      }
   }

   // relief
   for (int r = 0; r < 2; ++r) {
      int s = r ? f17 : f11;
      for (int i = 0; i < 2; ++i) {
         shadows.relief[r][i] = Tile::Set(relief(s, i), s/2,s/2,1,1);
         shadows.relief[r][i].setDefaultShape(Tile::Ring);
      }
   }
   
   // raised
   for (int r = 0; r < 2; ++r) {
      int s;  float f;
      if (r) {s = f17; f = 0.8;} else {s = f9; f = .8;}
      for (int i = 0; i < 2; ++i) // opaque?
         for (int j = 0; j < 2; ++j) { // sunken?
            shadows.raised[r][i][j] = Tile::Set(shadow(s,i,j,f), s/2, s/2, 1, 1);
            shadows.raised[r][i][j].setDefaultShape(Tile::Ring);
         }
   }

   // fallback ( sunken ) // TODO: raised
   int f6 = dpi.f6;
   QPixmap tmp = QPixmap(f9,f9); tmp.fill(Qt::transparent);
   QPainter p;
   p.begin(&tmp);
   p.fillRect(f1,0,f9-f2,f1, QColor(0,0,0,10));
   p.fillRect(f2,f1,f9-f4,f1, QColor(0,0,0,20));
   p.fillRect(f2,f2,f9-f4,f1, QColor(0,0,0,40));
   p.fillRect(f3,f3,f9-f6,f1, QColor(0,0,0,80));
   
   p.fillRect(f1,f9-f1,f9-f2,f1, QColor(255,255,255,10));
   p.fillRect(f2,f9-f2,f9-f4,f1, QColor(255,255,255,20));
   p.fillRect(f2,f9-f3,f9-f4,f1, QColor(255,255,255,40));
   p.fillRect(f3,f9-f4,f9-f6,f1, QColor(255,255,255,80));
   
   p.fillRect(0,f1,f1,f9-f2, QColor(128,128,128,10));
   p.fillRect(f1,f2,f1,f9-f4, QColor(128,128,128,20));
   p.fillRect(f2,f2,f1,f9-f4, QColor(128,128,128,40));
   p.fillRect(f3,f3,f1,f9-f6, QColor(128,128,128,80));
   
   p.fillRect(f9-f1,f1,f1,f9-f2, QColor(128,128,128,10));
   p.fillRect(f9-f2,f2,f1,f9-f4, QColor(128,128,128,20));
   p.fillRect(f9-f3,f2,f1,f9-f4, QColor(128,128,128,40));
   p.fillRect(f9-f4,f3,f1,f9-f6, QColor(128,128,128,80));
   
   p.end();
   shadows.fallback = Tile::Set(tmp,f9/2,f9/2,1,1);
   shadows.fallback.setDefaultShape(Tile::Ring);
   // ================================================================

   // LIGHTS ==================================
   for (int r = 0; r < 2; ++r) {
      int s = r ? f17 : f11;
      lights.rect[r] = Tile::Set(shadow(s, true, false, 3.0), s/2,s/2,1,1);
      lights.rect[r].setClipOffsets(f3,f3,f3,f3);
      lights.rect[r].setDefaultShape(Tile::Ring);
   }

   // toplight -- UNUSED!
//    renderLightLine(lights.top);
   
   // ================================================================
   
   // SLIDER =====================================
   // shadow
   for (int i = 0; i < 2; ++i) { // opaque?
         shadows.slider[i][false] = shadow(dpi.SliderControl, i,false);
         shadows.slider[i][true] = shadow(dpi.SliderControl-f2, i,true);
   }
   masks.slider = roundMask(dpi.SliderControl-f4);
   // ================================================================

   // RADIOUTTON =====================================
   // shadow
   for (int i = 0; i < 2; ++i) { // opaque?
      shadows.radio[i][false] = shadow(dpi.ExclusiveIndicator, i,false);
      shadows.radio[i][true] = shadow(dpi.ExclusiveIndicator-f2, i,true);
   }
   // mask
   masks.radio = roundMask(dpi.ExclusiveIndicator-f4);
   // mask fill
   masks.radioIndicator =
      roundMask(dpi.ExclusiveIndicator - (config.btn.layer ? dpi.f10 : dpi.f12));
   // ================================================================
   
   // NOTCH =====================================
   masks.notch = roundMask(dpi.f6);
   // ================================================================
   
   // GROUPBOX =====================================
   // shadow
   int f12 = dpi.f12;
   shadows.group = Tile::Set(groupShadow(f49),f12,f12,f49-2*f12,f1);
   shadows.group.setDefaultShape(Tile::Ring);
   // ================================================================
   
   // LINES =============================================
   int f49_2 = (f49-1)/2;
   QLinearGradient lg; QGradientStops stops;
   int w,h,c1,c2;
   for (int i = 0; i < 2; ++i) { // orientarion
      if (i) {
         w = f2; h = f49;
         lg = QLinearGradient(0,0,0,f49);
      }
      else {
         w = f49; h = f2;
         lg = QLinearGradient(0,0,f49,0);
      }
      tmp = QPixmap(w,h);
      for (int j = 0; j < 3; ++j) { // direction
         c1 = (j > 0) ? 255 : 111; c2 = (j > 0) ? 111 : 255;
         tmp.fill(Qt::transparent); p.begin(&tmp);

         stops << QGradientStop( 0, QColor(c1,c1,c1,0) )
            << QGradientStop( 0.5, QColor(c1,c1,c1,71) )
            << QGradientStop( 1, QColor(c1,c1,c1,0) );
         lg.setStops(stops);
         if (i) { p.fillRect(0,0,f1,f49,lg); }
         else {p.fillRect(0,0,f49,f1,lg);}
         stops.clear();
         
         stops << QGradientStop( 0, QColor(c2,c2,c2,0) )
            << QGradientStop( 0.5, QColor(c2,c2,c2,74) )
            << QGradientStop( 1, QColor(c2,c2,c2,0) );
         lg.setStops(stops);
         if (i) {p.fillRect(f1,0,f2-f1,f49,lg);}
         else {p.fillRect(0,f1,f49,f2-f1,lg);}
         stops.clear();

         p.end();
         shadows.line[i][j] =
            Tile::Line(tmp, i ? Qt::Vertical : Qt::Horizontal, f49_2, -f49_2);
      }
   }
#if SHAPE_POPUP
   // ================================================================
   // ================================================================
   // Popup corners - not really pxmaps, though ;) ===================
   // they at least break beryl's popup shadows...
   // see bespin.cpp#BespinStyle::eventfilter as well
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
#endif
}
#undef fillRect
