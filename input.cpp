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
#include <QComboBox>
#include "draw.h"
#include "animator/hover.h"

void
Style::drawLineEditFrame(const QStyleOption *option, QPainter *painter, const QWidget *) const
{
    // WARNING this is NOT used to draw lineedits - just the frame, see below!!
    OPT_ENABLED OPT_FOCUS

    QRect r = RECT;
    if (appType != GTK)
    {
        r.setBottom(r.bottom() - F(2));
        shadows.sunken[false][isEnabled].render(r, painter);
    }
    else
        shadows.fallback.render(RECT,painter);

    if (hasFocus)
    {
        QColor h = FCOLOR(Highlight); h.setAlpha(128);
        masks.rect[false].outline(r, painter, h, dpi.f3);
    }
}

void
Style::drawLineEdit(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    // spinboxes and combos allready have a lineedit as global frame
    // TODO: exclude Q3Combo??
    if (qstyleoption_cast<const QStyleOptionFrame *>(option) &&
        static_cast<const QStyleOptionFrame *>(option)->lineWidth < 1)
    {
        if (widget && widget->parentWidget() && ( qobject_cast<QComboBox*>(widget->parentWidget()) ||
            widget->parentWidget()->inherits("QAbstractSpinBox")))
            return;
        painter->fillRect(RECT, FCOLOR(Base));
        return;
    }
#if 0 // does not work for all plasma versions...
    if (appType == KRunner && widget && widget->inherits("KHistoryComboBox"))
        return;
#endif

    OPT_ENABLED OPT_FOCUS

    isEnabled = isEnabled && !(option->state & State_ReadOnly);
    QRect r = RECT;
    if (isEnabled)
    {
        const Tile::Set &mask = masks.rect[false];
        r.setBottom(r.bottom() - F(2));
        if (hasFocus)
        {
            mask.render(r, painter, FCOLOR(Base).light(112));
            r.setBottom(r.bottom() + F(1));
            QColor h = FCOLOR(Highlight); h.setAlpha(102);
//          Colors::mid(FCOLOR(Base), FCOLOR(Highlight), 3, 2);
            mask.outline(r, painter, h, F(3));
        }
        else
        {
//             r.setBottom(r.y() + r.height()/2);
            mask.render(r, painter, Gradients::Sunken, Qt::Vertical, FCOLOR(Base));
#if 0
            Tile::setShape(Tile::Full & ~Tile::Bottom);
            mask.render(r, painter, Gradients::Sunken, Qt::Vertical, FCOLOR(Base));
            r.setTop(r.bottom()); r.setBottom(RECT.bottom()-F(2));
            Tile::setShape(Tile::Full & ~Tile::Top);
            QColor bg = FCOLOR(Base);
            int h,s,v,a;
            bg.getHsv(&h, &s, &v, &a);
            if (v < 60) v = 60;
            v = (v * ( 100 + (250-v)/16 ) )/100;
            v = CLAMP(v,0,255);
            bg.setHsv(h,s,v,a);
            mask.render(r, painter, bg);
            Tile::reset();
#endif
        }
    }
    if (appType == GTK)
        shadows.fallback.render(RECT,painter);
    else
        shadows.sunken[false][isEnabled].render(RECT, painter);
}

static void
drawSBArrow(QStyle::SubControl sc, QPainter *painter, QStyleOptionSpinBox *option,
            const QWidget *widget, const QStyle *style)
{
    if (option->subControls & sc)
    {
        const int f2 = F(2);

        option->subControls = sc;
        RECT = style->subControlRect(QStyle::CC_SpinBox, option, sc, widget);

        Navi::Direction dir = Navi::N;
        QAbstractSpinBox::StepEnabledFlag sef = QAbstractSpinBox::StepUpEnabled;
        if (sc == QStyle::SC_SpinBoxUp)
            RECT.setTop(RECT.bottom() - 2*RECT.height()/3);
        else
        {
            dir = Navi::S; sef = QAbstractSpinBox::StepDownEnabled;
            RECT.setBottom(RECT.top() + 2*RECT.height()/3);
        }

        bool isEnabled = option->stepEnabled & sef;
        bool hover = isEnabled && (option->activeSubControls == (int)sc);
        bool sunken = hover && (option->state & QStyle::State_Sunken);
        

        if (!sunken)
        {
            painter->setBrush(FCOLOR(Base).dark(108));
            RECT.translate(0, f2);
            Style::drawArrow(dir, RECT, painter);
            RECT.translate(0, -f2);
        }

        QColor c;
        if (hover)
            c = FCOLOR(Highlight);
        else if (isEnabled)
            c = Colors::mid(FCOLOR(Base), FCOLOR(Text));
        else
            c = Colors::mid(FCOLOR(Base), PAL.color(QPalette::Disabled, QPalette::Text));

        painter->setBrush(c);
        Style::drawArrow(dir, RECT, painter);
    }
}

void
Style::drawSpinBox(const QStyleOptionComplex * option, QPainter * painter,
                         const QWidget * widget) const
{
    ASSURE_OPTION(sb, SpinBox);
    OPT_ENABLED

    QStyleOptionSpinBox copy = *sb;

   // this doesn't work (for the moment, i assume...)
    //    isEnabled = isEnabled && !(option->state & State_ReadOnly);
    if (isEnabled)
    if (const QAbstractSpinBox *box = qobject_cast<const QAbstractSpinBox*>(widget))
    {
        isEnabled = isEnabled && !box->isReadOnly();
        if (!isEnabled)
            copy.state &= ~State_Enabled;
    }

    if (sb->frame && (sb->subControls & SC_SpinBoxFrame))
        drawLineEdit(&copy, painter, widget);

    if (!isEnabled)
        return; // why bother the user with elements he can't use... ;)

    painter->setPen(Qt::NoPen);
    drawSBArrow(SC_SpinBoxUp, painter, &copy, widget, this);
    copy.rect = RECT;
    copy.subControls = sb->subControls;
    drawSBArrow(SC_SpinBoxDown, painter, &copy, widget, this);
}

static int animStep = -1;
static bool round_ = true;

void
Style::drawComboBox(const QStyleOptionComplex * option,
                          QPainter * painter, const QWidget * widget) const
{
    ASSURE_OPTION(cmb, ComboBox);
    B_STATES
    if ( widget && widget->inherits("WebView") ) widget = 0;

    const int f1 = F(1), f2 = F(2), f3 = F(3);
    QRect ar, r = RECT.adjusted(f1, f1, -f1, -f2);
    const QComboBox* combo = widget ? qobject_cast<const QComboBox*>(widget) : 0;
    QColor c = CONF_COLOR(btn.std, Bg);

    const bool listShown = combo && combo->view() && ((QWidget*)(combo->view()))->isVisible();
    if (listShown) // this messes up hover
        hover = hover ||
                QRect(widget->mapToGlobal(RECT.topLeft()), RECT.size()).contains(QCursor::pos());

    if (isEnabled && (cmb->subControls & SC_ComboBoxArrow) && (!combo || combo->count() > 0))
    {   // do we have an arrow?
        ar = subControlRect(CC_ComboBox, cmb, SC_ComboBoxArrow, widget);
        ar.setBottom(ar.bottom()-f2);
    }

    // the frame
    const Tile::Set &mask = masks.rect[round_];
    if ((cmb->subControls & SC_ComboBoxFrame) && cmb->frame)
    {
        if (cmb->editable)
            drawLineEdit(option, painter, widget);
        else
        {
            if (!ar.isNull())
            {
                // ground
                animStep = Animator::Hover::step(widget);
                if (listShown)
                    animStep = 6;

                const bool translucent = Gradients::isTranslucent(GRAD(chooser));
                c = btnBg(PAL, isEnabled, hasFocus, animStep, config.btn.fullHover, translucent);
                if (hasFocus)
                {
                    if (config.btn.backLightHover)
                        mask.outline(RECT, painter, Colors::mid(FCOLOR(Window), FCOLOR(Highlight)), f3);
                    else
                    {
                        const int contrast =  (config.btn.fullHover && animStep) ?
                        Colors::contrast(btnBg(PAL, isEnabled, hasFocus, 0, true, translucent), FCOLOR(Highlight)):
                        Colors::contrast(c, FCOLOR(Highlight));
                        if (contrast > 10)
                        {
                            mask.outline(RECT, painter, Colors::mid(FCOLOR(Window), FCOLOR(Highlight)), f3);
                            c = Colors::mid(c, FCOLOR(Highlight), contrast/4, 1);
                        }
                    }
                }

                mask.render(r, painter, GRAD(chooser), Qt::Vertical, c);

                if (animStep)
                {
                    if (!config.btn.fullHover)
                    {   // maybe hover indicator?
                        r.adjust(f3, f3, -f3, -f3);
                        c = Colors::mid(c, CONF_COLOR(btn.active, Bg), 6-animStep, animStep);
                        mask.render(r, painter, GRAD(chooser), Qt::Vertical, c, RECT.height()-f2, QPoint(0,f3));
                        r = RECT.adjusted(f1, f1, -f1, -f2); // RESET 'r' !!!
                    }
                    else if (config.btn.backLightHover)
                    {   // we MUST use alpha blending as this crosses between combo and bg
                        QColor c2 = CCOLOR(btn.active, Bg);
                        c2.setAlpha(c2.alpha()*animStep/8);
                        mask.outline(RECT, painter, c2, F(3));
                    }
                }
            }
            r.setBottom(RECT.bottom());
            shadows.sunken[round_][isEnabled].render(r, painter);
        }
    }

    // the arrow
    if (!ar.isNull())
    {
        if (!(ar.width()%2) )
            ar.setWidth(ar.width()-1);
        const int dy = ar.height()/4;
        QRect rect = ar.adjusted(0, dy, 0, -dy);

        Navi::Direction dir = Navi::S;
        bool upDown = false;
        if (listShown)
            dir = (config.leftHanded) ? Navi::E : Navi::W;
        else if (combo)
        {
            if (combo->currentIndex() == 0)
                dir = Navi::S;
            else if (combo->currentIndex() == combo->count()-1)
                dir = Navi::N;
            else
                upDown = true;
        }

        painter->save();
        painter->setPen(Qt::NoPen);
        if (cmb->editable)
        {
            if (upDown || dir == Navi::N)
                dir = Navi::S;
            upDown = false; // shall never look like spinbox!
            hover = hover && (cmb->activeSubControls == SC_ComboBoxArrow);
            if (!sunken)
            {
                painter->setBrush(FCOLOR(Base).dark(105));
                rect.translate(0, f2);
                drawArrow(dir, rect, painter);
                rect.translate(0, -f2);
            }
            if (hover || listShown)
                painter->setBrush(FCOLOR(Highlight));
            else
                painter->setBrush( Colors::mid(FCOLOR(Base), FCOLOR(Text)) );
        }
        else if (config.btn.backLightHover)
            painter->setBrush(Colors::mid(c, CONF_COLOR(btn.std, Fg), 6-animStep, 3+animStep));
        else
        {
            c = Colors::mid(c, CONF_COLOR(btn.active, Bg));
            c = Colors::mid(c, CONF_COLOR(btn.active, Bg), 6-animStep, animStep);
//          ar.adjust(f2, f3, -f2, -f3);
            mask.render(ar, painter, GRAD(chooser), Qt::Vertical, c, RECT.height()-f2, QPoint(0, ar.y() - RECT.y()) );
            painter->setBrush(Colors::mid(c, CONF_COLOR(btn.active, Fg), 1,2));
        }
        if (upDown)
        {
            rect.setBottom(rect.y() + rect.height()/2);
            rect.translate(0, -1);
            drawArrow(Navi::N, rect, painter);
            rect.translate(0, rect.height());
            drawArrow(Navi::S, rect, painter);
        }
        else
        {
            if (dir == Navi::N) // loooks unbalanced otherwise
                rect.translate(0, -f1);
            drawArrow(dir, rect, painter);
        }
        painter->restore();
    }
}


void
Style::drawComboBoxLabel(const QStyleOption * option, QPainter * painter,
                               const QWidget * widget) const
{
    ASSURE_OPTION(cb, ComboBox);
    OPT_ENABLED

    QRect editRect = subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, widget);
    painter->save();
    painter->setClipRect(editRect);

    if (!cb->currentIcon.isNull())
    {   // icon ===============================================
        QIcon::Mode mode = isEnabled ? QIcon::Normal : QIcon::Disabled;
        QPixmap pixmap = cb->currentIcon.pixmap(cb->iconSize, mode);
        QRect iconRect(editRect);
        iconRect.setWidth(cb->iconSize.width() + 4);
        iconRect = alignedRect( QApplication::layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter,
                                iconRect.size(), editRect);
//       if (cb->editable)
//          painter->fillRect(iconRect, opt->palette.brush(QPalette::Base));
        drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

        if (cb->direction == Qt::RightToLeft)
            editRect.translate(-4 - cb->iconSize.width(), 0);
        else
            editRect.translate(cb->iconSize.width() + 4, 0);
    }
    
    if (!cb->currentText.isEmpty() && !cb->editable)
    {   // text ==================================================
        if (cb->frame)
        {
            OPT_FOCUS
            if (animStep < 0)
            {
                OPT_HOVER
                animStep = hover ? 6 : 0;
            }
            else
            {
                if (const QComboBox* combo = qobject_cast<const QComboBox*>(widget))
                if (combo->view() && ((QWidget*)(combo->view()))->isVisible())
                    animStep = 6;
            }
            editRect.adjust(F(3),0, -F(3), 0);
            painter->setPen(btnFg(PAL, isEnabled, hasFocus, animStep));
        }
        drawItemText(painter, editRect, Qt::AlignCenter, PAL, isEnabled, cb->currentText);
    }
    painter->restore();
    animStep = -1;
}
