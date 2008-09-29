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

#ifdef QT3_SUPPORT
#include <Qt3Support/Q3ScrollView>
#endif
#include <QAbstractScrollArea>
#include "draw.h"
#include "animator/hover.h"
#include "animator/hovercomplex.h"

inline static bool
scrollAreaHovered(const QWidget* slider)
{
//     bool scrollerActive = false;
    if (!slider) return true;
    QWidget *scrollWidget = const_cast<QWidget*>(slider);
    if (!scrollWidget->isEnabled())
        return false;
    while (scrollWidget && !(qobject_cast<QAbstractScrollArea*>(scrollWidget) ||
#ifdef QT3_SUPPORT
                                qobject_cast<Q3ScrollView*>(scrollWidget) ||
#endif
                                Animator::Hover::managesArea(scrollWidget)))
        scrollWidget = const_cast<QWidget*>(scrollWidget->parentWidget());
    bool isActive = true;
    if (scrollWidget)
    {
        if (!scrollWidget->underMouse())
            return false;
//         QAbstractScrollArea* scrollWidget = (QAbstractScrollArea*)daddy;
        QPoint tl = scrollWidget->mapToGlobal(QPoint(0,0));
        QRegion scrollArea(tl.x(), tl.y(), scrollWidget->width(), scrollWidget->height());
        QList<QAbstractScrollArea*> scrollChilds = scrollWidget->findChildren<QAbstractScrollArea*>();
        for (int i = 0; i < scrollChilds.size(); ++i)
        {
            QPoint tl = scrollChilds[i]->mapToGlobal(QPoint(0,0));
            scrollArea -= QRegion(tl.x(), tl.y(), scrollChilds[i]->width(), scrollChilds[i]->height());
        }
#ifdef QT3_SUPPORT
        QList<Q3ScrollView*> scrollChilds2 = scrollWidget->findChildren<Q3ScrollView*>();
        for (int i = 0; i < scrollChilds2.size(); ++i)
        {
            QPoint tl = scrollChilds2[i]->mapToGlobal(QPoint(0,0));
            scrollArea -= QRegion(tl.x(), tl.y(), scrollChilds2[i]->width(), scrollChilds2[i]->height());
        }
#endif
//         scrollerActive = scrollArea.contains(QCursor::pos());
        isActive = scrollArea.contains(QCursor::pos());
    }
    return isActive;
}

#define PAINT_ELEMENT(_E_)\
if (scrollbar->subControls & SC_ScrollBar##_E_)\
{\
    optCopy.rect = scrollbar->rect;\
    optCopy.state = saveFlags;\
    optCopy.rect = subControlRect(CC_ScrollBar, &optCopy, SC_ScrollBar##_E_, widget);\
    if (optCopy.rect.isValid())\
    {\
        if (!(scrollbar->activeSubControls & SC_ScrollBar##_E_))\
            optCopy.state &= ~(State_Sunken | State_MouseOver);\
        if (info && (info->fades[Animator::In] & SC_ScrollBar##_E_ ||\
                    info->fades[Animator::Out] & SC_ScrollBar##_E_))\
            complexStep = info->step(SC_ScrollBar##_E_);\
        else \
            complexStep = 0; \
        drawScrollBar##_E_(&optCopy, cPainter, widget);\
    }\
}//

static bool isComboDropDownSlider, scrollAreaHovered_;
static int complexStep, widgetStep;
static const bool round_ = true;

static QPixmap *scrollBgCache = 0;
const static QWidget *cachedScroller = 0;
static QPainter *cPainter = 0;

void
BespinStyle::drawScrollBar(const QStyleOptionComplex * option,
                           QPainter * painter, const QWidget * widget) const
{

    ASSURE_OPTION(scrollbar, Slider);

    cPainter = painter;
    bool useCache = false, needsPaint = true;

    // we paint the slider bg ourselves, as otherwise a frame repaint would be
    // triggered (for no sense)
    if (!widget ) // fallback ===========
        painter->fillRect(RECT, FCOLOR(Window));

    else if (widget->testAttribute(Qt::WA_OpaquePaintEvent))
    {   /// fake a transparent bg (real transparency leads to frame painting overhead)
        // i.e. we erase the bg with the window background or any autofilled element between
        
        if (widget->parentWidget() && widget->parentWidget()->parentWidget() &&
            widget->parentWidget()->parentWidget()->inherits("QComboBoxListView"))
        {   /// catch combobox dropdowns ==========
            painter->fillRect(RECT, PAL.brush(QPalette::Base));
            isComboDropDownSlider = true;
        }

        else
        {   /// default scrollbar ===============
            isComboDropDownSlider = false;

            if (option->state & State_Sunken)
            {   /// use caching for sliding scrollers to gain speed
                useCache = true;
                if (widget != cachedScroller)
                {   // update cache
                    cachedScroller = widget;
                    delete scrollBgCache; scrollBgCache = 0L;
                }
                if (!scrollBgCache || scrollBgCache->size() != RECT.size())
                {   // we need a new cache pixmap
                    delete scrollBgCache;
                    scrollBgCache = new QPixmap(RECT.size());
                    cPainter = new QPainter(scrollBgCache);
                }
                else
                    needsPaint = false;
            }
            if (needsPaint)
                erase(option, cPainter, widget);
        }
    }
    // =================

    //BEGIN real scrollbar painting                                                                -
   
    OPT_ENABLED

    // Make a copy here and reset it for each primitive.
    QStyleOptionSlider optCopy = *scrollbar;
    State saveFlags = optCopy.state;
    if (scrollbar->minimum == scrollbar->maximum)
        saveFlags &= ~State_Enabled; // there'd be nothing to scroll anyway...
        
    /// hover animation =================
    if (scrollbar->activeSubControls & SC_ScrollBarSlider)
        { widgetStep = 0; scrollAreaHovered_ = true; }
    else
    {
        widgetStep = Animator::Hover::step(widget);
        scrollAreaHovered_ = scrollAreaHovered(widget);
    }
    
    SubControls hoverControls = scrollbar->activeSubControls &
                                (SC_ScrollBarSubLine | SC_ScrollBarAddLine | SC_ScrollBarSlider);
    const Animator::ComplexInfo *info = Animator::HoverComplex::info(widget, hoverControls);
    /// ================

    QRect groove;
    if (needsPaint)
    {   // NOTICE the scrollbar bg is cached for sliding scrollers to gain speed
        // the cache also includes the groove
        if (config.scroll.groove != Groove::Sunken)
            { PAINT_ELEMENT(Groove); } // leave brackts, MACRO!
        groove = optCopy.rect;
    }
    else
        groove = subControlRect(CC_ScrollBar, &optCopy, SC_ScrollBarGroove, widget);

    if (cPainter != painter) // unwrap cache painter
        { cPainter->end(); delete cPainter; cPainter = painter; }
   
    /// Background and groove have been painted
    if (useCache)  //flush the cache
        painter->drawPixmap(RECT.topLeft(), *scrollBgCache);

    if (config.scroll.showButtons)
    {   // nasty useless "click-to-scroll-one-single-line" buttons
        PAINT_ELEMENT(SubLine);
        PAINT_ELEMENT(AddLine);
    }

    const bool grooveIsSunken = config.scroll.groove > Groove::Groove;

    if ((saveFlags & State_Enabled) && (scrollbar->subControls & SC_ScrollBarSlider))
    {
        optCopy.rect = scrollbar->rect;
        optCopy.state = saveFlags;
        optCopy.rect = subControlRect(CC_ScrollBar, &optCopy, SC_ScrollBarSlider, widget);
        if (grooveIsSunken)
            optCopy.rect.adjust(-F(1),-F(1),F(1),0);

        if (optCopy.rect.isValid())
        {
            if (!(scrollbar->activeSubControls & SC_ScrollBarSlider))
                optCopy.state &= ~(State_Sunken | State_MouseOver);

            if (scrollbar->state & State_HasFocus)
                optCopy.state |= (State_Sunken | State_MouseOver);

            if (info && (   (info->fades[Animator::In] & SC_ScrollBarSlider) ||
                            (info->fades[Animator::Out] & SC_ScrollBarSlider)   ))
                complexStep = info->step(SC_ScrollBarSlider);
            else
                complexStep = 0;

            drawScrollBarSlider(&optCopy, cPainter, widget);
        }
    }
   
    if (!isComboDropDownSlider && grooveIsSunken)
        shadows.sunken[round_][isEnabled].render(groove, painter);
}
#undef PAINT_ELEMENT

void
BespinStyle::drawScrollBarButton(const QStyleOption * option, QPainter * painter,
                                 const QWidget *, bool up) const
{
    ASSURE_OPTION(opt, Slider);

    if (isComboDropDownSlider)
    {   // gets a classic triangular look and is allways shown
        OPT_HOVER

        painter->save();
        painter->setPen(hover ? FCOLOR(Text) : Colors::mid(FCOLOR(Base), FCOLOR(Text)));
        const int dx = RECT.width()/4, dy = RECT.height()/4;
        QRect rect = RECT.adjusted(dx, dy, -dx, -dy);
        if (option->state & QStyle::State_Horizontal)
            drawSolidArrow(up ? Navi::W : Navi::E, rect, painter);
        else
            drawSolidArrow(up ? Navi::N : Navi::S, rect, painter);
        painter->restore();
    }

    if (!config.scroll.showButtons)
        return;

    OPT_SUNKEN OPT_ENABLED OPT_HOVER

    QRect r = RECT.adjusted(F(2),F(2),-F(2),-F(2));
    bool alive = isEnabled && // visually inactivate if an extreme position is reached
                ((up && opt->sliderValue > opt->minimum) || (!up && opt->sliderValue < opt->maximum));
    hover = hover && alive;
    const int step = (hover && !complexStep) ? 6 : complexStep;
    
    const QColor c = alive ? Colors::mid(CCOLOR(btn.std, 0), CCOLOR(btn.active, 0), 6-step, step) :
                             FCOLOR(Window);
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
BespinStyle::drawScrollBarGroove(const QStyleOption * option, QPainter * painter, const QWidget *) const
{
    const bool horizontal = option->state & QStyle::State_Horizontal;

    if (isComboDropDownSlider)
    {   // get's a special solid groove
        QRect r;
        if (horizontal)
        {
            const int d = 2*RECT.height()/5;
            r = RECT.adjusted(F(2), d, -F(2), -d);
        }
        else
        {
            const int d = 2*RECT.width()/5;
            r = RECT.adjusted(d, F(2), -d, -F(2));
        }
        painter->fillRect(r, Colors::mid(FCOLOR(Base), FCOLOR(Text), 10, 1));
        return;
    }
    const Groove::Mode gType = config.scroll.groove;
    QColor bg = Colors::mid(FCOLOR(Window), FCOLOR(WindowText), 1 + gType*gType*gType, 1);
    if (gType)
        masks.rect[true].render(RECT, painter, Gradients::Sunken,
                                horizontal ? Qt::Vertical : Qt::Horizontal, bg);
    else
    {
        SAVE_PEN;
        painter->setPen(QPen(bg, F(1)));
        QPoint c = RECT.center();
        if (option->state & QStyle::State_Horizontal)
            painter->drawLine(RECT.left(), c.y(), RECT.right(), c.y());
        else
            painter->drawLine(c.x(), RECT.top(), c.x(), RECT.bottom());
        RESTORE_PEN;
    }
    return;
}

void
BespinStyle::drawScrollBarSlider(const QStyleOption * option, QPainter * painter,
                                 const QWidget * widget) const
{
    OPT_SUNKEN OPT_ENABLED OPT_HOVER
    const bool horizontal = option->state & QStyle::State_Horizontal;

    if (isComboDropDownSlider)
    {   //gets a special slimmer and simpler look
        QRect r;
        if (horizontal)
        {
            const int d = RECT.height()/3;
            r = RECT.adjusted(dpi.f2, d, -dpi.f2, -d);
        }
        else
        {
            const int d = RECT.width()/3;
            r = RECT.adjusted(d, dpi.f2, -d, -dpi.f2);
        }
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setRenderHint(QPainter::Antialiasing);
        if (sunken || (hover && !complexStep))
            complexStep = 6;
        painter->setBrush(Colors::mid(FCOLOR(Base), FCOLOR(Text), 6-complexStep, complexStep+1));
        painter->drawRoundedRect(r, dpi.f4, dpi.f4);
        painter->restore();
        return;
    }

    if (!isEnabled)
    {   // fallback, only if the slider primitive is painted directly
        if (config.scroll.groove != Groove::Sunken)
            drawScrollBarGroove(option, painter, widget);
        return;
    }

    // --> we need to paint a slider

    /// COLOR: the hover indicator (inside area)
#define SCROLL_COLOR(_X_) btnBg(PAL, true, false, _X_, true, Gradients::isReflective(GRAD(scroll)))
    if (scrollAreaHovered_ && !widgetStep)
        widgetStep = 6;

    QColor c;
    if (sunken)
        c = SCROLL_COLOR(6);
    else if (complexStep)
    {
        c = Colors::mid(CCOLOR(btn.std, Bg), SCROLL_COLOR(widgetStep));
        c = Colors::mid(c, SCROLL_COLOR(complexStep),6-complexStep,complexStep);
    }
    else if (hover)
        { complexStep = 6; c = SCROLL_COLOR(6); }
    else if (widgetStep)
        c = Colors::mid(CCOLOR(btn.std, Bg), SCROLL_COLOR(widgetStep));
    else
        c = CCOLOR(btn.std, 0);
#undef SCROLL_COLOR
   
    QRect r = RECT;
    const int f1 = dpi.f1, f2 = dpi.f2;
    const bool grooveIsSunken = config.scroll.groove >= Groove::Sunken;

    /// draw shadow
    // clip away innper part of shadow - hey why paint invisible alpha stuff =D   --------
    bool hadClip = painter->hasClipping();
    QRegion oldClip;
    if (hadClip)
        oldClip = painter->clipRegion();
    painter->setClipping(true);
    if (horizontal)
        painter->setClipRegion(QRegion(RECT) - r.adjusted(F(9), F(3), -F(9), -F(3)));
    else
        painter->setClipRegion(QRegion(RECT) - r.adjusted(F(3), F(9), -F(3), -F(9)));
    // --------------
    if (sunken && !grooveIsSunken)
    {
        r.adjust(f1, f1, -f1, -f1);
        shadows.raised[round_][true][true].render(r, painter);
        r.adjust(f1, f1, -f1, -f2 );
    }
    else
    {
        if (!sunken && config.btn.backLightHover && complexStep)
        {
            QColor blh = Colors::mid(c, CCOLOR(btn.active, Bg), 6-complexStep, complexStep);
            lights.rect[round_].render(r, painter, blh); // backlight
        }
        shadows.raised[round_][true][false].render(r, painter);
        r.adjust(f2, f2, -f2, horizontal && grooveIsSunken ? -f2 : -F(3) );
    }
    // restore clip---------------
    if (hadClip)
        // sic! clippping e.g. in webkit seems to be broken? at least querky with size and pos twisted...
        painter->setClipRegion(RECT);
    painter->setClipping(hadClip);

    /// the always shown base
    Qt::Orientation o; int size; Tile::PosFlags pf;
    if (horizontal)
    {
        o = Qt::Vertical; size = r.height();
        pf = Tile::Top | Tile::Bottom;
    }
    else {
        o = Qt::Horizontal; size = r.width();
        pf = Tile::Left | Tile::Right;
    }

    const QColor &bc = config.btn.fullHover ? c : CCOLOR(btn.std, Bg);
    masks.rect[round_].render(r, painter, GRAD(scroll), o, bc, size);
    if (!sunken && Gradients::isReflective(GRAD(scroll)))
        masks.rect[round_].outline(r, painter, Colors::mid(bc,Qt::white,2,1));

    /// the hover indicator (in case...)
    if (config.btn.fullHover || !(hover || complexStep || widgetStep))
        return;

    int dw, dh;
    if (horizontal)
    {
        dw = r.width()/8; dh = r.height()/4;
    }
    else
    {
        dw = r.width()/4; dh = r.height()/8;
    }
    r.adjust(dw, dh, -dw, -dh);
    masks.rect[false].render(r, painter, GRAD(scroll), o, c, size, QPoint(dw,dh));
}

//    case CE_ScrollBarFirst: // Scroll bar first line indicator (i.e., home).
//    case CE_ScrollBarLast: // Scroll bar last line indicator (i.e., end).
