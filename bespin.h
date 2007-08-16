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

#ifndef OXYGEN_STYLE_H
#define OXYGEN_STYLE_H

class QAbstractButton;
class QHeaderView;
class QMenuBar;
class QPushButton;
class QScrollBar;
class QTabBar;
class QPaintEvent;
class QFrame;
class QSettings;

#include <QCache>
#include <QHash>
#include <QMap>
#include <QCommonStyle>
#include <QBitmap>
#include <QRegion>
#include <QWidget>
#include "tileset.h"
#include "styleanimator.h"
#include "gradients.h"

namespace Bespin {

enum BGMode { Plain = 0, Scanlines, ComplexLights,
      BevelV, BevelH,
      LightV, LightH };

class BespinStyle;

enum Orientation3D {Sunken = 0, Relief, Raised};

typedef struct {
   int f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
   int f12, f13, f16, f32, f18, f20, f80;
   int ScrollBarExtent;
   int ScrollBarSliderMin;
   int SliderThickness;
   int SliderControl;
   int Indicator;
   int ExclusiveIndicator;
} Dpi;


typedef struct Config {
   struct bg {
      BGMode mode;
      int structure;
   } bg;
   
   struct btn {
      int layer, checkType;
      bool cushion, fullHover;
      Gradients::Type gradient;
      QPalette::ColorRole std_role[2], active_role[2];
   } btn;
   
   struct chooser {
      Gradients::Type gradient;
   } chooser;
   
   struct input {
      ushort pwEchoChar;
   } input;
   
   struct menu {
      QPalette::ColorRole std_role[2], active_role[2];
      Gradients::Type itemGradient;
      bool showIcons, shadow;
   } menu;
   
   struct progress {
      Gradients::Type gradient;
      QPalette::ColorRole std_role[2];
   } progress;
   
   double scale;
   
   struct scroll {
      bool showButtons;
   } scroll;
   
   bool strongFocus;
   
   struct tab {
      QPalette::ColorRole std_role[2], active_role[2];
      Gradients::Type gradient;
      int animSteps;
      TabAnimInfo::TabTransition transition;
   } tab;

} Config;

class BespinStyle : public QCommonStyle {
   Q_OBJECT
public:
//    enum WidgetState{Basic = 0, Hovered, Focused, Active};
   enum WidgetState{Bg = 0, Fg = 1};
   
   BespinStyle();
   ~BespinStyle();

   //inheritance from QStyle
   void drawComplexControl ( ComplexControl control,
                             const QStyleOptionComplex * option,
                             QPainter * painter,
                             const QWidget * widget = 0 ) const;
  
   void drawControl ( ControlElement element,
                      const QStyleOption * option,
                      QPainter * painter,
                      const QWidget * widget = 0 ) const;
   
   /**what do they do?
   virtual void drawItemPixmap ( QPainter * painter, const QRect & rect, int alignment, const QPixmap & pixmap ) const;
   virtual void drawItemText ( QPainter * painter, const QRect & rect, int alignment, const QPalette & pal, bool enabled, const QString & text, QPalette::ColorRole textRole = QPalette::NoRole ) const;
   */
   
   void drawPrimitive ( PrimitiveElement elem,
                                const QStyleOption * option,
                                QPainter * painter,
                                const QWidget * widget = 0 ) const;
   
   QPixmap standardPixmap ( StandardPixmap standardPixmap,
                                    const QStyleOption * option = 0,
                                    const QWidget * widget = 0 ) const;
   
//    what do they do? ========================================
//    QPixmap generatedIconPixmap ( QIcon::Mode iconMode,
//                                  const QPixmap & pixmap,
//                                  const QStyleOption * option ) const;
//    SubControl hitTestComplexControl ( ComplexControl control,
//                                       const QStyleOptionComplex * option,
//                                       const QPoint & pos,
//                                       const QWidget * widget = 0 ) const;
//    QRect itemPixmapRect ( const QRect & rect,
//                           int alignment,
//                           const QPixmap & pixmap ) const;
//    QRect itemTextRect ( const QFontMetrics & metrics,
//                         const QRect & rect,
//                         int alignment,
//                         bool enabled,
//                         const QString & text ) const;
//=============================================================
   
   int pixelMetric ( PixelMetric metric,
                             const QStyleOption * option = 0,
                             const QWidget * widget = 0 ) const;
   
   void polish( QWidget *w );
   void polish( QApplication * );
   void polish( QPalette &pal );
   
   QSize sizeFromContents ( ContentsType type,
                            const QStyleOption * option,
                            const QSize & contentsSize,
                            const QWidget * widget = 0 ) const;
      
   int styleHint ( StyleHint hint,
                   const QStyleOption * option = 0,
                   const QWidget * widget = 0,
                   QStyleHintReturn * returnData = 0 ) const;
   
   QRect subControlRect ( ComplexControl control,
                          const QStyleOptionComplex * option,
                          SubControl subControl,
                          const QWidget * widget = 0 ) const;
   
   QRect subElementRect ( SubElement element,
                                  const QStyleOption * option,
                                  const QWidget * widget = 0 ) const;
   
   QPalette standardPalette () const;
   
   void unPolish( QWidget *w );
   void unPolish( QApplication *a );
   
   // from QObject
   bool eventFilter( QObject *object, QEvent *event );
   
signals:
   void MDIPopup(QPoint);

protected:
   virtual void init(const QSettings *settings = 0L);
//private slots:
//   void fakeMouse();
   
private:
   BespinStyle( const BespinStyle & );
   BespinStyle& operator=( const BespinStyle & );
   
   void fillWithMask(QPainter *painter,
                     const QPoint &xy,
                     const QBrush &brush,
                     const QPixmap &mask,
                     QPoint offset = QPoint()) const;
   
   QColor mapFadeColor(const QColor &color, int index) const;
   QPixmap *tint(const QImage &img, const QColor& c) const;
   const Tile::Set &glow(const QColor & c, bool round = false) const;
   void generatePixmaps();
   void initMetrics();
   void makeStructure(int num, const QColor &c);
   bool scrollAreaHovered(const QWidget* slider) const;
   void readSettings(const QSettings *settings = 0L);
   
private:
   typedef QHash<uint, Tile::Set> TileCache;
   struct {
      Tile::Mask button, tab;
      QPixmap radio, radioIndicator, notch, slider[4];
      QPixmap winClose, winMin, winMax;
      QRegion corner[4];
   } masks;
   struct {
      Tile::Set button[2][2],
         tab[2][2], tabSunken,
         group, lineEdit[2],
         sunken, raised, relief;
      QPixmap radio[2][2];
      QPixmap winClose[2], winMin[2], winMax[2];
      Tile::Line line[2][3];
      QPixmap slider[4][2][2];
      QPixmap sliderRound[2][2];
   } shadows;
   struct {
      Tile::Set rect[3], round[3], button[2];
   } frames;
   struct {
      Tile::Line top;
      QPixmap slider[4];
      Tile::Mask button, tab;
   } lights;
   
   // pixmaps
   QPixmap *_scanlines[2];
   
   //anmiated progressbars
   StyleAnimator *animator;
   int complexStep, widgetStep;
   bool scrollAreaHovered_;
   
   // toolbar title functionality ========================
   QPoint cursorPos_;
   bool mouseButtonPressed_;
   bool internalEvent_;
};

} // namespace Bespin
#endif //OXYGEN_STYLE_H
