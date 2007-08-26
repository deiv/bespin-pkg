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

#include <Q3ScrollView>
#include <QAbstractScrollArea>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QPainter>
#include <QTime>
// #include <QPixmapCache>
#include <QStyleOptionComplex>
#include <cmath>
#include "bespin.h"
#include "colors.h"

using namespace Bespin;

extern Config config;
extern Dpi dpi;

#include "makros.h"

#include <QtDebug>

bool BespinStyle::scrollAreaHovered(const QWidget* slider) const {
//    bool scrollerActive = false;
   QWidget *scrollWidget = const_cast<QWidget*>(slider);
   if (!scrollWidget->isEnabled())
      return false;
   while (scrollWidget &&
          !(qobject_cast<QAbstractScrollArea*>(scrollWidget) ||
          qobject_cast<Q3ScrollView*>(scrollWidget) ||
           animator->handlesArea(scrollWidget)))
      scrollWidget = const_cast<QWidget*>(scrollWidget->parentWidget());
   bool isActive = true;
   if (scrollWidget) {
//       QAbstractScrollArea* scrollWidget = (QAbstractScrollArea*)daddy;
      QPoint tl = scrollWidget->mapToGlobal(QPoint(0,0));
      QRegion scrollArea(tl.x(),tl.y(),
                         scrollWidget->width(),
                         scrollWidget->height());
      QList<QAbstractScrollArea*> scrollChilds =
         scrollWidget->findChildren<QAbstractScrollArea*>();
      for (int i = 0; i < scrollChilds.size(); ++i) {
         QPoint tl = scrollChilds[i]->mapToGlobal(QPoint(0,0));
         scrollArea -= QRegion(tl.x(), tl.y(),
                               scrollChilds[i]->width(),
                               scrollChilds[i]->height());
      }
      QList<Q3ScrollView*> scrollChilds2 =
         scrollWidget->findChildren<Q3ScrollView*>();
      for (int i = 0; i < scrollChilds2.size(); ++i) {
         QPoint tl = scrollChilds[i]->mapToGlobal(QPoint(0,0));
         scrollArea -= QRegion(tl.x(), tl.y(),
                               scrollChilds2[i]->width(),
                               scrollChilds2[i]->height());
      }
//       scrollerActive = scrollArea.contains(QCursor::pos());
      isActive = scrollArea.contains(QCursor::pos());
   }
   return isActive;
}

void BespinStyle::drawComplexControl ( ComplexControl control, const QStyleOptionComplex * option, QPainter * painter, const QWidget * widget) const
{
   Q_ASSERT(option);
   Q_ASSERT(painter);
   
   bool sunken = option->state & State_Sunken;
   bool isEnabled = option->state & State_Enabled;
   bool hover = isEnabled && (option->state & State_MouseOver);
   bool hasFocus = option->state & State_HasFocus;
   
   switch ( control )
   {
   case CC_SpinBox: // A spinbox, like QSpinBox
      if (const QStyleOptionSpinBox *sb =
          qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
         QStyleOptionSpinBox copy = *sb;
         
         // this doesn't work (for the moment, i assume...)
//          isEnabled = isEnabled && !(option->state & State_ReadOnly);
         if (isEnabled)
         if (widget)
         if (const QAbstractSpinBox *box = qobject_cast<const QAbstractSpinBox*>(widget)) {
            isEnabled = isEnabled && !box->isReadOnly();
            if (!isEnabled)
            copy.state &= ~State_Enabled;
         }
         
         if (sb->frame && (sb->subControls & SC_SpinBoxFrame))
            drawPrimitive ( PE_PanelLineEdit, &copy, painter, widget );
         
         if (!isEnabled)
            break; // why bother the user with elements he can't use... ;)
         
         Tile::PosFlags pf;
         int uh = 0;
         
         if (sb->subControls & SC_SpinBoxUp) {
            copy.subControls = SC_SpinBoxUp;
            copy.rect = subControlRect(CC_SpinBox, sb, SC_SpinBoxUp, widget);
            uh = copy.rect.height();
            
            pf = Tile::Top | Tile::Left | Tile::Right;
            isEnabled = sb->stepEnabled & QAbstractSpinBox::StepUpEnabled;
            hover = isEnabled && (sb->activeSubControls == SC_SpinBoxUp);
            sunken = sunken && (sb->activeSubControls == SC_SpinBoxUp);
            
            int dx = copy.rect.width()/4, dy = copy.rect.height()/4;
            copy.rect.adjust(dx, 2*dy,-dx,-dpi.f1);
            
            if (!sunken) {
               painter->setPen(FCOLOR(Base).dark(105));
               copy.rect.translate(dpi.f2, dpi.f2);
               drawPrimitive(PE_IndicatorArrowUp, &copy, painter, widget);
               copy.rect.translate(-dpi.f2, -dpi.f2);
            }
            
            QColor c;
            if (hover)
               c = FCOLOR(Highlight);
            else if (isEnabled)
               c = Colors::mid(FCOLOR(Base), FCOLOR(Text));
            else
               c = Colors::mid(FCOLOR(Base), PAL.color(QPalette::Disabled, QPalette::Text));
            
            painter->setPen(c);
            drawPrimitive(PE_IndicatorSpinUp, &copy, painter, widget);
         }
         
         if (sb->subControls & SC_SpinBoxDown) {
            copy.subControls = SC_SpinBoxDown;
            copy.rect = subControlRect(CC_SpinBox, sb, SC_SpinBoxDown, widget);
            
            pf = Tile::Bottom | Tile::Left | Tile::Right;
            isEnabled = sb->stepEnabled & QAbstractSpinBox::StepDownEnabled;
            hover = isEnabled && (sb->activeSubControls == SC_SpinBoxDown);
            sunken = sunken && (sb->activeSubControls == SC_SpinBoxDown);
            
            int dx = copy.rect.width()/4, dy = copy.rect.height()/4;
            copy.rect.adjust(dx, dpi.f1,-dx,-2*dy);
            
            if (!sunken) {
               painter->setPen(FCOLOR(Base).dark(105));
               copy.rect.translate(dpi.f2, dpi.f2);
               drawPrimitive(PE_IndicatorArrowDown, &copy, painter, widget);
               copy.rect.translate(-dpi.f2, -dpi.f2);
            }
            
            QColor c;
            if (hover)
               c = FCOLOR(Highlight);
            else if (isEnabled)
               c = Colors::mid(FCOLOR(Base), FCOLOR(Text));
            else
               c = Colors::mid(FCOLOR(Base), PAL.color(QPalette::Disabled, QPalette::Text));
            
            painter->setPen(c);
            drawPrimitive(PE_IndicatorSpinDown, &copy, painter, widget);
         }
      }
      break;
   case CC_GroupBox:
      if (const QStyleOptionGroupBox *groupBox =
          qstyleoption_cast<const QStyleOptionGroupBox *>(option)) {
         if (groupBox->subControls & QStyle::SC_GroupBoxFrame) {
            QStyleOptionFrameV2 frame;
            frame.QStyleOption::operator=(*groupBox);
            frame.features = groupBox->features;
            frame.lineWidth = groupBox->lineWidth;
            frame.midLineWidth = groupBox->midLineWidth;
            frame.rect = subControlRect(CC_GroupBox, option, SC_GroupBoxFrame, widget);
            drawPrimitive(PE_FrameGroupBox, &frame, painter, widget);
         }
         
         // Draw title
         if ((groupBox->subControls & QStyle::SC_GroupBoxLabel) &&
             !groupBox->text.isEmpty()) {
            QColor textColor = groupBox->textColor;
            if (textColor.isValid()) painter->setPen(textColor);
            QFont tmpfnt = painter->font(); tmpfnt.setBold(true);
            painter->setFont ( tmpfnt );
            QStyleOptionGroupBox copy = *groupBox; copy.fontMetrics = QFontMetrics(tmpfnt);
            QRect textRect = subControlRect(CC_GroupBox, &copy, SC_GroupBoxLabel, widget);
            int alignment = Qt::AlignCenter; //int(groupBox->textAlignment);
            if (!styleHint(QStyle::SH_UnderlineShortcut, option, widget))
               alignment |= Qt::TextHideMnemonic;
            else
               alignment |= Qt::TextShowMnemonic;
            
            drawItemText(painter, textRect,  alignment, groupBox->palette, isEnabled, groupBox->text,
                           textColor.isValid() ? QPalette::NoRole : QPalette::Foreground);
            int x = textRect.bottom(); textRect = RECT; textRect.setTop(x);
            x = textRect.width()/4; textRect.adjust(x,0,-x,0);
            shadows.line[0][Sunken].render(textRect, painter);
         }
         
         // Draw checkbox // TODO: sth better - maybe a round thing in the upper left corner...? also doesn't hover - yet.
         if (groupBox->subControls & SC_GroupBoxCheckBox) {
            QStyleOptionButton box;
            box.QStyleOption::operator=(*groupBox);
            box.rect = subControlRect(CC_GroupBox, option, SC_GroupBoxCheckBox, widget);
            drawPrimitive(PE_IndicatorCheckBox, &box, painter, widget);
         }
      }
      break;
    case CC_Q3ListView:
        if (const QStyleOptionQ3ListView *lv =
            qstyleoption_cast<const QStyleOptionQ3ListView *>(option)) {
            int i;
            if (lv->subControls & SC_Q3ListView)
                QCommonStyle::drawComplexControl(control, lv, painter, widget);
            if (lv->subControls & (SC_Q3ListViewBranch | SC_Q3ListViewExpand)) {
               if (lv->items.isEmpty())
                  break;
               QStyleOptionQ3ListViewItem item = lv->items.at(0);
               int y = lv->rect.y();
               int c;
               int dotoffset = 0;
               QPolygon dotlines;
               if ((lv->activeSubControls & SC_All) &&
                  (lv->subControls & SC_Q3ListViewExpand)) {
                  c = 2;
                  dotlines.resize(2);
                  dotlines[0] = QPoint(lv->rect.right(), lv->rect.top());
                  dotlines[1] = QPoint(lv->rect.right(), lv->rect.bottom());
               }
               else {
                  int linetop = 0, linebot = 0;
                  // each branch needs at most two lines, ie. four end points
                  dotoffset = (item.itemY + item.height - y) % 2;
                  dotlines.resize(item.childCount * 4);
                  c = 0;

                  // skip the stuff above the exposed rectangle
                  for (i = 1; i < lv->items.size(); ++i) {
                     QStyleOptionQ3ListViewItem child = lv->items.at(i);
                     if (child.height + y > 0)
                           break;
                     y += child.totalHeight;
                  }
                  int bx = lv->rect.width() / 2;

                  // paint stuff in the magical area
                  while (i < lv->items.size() && y < lv->rect.height()) {
                     QStyleOptionQ3ListViewItem child = lv->items.at(i);
                     if (child.features & QStyleOptionQ3ListViewItem::Visible) {
                           int lh;
                           if (!(item.features & QStyleOptionQ3ListViewItem::MultiLine))
                              lh = child.height;
                           else
                              lh = painter->fontMetrics().height() + 2 * lv->itemMargin;
                           lh = qMax(lh, QApplication::globalStrut().height());
                           if (lh % 2 > 0)
                              ++lh;
                           linebot = y + lh / 2;
                           if (child.features & QStyleOptionQ3ListViewItem::Expandable
                              || child.childCount > 0 && child.height > 0) {
                              // needs a box
                              painter->setPen(lv->palette.mid().color());
                              painter->drawRect(bx - 4, linebot - 4, 8, 8);
                              // plus or minus
                              painter->setPen(lv->palette.text().color());
                              painter->drawLine(bx - 2, linebot, bx + 2, linebot);
                              if (!(child.state & State_Open))
                                 painter->drawLine(bx, linebot - 2, bx, linebot + 2);
                              // dotlinery
                              painter->setPen(lv->palette.mid().color());
                              dotlines[c++] = QPoint(bx, linetop);
                              dotlines[c++] = QPoint(bx, linebot - 4);
                              dotlines[c++] = QPoint(bx + 5, linebot);
                              dotlines[c++] = QPoint(lv->rect.width(), linebot);
                              linetop = linebot + 5;
                           } else {
                              // just dotlinery
                              dotlines[c++] = QPoint(bx+1, linebot -1);
                              dotlines[c++] = QPoint(lv->rect.width(), linebot -1);
                           }
                           y += child.totalHeight;
                     }
                     ++i;
                  }
                  // Expand line height to edge of rectangle if there's any
                  // visible child below
                  while (i < lv->items.size() && lv->items.at(i).height <= 0)
                     ++i;
                  if (i < lv->items.size())
                     linebot = lv->rect.height();

                  if (linetop < linebot) {
                     dotlines[c++] = QPoint(bx, linetop);
                     dotlines[c++] = QPoint(bx, linebot);
                  }
               }
                painter->setPen(lv->palette.text().color());

                int line; // index into dotlines
                if (lv->subControls & SC_Q3ListViewBranch)
                   for(line = 0; line < c; line += 2) {
                    // assumptions here: lines are horizontal or vertical.
                    // lines always start with the numerically lowest
                    // coordinate.

                    // point ... relevant coordinate of current point
                    // end ..... same coordinate of the end of the current line
                    // other ... the other coordinate of the current point/line
                    if (dotlines[line].y() == dotlines[line+1].y()) {
                        int end = dotlines[line + 1].x();
                        int point = dotlines[line].x();
                        int other = dotlines[line].y();
                        while (point < end) {
                            int i = 128;
                            if (i + point > end)
                                i = end-point;
                           painter->drawLine(point, other, point+i, other);
                            point += i;
                        }
                    } else {
                        int end = dotlines[line + 1].y();
                        int point = dotlines[line].y();
                        int other = dotlines[line].x();
                        while(point < end) {
                            int i = 128;
                            if (i + point > end)
                                i = end-point;
                           painter->drawLine(other, point, other, point+i);
                           point += i;
                        }
                    }
                }
            }
        }
        break;
   case CC_ComboBox: // A combobox, like QComboBox
      if (const QStyleOptionComboBox *cmb =
          qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
         const int f2 = dpi.f2, f3 = dpi.f3;
         QRect ar, r = RECT; r.setBottom(r.bottom()-f2);
         const QComboBox* combo = widget ?
                qobject_cast<const QComboBox*>(widget) : 0;
         const bool listShown = combo && combo->view() &&
               ((QWidget*)(combo->view()))->isVisible();
         int step = 0; QColor c = CONF_COLOR(btn.std, 0);
         
         if (listShown) { // this messes up hover
            hover = hover || QRect(widget->mapToGlobal(RECT.topLeft()),
                                   RECT.size()).contains(QCursor::pos());
         }
         
         // do we have an arrow?
         if (isEnabled &&
             (cmb->subControls & SC_ComboBoxArrow) &&
             (!combo || combo->count() > 0)) {
            ar = subControlRect(CC_ComboBox, cmb, SC_ComboBoxArrow, widget);
            ar.setBottom(ar.bottom()-f2);
         }
             

         // the label
         if ((cmb->subControls & SC_ComboBoxFrame) && cmb->frame) {
            if (cmb->editable) {
               drawPrimitive(PE_PanelLineEdit, option, painter, widget);
            }
            else {
               if (!ar.isNull()) {
                  // ground
                  step = animator->hoverStep(widget);
                  if (listShown) step = 6;
                  
                  c = Colors::btnBg(PAL, isEnabled, hasFocus, 0);
                  if (config.btn.fullHover)
                     c = Colors::mid(c, CONF_COLOR(btn.active, 0), 6-step, step);
                  
                  masks.tab.render(r, painter, GRAD(chooser), Qt::Vertical, c);
                  
                  if (hasFocus) {
                     const QColor hlc =
                           Colors::mid(CONF_COLOR(btn.std, 0), FCOLOR(Highlight), 2, 1);
                     masks.tab.outline(r, painter, hlc, f3);
                  }
                  
                  // maybe hover indicator?
                  if (!config.btn.fullHover && step) { // jupp ;)
                     r.adjust(f3, f3, -f3, -f3);
                     const QColor c2 =
                           Colors::mid(c, CONF_COLOR(btn.active, 0), 6-step, step);
                     masks.tab.render(r, painter, GRAD(chooser), Qt::Vertical,
                                      c2, RECT.height()-f2, QPoint(0,f3));
                  }
               }
               shadows.tabSunken.render(RECT, painter);
            }
         }
         
         // the arrow
         if (!ar.isNull()) {
            QStyleOptionComboBox tmpOpt = *cmb;
            const int dx = ar.width()/3, dy = ar.height()/3;
            tmpOpt.rect = ar.adjusted(dx, dy, -dx, -dy);
            PrimitiveElement arrow;
            
            if (!listShown)
               arrow = PE_IndicatorArrowDown;
            else if (config.leftHanded)
               arrow = PE_IndicatorArrowRight;
            else
               arrow = PE_IndicatorArrowLeft;

            painter->setRenderHint ( QPainter::Antialiasing, true );
            painter->save();
            if (cmb->editable) {
               hover = hover && (cmb->activeSubControls == SC_ComboBoxArrow);
               if (!sunken) {
                  painter->setPen(FCOLOR(Base).dark(105));
                  tmpOpt.rect.translate(-f2, f2);
                  drawPrimitive(arrow, &tmpOpt, painter, widget);
                  tmpOpt.rect.translate(f2, -f2);
               }
               if (hover || listShown)
                  painter->setPen(FCOLOR(Highlight));
               else
                  painter->setPen( Colors::mid(FCOLOR(Base), FCOLOR(Text)) );
            }
            else {
               c = Colors::mid(c, CONF_COLOR(btn.active, 0));
               c = Colors::mid(c, CONF_COLOR(btn.active, 0), 6-step, step);
               ar.adjust(f2, dpi.f4, -f2, -dpi.f4);
               masks.tab.render(ar, painter, GRAD(chooser), Qt::Vertical, c,
                                RECT.height()-f2, QPoint(0,dpi.f4));
               painter->setPen(CONF_COLOR(btn.active, 1));
            }
            drawPrimitive(arrow, &tmpOpt, painter, widget);
            painter->restore();
         }
      }
      break;
   case CC_ScrollBar: // A scroll bar, like QScrollBar
      if (const QStyleOptionSlider *scrollbar =
         qstyleoption_cast<const QStyleOptionSlider *>(option)) {
         // Make a copy here and reset it for each primitive.
         QStyleOptionSlider newScrollbar = *scrollbar;
         BespinStyle *that = const_cast<BespinStyle*>( this );
         
         // TODO: this is a stupid hack, move the whole special scrollbar painting here!
         if (widget && widget->parentWidget() &&
              widget->parentWidget()->parentWidget() &&
              widget->parentWidget()->parentWidget()->inherits("QComboBoxListView")) {
            painter->fillRect(RECT, PAL.brush(QPalette::Base));
            newScrollbar.state |= State_Item;
         }
         
         State saveFlags = newScrollbar.state;
         if (scrollbar->minimum == scrollbar->maximum)
               saveFlags &= ~State_Enabled;
         
         if (scrollbar->activeSubControls & SC_ScrollBarSlider) {
            that->widgetStep = 0;
            that->scrollAreaHovered_ = true;
         }
         else {
            that->widgetStep = animator->hoverStep(widget);
            that->scrollAreaHovered_ = scrollAreaHovered(widget);
         }
         
         SubControls hoverControls = scrollbar->activeSubControls &
               (SC_ScrollBarSubLine | SC_ScrollBarAddLine | SC_ScrollBarSlider);
         const ComplexHoverFadeInfo *info =
               animator->fadeInfo(widget, hoverControls);
         
#define PAINT_ELEMENT(_E_)\
         if (scrollbar->subControls & SC_ScrollBar##_E_) {\
            newScrollbar.rect = scrollbar->rect;\
            newScrollbar.state = saveFlags;\
            newScrollbar.rect =\
               subControlRect(control, &newScrollbar, SC_ScrollBar##_E_, widget);\
            if (newScrollbar.rect.isValid()) {\
               if (!(scrollbar->activeSubControls & SC_ScrollBar##_E_))\
                  newScrollbar.state &= ~(State_Sunken | State_MouseOver);\
               if (info && (info->fadeIns & SC_ScrollBar##_E_ ||\
                  info->fadeOuts & SC_ScrollBar##_E_))\
                  that->complexStep = info->step(SC_ScrollBar##_E_);\
               else \
                  that->complexStep = 0; \
               drawControl(CE_ScrollBar##_E_, &newScrollbar, painter, widget);\
            }\
         }//
         QRect groove = RECT;
         if (config.scroll.showButtons) {
            PAINT_ELEMENT(SubLine);
            PAINT_ELEMENT(AddLine);
            if (config.scroll.sunken)
               groove = subControlRect(control, option, SC_ScrollBarGroove, widget);
         }

         if (!config.scroll.sunken) {
            PAINT_ELEMENT(SubPage);
            PAINT_ELEMENT(AddPage);
         }
//          PAINT_ELEMENT(SC_ScrollBarFirst, CE_ScrollBarFirst);
//          PAINT_ELEMENT(SC_ScrollBarLast, CE_ScrollBarLast);
         if (config.scroll.groove)
            masks.tab.render(groove, painter, Gradients::Sunken,
                             option->state & QStyle::State_Horizontal ?
                                   Qt::Vertical : Qt::Horizontal, FCOLOR(Window));
         
         if (isEnabled && scrollbar->subControls & SC_ScrollBarSlider) {
            newScrollbar.rect = scrollbar->rect;
            newScrollbar.state = saveFlags;
            newScrollbar.rect = subControlRect(control, &newScrollbar,
                                               SC_ScrollBarSlider, widget);
            if (config.scroll.sunken)
               newScrollbar.rect.adjust(-1,-1,1,1);
            if (newScrollbar.rect.isValid()) {
               if (!(scrollbar->activeSubControls & SC_ScrollBarSlider))
                  newScrollbar.state &= ~(State_Sunken | State_MouseOver);
               if (scrollbar->state & State_HasFocus)
                  newScrollbar.state |= (State_Sunken | State_MouseOver);
               if (info && (info->fadeIns & SC_ScrollBarSlider ||
                   info->fadeOuts & SC_ScrollBarSlider))
                  that->complexStep = info->step(SC_ScrollBarSlider);
               else
                  that->complexStep = 0;
               drawControl(CE_ScrollBarSlider, &newScrollbar, painter, widget);
            }
         }
         if (config.scroll.sunken)
            shadows.tabSunken.render(groove, painter);
      }
      break;
   case CC_Slider: // A slider, like QSlider
      if (const QStyleOptionSlider *slider =
          qstyleoption_cast<const QStyleOptionSlider *>(option)) {
         QRect groove = QCommonStyle::subControlRect(CC_Slider, slider, SC_SliderGroove, widget);
         QRect handle = QCommonStyle::subControlRect(CC_Slider, slider, SC_SliderHandle, widget);

         isEnabled = isEnabled && (slider->maximum > slider->minimum);
         hover = isEnabled && hover && (slider->activeSubControls & SC_SliderHandle);
         sunken = sunken && (slider->activeSubControls & SC_SliderHandle);
         
         const int ground = 0;
         
         if ((slider->subControls & SC_SliderGroove) && groove.isValid()) {
            painter->save();

            QRect r;
            const QColor c = Colors::mid(FCOLOR(Window), CONF_COLOR(progress.std, 0), 3,1);
            if ( slider->orientation == Qt::Horizontal ) {
               
               int y = groove.center().y();
               painter->setPen(FCOLOR(Window).dark(115));
               painter->drawLine(groove.x(),y,groove.right(),y);
               ++y;
               painter->setPen(FCOLOR(Window).light(108));
               painter->drawLine(groove.x(),y,groove.right(),y);

               // the "temperature"
               if (slider->sliderPosition != ground &&
                   slider->maximum > slider->minimum) {
                  --y;
                  
                  int groundX = groove.width() * (ground - slider->minimum) /
                        (slider->maximum - slider->minimum);
                  bool rightSide = slider->sliderPosition > ground;
                  
                  if (slider->upsideDown) {
                     rightSide = !rightSide;
                     groundX = groove.right() - groundX;
                  }
                  else
                     groundX += groove.left();
                  
                  if (rightSide) {
                     groove.setLeft(groundX);
                     groove.setRight(handle.center().x());
                  }
                  else {
                     groove.setLeft(handle.center().x());
                     groove.setRight(groundX);
                  }
                  painter->setPen(c.dark(115));
                  painter->drawLine(groove.x(), y, groove.right(), y);
                  ++y;
                  painter->setPen(c.light(108));
                  painter->drawLine(groove.x(), y, groove.right(), y);
               }
            }
            else { // Vertical

               int x = groove.center().x();
               painter->setPen(FCOLOR(Window).dark(115));
               painter->drawLine(x, groove.y(), x, groove.bottom());
               ++x;
               painter->setPen(FCOLOR(Window).light(108));
               painter->drawLine(x, groove.y(), x, groove.bottom());

               // the "temperature"
               if (slider->sliderPosition != ground &&
                   slider->maximum > slider->minimum) {
                  --x;
                  
                  int groundY = groove.height() * (ground - slider->minimum) /
                        (slider->maximum - slider->minimum);
                  bool upside = slider->sliderPosition > ground;
                  
                  if (slider->upsideDown) {
                     upside = !upside;
                     groundY = groove.bottom() - groundY;
                  }
                  else
                     groundY += groove.top();
                  
                  if (upside) {
                     groove.setBottom(handle.center().y());
                     groove.setTop(groundY);
                  }
                  else {
                     groove.setBottom(groundY);
                     groove.setTop(handle.center().y());
                  }
                  
                  painter->setPen(c.dark(115));
                  painter->drawLine(x, groove.y(), x, groove.bottom());
                  ++x;
                  painter->setPen(c.light(108));
                  painter->drawLine(x, groove.y(), x, groove.bottom());

               }
               // for later (cosmetic)
               if (!slider->upsideDown)
                  handle.translate(dpi.f6, 0);
            }
            painter->restore();
         }
         
         int direction = 0;
         if (slider->orientation == Qt::Vertical)
            ++direction;
         
         // ticks - TODO: paint our own ones?
         if ((slider->subControls & SC_SliderTickmarks) &&
              (slider->tickPosition != QSlider::NoTicks) ) {
            if (slider->tickPosition == QSlider::TicksAbove) {
               direction += 2;
               if (slider->orientation == Qt::Horizontal)
                  handle.translate(0,-dpi.f6);
               else
                  handle.translate(-dpi.f6,0);
            }
            QStyleOptionSlider tmpSlider = *slider;
            tmpSlider.subControls = SC_SliderTickmarks;
            QCommonStyle::drawComplexControl(control, &tmpSlider, painter, widget);
         }
         
         // handle
         if (slider->subControls & SC_SliderHandle) {
            int step;
            if (sunken)
               step = 6;
            else {
               step = 0;
               const ComplexHoverFadeInfo *info = animator->fadeInfo(widget,
                     slider->activeSubControls & SC_SliderHandle);
               if (info && (info->fadeIns & SC_SliderHandle ||
                     info->fadeOuts & SC_SliderHandle))
                  step = info->step(SC_SliderHandle);
               if (hover && !step)
                  step = 6;
            }

            // shadow
            QPoint xy = handle.topLeft();
            painter->drawPixmap(sunken ? xy + QPoint(dpi.f1,dpi.f1) : xy,
                                shadows.sliderRound[sunken][isEnabled]);
            // gradient
            xy += QPoint(dpi.f2, dpi.f1/*direction?dpi.f1:0*/);
            QColor c = Colors::mid(CONF_COLOR(btn.std, 0), CONF_COLOR(btn.active, 0), 6-step, step);
            const int sz = dpi.SliderControl-dpi.f4;
            const QBrush fill = Gradients::brush(c, sz, Qt::Vertical,
                  isEnabled ? GRAD(btn) : Gradients::None);
//             fillWithMask(painter, xy, fill, masks.radio);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(QPen(CONF_COLOR(btn.std, 0), dpi.f2));
            painter->setBrush(fill);
            painter->setBrushOrigin(xy);
            painter->drawEllipse(QRect(xy, QSize(sz,sz)));
            c = Colors::mid(CONF_COLOR(btn.std, 1), CONF_COLOR(btn.active, 1), 6-step, step);
            painter->setPen(QPen(c, dpi.f4, Qt::SolidLine, Qt::RoundCap));
            painter->drawPoint(handle.center());
            painter->restore();
//             painter->drawPixmap(xy, lights.slider[direction]);
//             SAVE_PEN;
//             painter->setPen(Colors::btnFg(PAL, isEnabled, hover || hasFocus, step));
//             painter->drawPoint(handle.center());
//             RESTORE_PEN;
         }
      }
      break;
   case CC_ToolButton: // A tool button, like QToolButton
      // special handling for the tabbar scrollers ----------------------------------
      if (widget && widget->parentWidget() &&
          qobject_cast<QTabBar*>(widget->parent())) {
         QColor c = widget->parentWidget()->palette().color(config.tab.std_role[0]);
         QColor c2 = widget->parentWidget()->palette().color(config.tab.std_role[1]);
         if (sunken) {
            int dy = (RECT.height()-RECT.width())/2;
            QRect r = RECT.adjusted(dpi.f2,dy,-dpi.f2,-dy);
            painter->drawTiledPixmap(r, Gradients::pix(c, r.height(), Qt::Vertical, Gradients::Sunken));
         }
         painter->save();
         painter->setPen( isEnabled ? c2 : Colors::mid(c, c2) );
         drawControl(CE_ToolButtonLabel, option, painter, widget);
         painter->restore();
         break;
      }
      // --------------------------------------------------------------------
      
      if (const QStyleOptionToolButton *toolbutton
          = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
         QRect menuarea = subControlRect(control, toolbutton, SC_ToolButtonMenu, widget);
         QRect button = subControlRect(control, toolbutton, SC_ToolButton, widget);
         State bflags = toolbutton->state;
             
         if ((bflags & State_AutoRaise) && !hover)
            bflags &= ~State_Raised;
         
         State mflags = bflags;

         if (toolbutton->activeSubControls & SC_ToolButton)
            bflags |= State_Sunken;
         
         hover = isEnabled && (bflags & (State_Sunken | State_On |
                                         State_Raised | State_HasFocus));

         QStyleOption tool(0); tool.palette = toolbutton->palette;
         
         // frame around whole button
         /*if (hover)*/ {
            tool.rect = RECT; tool.state = bflags;
            drawPrimitive(PE_PanelButtonTool, &tool, painter, widget);
         }
         
         // don't paint a dropdown arrow iff the button's really pressed
         if (!(bflags & State_Sunken) &&
             (toolbutton->subControls & SC_ToolButtonMenu)) {
            if (toolbutton->activeSubControls & SC_ToolButtonMenu)
               painter->drawTiledPixmap(menuarea, Gradients::pix(FCOLOR(Window),
                                        menuarea.height(), Qt::Vertical,
                                        Gradients::Sunken));
            QPen oldPen = painter->pen();
            painter->setPen(Colors::mid(FCOLOR(Window), FCOLOR(WindowText), 2, 1));
            tool.rect = menuarea; tool.state = mflags;
            drawPrimitive(PE_IndicatorButtonDropDown, &tool, painter, widget);
            painter->setPen(oldPen);
            if (hover) {
               menuarea.setLeft(button.right()-shadows.line[1][Sunken].thickness()/2);
               shadows.line[1][Sunken].render(menuarea, painter);
            }
         }
         
         // label in the toolbutton area
         QStyleOptionToolButton label = *toolbutton;
         label.rect = button;
         drawControl(CE_ToolButtonLabel, &label, painter, widget);
      }
      break;
   case CC_TitleBar: // A Title bar, like what is used in Q3Workspace
      if (const QStyleOptionTitleBar *tb =
          qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
         painter->fillRect(RECT, PAL.brush(QPalette::Window));
         QRect ir;
         
         // the label
         if (option->subControls & SC_TitleBarLabel) {
            ir = subControlRect(CC_TitleBar, tb, SC_TitleBarLabel, widget);
            painter->setPen(PAL.color(QPalette::WindowText));
            ir.adjust(dpi.f2, 0, -dpi.f2, 0);
            painter->drawText(ir, Qt::AlignCenter | Qt::TextSingleLine, tb->text);
            
            // button grounds
            QRect gr = RECT;//.adjusted(0,dpi.f2, 0, -dpi.f2);
            gr.setRight(ir.left() - dpi.f4);
            const QPixmap &groove = Gradients::pix(FCOLOR(Window), gr.height(),
                  Qt::Vertical, Gradients::Sunken);
            Tile::setShape(Tile::Full &~ Tile::Left);
            masks.tab.render(gr, painter, groove);
            gr.setRight(RECT.right());
            gr.setLeft(ir.right() + dpi.f4);
            Tile::setShape(Tile::Full &~ Tile::Right);
            masks.tab.render(gr, painter, groove);
            Tile::reset();
         }
         
#define PAINT_WINDOW_BUTTON(_btn_) {\
            tmpOpt.rect = subControlRect(CC_TitleBar, tb, SC_TitleBar##_btn_##Button, widget);\
            if (!tmpOpt.rect.isNull()) { \
               if (tb->activeSubControls & SC_TitleBar##_btn_##Button)\
                  tmpOpt.state = tb->state;\
               else\
                  tmpOpt.state &= ~(State_Sunken | State_MouseOver);\
               pm = standardPixmap ( SP_TitleBar##_btn_##Button, &tmpOpt, widget );\
               painter->drawPixmap(tmpOpt.rect.topLeft(), pm);\
            }\
         }
         
         QPixmap pm;
         QStyleOptionTitleBar tmpOpt = *tb;
         if (tb->subControls & SC_TitleBarCloseButton)
            PAINT_WINDOW_BUTTON(Close)
         
         if (tb->subControls & SC_TitleBarMaxButton
             && tb->titleBarFlags & Qt::WindowMaximizeButtonHint) {
            if (tb->titleBarState & Qt::WindowMaximized)
               PAINT_WINDOW_BUTTON(Normal)
            else
               PAINT_WINDOW_BUTTON(Max)
         }
         
         if (tb->subControls & SC_TitleBarMinButton
             && tb->titleBarFlags & Qt::WindowMinimizeButtonHint) {
            if (tb->titleBarState & Qt::WindowMinimized)
               PAINT_WINDOW_BUTTON(Normal)
            else
               PAINT_WINDOW_BUTTON(Min)
         }
         
         if (tb->subControls & SC_TitleBarNormalButton &&
             tb->titleBarFlags & Qt::WindowMinMaxButtonsHint)
            PAINT_WINDOW_BUTTON(Normal)
         
         if (tb->subControls & SC_TitleBarShadeButton)
            PAINT_WINDOW_BUTTON(Shade)
         
         if (tb->subControls & SC_TitleBarUnshadeButton)
            PAINT_WINDOW_BUTTON(Unshade)
         
         if (tb->subControls & SC_TitleBarContextHelpButton
             && tb->titleBarFlags & Qt::WindowContextHelpButtonHint)
            PAINT_WINDOW_BUTTON(ContextHelp)
         
         if (tb->subControls & SC_TitleBarSysMenu
             && tb->titleBarFlags & Qt::WindowSystemMenuHint) {
            if (!tb->icon.isNull()) {
               ir = subControlRect(CC_TitleBar, tb, SC_TitleBarSysMenu, widget);
               tb->icon.paint(painter, ir);
            }
//             else
//                PAINT_WINDOW_BUTTON(SC_TitleBarSysMenu, SP_TitleBarMenuButton)
         }
      }
      break;
//    case CC_Q3ListView: // Used for drawing the Q3ListView class
   case CC_Dial: // A dial, like QDial
      if (const QStyleOptionSlider *dial =
          qstyleoption_cast<const QStyleOptionSlider *>(option)) {
         painter->save();
         QRect rect = RECT;
         if (rect.width() > rect.height()) {
            rect.setLeft(rect.x()+(rect.width()-rect.height())/2); rect.setWidth(rect.height());
         }
         else {
            rect.setTop(rect.y()+(rect.height()-rect.width())/2); rect.setHeight(rect.width());
         }
         
         int d = qMax(rect.width()/6, dpi.f10);
         int r = (rect.width()-d)/2;
         qreal a;
         if (dial->maximum == dial->minimum)
            a = M_PI / 2;
         else if (dial->dialWrapping)
            a = M_PI * 3 / 2 - (dial->sliderValue - dial->minimum) * 2 * M_PI
            / (dial->maximum - dial->minimum);
         else
            a = (M_PI * 8 - (dial->sliderValue - dial->minimum) * 10 * M_PI
                  / (dial->maximum - dial->minimum)) / 6;
         
         QPoint cp((int)(r * cos(a)), -(int)(r * sin(a)));
         cp += rect.center();
         
         // the huge ring
         r = d/2; rect.adjust(r,r,-r,-r);
         painter->setPen(FCOLOR(Window).dark(115));
         painter->setRenderHint( QPainter::Antialiasing );
         painter->drawEllipse(rect);
         rect.translate(0, 1);
         painter->setPen(FCOLOR(Window).light(108));
         painter->drawEllipse(rect);
         // the value
         QFont fnt = painter->font();
         fnt.setPixelSize( rect.height()/3 );
         painter->setFont(fnt);
         painter->setBrush(Qt::NoBrush);
         painter->setPen(Colors::mid(PAL.background().color(), PAL.foreground().color()));
         drawItemText(painter, rect,  Qt::AlignCenter, PAL, isEnabled,
                      QString::number(dial->sliderValue));
         // the drop
         painter->setPen(Qt::NoPen);
         rect = QRect(0,0,d,d);
         rect.moveCenter(cp);
         painter->setBrush(QColor(0,0,0,50));
         painter->drawEllipse(rect);
         rect.adjust(dpi.f2,dpi.f1,-dpi.f2,-dpi.f2);
         painter->setPen(QPen(CONF_COLOR(btn.std, 0), dpi.f2));
         painter->setBrushOrigin(rect.topLeft());
         const QColor c = hover ? CONF_COLOR(btn.active, 0) : hasFocus ?
                FCOLOR(Highlight) : CONF_COLOR(btn.std, 0);
         const QPixmap &fill =
                Gradients::pix(c, rect.height(), Qt::Vertical, GRAD(btn));
         painter->setBrush(fill);
         painter->drawEllipse(rect);
         painter->restore();
      }
      break;
   default:
      QCommonStyle::drawComplexControl( control, option, painter, widget );
   } // switch
}
