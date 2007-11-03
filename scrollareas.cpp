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
#include "draw.h"

inline static bool
scrollAreaHovered(const QWidget* slider, StyleAnimator *animator)
{
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

#define PAINT_ELEMENT(_E_)\
if (scrollbar->subControls & SC_ScrollBar##_E_) {\
newScrollbar.rect = scrollbar->rect;\
newScrollbar.state = saveFlags;\
newScrollbar.rect =\
subControlRect(CC_ScrollBar, &newScrollbar, SC_ScrollBar##_E_, widget);\
if (newScrollbar.rect.isValid()) {\
if (!(scrollbar->activeSubControls & SC_ScrollBar##_E_))\
newScrollbar.state &= ~(State_Sunken | State_MouseOver);\
if (info && (info->fadeIns & SC_ScrollBar##_E_ ||\
info->fadeOuts & SC_ScrollBar##_E_))\
complexStep = info->step(SC_ScrollBar##_E_);\
else \
complexStep = 0; \
drawScrollBar##_E_(&newScrollbar, cPainter, widget);\
}\
}//

static bool isComboDropDownSlider, scrollAreaHovered_;
static int complexStep, widgetStep;

static QPixmap *scrollBgCache = 0;
const static QWidget *cachedScroller = 0;
static QPainter *cPainter = 0;

void
BespinStyle::drawScrollBar(const QStyleOptionComplex * option,
                           QPainter * painter, const QWidget * widget) const
{

   const QStyleOptionSlider *scrollbar =
      qstyleoption_cast<const QStyleOptionSlider *>(option);
   if (!scrollbar) return;

   cPainter = painter;
   bool flushCache = false, needsPaint = true;

   // we paint the slider bg ourselves, as otherwise a frame repaint would be
   // triggered (for no sense)
   if (!widget) // fallback ===========
      painter->fillRect(RECT, FCOLOR(Window));
   else {

      // catch combobox dropdowns ==========
      if (widget->parentWidget() && widget->parentWidget()->parentWidget() &&
          widget->parentWidget()->parentWidget()->inherits("QComboBoxListView")) {
         painter->fillRect(RECT, PAL.brush(QPalette::Base));
         isComboDropDownSlider = true;
      }
      else { // default scrollbar ===============
         isComboDropDownSlider = false;

         // opaque fill (default)
         if (widget->testAttribute(Qt::WA_OpaquePaintEvent)) {

            if (option->state & State_Sunken) { // use the caching
               flushCache = true;
               if (widget != cachedScroller) { // update cache
                  cachedScroller = widget;
                  if (!scrollBgCache ||
                      scrollBgCache->size() != RECT.size()) {
                     delete scrollBgCache;
                     scrollBgCache = new QPixmap(RECT.size());
                  }
                  cPainter = new QPainter(scrollBgCache);
               }
               else
                  needsPaint = false;
            }

            if (needsPaint) {
               // draw background
               const QWidget *grampa = widget;
               while (!(grampa->isWindow() || grampa->autoFillBackground()))
                  grampa = grampa->parentWidget();

               QPoint tl = widget->mapFrom(const_cast<QWidget*>(grampa), QPoint());
               cPainter->save();
               cPainter->setPen(Qt::NoPen);
               cPainter->setBrush(grampa->palette().brush(grampa->backgroundRole()));
               cPainter->setBrushOrigin(tl);
               cPainter->drawRect(RECT);
               if (grampa->isWindow())  { // means we need to paint the global bg as well
                  cPainter->setClipRect(RECT, Qt::IntersectClip);
                  QStyleOption tmpOpt = *option;
                  tmpOpt.rect = QRect(tl, grampa->size());
                  cPainter->fillRect(RECT, grampa->palette().brush(QPalette::Window));
                  drawWindowBg(&tmpOpt, cPainter, grampa);
               }
               cPainter->restore();
            }
         }
      }
   }
   // =================

   OPT_ENABLED

   // Make a copy here and reset it for each primitive.
   QStyleOptionSlider newScrollbar = *scrollbar;
   State saveFlags = newScrollbar.state;
   if (scrollbar->minimum == scrollbar->maximum)
      saveFlags &= ~State_Enabled;

   // hover animations =================
   if (scrollbar->activeSubControls & SC_ScrollBarSlider) {
      widgetStep = 0; scrollAreaHovered_ = true;
   }
   else {
      widgetStep = animator->hoverStep(widget);
      scrollAreaHovered_ = scrollAreaHovered(widget, animator);
   }

   SubControls hoverControls = scrollbar->activeSubControls &
      (SC_ScrollBarSubLine | SC_ScrollBarAddLine | SC_ScrollBarSlider);
   const ComplexHoverFadeInfo *info = animator->fadeInfo(widget, hoverControls);
   // =======================================

   QRect groove;
   if (needsPaint) {
      PAINT_ELEMENT(Groove);
      groove = newScrollbar.rect;
   }
   else
      groove =
      subControlRect(CC_ScrollBar, &newScrollbar, SC_ScrollBarGroove, widget);

   if (cPainter != painter) {
      cPainter->end(); delete cPainter; cPainter = painter;
   }

   // Background and groove have been painted - flush the cache (in case)
   if (flushCache)
      painter->drawPixmap(RECT.topLeft(), *scrollBgCache);
       
   if (config.scroll.showButtons) {
      PAINT_ELEMENT(SubLine);
      PAINT_ELEMENT(AddLine);
   }
       
   if (isEnabled && scrollbar->subControls & SC_ScrollBarSlider) {
      newScrollbar.rect = scrollbar->rect;
      newScrollbar.state = saveFlags;
      newScrollbar.rect = subControlRect(CC_ScrollBar, &newScrollbar,
                                       SC_ScrollBarSlider, widget);
      if (config.scroll.sunken) {
         const int f1 = dpi.f1;
         newScrollbar.rect.adjust(-f1,-f1,f1,0);
      }
      if (newScrollbar.rect.isValid()) {
         if (!(scrollbar->activeSubControls & SC_ScrollBarSlider))
            newScrollbar.state &= ~(State_Sunken | State_MouseOver);
         if (scrollbar->state & State_HasFocus)
            newScrollbar.state |= (State_Sunken | State_MouseOver);
         if (info && (info->fadeIns & SC_ScrollBarSlider ||
                     info->fadeOuts & SC_ScrollBarSlider))
            complexStep = info->step(SC_ScrollBarSlider);
         else
            complexStep = 0;
         drawScrollBarSlider(&newScrollbar, cPainter, widget);
      }
   }
   
   if (!isComboDropDownSlider && config.scroll.sunken)
      shadows.tabSunken.render(groove, painter);
}
#undef PAINT_ELEMENT

void
BespinStyle::drawScrollBarButton(const QStyleOption * option,
                                  QPainter * painter,
                                  const QWidget *, bool up) const
{
   const QStyleOptionSlider *opt =
      qstyleoption_cast<const QStyleOptionSlider *>(option);
   if (!opt) return;

   if (isComboDropDownSlider) {
      OPT_HOVER
         
      painter->save();
      painter->setPen(hover ? FCOLOR(Text) :
                        Colors::mid(FCOLOR(Base), FCOLOR(Text)));
      QRect rect = RECT.adjusted(RECT.width()/4, RECT.height()/4,
                                 -RECT.width()/4, -RECT.height()/4);
      if (option->state & QStyle::State_Horizontal)
         drawSolidArrow(up ? Navi::W : Navi::E, rect, painter);
      else
         drawSolidArrow(up ? Navi::N : Navi::S, rect, painter);
      painter->restore();
   }

   if (!config.scroll.showButtons)
      return;

   B_STATES

   QRect r = RECT.adjusted(dpi.f2,dpi.f2,-dpi.f2,-dpi.f2);
   bool alive = isEnabled && ((up && opt->sliderValue > opt->minimum) ||
                              (!up && opt->sliderValue < opt->maximum));
   hover = hover && alive;
   const int step = (hover && !complexStep) ? 6 : complexStep;
   const QColor c =
      alive ? Colors::mid(CCOLOR(btn.std, 0), CCOLOR(btn.active, 0),
                        6-step, step) : FCOLOR(Window);
   painter->save();
   painter->setRenderHint(QPainter::Antialiasing);
   if (alive)
      painter->setPen(CCOLOR(btn.std, 0).dark(120));
   else
      painter->setPen(Qt::NoPen);
   painter->setBrush(Gradients::pix(c, r.height(), Qt::Vertical,
                                    (sunken || !alive) ? Gradients::Sunken : Gradients::Button));
   painter->setBrushOrigin(r.topLeft());
   painter->drawEllipse(r);
   painter->restore();
}

void
BespinStyle::drawScrollBarGroove(const QStyleOption * option,
                                 QPainter * painter,
                                 const QWidget *) const
{
   if (isComboDropDownSlider) return;

   if (config.scroll.groove) {
      masks.tab.render(RECT, painter, Gradients::Sunken,
                       option->state & QStyle::State_Horizontal ?
                       Qt::Vertical : Qt::Horizontal, FCOLOR(Window));
      return;
   }

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
}

void
BespinStyle::drawScrollBarSlider(const QStyleOption * option,
                                 QPainter * painter,
                                 const QWidget * widget) const
{
   B_STATES
      
   if (isComboDropDownSlider) {
      painter->fillRect(RECT.adjusted(dpi.f2, 0, -dpi.f2, 0),
                        (hover || sunken) ? FCOLOR(Text) :
                        Colors::mid(FCOLOR(Base), FCOLOR(Text), 8, 1));
      return;
   }

   if (!isEnabled) {
      if (!config.scroll.sunken)
         drawScrollBarGroove(option, painter, widget);
      return;
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

   // shadow
   if (sunken) {
      r.adjust(f1, f1, -f1, -f1);
      shadows.tab[true][true].render(r, painter);
      r.adjust(f1, f1, -f1,
               (option->state & QStyle::State_Horizontal) &&
               config.scroll.sunken ? -f1 : -f2 );
   }
   else {
      shadows.tab[true][false].render(r, painter);
      r.adjust(f2, f2, -f2,
               (option->state & QStyle::State_Horizontal) &&
               config.scroll.sunken ? -f2 : -dpi.f3 );
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
   const QColor &bc = config.btn.fullHover ? c : CCOLOR(btn.std, Bg);
   masks.tab.render(r, painter, GRAD(scroll), o, bc, size);
   if (Gradients::isReflective(GRAD(scroll)))
      masks.tab.outline(r, painter, Colors::mid(bc,Qt::white,2,1));

   if (config.btn.fullHover) return;

   int dw, dh;
   if (o == Qt::Vertical) {
      dw = r.width()/8; dh = r.height()/4;
   }
   else {
      dw = r.width()/4; dh = r.height()/8;
   }
   r.adjust(dw, dh, -dw, -dh);
   masks.button.render(r, painter, GRAD(scroll), o, c, size, QPoint(dw,dh));
}

//    case CE_ScrollBarFirst: // Scroll bar first line indicator (i.e., home).
//    case CE_ScrollBarLast: // Scroll bar last line indicator (i.e., end).
