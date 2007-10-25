#include <QImage>
#include <QVector>
#include <QPainter>
#include <QRect>
#include <cmath>

enum Shape {Close = 0, Min, Max, Restore, Stick, Unstick, Above, Below, Menu, Help, NumButtons};
static const char *name[NumButtons] =
{"close", "min", "max", "restore", "stick", "unstick", "above", "below", "menu", "help"};
static QVector<QRect> shape[NumButtons];
static const float afactor[6] = {.15, .5, 1, .07, .4, 1};
static const QString prefix("buttons.");

int main (int argc, char **argv)
{
   int sz = 14;
   if (argc > 1) sz = QString(argv[1]).toUInt();
   
   QColor c = Qt::black; c.setAlpha(0xaa);
   if (argc > 2) c.setRgba(QString(argv[2]).toUInt(0,16));
   int alpha = c.alpha();
   
   const int s2 = lround(sz/2.0), s3 = lround(sz/3.0),
      s4 = lround(sz/4.0), s6 = lround(sz/6.0);
   shape[Close] << QRect(0,0,sz,s4) << QRect(0,sz-s4,sz,s4) <<
      QRect(0,s4,s4,sz-2*s4) << QRect(sz-s4,s4,s4,sz-2*s4) <<
      QRect(s3, s3, sz-2*s3, sz-2*s3);
   shape[Min] << QRect(0,0,s4,sz) << QRect(s4,sz-s4,sz-s4,s4) << QRect(sz-s4,0,s4,s4);
   shape[Max] << QRect(0,0,sz,s4) << QRect(sz-s4,s4,s4,sz-s4) << QRect(0,sz-s4,s4,s4);
   shape[Restore] << QRect(sz-s4,0,s4,sz) << QRect(0,sz-s4,sz-s4,s4) << QRect(0,0,s4,s4);
   shape[Stick] << QRect(s6,s6,sz-2*s6,sz-2*s6);
   shape[Unstick] << QRect(s3,s3,sz-2*s3,sz-2*s3);
   shape[Above] << QRect(0,0,sz,s3) << QRect(s4,sz-s3,sz-2*s4,s3);
   shape[Below] << QRect(0,sz-s3,sz,s3) << QRect(s4,0,sz-2*s4,s3);
   shape[Menu] << QRect(0,0,sz,s4) << QRect(sz-s3,s4,s3,sz-s4);
   shape[Help] << QRect(0,0,s2,s4) << QRect(sz-s2-s4,s4,s4,sz-2*s4-s6) <<
      QRect(sz-s2-s4,sz-s4,s4,s4);
   
   QImage img(6*sz, sz, QImage::Format_ARGB32);
   QPainter p;
   for (int b = 0; b < NumButtons; ++b) {
      img.fill(Qt::transparent);
      p.begin(&img); p.setPen(Qt::NoPen);
      for (int d = 0; d < 6; ++d) {
         c.setAlpha(alpha*afactor[d]); p.setBrush(c);
         for (int r = 0; r < shape[b].size(); ++r)
            p.drawRect(shape[b].at(r).translated(d*sz,0));
      }
      p.end();
      img.save(prefix + name[b] + ".png", "png");
   }
}
