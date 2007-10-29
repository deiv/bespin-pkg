
#include <cmath>
#include <QPainter>
#include <QPixmap>

#include "bespin.h"
#include "makros.h"

using namespace Bespin;

extern Dpi dpi;
extern Config config;

// #define fillRect(_X_,_Y_,_W_,_H_,_B_) setPen(Qt::NoPen); p.setBrush(_B_); p.drawRect(_X_,_Y_,_W_,_H_)

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

#define SET_ALPHA(_A_) black.setAlpha(_A_); p.setBrush(black)

static int f1, f2, f3, f4, f7, f9, f11, f2_2, f9_2, f11_2;

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

static QPixmap
shadow(int size, bool opaque, bool sunken, float factor = 1.0)
{
   QPixmap pix(size, size);
   pix.fill(Qt::transparent);
   QPainter p(&pix);
   p.setRenderHint(QPainter::Antialiasing);
   p.setPen(Qt::NoPen);
   float d = size/2.0;
   QRadialGradient rg(d, d, d);
   int alpha = config.shadowIntensity * factor * (sunken ? 100 : (opaque ? 48 : 20));
   rg.setColorAt(0.7, BLACK(CLAMP(alpha,0,255)));
   rg.setColorAt(1.0, BLACK(0));
   p.fillRect(pix.rect(), rg); p.end();
   return pix;
}

static QPixmap
roundMask(int size)
{
   QPixmap pix(size, size); pix.fill(Qt::transparent);
   QPainter p(&pix); p.setRenderHint(QPainter::Antialiasing);
   p.setPen(Qt::NoPen); p.setBrush(Qt::black);
   p.drawEllipse(pix.rect()); p.end();
   UPDATE_COLORS(pix);
   return pix;
}

static QPixmap
roundedMask(int size, int factor)
{
   QPixmap pix(size, size); pix.fill(Qt::transparent);
   QPainter p(&pix); p.setRenderHint(QPainter::Antialiasing);
   p.setPen(Qt::NoPen); p.setBrush(Qt::black);
   p.drawRoundRect(pix.rect(),factor,factor); p.end();
   UPDATE_COLORS(pix);
   return pix;
}

static void
renderLineEditShadow(Tile::Set &set, bool enabled)
{
   QImage *tmpImg = new QImage(f9,f9, QImage::Format_ARGB32);
   tmpImg->fill(Qt::transparent);
   QPainter p(tmpImg);
   p.setRenderHint(QPainter::Antialiasing);
   p.setPen(Qt::NoPen);
   
   int add = enabled*30;
   SET_ALPHA(55+add);
   p.drawRoundRect(0,0,f9,f7,80,80);

   p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
   add = 30 - add;
   SET_ALPHA(120+add); p.drawRoundRect(0,f1,f9,dpi.f6,75,75);
   SET_ALPHA(140+add); p.drawRoundRect(0,f2,f9,dpi.f5,80,80);
   SET_ALPHA(160+add); p.drawRoundRect(f1,f3,f7,f4,85,85);
   SET_ALPHA(180+add); p.drawRoundRect(f2,f4,dpi.f5,f3,90,90);

   p.setCompositionMode( QPainter::CompositionMode_SourceOver );
   QLinearGradient lg(0,0,f9,0);
   QGradientStops stops;
   stops << QGradientStop( 0, WHITE(20) ) << QGradientStop( 0.5, WHITE(90) ) <<
      QGradientStop( 1, WHITE(20) );
   lg.setStops(stops);
   p.fillRect(f2,f9-f2,f9-f4,f1, lg);
   stops.clear();

   lg = QLinearGradient(0,0,f9,0);
   stops << QGradientStop( 0, WHITE(10) ) << QGradientStop( 0.5, WHITE(55) ) <<
      QGradientStop( 1, WHITE(10) );
   lg.setStops(stops);
   p.fillRect(f3,f9-f1,f3,f1, lg);
   stops.clear();

   p.end();

   set = Tile::Set(QPixmap::fromImage(*tmpImg),f9_2,f9_2,f9-2*f9_2,f9-2*f9_2);
   set.setDefaultShape(Tile::Ring);
   delete tmpImg;
}

static void
renderRelief(Tile::Set &set)
{
   NEW_EMPTY_PIX(f11, f11);
   QPen pen = p.pen(); pen.setWidth(f1); p.setPen(pen);
   p.setBrush(Qt::NoBrush);
   p.setPen(WHITE(95));
   p.drawRoundRect(0,0,f11,dpi.f10,80,80);
   black.setAlpha(70); p.setPen(black);
   p.drawRoundRect(f1,0,f9,f9,80,80);
//    p.setPen(WHITE(60));
//    p.drawRoundRect(f2,f2,f9-f2,f9-f2,60,60);
   p.end();
   set = Tile::Set(*pix,f11_2,f11_2,f11-2*f11_2,f11-2*f11_2);
   set.setDefaultShape(Tile::Ring);
   delete pix;
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

   f1 = dpi.f1; f2 = dpi.f2; f3 = dpi.f3;
   f4 = dpi.f4; f7 = dpi.f7; f9 = dpi.f9;
   f11 = SCALE(11);

   f2_2 = lround(f2/2.0); f9_2 = (f9-1)/2; f11_2 = f11/2;

   // Pushbutton mask
   masks.button = Tile::Set(roundedMask(f9, 70),f9/2,f9/2,1,1, 70);
   masks.button.setClipOffsets(0,0,0,0);
   
   // Pushbutton shadows
   for (int i = 0; i < 2; ++i) // opaque?
      for (int j = 0; j < 2; ++j) {// sunken?
         shadows.button[i][j] = Tile::Set(shadow(f9,i,j,.8), f9/2, f9/2, 1, 1);
         shadows.button[i][j].setDefaultShape(Tile::Ring);
      }
   
   // Pushbutton light
//    renderButtonLight(lights.button);

   renderLineEditShadow(shadows.lineEdit[false], false);
   renderLineEditShadow(shadows.lineEdit[true], true);

   // relief
   renderRelief(shadows.relief);

   // toplight -- UNUSED!
//    renderLightLine(lights.top);

   QPixmap tmp = QPixmap(f9,f9); QPainter p;
   int f49 = SCALE(49);
   int f49_2 = (f49-1)/2;
   // ================================================================
   
   // SLIDER =====================================
   // shadow
   for (int i = 0; i < 2; ++i) { // opaque?
         shadows.sliderRound[i][false] = shadow(dpi.SliderControl, i,false);
         shadows.sliderRound[i][true] = shadow(dpi.SliderControl-f2, i,true);
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
   
   // RECTANGULAR =====================================
   // raised
   // TODO?!
   // sunken
   int f6 = dpi.f6;
   tmp = QPixmap(f9,f9);
   tmp.fill(Qt::transparent);
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
   shadows.sunken = Tile::Set(tmp,f9_2,f9_2,f9-2*f9_2,f9-2*f9_2);
   shadows.sunken.setDefaultShape(Tile::Ring);
   // ================================================================
   
   // TABBAR =====================================
   // mask
   int f13 = SCALE(13), f13_2 = f13/2;
   masks.tab = Tile::Set(roundedMask(f13,99),f13/2,f13/2,1,1, 99);
   masks.tab.setClipOffsets(0,0,0,0);
   
   // light
   lights.tab = Tile::Set(shadow(f11, true, false, 3.0), f11/2,f11/2,1,1);
   lights.tab.setClipOffsets(f3,f3,f3,f3);
   lights.tab.setDefaultShape(Tile::Ring);
   
   // shadow
   int f17 = SCALE(17), f17_2 = (f17-1)/2;
   for (int i = 0; i < 2; ++i) // opaque?
      for (int j = 0; j < 2; ++j) {// sunken?
         shadows.tab[i][j] = Tile::Set(shadow(f17, i, j), f17/2, f17/2, 1, 1);
         shadows.tab[i][j].setDefaultShape(Tile::Ring);
      }

   
   QImage tmpImg = QImage(f17,f17, QImage::Format_ARGB32);

   // sunken
   int f15 = SCALE(15);
   tmpImg.fill(Qt::transparent);
   p.begin(&tmpImg);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,85)); p.drawRoundRect(0,0,f17,f17-f2,80,80);
   p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
   p.setBrush(QColor(0,0,0,120)); p.drawRoundRect(0,f1,f17,f17-f3,75,75);
   p.setBrush(QColor(0,0,0,140)); p.drawRoundRect(0,f2,f17,dpi.f13,80,80);
   p.setBrush(QColor(0,0,0,160)); p.drawRoundRect(f1,f3,f15,dpi.f12,85,85);
   p.setBrush(QColor(0,0,0,180)); p.drawRoundRect(f2,dpi.f4,dpi.f13,f11,90,90);
   p.setCompositionMode( QPainter::CompositionMode_SourceOver );
   QLinearGradient lg(dpi.f4,0,f17-dpi.f4,0);
   QGradientStops stops;
   stops << QGradientStop( 0, QColor(255,255,255, 20) )
      << QGradientStop( 0.5, QColor(255,255,255, 90) )
      << QGradientStop( 1, QColor(255,255,255, 20) );
   lg.setStops(stops);
   p.fillRect(f2,f17-f2,f13,f1, lg);
   stops.clear();
   stops << QGradientStop( 0, QColor(255,255,255, 10) )
      << QGradientStop( 0.5, QColor(255,255,255, 55) )
      << QGradientStop( 1, QColor(255,255,255, 10) );
   lg.setStops(stops);
   p.fillRect(f4,f17-f1,f9,f1, lg);
   stops.clear();
   p.end();
   shadows.tabSunken = Tile::Set(QPixmap::fromImage(tmpImg),f17_2,f17_2,f17-2*f17_2,f17-2*f17_2);
   shadows.tabSunken.setDefaultShape(Tile::Ring);
   // ================================================================
   
   // GROUPBOX =====================================
   // shadow
   tmpImg = QImage(f49,f49, QImage::Format_ARGB32);
   tmpImg.fill(Qt::transparent);
   p.begin(&tmpImg);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,5)); p.drawRoundRect(0,0,f49,2*f49,14,7);
   p.setBrush(QColor(0,0,0,9)); p.drawRoundRect(f1,f1,f49-f2,2*f49,13,7);
   p.setBrush(QColor(0,0,0,11)); p.drawRoundRect(f2,f2,f49-dpi.f4,2*f49,12,6);
   p.setBrush(QColor(0,0,0,13)); p.drawRoundRect(dpi.f3,dpi.f3,f49-dpi.f6,2*f49,48,24);
   p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
   p.setBrush(QColor(0,0,0,0)); p.drawRoundRect(dpi.f4,dpi.f2,f49-dpi.f8,2*f49,11,6);
//    p.setCompositionMode( QPainter::CompositionMode_SourceOver );
//    p.setPen(QColor(255,255,255,200)); p.setBrush(Qt::NoBrush);
//    p.drawRoundRect(dpi.f4,dpi.f2,f49-dpi.f8,2*f49,11,6);
   p.setRenderHint(QPainter::Antialiasing, false);
//    p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
   int f33 = SCALE(33);
   for (int i = 1; i < f33; ++i) {
      p.setPen(QColor(0,0,0,CLAMP(i*lround(255.0/dpi.f32),0,255)));
      p.drawLine(0, f49-i, f49, f49-i);
   }
   p.end();
   int f12 = dpi.f12;
   shadows.group = Tile::Set(QPixmap::fromImage(tmpImg),f12,f12,f49-2*f12,f1);
   shadows.group.setDefaultShape(Tile::Ring);
   // ================================================================
   
   // LINES =============================================
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
         tmp.fill(Qt::transparent);
         p.begin(&tmp);
         stops << QGradientStop( 0, QColor(c1,c1,c1,0) )
            << QGradientStop( 0.5, QColor(c1,c1,c1,71) )
            << QGradientStop( 1, QColor(c1,c1,c1,0) );
         lg.setStops(stops);
         if (i) {
            p.fillRect(0,0,f1,f49,lg);
         }
         else {
            p.fillRect(0,0,f49,f1,lg);
         }
         stops.clear();
         stops << QGradientStop( 0, QColor(c2,c2,c2,0) )
            << QGradientStop( 0.5, QColor(c2,c2,c2,74) )
            << QGradientStop( 1, QColor(c2,c2,c2,0) );
         lg.setStops(stops);
         if (i) {
            p.fillRect(f1,0,f2-f1,f49,lg);
         }
         else {
            p.fillRect(0,f1,f49,f2-f1,lg);
         }
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
