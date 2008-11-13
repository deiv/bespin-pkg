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

#include <QApplication>
#include <QAbstractButton>
#include <QStyleOptionButton>

#include "draw.h"
#include "animator/hover.h"

#include <QtDebug>

#define HOVER_STEP sunken ? 6 : ((appType == GTK || !widget) ? 6*hover : Animator::Hover::step(widget))

static int animStep = -1;
static bool isCheckbox = false;

void
Style::drawPushButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    ASSURE_OPTION(btn, Button);
    OPT_SUNKEN OPT_HOVER;

    QRect oldRect = btn->rect;
    QStyleOptionButton *_btn = const_cast<QStyleOptionButton*>(btn);
    if (btn->features & QStyleOptionButton::Flat)
    {   // more like a toolbtn
        if (option->state & State_Enabled)
        {
            animStep = HOVER_STEP;
            if (option->state & State_HasFocus)
                masks.rect[true].outline(RECT, painter, Colors::mid(FCOLOR(Window), FCOLOR(Highlight)), dpi.f3);
            if (sunken)
                shadows.sunken[true][true].render(RECT, painter);
            else
                shadows.relief[true][true].render(RECT, painter);
        }
    }
    else
    {
        if (sunken && !config.btn.cushion)
        {
            if (config.btn.layer == 1)
                _btn->rect.adjust(dpi.f1,dpi.f1,-dpi.f1,0);
            else if (!config.btn.layer)
                _btn->rect.adjust(0,dpi.f1,0,dpi.f1);
        }
        drawPushButtonBevel(btn, painter, widget);
    }
//    tmpBtn.rect = subElementRect(SE_PushButtonContents, btn, widget);
    if (appType == GTK)
        return; // GTK paints the label itself
    _btn->rect.adjust(dpi.f6,dpi.f4,-dpi.f6,-dpi.f4);
    drawPushButtonLabel(btn, painter, widget);
    _btn->rect = oldRect;
}

void
Style::drawPushButtonBevel(const QStyleOption * option,
                                 QPainter * painter,
                                 const QWidget * widget) const
{
    ASSURE_OPTION(btn, Button);
    OPT_SUNKEN OPT_HOVER

    animStep = HOVER_STEP;

    drawButtonFrame(option, painter, widget);
    if (btn->features & QStyleOptionButton::HasMenu)
    {
        int sz = (RECT.height()-dpi.f6)/2;
        QRect rect = RECT;
        rect.setLeft(RECT.right() - (dpi.f10+sz));
        shadows.line[1][Plain].render(rect, painter);
        rect.setLeft(rect.left() + dpi.f4);
        rect.setWidth(sz);
        painter->save();
        const QColor c = Colors::mid(Colors::mid(CCOLOR(btn.std, Bg), CCOLOR(btn.std, Fg)),
                                     Colors::mid(CCOLOR(btn.active, Bg), CCOLOR(btn.active, Fg)),
                                     6-animStep, animStep);
        painter->setPen(c); painter->setBrush(c);
        drawArrow(Navi::S, rect, painter);
        painter->restore();
    }
    // toggle indicator
    ASSURE(widget);
    ASSURE_WIDGET(b, QAbstractButton);

    if (b->isCheckable())
    {
        QRect r = RECT;
        const int h = r.height()/3;
        r.setTop(r.top() + h);
        r.setLeft(r.x() + dpi.f10);
        r.setWidth(h); r.setHeight(h);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        const QColor c = config.btn.backLightHover ? CCOLOR(btn.std, Fg) :
                        Colors::mid(CCOLOR(btn.std, Fg), CCOLOR(btn.active, Fg), 6-animStep, animStep);
        if (option->state & State_On)
        {
            const QPixmap &fill = Gradients::pix(c, r.height(), Qt::Vertical, GRAD(btn));
            painter->setBrush(fill);
            painter->setBrushOrigin(r.topLeft());
        }
        else
            painter->setBrush(Qt::NoBrush);
        painter->setPen(c);
        painter->drawEllipse(r);
        painter->restore();
    }
}
#include <QtDebug>
void
Style::drawButtonFrame(const QStyleOption * option,
                             QPainter * painter, const QWidget * widget) const
{
    const int f1 = F(1), f2 = F(2);
    B_STATES

    const QAbstractButton* btn = qobject_cast<const QAbstractButton*>(widget);
    if (widget && !btn && widget->inherits("QAbstractItemView"))
        { hover = false; animStep = 0; }
    else if (animStep < 0)
        animStep = HOVER_STEP;
        
//    const bool toggled = !hover && (option->state & State_On);
    const bool round = !isCheckbox && (config.btn.round || (btn && btn->isCheckable()));
    const bool fullHover = config.btn.fullHover ||
                            (isCheckbox && (config.btn.layer || config.btn.checkType == Check::O));

    int iOff[4] = {0,0,0,0};
    QRect r = RECT;

    Gradients::Type gt = GRAD(btn);

    QColor c = btnBg(PAL, isEnabled, hasFocus, animStep, fullHover, Gradients::isReflective(gt));
    QColor iC = CCOLOR(btn.std, Bg);

    bool drawInner = false;
    if (config.btn.cushion && sunken)
    {
        gt = Gradients::Sunken;
        drawInner = true;
        iC = CCOLOR(btn.active, Bg);
    }
    else if (animStep)
    {
        if (!fullHover)
            drawInner = true;

        if (drawInner || config.btn.backLightHover)
            iC = Colors::mid(c, CCOLOR(btn.active, Bg), 6-animStep, animStep);

      // gtk HATES color inversion on labels, so we invert the nonlabled part...
//       if (appType == GTK && !isCheckbox &&
//           !Colors::haveContrast(FCOLOR(WindowText), CCOLOR(btn.active, Bg))) {
//          QColor h = c; c = iC; iC = h;
//       }
    }

    if (sunken)
        hasFocus = false; // no frame add on trigger - looks nasty

    // sunken variant
    if (config.btn.layer)
    {
        sunken = (sunken && !config.btn.cushion) || config.btn.layer == 2;
        if (isEnabled)
        {
            if (sunken)
                r.setBottom(r.bottom()-f2);
            else
                r.adjust(0, f1,0,-f2);
            masks.rect[round].render(r, painter, gt, Qt::Vertical, c);
            if (drawInner)
            {
                iOff[0] = iOff[2] = F(3);
                iOff[1] = iOff[3] = sunken ? F(3) : F(2);
            }
            if (animStep && !hasFocus && config.btn.backLightHover)
            {   // we MUST use alpha blending as this crosses between button and bg
                QColor c2 = CCOLOR(btn.active, Bg); c2.setAlpha(c2.alpha()*animStep/10);
                masks.rect[round].outline(RECT, painter, c2, F(3));
            }
//             if (hasFocus) {
//                 QColor fc = fullHover ? iC : CCOLOR(btn.std, Bg);
//                 const int contrast = Colors::contrast(fc, FCOLOR(Highlight));
//                 r = RECT;
//                 if (sunken) r.setBottom(r.bottom()-f1);
//                 masks.rect[round].outline(r, painter, Colors::mid(fc, FCOLOR(Highlight), contrast/10, 1), dpi.f3);
//             }
        }
        if (sunken)
            shadows.sunken[round][isEnabled].render(RECT, painter);
        else
            shadows.relief[round][isEnabled].render(RECT, painter);
    }
    else
    {   // normal buttons ---------------
        if (drawInner)
        {
            iOff[0] = iOff[2] = isCheckbox ? F(2) : F(3);
            iOff[1] = iOff[3] = F(2);
        }

        if (hasFocus)
        {   // focus?
            if (!config.btn.cushion && sunken)
                r.setBottom(r.bottom()-dpi.f1);
            const int contrast = Colors::contrast(FCOLOR(Window), FCOLOR(Highlight));
            QColor fc = (config.btn.backLightHover && animStep) ? iC : FCOLOR(Window);
            fc = Colors::mid(fc, FCOLOR(Highlight), contrast/20, 1);
            lights.rect[round].render(r, painter, fc);
            r = RECT;
        }
        else if (config.btn.backLightHover && animStep)
            lights.rect[round].render(RECT, painter, iC); // backlight

        if (sunken && !config.btn.cushion)
        {   // shadow
            r.adjust(f1, f1, -f1, -f2);
            shadows.raised[round][isEnabled][true].render(r, painter);
            r.adjust(f1, f1, -f1, -f1);
        }
        else
        {
            r.adjust(0, f1, 0, 0);
            shadows.raised[round][isEnabled][false].render(r, painter);
            r.adjust(f2, f1, -f2, -dpi.f3);
        }

        // plate
        masks.rect[round].render(r, painter, GRAD(btn), Qt::Vertical, c);

        // outline
        int g = qGray(c.rgb());
        if (g > 128 || Gradients::isReflective(GRAD(btn)))
            masks.rect[round].outline(r.adjusted(f1,f1,-f1,-f1), painter, Colors::mid(c, Qt::white, (isEnabled ? 6:9) * (255-g), 255), f1);

    }
    
    if (isEnabled)
    {
        if (drawInner)
            masks.rect[round].render(r.adjusted(iOff[0], iOff[1], -iOff[2], -iOff[3]), painter,
                                     gt, Qt::Vertical, config.btn.backLightHover ? c : iC,
                                     r.height(), QPoint(0, iOff[1]));

        if (config.btn.ambientLight && !(sunken || isCheckbox)) // do we want ambient lights?
            painter->drawPixmap(QPoint(r.right()+1-16*r.height()/9, r.top()), Gradients::ambient(r.height()));

        if (config.btn.bevelEnds && !isCheckbox)
        {   // and a bevel in case (e.g. for Aqua look)
            QRect bevelRect = r; bevelRect.setWidth(dpi.f8);
            masks.rect[round].render(bevelRect, painter, Gradients::bevel());
            bevelRect.moveTopRight(r.topRight());
            masks.rect[round].render(bevelRect, painter, Gradients::bevel(false));
        }
    }
}

void
Style::drawPushButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget*) const
{
    OPT_ENABLED OPT_FOCUS OPT_HOVER;
    ASSURE_OPTION(btn, Button);

    QRect ir = btn->rect;
    uint tf = Qt::AlignCenter | BESPIN_MNEMONIC;

    if (!btn->icon.isNull())
    {   // The ICON ================================================
        QIcon::Mode mode = isEnabled ? QIcon::Normal : QIcon::Disabled;
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
            point.rx() += pixw/2;

        painter->drawPixmap(visualPos(btn->direction, btn->rect, point), pixmap);

        if (btn->direction == Qt::LeftToRight)
            ir.setLeft(ir.left() + pixw + dpi.f4);
        else
            ir.setWidth(ir.width() - (pixw + dpi.f4));
    }

    if (btn->text.isEmpty())
        return;

    // The TEXT ============================================
    const bool flat = btn->features & QStyleOptionButton::Flat;
    const QColor fg = btnFg(PAL, isEnabled, hasFocus, animStep, flat);
    if (config.btn.backLightHover)
        { hover = 0; animStep = 0; }

    const QColor &bg = flat ? FCOLOR(Window) : (hover ? CCOLOR(btn.active, Bg) : CCOLOR(btn.std, Bg));

    painter->save();
    if (hasFocus || (btn->features & QStyleOptionButton::DefaultButton))
    {
        QFont tmpFnt = painter->font();
        tmpFnt.setBold(true);
        QFontMetrics fm(tmpFnt);
        QRect tr = fm.boundingRect ( btn->text );
        if (!tr.isValid())
            { painter->restore(); return; }
        while (tr.width() > ir.width())
        {
            if (tmpFnt.pointSize() > -1)
                tmpFnt.setPointSize(tmpFnt.pointSize()*ir.width()/tr.width());
            else
                tmpFnt.setPixelSize(tmpFnt.pixelSize()*ir.width()/tr.width());
            fm = QFontMetrics(tmpFnt);
            tr = fm.boundingRect ( btn->text );
        }
        painter->setFont(tmpFnt);
    }

    if (!flat && isEnabled)
    {
        painter->setPen(bg.dark(120));
        ir.translate(0,-1);
        drawItemText(painter, ir, tf, PAL, isEnabled, btn->text);
        ir.translate(0,1);
    }

    painter->setPen(fg);

    drawItemText(painter, ir, tf, PAL, isEnabled, btn->text);
    painter->restore();
    animStep = -1;
}

void
Style::drawCheckBox(const QStyleOption * option, QPainter * painter,
                          const QWidget * widget) const
{
    B_STATES

    // the button -----------------
    QStyleOption copy = *option;
    if (config.btn.layer == 1)
        copy.rect.adjust(0,dpi.f1,0,-dpi.f1); // get rect appereance again
    else if (config.btn.layer == 0)
        copy.rect.adjust(dpi.f1,dpi.f1,-dpi.f1,0); // get rect appereance again
    isCheckbox = true;
    drawButtonFrame(&copy, painter, widget);
    isCheckbox = false;

    if (!(sunken || (option->state & State_Off)))
    {   // the checkmark -----------------
        painter->save();
        if (config.btn.backLightHover)
            { hover = 0; animStep = 0; }
        QPoint center = copy.rect.center();
        if (config.btn.checkType == Check::V && (option->state & State_On))
            center += QPoint(F(2), -F(2));
        else
            center += QPoint(0, -F(1));
        painter->setBrush(btnFg(PAL, isEnabled, hasFocus, animStep));
        const int d = dpi.f5 - (bool(config.btn.checkType) + config.btn.layer) * dpi.f1;
        copy.rect.adjust(d, d, -d, -d);
        if (copy.rect.width() > copy.rect.height())
            copy.rect.setWidth(copy.rect.height());
        else
            copy.rect.setHeight(copy.rect.width());
        copy.rect.moveCenter(center);
        drawCheckMark(&copy, painter, config.btn.checkType);
        painter->restore();
    }
    animStep = -1;
}

void
Style::drawRadio(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    B_STATES

    const int f1 = F(1);
    bool isOn = option->state & State_On;
    if (isOn)
        hover = false;
   
//       else if (hover && sunken) isOn = true;

#if 0
	QPoint xy = RECT.topLeft();
	Gradients::Type gt = isEnabled ? GRAD(btn) : Gradients::None;
	animStep = isOn ? 0 : HOVER_STEP;
   QColor bc = btnBg(PAL, isEnabled, hasFocus, 0,
                     false, Gradients::isReflective(gt));
   QColor c = bc;
	
   if (animStep)
      c = Colors::mid(c, CCOLOR(btn.active, Bg), 6-animStep, animStep);

   if (config.btn.layer == 2) { // sunken ==================
      QRect r = RECT.adjusted(dpi.f1,0,-dpi.f1,-f2);
      masks.rect[true].render(r, painter, sunken || isOn ?
                              Gradients::Sunken : gt, Qt::Vertical, c);
      r.setBottom(RECT.bottom());
      shadows.sunken[true][isEnabled].render(r, painter);
      xy += QPoint(f1, 0);

   } else if (config.btn.layer) { // embedded ==================
      
      QRect r = RECT.adjusted(2,2,-2,-2);
      painter->save();
      painter->setRenderHint(QPainter::Antialiasing);
      painter->setBrushOrigin(r.topLeft());
      painter->setBrush(Gradients::brush( c, r.height(), Qt::Vertical, gt));
      if (!sunken && hasFocus)
         painter->setPen(Colors::mid(FCOLOR(Window), FCOLOR(Highlight), 24-animStep, animStep));
      else
         painter->setPen(Qt::NoPen);
      painter->drawEllipse(r);
      painter->setBrush(Qt::NoBrush); r.adjust(-1,-1,1,1);
      painter->setPen(QColor(0,0,0,70)); painter->drawEllipse(r);
      r.translate(0,1);
      painter->setPen(QColor(255,255,255,90)); painter->drawEllipse(r);
      painter->restore();
      xy += QPoint(config.btn.layer*f1,f1);

   } else { //raised ==================

      sunken = sunken || isOn;

      painter->save();
      if (hasFocus || (animStep && config.btn.backLightHover)) {
         if (hasFocus) {
            const int contrast = Colors::contrast(FCOLOR(Window), FCOLOR(Highlight));
            painter->setBrush(Colors::mid(FCOLOR(Window), FCOLOR(Highlight), contrast/5, 1));
         }
         else
            painter->setBrush(Colors::mid(FCOLOR(Window), CCOLOR(btn.active, Bg),
                                          40/animStep, animStep));
         painter->setPen(Qt::NoPen);
         painter->setRenderHint(QPainter::Antialiasing);
         QRect r = RECT; if (sunken) r.setBottom(r.bottom()-f1);
         painter->drawEllipse(r);
      }
      
      // shadow
      painter->drawPixmap(sunken ? xy + QPoint(f1,f1) : xy,
                          shadows.radio[isEnabled][sunken]);
      
      // plate
      xy += QPoint(f2,f1); const int sz = dpi.ExclusiveIndicator - f4;
      if (config.btn.fullHover && !config.btn.backLightHover)
         fillWithMask(painter, xy, Gradients::brush(c, sz, Qt::Vertical, gt), masks.radio);
      else {
         fillWithMask(painter, xy, Gradients::brush(bc, sz, Qt::Vertical, gt), masks.radio);
         if (animStep) {
            fillWithMask(painter, xy + QPoint(f4, f4),
                         Gradients::brush(c, sz, Qt::Vertical, gt),
                         masks.radioIndicator, QPoint(0, dpi.f4));
         }
      }
      if (isEnabled && (Gradients::isReflective(GRAD(btn)) || qGray(c.rgb()) > 128)) {
         painter->setPen(QPen(Colors::mid(config.btn.fullHover ? c : bc,
                                          Qt::white),f1));
         painter->setBrush(Qt::NoBrush);
         painter->setRenderHint(QPainter::Antialiasing);
         QRect r(xy, QSize(sz, sz));
         r.adjust(f1,f1,-f1,-f1); painter->drawEllipse(r);
      }
      painter->restore();
   }
	// drop
   if (isOn) {
      xy += QPoint(f4, f4);
      fillWithMask(painter, xy, btnFg(PAL, isEnabled, hasFocus, animStep),
                   masks.radioIndicator);
   }
#else
//     painter->fillRect(RECT, Qt::red);
    QRect r = RECT.adjusted(f1,f1,-f1,-f1);
    QColor bg = isEnabled ? CCOLOR(btn.std, Bg) : FCOLOR(Window);
    if (hasFocus)
    {
        const int contrast = Colors::contrast(bg, FCOLOR(Highlight));
        if (contrast > 10) {
            masks.rect[true].outline(RECT, painter, Colors::mid(FCOLOR(Window), FCOLOR(Highlight)), F(3));
            bg = Colors::mid(bg, FCOLOR(Highlight), contrast/5, 1);
        }
    }
    masks.rect[true].render(r, painter, GRAD(chooser), Qt::Vertical, bg);

    r.setBottom(RECT.bottom());
    shadows.sunken[true][isEnabled].render(r, painter);

    animStep = isOn ? 12 : HOVER_STEP;
    if (animStep)
    {   // the drop ============================
        QColor c = Colors::mid(CCOLOR(btn.std, Bg), CCOLOR(btn.std, Fg), 12-animStep, animStep);
        const int off = dpi.ExclusiveIndicator/4;
        QPoint xy = r.topLeft() + QPoint(off, off);
//         const Gradients::Type gt = isEnabled ? GRAD(btn) : Gradients::None;
        fillWithMask(painter, xy, c, masks.radioIndicator);
    }

//     if (hasFocus)
//     {
//         r = RECT; r.setTop(r.top()+F(2));
//         masks.rect[true].outline(RECT, painter, Colors::mid(FCOLOR(Window), FCOLOR(Highlight)), F(3));
//     }
#endif
   animStep = -1;
}

//    case PE_FrameButtonBevel: // Panel frame for a button bevel

void
Style::drawRadioItem(const QStyleOption * option, QPainter * painter,
                           const QWidget * widget) const
{
   ASSURE_OPTION(btn, Button);
   QStyleOptionButton subopt = *btn;
   subopt.rect = subElementRect(SE_RadioButtonIndicator, btn, widget);
   drawRadio(&subopt, painter, widget);
   subopt.rect = subElementRect(SE_RadioButtonContents, btn, widget);
   drawCheckLabel(&subopt, painter, widget);
}

void
Style::drawCheckBoxItem(const QStyleOption * option, QPainter * painter,
                              const QWidget * widget) const
{
   ASSURE_OPTION(btn, Button);
   QStyleOptionButton subopt = *btn;
   subopt.rect = subElementRect(SE_CheckBoxIndicator, btn, widget);
   drawCheckBox(&subopt, painter, widget);
   subopt.rect = subElementRect(SE_CheckBoxContents, btn, widget);
   drawCheckLabel(&subopt, painter, widget);
}

void
Style::drawCheckLabel(const QStyleOption * option, QPainter * painter,
                            const QWidget * /*widget*/) const
{
   ASSURE_OPTION(btn, Button);
   OPT_ENABLED;
   
   uint alignment =
      visualAlignment(btn->direction, Qt::AlignLeft | Qt::AlignVCenter);

   QRect textRect = RECT;
   if (!btn->icon.isNull()) {
      const QPixmap pix =
         btn->icon.pixmap(btn->iconSize,
                          isEnabled ? QIcon::Normal : QIcon::Disabled);
      drawItemPixmap(painter, btn->rect, alignment, pix);
      if (btn->direction == Qt::RightToLeft)
         textRect.setRight(textRect.right() - btn->iconSize.width() - dpi.f4);
      else
         textRect.setLeft(textRect.left() + btn->iconSize.width() + dpi.f4);
   }
   if (!btn->text.isEmpty())
      drawItemText(painter, textRect, alignment | BESPIN_MNEMONIC, PAL, isEnabled, btn->text, QPalette::WindowText);
}
