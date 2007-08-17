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

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QHeaderView>
#include <QMenuBar>
#include <QStyleOptionTab>
#include <QStyleOptionHeader>
#include <QStyleOptionSlider>
#include <QStyleOptionProgressBarV2>
#include <QStyleOptionToolBox>
#include <QPainter>

#include "bespin.h"
#include "colors.h"
#include "makros.h"

using namespace Bespin;

extern Config config;
extern Dpi dpi;
// extern HoverFades hoverWidgets;q
extern bool animationUpdate;

static const int windowsItemFrame	= 1; // menu item frame width
static const int windowsItemHMargin	= 3; // menu item hor text margin
static const int windowsItemVMargin	= 1; // menu item ver text margin
static const int windowsRightBorder	= 12; // right border on windows

static void drawArrow(const QStyle *style, const QStyleOptionToolButton *toolbutton,
                      const QRect &rect, QPainter *painter, const QWidget *widget = 0)
{
   QStyle::PrimitiveElement pe;
   switch (toolbutton->arrowType) {
   case Qt::LeftArrow:
      pe = QStyle::PE_IndicatorArrowLeft;
      break;
   case Qt::RightArrow:
      pe = QStyle::PE_IndicatorArrowRight;
      break;
   case Qt::UpArrow:
      pe = QStyle::PE_IndicatorArrowUp;
      break;
   case Qt::DownArrow:
      pe = QStyle::PE_IndicatorArrowDown;
      break;
   default:
      return;
   }
   QStyleOption arrowOpt;
   arrowOpt.rect = rect;
   arrowOpt.palette = toolbutton->palette;
   arrowOpt.state = toolbutton->state;
   style->drawPrimitive(pe, &arrowOpt, painter, widget);
}

inline static bool verticalTabs(QTabBar::Shape shape) {
   return shape == QTabBar::RoundedEast ||
      shape == QTabBar::TriangularEast ||
      shape == QTabBar::RoundedWest ||
      shape == QTabBar::TriangularWest;
}

void BespinStyle::drawControl ( ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget) const
{
   Q_ASSERT(option);
   Q_ASSERT(painter);
   
   bool sunken = option->state & State_Sunken;
   bool isEnabled = option->state & State_Enabled;
   bool hover = isEnabled && (option->state & State_MouseOver);
   bool hasFocus = option->state & State_HasFocus;
   
   switch ( element ) {
   case CE_PushButton:
      if (const QStyleOptionButton *btn =
          qstyleoption_cast<const QStyleOptionButton *>(option)) {
         QStyleOptionButton tmpBtn = *btn;
         if (btn->features & QStyleOptionButton::Flat) { // more like a toolbtn
            //TODO: handle focus indication here (or in the primitive...)!
            drawPrimitive(PE_PanelButtonTool, option, painter, widget);
         }
         else {
            if (sunken && config.btn.layer == 1 && !config.btn.cushion)
               tmpBtn.rect.adjust(dpi.f1,dpi.f1,-dpi.f1,0);
            drawControl(CE_PushButtonBevel, &tmpBtn, painter, widget);
         }
//          tmpBtn.rect = subElementRect(SE_PushButtonContents, btn, widget);
         if (config.btn.layer == 2)
            tmpBtn.rect.adjust(dpi.f4,dpi.f3,-dpi.f4,-dpi.f2);
         else
            tmpBtn.rect.adjust(dpi.f4,dpi.f4,-dpi.f4,-dpi.f4);
         drawControl(CE_PushButtonLabel, &tmpBtn, painter, widget);
         
         // toggle indicator
         if (!widget) break;
         if (const QAbstractButton *b =
             qobject_cast<const QAbstractButton*>(widget))
         if (b->isCheckable()) {
            QRect r = RECT;
            const int h = r.height()/3;
            r.setTop(r.top() + h);
            r.setLeft(r.right()- h -dpi.f6);
            r.setWidth(h); r.setHeight(h);
            
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            const QPixmap &fill =
                  Gradients::pix((option->state & State_On) ?
                  Colors::mid(FCOLOR(Highlight), CCOLOR(btn.std, 0)) :
                  CCOLOR(btn.std, 0), r.height(), Qt::Vertical, GRAD(btn));
            painter->setBrush(fill);
            painter->setPen(CCOLOR(btn.std, 0).dark(124));
            painter->setBrushOrigin(r.topLeft());
            painter->drawEllipse(r);
            painter->restore();
         }
      }
      break;
   case CE_PushButtonBevel:
      if (const QStyleOptionButton *btn =
          qstyleoption_cast<const QStyleOptionButton *>(option)) {
         drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
         if (btn->features & QStyleOptionButton::HasMenu) {
//             int mbi = pixelMetric(PM_MenuButtonIndicator, btn, widget);
            QStyleOptionButton newBtn = *btn;
            int sz = (RECT.height()-dpi.f6)/2;
            newBtn.rect = RECT;
            newBtn.rect.setLeft(RECT.right() - (dpi.f10+sz));
            shadows.line[1][Sunken].render(newBtn.rect, painter);
            newBtn.rect.setLeft(newBtn.rect.left() + dpi.f4);
            newBtn.rect.setTop((RECT.height()-sz)/2 + dpi.f2);
            newBtn.rect.setHeight(sz); newBtn.rect.setWidth(sz);
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(Colors::mid(CCOLOR(btn.std, 0), CCOLOR(btn.active, 1)));
            drawPrimitive(PE_IndicatorArrowDown, &newBtn, painter, widget);
            painter->restore();
         }
      }
      break;
   case CE_PushButtonLabel:
      if (const QStyleOptionButton *btn =
          qstyleoption_cast<const QStyleOptionButton *>(option)) {
         QRect ir = btn->rect;
         uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
         if (!styleHint(SH_UnderlineShortcut, btn, widget))
            tf |= Qt::TextHideMnemonic;
         
         if (!btn->icon.isNull()) {
            QIcon::Mode mode = isEnabled ? QIcon::Normal
               : QIcon::Disabled;
            if (mode == QIcon::Normal && hasFocus)
               mode = QIcon::Active;
            QIcon::State state = QIcon::Off;
            if (btn->state & State_On)
               state = QIcon::On;
            QPixmap pixmap = btn->icon.pixmap(btn->iconSize, mode, state);
            int pixw = pixmap.width();
            int pixh = pixmap.height();
            
            //Center the icon if there is no text
            QPoint point;
            if (btn->text.isEmpty())
               point = QPoint(ir.x() + ir.width() / 2 - pixw / 2, ir.y() + ir.height() / 2 - pixh / 2);
            else
               point = QPoint(ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2);
            
            if (btn->direction == Qt::RightToLeft)
               point.rx() += pixw;
            
            painter->drawPixmap(visualPos(btn->direction, btn->rect, point), pixmap);
            
            if (btn->direction == Qt::RightToLeft)
               ir.translate(-4, 0);
            else
               ir.translate(pixw + 4, 0);
            ir.setWidth(ir.width() - (pixw + 4));
                // left-align text if there is
            if (!btn->text.isEmpty())
               tf |= Qt::AlignLeft;
         }
         else
            tf |= Qt::AlignHCenter;
             
         if (btn->features & QStyleOptionButton::HasMenu) {
            ir.setRight(ir.right() - ir.height()/2 - dpi.f10);
         }
         else if (widget)
         if (const QAbstractButton* btn =
             qobject_cast<const QAbstractButton*>(widget))
         if (btn->isCheckable())
            ir.setRight(ir.right() - ir.height()/2 - dpi.f10);
         
         if (btn->features & QStyleOptionButton::Flat) {
            drawItemText(painter, ir, tf, PAL, isEnabled, btn->text,
                         QPalette::WindowText);
            break;
         }
         painter->save();
         QColor fg = Colors::btnFg(PAL, isEnabled, hover);
         if (btn->features & QStyleOptionButton::DefaultButton) {
            painter->setPen(Colors::mid(hover ? CCOLOR(btn.active, 0) :
                  CCOLOR(btn.std, 0), fg, 3,1));
            ir.translate(0,1);
            drawItemText(painter, ir, tf, PAL, isEnabled, btn->text);
//             ir.translate(2,2);
//             drawItemText(painter, ir, tf, PAL, isEnabled, btn->text);
            ir.translate(0,-1);
         }
         painter->setPen(fg);
         drawItemText(painter, ir, tf, PAL, isEnabled, btn->text);
         painter->restore();
      }
      break;
   case CE_DockWidgetTitle: // Dock window title.
      if (const QStyleOptionDockWidget *dwOpt =
          qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {
         QRect textRect;
         int x3 = RECT.right()-7;
         if (dwOpt->floatable)
            x3 -= 18;
         if (dwOpt->closable)
            x3 -= 18;
         int x2 = x3;
         if (!dwOpt->title.isEmpty()) {
            int itemtextopts = Qt::AlignCenter | Qt::TextShowMnemonic;
            drawItemText(painter, RECT, itemtextopts, PAL, isEnabled, dwOpt->title, QPalette::WindowText);
            textRect = painter->boundingRect ( RECT, itemtextopts, dwOpt->title );
            x2 = textRect.x()-8;
         }

         const Tile::Line &line = shadows.line[0][Sunken];
         textRect.setTop(textRect.top()+(textRect.height()-line.thickness())/2);
         int x = textRect.right()+dpi.f4;
         textRect.setRight(textRect.left()-dpi.f4);
         textRect.setLeft(qMin(RECT.x()+RECT.width()/4,textRect.x()-(textRect.x()-RECT.x())/2));
         line.render(textRect, painter, Tile::Left|Tile::Center);
         textRect.setLeft(x);
         textRect.setRight(qMax(RECT.right()-RECT.width()/4,x+(RECT.right()-x)/2));
         line.render(textRect, painter, Tile::Right|Tile::Center);
         //TODO: hover?
      }
      break;
   case CE_Splitter: // Splitter handle; see also QSplitter.
      drawPrimitive(PE_IndicatorDockWidgetResizeHandle,option,painter,widget);
      break;
   case CE_RadioButton: // A QRadioButton, draws a PE_ExclusiveRadioButton, a case CE_RadioButtonLabel
   case CE_CheckBox: // A QCheckBox, draws a PE_IndicatorCheckBox, a case CE_CheckBoxLabel
      if (const QStyleOptionButton *btn =
          qstyleoption_cast<const QStyleOptionButton *>(option)) {
         QStyleOptionButton subopt = *btn;
         if (element == CE_RadioButton) {
            subopt.rect = subElementRect(SE_RadioButtonIndicator, btn, widget);
            drawPrimitive(PE_IndicatorRadioButton, &subopt, painter, widget);
            subopt.rect = subElementRect(SE_RadioButtonContents, btn, widget);
            drawControl(CE_RadioButtonLabel, &subopt, painter, widget);
         }
         else {
            subopt.rect = subElementRect(SE_CheckBoxIndicator, btn, widget);
            drawPrimitive(PE_IndicatorCheckBox, &subopt, painter, widget);
            subopt.rect = subElementRect(SE_CheckBoxContents, btn, widget);
            drawControl(CE_CheckBoxLabel, &subopt, painter, widget);
         }
      }
      break;
//    case CE_CheckBoxLabel: // The label (text or pixmap) of a QCheckBox
//    case CE_RadioButtonLabel: // The label (text or pixmap) of a QRadioButton
   case CE_TabBarTab:
      if (const QStyleOptionTab *tab =
          qstyleoption_cast<const QStyleOptionTab *>(option)) {
         // do we have to exclude the scrollers?
         bool needRestore = false;
         if (widget && (RECT.right() > widget->width())) {
            painter->save();
            needRestore = true;
            QRect r = RECT;
            r.setRight(widget->width() -
                  2*pixelMetric(PM_TabBarScrollButtonWidth,option,widget));
            painter->setClipRect(r);
         }
         // paint shape and label
         QStyleOptionTab copy = *tab;
         copy.rect.setBottom(copy.rect.bottom()-dpi.f2);
         drawControl(CE_TabBarTabShape, &copy, painter, widget);
         drawControl(CE_TabBarTabLabel, &copy, painter, widget);
         if (needRestore)
            painter->restore();
      }
      break;
   case CE_TabBarTabShape: // The tab shape within a tab bar
      if (const QStyleOptionTab *tab =
          qstyleoption_cast<const QStyleOptionTab *>(option)) {
         
         sunken = sunken || (option->state & State_Selected);

         int step = 0;
         // animation stuff
         if (isEnabled && !sunken) {
            IndexedFadeInfo *info = 0;
            int index = -1, hoveredIndex = -1;
            if (widget)
            if (const QTabBar* tbar =
                   qobject_cast<const QTabBar*>(widget)) {
               // NOTICE: the index increment is IMPORTANT to make sure it's no "0"
               index = tbar->tabAt(RECT.topLeft()) + 1; // is the action for this item!
               hoveredIndex = hover ? index :
                     tbar->tabAt(tbar->mapFromGlobal(QCursor::pos())) + 1;
               info = const_cast<IndexedFadeInfo *>
                  (animator->fadeInfo(widget, hoveredIndex));
            }
            if (info)
               step = info->step(index);
            if (hover && !step) step = 6;
         }
         
         // maybe we're done here?!
         if (!(step || sunken))
            break;

         const int f2 = dpi.f2;
         QRect rect = RECT.adjusted(dpi.f3, dpi.f5, -dpi.f4, -dpi.f5);
         int size = RECT.height()-f2;
         Qt::Orientation o = Qt::Vertical;

         switch (tab->shape) {
         case QTabBar::RoundedNorth: case QTabBar::TriangularNorth:
         case QTabBar::RoundedSouth: case QTabBar::TriangularSouth:
            break;
         case QTabBar::RoundedEast: case QTabBar::TriangularEast:
         case QTabBar::RoundedWest: case QTabBar::TriangularWest:
            o = Qt::Horizontal;
            size = RECT.width()-f2;
            break;
         }
         
         QColor c;
         int d = 0;
         if (sunken) {
            rect.adjust(f2, -dpi.f1, -f2, dpi.f1);
            c = CCOLOR(tab.active, 0);
            d = (o == Qt::Vertical) ? -dpi.f1 : f2;
         }
         else {
            c = Colors::mid(CCOLOR(tab.std, 0), FCOLOR(Window), 2, 1);
            int quota = 6 + (int) (.16 * Colors::contrast(c, CCOLOR(tab.active, 0)));
            c = Colors::mid(c, CCOLOR(tab.active, 0), quota, step);
         }
         const QPoint off(d, d+dpi.f4);
         masks.tab.render(rect, painter, GRAD(tab), o, c, size, off);

      }
      break;
   case CE_TabBarTabLabel: // The label within a tab
      if (const QStyleOptionTab *tab =
          qstyleoption_cast<const QStyleOptionTab *>(option)) {
         painter->save();
         QStyleOptionTabV2 tabV2(*tab);
         QRect tr = tabV2.rect;
         bool verticalTabs = false;
         bool east = false;
         bool selected = tabV2.state & State_Selected;
         if (selected) hover = false;
         int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
         
         switch(tab->shape) {
         case QTabBar::RoundedNorth: case QTabBar::TriangularNorth:
         case QTabBar::RoundedSouth: case QTabBar::TriangularSouth:
            break;
         case QTabBar::RoundedEast: case QTabBar::TriangularEast:
            east = true;
         case QTabBar::RoundedWest: case QTabBar::TriangularWest:
            verticalTabs = true;
            break;
         }
         
         if (verticalTabs) {
            int newX, newY, newRot;
            if (east) {
               newX = tr.width(); newY = tr.y(); newRot = 90;
            }
            else {
               newX = 0; newY = tr.y() + tr.height(); newRot = -90;
            }
            tr.setRect(0, 0, tr.height(), tr.width());
            QMatrix m;
            m.translate(newX, newY); m.rotate(newRot);
            painter->setMatrix(m, true);
         }
         
         if (!tabV2.icon.isNull()) {
            QSize iconSize = tabV2.iconSize;
            if (!iconSize.isValid()) {
               int iconExtent = pixelMetric(PM_SmallIconSize);
               iconSize = QSize(iconExtent, iconExtent);
            }
            QPixmap tabIcon = tabV2.icon.pixmap(iconSize, (isEnabled) ?
                  QIcon::Normal : QIcon::Disabled);
            painter->drawPixmap(tr.left() + dpi.f6,
                                tr.center().y() - tabIcon.height() / 2, tabIcon);
            tr.setLeft(tr.left() + iconSize.width() + dpi.f4);
         }
         
         // color adjustment
         QColor cF, cB;
         if (selected || sunken) {
            cF = CCOLOR(tab.active, 1);
            cB = CCOLOR(tab.active, 0);
         }
         else if (hover) {
            cF = CCOLOR(tab.std, 1);
            cB = Colors::mid(CCOLOR(tab.std ,0 ), FCOLOR(Window), 2, 1);
            cB = Colors::mid(cB, CCOLOR(tab.active, 0));
            if (Colors::contrast(CCOLOR(tab.active, 1), cB) > Colors::contrast(cF, cB))
               cF = CCOLOR(tab.active, 1);
         }
         else {
            cB = Colors::mid(CCOLOR(tab.std, 0), FCOLOR(Window), 2, 1);
            cF = Colors::mid(cB, CCOLOR(tab.std, 1), 1,4);
         }

         // dark background, let's paint an emboss
         if (qGray(cB.rgb()) < 148) {
            painter->setPen(cB.dark(120));
            tr.moveTop(tr.top()-1);
            drawItemText(painter, tr, alignment, PAL, isEnabled, tab->text);
            tr.moveTop(tr.top()+1);
         }
         painter->setPen(cF);
         drawItemText(painter, tr, alignment, PAL, isEnabled, tab->text);
         
         painter->restore();
      }
      break;
      case CE_ProgressBar: // CE_ProgressBarGroove, CE_ProgressBarContents, CE_ProgressBarLabel
      if (const QStyleOptionProgressBar *pb
          = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
         
         QStyleOptionProgressBarV2 subopt = *pb;
         // groove + contents
         subopt.rect = subElementRect(SE_ProgressBarGroove, pb, widget);
         drawControl(CE_ProgressBarGroove, pb, painter, widget);
         // subopt.rect = subElementRect(SE_ProgressBarContents, pb, widget);
         drawControl(CE_ProgressBarContents, &subopt, painter, widget);
         // label?
         if (hover && pb->textVisible) {
            subopt.rect = subElementRect(SE_ProgressBarLabel, pb, widget);
            drawControl(CE_ProgressBarLabel, &subopt, painter, widget);
         }
         
      }
      break;
   case CE_ProgressBarGroove:
   case CE_ProgressBarContents:
      if (const QStyleOptionProgressBarV2 *pb =
            qstyleoption_cast<const QStyleOptionProgressBarV2*>(option)) {
         
         bool reverse = option->direction == Qt::RightToLeft;
         if (pb->invertedAppearance) reverse = !reverse;
         const bool vertical = pb->orientation == Qt::Vertical;
         const bool busy = pb->maximum == 0 && pb->minimum == 0;
         
         int x,y,l,t;
         RECT.getRect(&x,&y,&l,&t);
         if (vertical) {
            int h = x; x = y; y = h;
            l = RECT.height(); t = RECT.width();
         }
         
         double val = 0.0;
         if (busy)
            val = -3.0*animator->progressStep(widget)/l;
         else
            val = pb->progress / double(pb->maximum - pb->minimum);
         if (element == CE_ProgressBarContents) {
            if (val == 0.0)
               break;
         }
         else if (val == 1.0)
            break;
            
         
         int s = qMin(qMax(l / 10, dpi.f16), t /*16*t/10*/);
         int ss = (10*s)/16;
         
         int n = l/s;
         if (vertical || reverse) {
            x = vertical ? RECT.bottom() : RECT.right();
            x -= ((l - n*s) + (s - ss))/2 + ss;
            s = -s;
         }
         else
            x += (l - n*s + s - ss)/2;
         y += (t-ss)/2;
         
         --x; --y;
         
         QPixmap renderPix(ss+2,ss+2);
         renderPix.fill(Qt::transparent);
         QPainter p(&renderPix);
         p.setRenderHint(QPainter::Antialiasing);
         
         int nn = (val < 0) ? 0 : int(n*val);
         if (element == CE_ProgressBarContents) {
            p.setBrush(Gradients::pix(CCOLOR(progress.std, 0), ss,
                              Qt::Vertical, GRAD(progress) ));
         }
         else {
            if (busy)
               nn = n;
            else {
               x += nn*s; nn = n - nn;
            }
            const QColor c = FCOLOR(Window).dark(110);
            p.setBrush(Gradients::pix(c, ss, Qt::Vertical,
                              GRAD(progress) ));
         }
         p.setPen(Qt::NoPen);
         p.setBrushOrigin(-1,-1);
         p.drawEllipse(1,1,ss,ss);
         p.setBrush(Qt::NoBrush);
         p.setPen(QColor(0,0,0,70));
         p.drawEllipse(1,1,ss,ss);
         p.setPen(QColor(255,255,255,70));
         p.drawEllipse(1,2,ss,ss);
         p.end();

         if (vertical) {
            for (int i = 0; i < nn; ++i) { // x is in fact y!
               painter->drawPixmap(y,x, renderPix);
               x+=s;
            }
         }
         else {
            for (int i = 0; i < nn; ++i) {
               painter->drawPixmap(x,y, renderPix);
               x+=s;
            }
         }
         if (element == CE_ProgressBarContents) {
            bool b = (nn < n);
            x+=2; y+=2; ss-=2;
            if (busy) { // busy
               b = true;
               val = -val; nn = int(n*val); x += nn*s;
               double o = n*val - nn;
               if (o < .5)
                  val += o/n;
               else
                  val += (1.0-2*o)/n;
            }
            if (b) {
               int q = int((10*n)*val) - 10*nn;
               if (q) {
                  painter->save();
                  painter->setRenderHint(QPainter::Antialiasing);
                  
                  const QColor c = Colors::mid(FCOLOR(Window).dark(110),
                                          CCOLOR(progress.std, 0), 10-q, q);
                  painter->setBrush(Gradients::pix(c, ss, Qt::Vertical,
                                    GRAD(progress) ));
                  painter->setPen(Qt::NoPen);
                  
                  if (vertical) {
                     painter->setBrushOrigin(0, x);
                     painter->drawEllipse(y,x,ss,ss);
                  }
                  else {
                     painter->setBrushOrigin(0, y);
                     painter->drawEllipse(x,y,ss,ss);
                  }
               painter->restore();
               }
            }
         }
      }
      break;
   case CE_ProgressBarLabel:
      if (const QStyleOptionProgressBarV2 *progress =
          qstyleoption_cast<const QStyleOptionProgressBarV2*>(option)) {
         painter->save();

         QRect rect = RECT;

         if (progress->orientation == Qt::Vertical) {
            QMatrix m;
            rect.setRect(RECT.x(), RECT.y(), RECT.height(), RECT.width());
            if (progress->bottomToTop) {
               m.translate(0.0, RECT.height()); m.rotate(-90);
            }
            else {
               m.translate(RECT.width(), 0.0); m.rotate(90);
            }
            painter->setMatrix(m);
         }
         painter->setPen(FCOLOR(Window));
         int flags = Qt::AlignCenter | Qt::TextSingleLine;
         rect.translate(-1,-1);
         painter->drawText(rect, flags, progress->text);
         rect.translate(0,2);
         painter->drawText(rect, flags, progress->text);
         rect.translate(2,0);
         painter->drawText(rect, flags, progress->text);
         rect.translate(0,-2);
         painter->drawText(rect, flags, progress->text);
         rect.translate(-1,1);
         painter->setPen(FCOLOR(WindowText));
         painter->drawText(rect, flags, progress->text);
         painter->restore();
      }
      break;
   case CE_ToolButtonLabel: // A tool button's label
      if (const QStyleOptionToolButton *toolbutton
          = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
         // Arrow type always overrules and is always shown
         bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
         if ((!hasArrow && toolbutton->icon.isNull()) && !toolbutton->text.isEmpty() ||
             toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {
            drawItemText(painter, RECT,
                         Qt::AlignCenter | Qt::TextShowMnemonic, PAL,
                         isEnabled, toolbutton->text, QPalette::WindowText);
         }
         else {
            QPixmap pm;
            QSize pmSize = toolbutton->iconSize;
            if (!toolbutton->icon.isNull()) {
               QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
               QIcon::Mode mode;
               if (!isEnabled)
                  mode = QIcon::Disabled;
               else if (hover && (option->state & State_AutoRaise))
                  mode = QIcon::Active;
               else
                  mode = QIcon::Normal;
               pm = toolbutton->icon.pixmap(RECT.size().boundedTo(toolbutton->iconSize), mode, state);
               pmSize = pm.size();
            }
                    
            if (toolbutton->toolButtonStyle != Qt::ToolButtonIconOnly) {
               painter->setFont(toolbutton->font);
               QRect pr = RECT, tr = RECT;
               int alignment = Qt::TextShowMnemonic;
               
               if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                  int fh = painter->fontMetrics().height();
                  pr.adjust(0, dpi.f3, 0, -fh - dpi.f5);
                  tr.adjust(0, pr.bottom(), 0, -dpi.f3);
                  if (!hasArrow)
                     drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
                  else
                     drawArrow(this, toolbutton, pr, painter, widget);
                  alignment |= Qt::AlignCenter;
               }
               else {
                  pr.setWidth(pmSize.width() + dpi.f8);
                  tr.adjust(pr.right(), 0, 0, 0);
                  if (!hasArrow)
                     drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
                  else
                     drawArrow(this, toolbutton, pr, painter, widget);
                  alignment |= Qt::AlignLeft | Qt::AlignVCenter;
               }
               drawItemText(painter, tr, alignment, PAL, isEnabled, toolbutton->text, QPalette::WindowText);
            }
            else {
               if (hasArrow)
                  drawArrow(this, toolbutton, RECT.adjusted(dpi.f5,dpi.f5,-dpi.f5,-dpi.f5), painter, widget);
               else
                  drawItemPixmap(painter, RECT, Qt::AlignCenter, pm);
            }
         }
      }
      break;
   case CE_MenuBarItem: // A menu item in a QMenuBar
      if (const QStyleOptionMenuItem *mbi =
          qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
         ROLES(menu.active);
         hover = option->state & State_Selected;
         IndexedFadeInfo *info = 0;
         QAction *action = 0, *activeAction = 0;
         int step = 0;
         if (sunken)
            step = 6;
         else {
            if (widget)
            if (const QMenuBar* mbar = qobject_cast<const QMenuBar*>(widget)) {
               action = mbar->actionAt(RECT.topLeft()); // is the action for this item!
               activeAction = mbar->activeAction();
               info = const_cast<IndexedFadeInfo *>
                  (animator->fadeInfo(widget, (long int)activeAction));
            }
            if (info && (!activeAction || !activeAction->menu() ||
               activeAction->menu()->isHidden()))
               step = info->step((long int)action);
         }
         if (step || hover) {
            if (!step) step = 6;
            QRect r = RECT.adjusted(0, dpi.f2, 0, -dpi.f2);
            const QColor c =
                  Colors::mid(FCOLOR(Window), COLOR(ROLE[Bg]), 9-step,step);

            int dy = 0;
            if (!sunken) {
               step = 6-step;
               int dx = step*r.width()/18;
               dy = step*r.height()/18;
               r.adjust(dx, dy, -dx, -dy);
            }
            const Gradients::Type gt =
                  sunken ? Gradients::Sunken : config.menu.itemGradient;
            masks.tab.render(r, painter, gt, Qt::Vertical, c, r.height(), QPoint(0,dy));
         }
         QPixmap pix =
                mbi->icon.pixmap(pixelMetric(PM_SmallIconSize), isEnabled ?
                                 QIcon::Normal : QIcon::Disabled);
         const uint alignment =
                Qt::AlignCenter | Qt::TextShowMnemonic |
                Qt::TextDontClip | Qt::TextSingleLine;
         if (!pix.isNull())
            drawItemPixmap(painter,mbi->rect, alignment, pix);
         else
            drawItemText(painter, mbi->rect, alignment, mbi->palette, isEnabled,
                         mbi->text, hover ? ROLE[Fg] : QPalette::WindowText);
      }
      break;
   case CE_MenuBarEmptyArea: // The empty area of a QMenuBar
      break;
   case CE_MenuItem: // A menu item in a QMenu
        // Draws one item in a popup menu.
      if (const QStyleOptionMenuItem *menuItem =
          qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
         ROLES(menu.std);
         
         // separator
         if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
            int dx = RECT.width()/10,
            dy = (RECT.height()-shadows.line[0][Sunken].thickness())/2;
            painter->save();
            const QRegion rgn =
                  QRegion(RECT).subtract( painter->
                  boundingRect( RECT, Qt::AlignCenter, menuItem->text).
                  adjusted(-dpi.f4,0,dpi.f4,0));
            painter->setClipRegion(rgn, Qt::IntersectClip);
            shadows.line[0][Sunken].render(RECT.adjusted(dx,dy,-dx,-dy), painter);
            painter->restore();
            if (!menuItem->text.isEmpty()) {
               painter->setFont(menuItem->font);
               drawItemText(painter, RECT, Qt::AlignCenter, PAL, isEnabled,
                            menuItem->text, ROLE[Fg]);
            }
            break;
         }

         QRect r = RECT.adjusted(0,0,-1,-1);
         bool selected = menuItem->state & State_Selected;
         
         QColor bg = COLOR(ROLE[Bg]);
         QColor fg = isEnabled ? COLOR(ROLE[Fg]) :
               Colors::mid(COLOR(ROLE[Bg]), COLOR(ROLE[Fg]), 2,1);

         painter->save();
         bool checkable =
               (menuItem->checkType != QStyleOptionMenuItem::NotCheckable);
         bool checked = checkable && menuItem->checked;
         
         if (selected && isEnabled) {
            if (ROLE[Bg] == QPalette::Window) {
               bg = Colors::mid(COLOR(ROLE[Bg]), CCOLOR(menu.active, Bg), 1, 2);
               fg = CCOLOR(menu.active, Fg);
            }
            else {
               bg = Colors::mid(COLOR(ROLE[Bg]), COLOR(ROLE[Fg]), 1, 2);
               fg = COLOR(ROLE[Bg]);
            }
#if 0
            painter->save();
            painter->setBrush(Gradients::brush(bg, r.height(), Qt::Vertical,
                              sunken ? Gradients::Sunken : config.menu.itemGradient));
            painter->setBrushOrigin(r.topLeft());
            painter->setPen(CCOLOR(menu.active, Bg));
            painter->drawRect(r);
            painter->restore();
#else
            masks.tab.render(r, painter, sunken ?
                  Gradients::Sunken : config.menu.itemGradient, Qt::Vertical, bg);
//             masks.tab.outline(r, painter, QColor(0,0,0,45), true);
#endif
         }

         // Text and icon, ripped from windows style
         const QStyleOptionMenuItem *menuitem = menuItem;
         int iconCol = config.menu.showIcons*menuitem->maxIconWidth;
         
         if (config.menu.showIcons && !menuItem->icon.isNull()) {
            QRect vCheckRect = visualRect(option->direction, r,
                                       QRect(r.x(), r.y(), iconCol, r.height()));
            QIcon::Mode mode = isEnabled ? (selected ? QIcon::Active :
                  QIcon::Normal) : QIcon::Disabled;
            QPixmap pixmap = menuItem->icon.pixmap(pixelMetric(PM_SmallIconSize),
                  mode, checked ? QIcon::On : QIcon::Off);
            
            QRect pmr(QPoint(0, 0), pixmap.size());
            pmr.moveCenter(vCheckRect.center());
            
            painter->drawPixmap(pmr.topLeft(), pixmap);
         }
             
         painter->setPen ( fg );
         painter->setBrush ( Qt::NoBrush );
         
         int x, y, w, h;
         r.getRect(&x, &y, &w, &h);
         int tab = menuitem->tabWidth;
         int cDim = 2*(r.height() - dpi.f4)/3;
         int xm = windowsItemFrame + iconCol + windowsItemHMargin;
         int xpos = r.x() + xm;
         QRect textRect(xpos, y + windowsItemVMargin,
                        w - xm - menuItem->menuHasCheckableItems*(cDim+dpi.f7) -
                              windowsRightBorder - tab + 1,
                        h - 2 * windowsItemVMargin);
         QRect vTextRect = visualRect(option->direction, r, textRect);
         QString s = menuitem->text;
         if (!s.isEmpty()) {
            // draw text
            int t = s.indexOf('\t');
            const int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic |
                  Qt::TextDontClip | Qt::TextSingleLine;
            if (t >= 0) {
               QRect vShortcutRect = visualRect(option->direction, r,
                     QRect(textRect.topRight(),
                           QPoint(textRect.right()+tab, textRect.bottom())));
               painter->drawText(vShortcutRect, text_flags | Qt::AlignLeft, s.mid(t + 1));
               s = s.left(t);
            }
            if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem) {
               QFont font = menuitem->font;
               font.setBold(true);
               painter->setFont(font);
            }
            painter->drawText(vTextRect, text_flags | Qt::AlignLeft, s.left(t));
         }
         // Arrow
         if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {
            // draw sub menu arrow
            PrimitiveElement arrow = (option->direction == Qt::RightToLeft) ?
                  PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
            
            int dim = r.height()/3;
            xpos = r.x() + r.width() - dpi.f7 - dim;
            
            QStyleOptionMenuItem tmpOpt = *menuItem;
            tmpOpt.rect = visualRect(option->direction, r,
                                     QRect(xpos, r.y() +
                                           (r.height() - dim)/2, dim, dim));
            painter->setPen(Colors::mid(bg, fg, 1, 3));
            drawPrimitive(arrow, &tmpOpt, painter, widget);
         }
         else if (checkable) { // Checkmark
            xpos = r.right() - dpi.f7 - cDim;
            if (menuItem->checkType & QStyleOptionMenuItem::Exclusive) {
               QRect checkRect(xpos, r.y() + (r.height() - cDim)/2, cDim, cDim);
               checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);
               // Radio button
               painter->setRenderHint ( QPainter::Antialiasing );
               painter->drawEllipse ( checkRect );
               if (checked || sunken) {
                  painter->setBrush ( fg );
                  painter->drawEllipse ( checkRect.adjusted(3*checkRect.width()/8,
                                         3*checkRect.height()/8, -3*checkRect.width()/8,
                                         -3*checkRect.height()/8) );
//                   painter->setBrush ( Qt::NoBrush );
               }
            }
            else {
               // Check box
               QRect checkRect(xpos, r.y()+dpi.f3, r.height()-dpi.f6, r.height()-dpi.f6);
               checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);
//                painter->setBrush ( Qt::NoBrush );
               QStyleOptionMenuItem tmpOpt = *menuItem;
               tmpOpt.rect = checkRect;
               tmpOpt.state &= ~State_Selected; // cause of color, not about checkmark!
               if (checked) {
                  tmpOpt.state |= State_On;
                  tmpOpt.state &= ~State_Off;
               }
               else {
                  tmpOpt.state |= State_Off;
                  tmpOpt.state &= ~State_On;
               }
               drawPrimitive(PE_IndicatorMenuCheckMark, &tmpOpt, painter, widget);
            }
         }
         painter->restore();
      }
      break;
   case CE_MenuScroller: { // Scrolling areas in a QMenu when the style supports scrolling
      QPalette::ColorRole bg = config.menu.std_role[0];
      if (option->state & State_DownArrow) {
         const QPixmap &gradient =
               Gradients::pix(PAL.color(QPalette::Active, bg),
                              RECT.height()*2, Qt::Vertical, sunken ?
                                    Gradients::Sunken : Gradients::Button);
         painter->drawTiledPixmap(RECT, gradient, QPoint(0,RECT.height()));
         drawPrimitive(PE_IndicatorArrowDown, option, painter, widget);
      }
      else {
         const QPixmap &gradient =
               Gradients::pix(PAL.color(QPalette::Active, bg),
                              RECT.height()*2, Qt::Vertical, sunken ?
                                    Gradients::Sunken : Gradients::Button);
         painter->drawTiledPixmap(RECT, gradient);
         drawPrimitive(PE_IndicatorArrowUp, option, painter, widget);
      }
      break;
   }
//    case CE_MenuTearoff: // A menu item representing the tear off section of a QMenu
   case CE_MenuEmptyArea: // The area in a menu without menu items
   case CE_MenuHMargin: // The horizontal extra space on the left/right of a menu
   case CE_MenuVMargin: { // The vertical extra space on the top/bottom of a menu
//       QPalette::ColorRole role = QPalette::Window;
//       if (widget)
//          role = qobject_cast<const QComboBox*>(widget) ?
//          QPalette::WindowText : widget->backgroundRole();
//       painter->fillRect(RECT, PAL.brush(role));
      break;
   }
   case CE_Q3DockWindowEmptyArea: // The empty area of a QDockWidget
      break;
   case CE_ToolBoxTabShape: {
      if (!isEnabled) break;
//       if (const QStyleOptionToolBoxV2* tbt =
//          qstyleoption_cast<const QStyleOptionToolBoxV2*>(option)) {
         
//          Tile::PosFlags pf = Tile::Full;
//          switch (tbt->position) {
//          case QStyleOptionToolBoxV2::Beginning:
//             pf &= ~Tile::Bottom; break;
//          case QStyleOptionToolBoxV2::End:
//             pf &= ~Tile::Top; break;
//          case QStyleOptionToolBoxV2::Middle:
//             pf &= ~(Tile::Top | Tile::Bottom); break;
//          default:
//                break;
//          }
         QColor c = Colors::mid(CCOLOR(tab.std, 0), FCOLOR(Window), 2, 1);
         QRect r = RECT.adjusted(0, 0, 0, -dpi.f2);
         masks.tab.render(r, painter, GRAD(tab), Qt::Vertical, c);
         
         if (sunken || option->state & State_Selected) {
            r.adjust(dpi.f2, dpi.f3, -dpi.f2, -dpi.f3);
            masks.tab.render(r, painter, GRAD(tab), Qt::Vertical, CCOLOR(tab.active, 0));
         }
         else if (hover) {
            r.adjust(dpi.f3, dpi.f4, -dpi.f3, -dpi.f4);
            c = Colors::mid(c, CCOLOR(tab.active, 0), 21, 6);
            masks.tab.render(r, painter, GRAD(tab), Qt::Vertical, c);
         }
         shadows.tabSunken.render(RECT, painter);
//       }
      break;
   }
   case CE_ToolBoxTabLabel:
      if (const QStyleOptionToolBox* tbt =
          qstyleoption_cast<const QStyleOptionToolBox*>(option)) {
         
         sunken = sunken || option->state & (State_Selected);
         
         QColor cB, cF;
         if (sunken) {
            cB = CCOLOR(tab.active, 0);
            cF = CCOLOR(tab.active, 1);
         }
         else {
            cB = CCOLOR(tab.std, 0);
            cF = CCOLOR(tab.std, 1);
         }
         
         if (sunken) hover = false;
               
         if (hover) {
            cB = Colors::mid(cB, CCOLOR(tab.active, 0));
            if (Colors::contrast(CCOLOR(tab.active, 1), cB) >
                Colors::contrast(cF, cB)) {
               cF = CCOLOR(tab.active, 1);
            }
         }
         
         painter->save();
         
         if ((option->state & State_Selected)) {
            QFont tmpFnt = painter->font(); tmpFnt.setBold(true);
            painter->setFont(tmpFnt);
         }
         
         // on dark background, let's paint an emboss
         if (qGray(cB.rgb()) < 128) {
            QRect tr = RECT;
            painter->setPen(cB.dark(120));
            tr.moveTop(tr.top()-1);
            drawItemText(painter, tr, Qt::AlignCenter | Qt::TextShowMnemonic,
                         PAL, isEnabled, tbt->text);
            tr.moveTop(tr.top()+1);
         }

         painter->setPen(cF);
         drawItemText(painter, RECT, Qt::AlignCenter | Qt::TextShowMnemonic,
                      PAL, isEnabled, tbt->text);
         painter->restore();
      }
      break;

   case CE_ToolBoxTab: // The toolbox's tab area
      if (const QStyleOptionToolBox* tbt =
          qstyleoption_cast<const QStyleOptionToolBox*>(option)) {
         if (widget && widget->parentWidget()) // color fix...
            const_cast<QStyleOption*>(option)->palette =
                  widget->parentWidget()->palette();
         drawControl(CE_ToolBoxTabShape, tbt, painter, widget);
         QStyleOptionToolBox copy = *tbt;
         copy.rect.setBottom(copy.rect.bottom()-dpi.f2);
         drawControl(CE_ToolBoxTabLabel, &copy, painter, widget);
      }
      break;
   case CE_SizeGrip: {
      Qt::Corner corner;
      if (const QStyleOptionSizeGrip *sgOpt =
         qstyleoption_cast<const QStyleOptionSizeGrip *>(option))
         corner = sgOpt->corner;
      else if (option->direction == Qt::RightToLeft)
         corner = Qt::BottomLeftCorner;
      else
         corner = Qt::BottomRightCorner;
      
      QRect rect = RECT;
      rect.setWidth(7*RECT.width()/4);
      rect.setHeight(7*RECT.height()/4);
      painter->save();
      painter->setRenderHint(QPainter::Antialiasing);
      int angle = 90<<4;
      switch (corner) {
      default:
      case Qt::BottomLeftCorner:
         angle = 0;
         rect.moveRight(RECT.right());
      case Qt::BottomRightCorner:
         painter->setBrush(FCOLOR(Window).dark(120));
         painter->setPen(FCOLOR(Window).dark(140));
         break;
      case Qt::TopLeftCorner:
         angle += 90<<4;
         rect.moveBottomRight(RECT.bottomRight());
      case Qt::TopRightCorner:
         angle += 90<<4;
         rect.moveBottom(RECT.bottom());
         painter->setBrush(FCOLOR(Window).dark(110));
         painter->setPen(FCOLOR(Window).dark(116));
         painter->drawPie(RECT, -(90<<4), 90<<4);
         break;
      }
      painter->drawPie(rect, angle, 90<<4);
      painter->restore();
      break;
   }
   case CE_Header: // A header
   if (const QStyleOptionHeader *header =
       qstyleoption_cast<const QStyleOptionHeader *>(option)) {
      
      // init
      const QRegion clipRegion = painter->clipRegion();
      painter->setClipRect(RECT, Qt::IntersectClip);
      QStyleOptionHeader subopt = *header;
      
      // extend the sunken state on sorting headers
      sunken = sunken || (subopt.sortIndicator != QStyleOptionHeader::None);
      if (sunken)
         subopt.state |= State_Sunken;
      
      // base
      drawControl(CE_HeaderSection, &subopt, painter, widget);
          
      // label
      subopt.rect = subElementRect(SE_HeaderLabel, header, widget);
      if (subopt.rect.isValid())
         drawControl(CE_HeaderLabel, &subopt, painter, widget);
          
      // sort Indicator on sorting or (inverted) on hovered headers
      if (subopt.sortIndicator != QStyleOptionHeader::None) {
         subopt.rect = subElementRect(SE_HeaderArrow, option, widget);
         painter->save();
         painter->setPen(Qt::NoPen);
         painter->setBrush(FCOLOR(Base));
         drawPrimitive(PE_IndicatorHeaderArrow, &subopt, painter, widget);
         painter->restore();
      }
      painter->setClipRegion(clipRegion);
      break;
   }
   case CE_HeaderSection: { // A header section
      const QStyleOptionHeader *header =
            qstyleoption_cast<const QStyleOptionHeader *>(option);
      Qt::Orientation o = Qt::Vertical; int s = RECT.height();
      if (header && header->orientation == Qt::Vertical) {
         o = Qt::Horizontal;
         s = RECT.width();
      }
      if (hover) sunken = false;
      if (sunken) {
         const QPixmap &sunk = Gradients::pix(FCOLOR(Text), s, o, Gradients::Sunken);
         painter->drawTiledPixmap(RECT, sunk);
      }
//       else if (header && header->orientation == Qt::Vertical) {
//          painter->save();
//          painter->setPen(FCOLOR(Text));
//          painter->setBrush(Colors::mid(FCOLOR(Text), FCOLOR(Base), hover ? 7 : 10, 1));
//          painter->drawRect(RECT.adjusted(0,0,-1,0));
//          painter->restore();
//       }
      else {
         const QPixmap &norm =
               Gradients::pix(FCOLOR(Text), s, o, hover ? Gradients::Glass : Gradients::Button);
         painter->drawTiledPixmap(RECT, norm);
         if (o == Qt::Vertical) {
            QRect r = RECT;
            r.setWidth(RECT.width() - dpi.f1);
            r = RECT; r.setLeft(r.right() - dpi.f1);
            const QPixmap &sunk = Gradients::pix(FCOLOR(Text), s, o, Gradients::Sunken);
            painter->drawTiledPixmap(r, sunk);
         }
      }
      break;
   }
   case CE_HeaderLabel: { // The header's label
      const QStyleOptionHeader* hopt =
            qstyleoption_cast<const QStyleOptionHeader*>(option);
      QRect rect = RECT;
      
      // iconos
      if ( !hopt->icon.isNull() ) {
         QPixmap pixmap =
               hopt->icon.pixmap( 22,22, isEnabled ? QIcon::Normal : QIcon::Disabled );
         int pixw = pixmap.width();
         int pixh = pixmap.height();
         
         rect.setY( rect.center().y() - (pixh - 1) / 2 );
         // "pixh - 1" because of tricky integer division
         drawItemPixmap ( painter, rect, Qt::AlignCenter, pixmap );
         rect = RECT; rect.setLeft( rect.left() + pixw + 2 );
      }
      
      if (hopt->text.isEmpty())
         break;
      // textos ;)
      painter->save();
      
      // this works around a possible Qt bug?!?
      QFont tmpFnt = painter->font(); tmpFnt.setBold(sunken);
      painter->setFont(tmpFnt);

      QColor bg = FCOLOR(Text), fg = FCOLOR(Base);
      if (qGray(bg.rgb()) < 148) { // dark background, let's paint an emboss
         rect.moveTop(rect.top()-1);
         painter->setPen(bg.dark(120));
         drawItemText ( painter, rect, Qt::AlignCenter, PAL, isEnabled, hopt->text);
         rect.moveTop(rect.top()+1);
      }
      painter->setPen(fg);
      drawItemText ( painter, rect, Qt::AlignCenter, PAL, isEnabled, hopt->text);
      painter->restore();
      break;
   }
   case CE_ScrollBarAddLine: // ======= scroll down
      if (!config.scroll.showButtons)
         break;
      if (option->state & State_Item) { // combobox scroller
         painter->save();
         painter->setPen(hover?FCOLOR(Text):Colors::mid(FCOLOR(Base),FCOLOR(Text)));
         QStyleOption opt = *option;
         opt.rect = RECT.adjusted(RECT.width()/4, RECT.height()/4,
                                  -RECT.width()/4, -RECT.height()/4);
         if (option->state & QStyle::State_Horizontal)
            drawPrimitive (PE_IndicatorArrowRight, &opt, painter, widget);
         else
            drawPrimitive (PE_IndicatorArrowDown, &opt, painter, widget);
         painter->restore();
         break;
      }
   case CE_ScrollBarSubLine: // ======= scroll up
      if (!config.scroll.showButtons)
         break;
      if (option->state & State_Item) { // combobox scroller
         painter->save();
         painter->setPen(hover?FCOLOR(Text):Colors::mid(FCOLOR(Base),FCOLOR(Text)));
         QStyleOption opt = *option;
         opt.rect = RECT.adjusted(RECT.width()/4, RECT.height()/4,
                                  -RECT.width()/4, -RECT.height()/4);
         if (option->state & QStyle::State_Horizontal)
            drawPrimitive (PE_IndicatorArrowLeft, &opt, painter, widget);
         else
            drawPrimitive (PE_IndicatorArrowUp, &opt, painter, widget);
         painter->restore();
         break;
      }
      if (const QStyleOptionSlider *opt =
            qstyleoption_cast<const QStyleOptionSlider *>(option)) {
         bool alive = isEnabled && ((element == CE_ScrollBarAddLine &&
                                     opt->sliderValue < opt->maximum) ||
                                    (element == CE_ScrollBarSubLine &&
                                     opt->sliderValue > opt->minimum));
         hover = hover && alive;
         QPoint xy = RECT.topLeft();
         const int sz = dpi.ExclusiveIndicator - dpi.f4;
         const int step = (hover && !complexStep) ? 6 : complexStep;
         const QColor c = (!alive) ? FCOLOR(Window) :
               Colors::mid(CCOLOR(btn.std, 0), CCOLOR(btn.active, 0), 18, step);
         const QPixmap &fill =
               Gradients::pix(c, sz, Qt::Vertical, (sunken || !alive) ?
               Gradients::Sunken : Gradients::Button);
         fillWithMask(painter, xy, fill, masks.radio);
         if (alive) {
            painter->save();
            painter->setPen(CCOLOR(btn.std, 0).dark(120));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(QRect(xy, QSize(sz, sz)));
            painter->restore();
         }
         break;
      }
   case CE_ScrollBarSubPage: // Scroll bar page decrease indicator (i.e., page up).
   case CE_ScrollBarAddPage: {// Scolllbar page increase indicator (i.e., page down).
      if (option->state & State_Item) // combobox scroller
         break;
      
      // the groove TODO: might be Colors::mid(bg, fg) is better?!
      SAVE_PEN;
      painter->setPen(FCOLOR(Window).dark(115));
      if (option->state & QStyle::State_Horizontal) {
         int y = RECT.center().y()-dpi.f1;
         painter->drawLine(RECT.x(), y, RECT.right(), y);
         ++y; painter->setPen(FCOLOR(Window).light(108));
         painter->drawLine(RECT.x(), y, RECT.right(), y);
      }
      else {
         int x = RECT.center().x();
         painter->drawLine(x, RECT.y(), x, RECT.bottom());
         ++x; painter->setPen(FCOLOR(Window).light(108));
         painter->drawLine(x, RECT.y(), x, RECT.bottom());
      }
      RESTORE_PEN;

      break;
   }
   case CE_ScrollBarSlider: // Scroll bar slider.
      if (option->state & State_Item) {
         painter->fillRect(RECT.adjusted(dpi.f2, 0, -dpi.f2, 0),
                           (hover || sunken) ? FCOLOR(Text) :
                                 Colors::mid(FCOLOR(Base), FCOLOR(Text), 8, 1));
         break;
      }
      if (/*const QStyleOptionSlider *opt =*/
          qstyleoption_cast<const QStyleOptionSlider *>(option)) {
  
         if (!isEnabled) {
            if (config.scroll.sunken)
               shadows.tabSunken.render(RECT, painter);
            else
               drawControl(CE_ScrollBarSubPage, option, painter, widget);
            break;
         }

         // --> we need to paint a slider

         // the hover indicator color (inside area)
         QColor c;
         if (sunken)
            c = CCOLOR(btn.active, 0);
         else if (complexStep) {
            c = Colors::mid(CCOLOR(btn.std, 0), CCOLOR(btn.active, 0), 2, 1);
            c = Colors::mid(c, CCOLOR(btn.active, 0), 6-complexStep, complexStep);
         }
         else if (hover)
            c = CCOLOR(btn.active, 0);
         else if (widgetStep)
            c = Colors::mid(CCOLOR(btn.std, 0), CCOLOR(btn.active, 0), 18-widgetStep, widgetStep);
         else if (scrollAreaHovered_)
            c = Colors::mid(CCOLOR(btn.std, 0), CCOLOR(btn.active, 0), 2, 1);
         else
            c = CCOLOR(btn.std, 0);

         QRect r = RECT;
         const int f1 = dpi.f1, f2 = dpi.f2;
         int d = f2;

         // shadow
         if (config.scroll.sunken &&
             option->state & QStyle::State_Horizontal) {
            r.setBottom(r.bottom()-dpi.f2);
            d = 0;
         }
         if (sunken) {
            r.adjust(f1, f1, -f1, -f1);
            shadows.tab[true][true].render(r, painter);
            r.adjust(f1, f1, -f1, -d);
         }
         else {
            shadows.tab[true][false].render(r, painter);
            r.adjust(f2, f2, -f2, -(d+f1));
         }

         // gradient setup
         Qt::Orientation o; int size; Tile::PosFlags pf;
         if (option->state & QStyle::State_Horizontal) {
            o = Qt::Vertical; size = r.height();
            pf = Tile::Top | Tile::Bottom;
         }
         else {
            o = Qt::Horizontal; size = r.width();
            pf = Tile::Left | Tile::Right;
         }

         // the allways shown base
         masks.tab.render(r, painter, GRAD(scroll), o,
                          config.btn.fullHover ? c : CCOLOR(btn.std, 0), size);
         masks.tab.outline(r, painter, Colors::mid(CCOLOR(btn.std, 0), Qt::white,1,2), true);

         if (config.btn.fullHover)
            break; // really - nothing to do anymore!

         r.adjust(f2, f2, -f2, -f2);
         masks.button.render(r, painter, Gradients::Progress/*GRAD(btn)*/, o, c, size, QPoint(f2,f2));
      }
      break;
//    case CE_ScrollBarFirst: // Scroll bar first line indicator (i.e., home).
//    case CE_ScrollBarLast: // Scroll bar last line indicator (i.e., end).
   case CE_RubberBand: {// Rubber band used in such things as iconview.
      painter->save();
      QColor c = FCOLOR(Highlight);
      painter->setPen(c);
      c.setAlpha(100);
      painter->setBrush(c);
      painter->drawRect(RECT.adjusted(0,0,-1,-1));
      painter->restore();
      break;
   }
   case CE_FocusFrame: // Focus Frame that can is style controled.
      break;
   case CE_ComboBoxLabel: // The label of a non-editable QComboBox
      if (const QStyleOptionComboBox *cb =
          qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
         QRect editRect = subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, widget);
         painter->save();
         painter->setClipRect(editRect);
         // icon
         if (!cb->currentIcon.isNull()) {
            QIcon::Mode mode = isEnabled ? QIcon::Normal
               : QIcon::Disabled;
            QPixmap pixmap = cb->currentIcon.pixmap(cb->iconSize, mode);
            QRect iconRect(editRect);
            iconRect.setWidth(cb->iconSize.width() + 4);
            iconRect = alignedRect(QApplication::layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter, iconRect.size(), editRect);
/*            if (cb->editable)
               painter->fillRect(iconRect, opt->palette.brush(QPalette::Base));*/
            drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
            
            if (cb->direction == Qt::RightToLeft)
               editRect.translate(-4 - cb->iconSize.width(), 0);
            else
               editRect.translate(cb->iconSize.width() + 4, 0);
         }
         // text
         if (!cb->currentText.isEmpty() && !cb->editable) {
            if (cb->frame) {
               const QComboBox* combo = widget ?
                     qobject_cast<const QComboBox*>(widget) : 0;
               hover = hover || ( combo && combo->view() &&
                     ((QWidget*)(combo->view()))->isVisible());
               int f3 = dpi.f3;
               editRect.adjust(f3,0, -f3, 0);
               painter->setPen(hover ? CCOLOR(btn.active, 1) : CCOLOR(btn.std, 1));
            }
            painter->drawText(editRect, Qt::AlignCenter, cb->currentText);
         }
         painter->restore();
      }
      break;
   case CE_ToolBar:
      break;
   default:
         QCommonStyle::drawControl( element, option, painter, widget );
   } // switch
}
