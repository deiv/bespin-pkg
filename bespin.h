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

#ifndef BESPIN_STYLE_H
#define BESPIN_STYLE_H

class QSettings;

#include <QCommonStyle>
#include <QStyleOption>

#include "tileset.h"
#include "styleanimator.h"
#include "gradients.h"
#include "types.h"
#include "config.h"
#include "debug.h"

#ifndef Q_WS_X11
#define QT_NO_XRENDER #
#endif

namespace Bespin {

// class BespinStyle;

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

protected:
   virtual void init(const QSettings *settings = 0L);

   QColor btnBg(const QPalette &pal, bool isEnabled, int hasFocus = false,
                int step = 0, bool fullHover = true, bool reflective = false) const;
   QColor btnFg(const QPalette &pal, bool isEnabled, int hover, int step = 0) const;

   // element painting routines ===============
   void skip(const QStyleOption*, QPainter*, const QWidget*) const {}
   // buttons.cpp
   void drawButtonFrame(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawPushButton(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawPushButtonBevel(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawPushButtonLabel(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawCheckBox(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawRadio(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawCheckBoxItem(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawRadioItem(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawCheckLabel(const QStyleOption*, QPainter*, const QWidget*) const;
   // docks.cpp
   void drawDockTitle(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawDockHandle(const QStyleOption*, QPainter*, const QWidget*) const;
   // frames.cpp
   void drawFocusFrame(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawFrame(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawGroupBox(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
   void drawGroupBoxFrame(const QStyleOption*, QPainter*, const QWidget*) const;
   // input.cpp
   void drawLineEditFrame(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawLineEdit(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawSpinBox(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
   void drawComboBox(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
   void drawComboBoxLabel(const QStyleOption*, QPainter*, const QWidget*) const;
   // menus.cpp
   void drawMenuBarBg(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawMenuBarItem(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawMenuFrame(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawMenuItem(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawMenuScroller(const QStyleOption*, QPainter*, const QWidget*) const;
   // progress.cpp
   void drawProgressBar(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawProgressBarGC(const QStyleOption*, QPainter*, const QWidget*, bool) const;
   inline void drawProgressBarGroove(const QStyleOption * option,
                                     QPainter * painter,
                                     const QWidget * widget) const {
      drawProgressBarGC(option, painter, widget, false);
   }
   inline void drawProgressBarContents(const QStyleOption * option,
                                       QPainter * painter,
                                       const QWidget * widget) const {
      drawProgressBarGC(option, painter, widget, true);
   }

   void drawProgressBarLabel(const QStyleOption*, QPainter*, const QWidget*) const;
   // scrollareas.cpp
   void drawScrollBar(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
   void drawScrollBarButton(const QStyleOption*, QPainter*, const QWidget* , bool) const;
   inline void drawScrollBarAddLine(const QStyleOption * option,
                                    QPainter * painter,
                                    const QWidget * widget) const {
      drawScrollBarButton(option, painter, widget, false);
   }
   
   inline void drawScrollBarSubLine(const QStyleOption * option,
                                    QPainter * painter,
                                    const QWidget * widget) const {
      drawScrollBarButton(option, painter, widget, true);
   }
   void drawScrollBarGroove(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawScrollBarSlider(const QStyleOption*, QPainter*, const QWidget*) const;
   // shapes.cpp
   void drawCheckMark(const QStyleOption*, QPainter*, Check::Type = Check::V) const;
   void drawCheck(const QStyleOption*, QPainter*, const QWidget*, bool) const;
   inline void drawItemCheck(const QStyleOption * option, QPainter * painter,
                             const QWidget * widget) const {
      drawCheck(option, painter, widget, true);
   }
   inline void drawMenuCheck(const QStyleOption * option, QPainter * painter,
                             const QWidget * widget) const {
      drawCheck(option, painter, widget, false);
   }
   void drawExclusiveCheck(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawArrow(Navi::Direction, const QRect&, QPainter*) const;
   void drawSolidArrow(Navi::Direction, const QRect&, QPainter*) const;
#define INDI_ARROW(_D_)\
   inline void drawSolidArrow##_D_(const QStyleOption * option,\
                                         QPainter * painter, const QWidget *) const {\
      const int dx = option->rect.width()/8, dy = option->rect.height()/8;\
      drawSolidArrow(Navi::_D_, option->rect.adjusted(dx,dy,-dx,-dy), painter);\
}
   INDI_ARROW(N) INDI_ARROW(S) INDI_ARROW(E) INDI_ARROW(W)
#undef INDI_ARROW
   // slider.cpp
   void drawSlider(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
   void drawDial(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
   // tabbing.cpp
   void drawTabWidget(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawTabBar(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawTab(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawTabShape(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawTabLabel(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawToolboxTab(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawToolboxTabShape(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawToolboxTabLabel(const QStyleOption*, QPainter*, const QWidget*) const;
   // toolbars.cpp
   void drawToolButton(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
   void drawToolButtonShape(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawToolButtonLabel(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawToolBarHandle(const QStyleOption*, QPainter*, const QWidget*) const;
   // views.cpp
   void drawHeader(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawHeaderSection(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawHeaderLabel(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawBranch(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawTree(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
   void drawRubberBand(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawHeaderArrow(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawItem(const QStyleOption*, QPainter*, const QWidget*) const;
   // window.cpp
   void drawWindowFrame(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawWindowBg(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawToolTip(const QStyleOption*, QPainter*, const QWidget*) const;
   void drawTitleBar(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
   void drawSizeGrip(const QStyleOption*, QPainter*, const QWidget*) const;
   // ==========================================
   void fillWithMask(QPainter *painter,
                     const QPoint &xy,
                     const QBrush &brush,
                     const QPixmap &mask,
                     QPoint offset = QPoint()) const;
//private slots:
//   void fakeMouse();
   
private:
   BespinStyle( const BespinStyle & );
   BespinStyle& operator=( const BespinStyle & );
   QColor mapFadeColor(const QColor &color, int index) const;
   void generatePixmaps();
   void initMetrics();
   void readSettings(const QSettings *settings = 0L);
   void registerRoutines();
   static bool isSpecialFrame(const QWidget *w);
   void erase(const QStyleOption*, QPainter*, const QWidget*) const;
   
private:
   typedef QHash<uint, Tile::Set> TileCache;
   struct {
      Tile::Set rect[2]; // round
      QPixmap radio, radioIndicator, notch, slider;
      QPixmap winClose, winMin, winMax;
#if SHAPE_POPUP
      QRegion corner[4];
#endif
   } masks;
   struct {
      Tile::Set
         fallback, group,
         sunken[2][2], // round/enabled
         raised[2][2][2],  // round/enabled/sunken
         relief[2][2]; // round/enabled
      QPixmap radio[2][2];
      Tile::Line line[2][3];
//       QPixmap slider[4][2][2]; // for triangles, currently not...
      QPixmap slider[2][2];
   } shadows;
   
   struct {
      Tile::Line top;
//       QPixmap slider[4];
      Tile::Set rect[2];
   } lights;
   
   // pixmaps
   QPixmap *_scanlines[2];
   
   //anmiated progressbars
   StyleAnimator *animator;

   // toolbar title functionality ========================
   QPoint cursorPos_;
   bool mouseButtonPressed_;
   bool internalEvent_;
   bool isGTK;
private slots:
   void fixKdePalette();
};

} // namespace Bespin
#endif //BESPIN_STYLE_H
