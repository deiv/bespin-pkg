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
#include <cmath>
#include "oxrender.h"
#include "draw.h"
#include "animator/hover.h"


/*
// Exponential blur, Jani Huhtanen, 2006 ==========================
*  expblur(QImage &img, int radius)
*
*  In-place blur of image 'img' with kernel of approximate radius 'radius'.
*  Blurs with two sided exponential impulse response.
*
*  aprec = precision of alpha parameter in fixed-point format 0.aprec
*  zprec = precision of state parameters zR,zG,zB and zA in fp format 8.zprec
*/

template<int aprec, int zprec>
static inline void blurinner(unsigned char *bptr, int &zR, int &zG, int &zB, int &zA, int alpha)
{
    int R,G,B,A;
    R = *bptr;
    G = *(bptr+1);
    B = *(bptr+2);
    A = *(bptr+3);

    zR += (alpha * ((R<<zprec)-zR))>>aprec;
    zG += (alpha * ((G<<zprec)-zG))>>aprec;
    zB += (alpha * ((B<<zprec)-zB))>>aprec;
    zA += (alpha * ((A<<zprec)-zA))>>aprec;

    *bptr =     zR>>zprec;
    *(bptr+1) = zG>>zprec;
    *(bptr+2) = zB>>zprec;
    *(bptr+3) = zA>>zprec;
}

template<int aprec,int zprec>
static inline void blurrow( QImage & im, int line, int alpha)
{
    int zR,zG,zB,zA;

    QRgb *ptr = (QRgb *)im.scanLine(line);

    zR = *((unsigned char *)ptr    )<<zprec;
    zG = *((unsigned char *)ptr + 1)<<zprec;
    zB = *((unsigned char *)ptr + 2)<<zprec;
    zA = *((unsigned char *)ptr + 3)<<zprec;

    for(int index=1; index<im.width(); index++)
        blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);

    for(int index=im.width()-2; index>=0; index--)
        blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
}

template<int aprec, int zprec>
static inline void blurcol( QImage & im, int col, int alpha)
{
    int zR,zG,zB,zA;

    QRgb *ptr = (QRgb *)im.bits();
    ptr+=col;

    zR = *((unsigned char *)ptr    )<<zprec;
    zG = *((unsigned char *)ptr + 1)<<zprec;
    zB = *((unsigned char *)ptr + 2)<<zprec;
    zA = *((unsigned char *)ptr + 3)<<zprec;

    for(int index=im.width(); index<(im.height()-1)*im.width(); index+=im.width())
        blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);

    for(int index=(im.height()-2)*im.width(); index>=0; index-=im.width())
        blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
}

template<int aprec,int zprec>
void expblur( QImage &img, int radius )
{
    if(radius<1)
        return;

    // Calculate the alpha such that 90% of the kernel is within the radius. (Kernel extends to infinity)
    int alpha = (int)((1<<aprec)*(1.0f-expf(-2.3f/(radius+1.f))));

    for(int row=0;row<img.height();row++)
        blurrow<aprec,zprec>(img,row,alpha);

    for(int col=0;col<img.width();col++)
        blurcol<aprec,zprec>(img,col,alpha);
}
// ======================================================

static int step = 0;

void
Style::drawToolButton(const QStyleOptionComplex * option,
                            QPainter * painter, const QWidget * widget) const
{
    OPT_SUNKEN OPT_ENABLED OPT_HOVER

    if (widget && widget->parentWidget() && qobject_cast<QTabBar*>(widget->parent()))
    {   // special handling for the tabbar scrollers ------------------------------
        QColor c = widget->parentWidget()->palette().color(config.tab.std_role[0]);
        QColor c2 = widget->parentWidget()->palette().color(config.tab.std_role[1]);
        if (sunken)
        {
            int dy = (RECT.height()-RECT.width())/2;
            QRect r = RECT.adjusted(dpi.f2,dy,-dpi.f2,-dy);
            masks.rect[true].render(r, painter, Gradients::Sunken, Qt::Vertical, c);
        }
        painter->save();
        painter->setPen( isEnabled ? c2 : Colors::mid(c, c2) );
        drawToolButtonLabel(option, painter, widget);
        painter->restore();
        return;
    } // --------------------------------------------------------------------

    ASSURE_OPTION(toolbutton, ToolButton);

    QRect button = subControlRect(CC_ToolButton, toolbutton, SC_ToolButton, widget);
    State bflags = toolbutton->state;

    if ((bflags & State_AutoRaise) && !hover)
        bflags &= ~State_Raised;

    State mflags = bflags;

    if (toolbutton->activeSubControls & SC_ToolButton)
        bflags |= State_Sunken;

    hover = isEnabled && (bflags & (State_Sunken | State_On | State_Raised | State_HasFocus));

    step = Animator::Hover::step(widget);

    // frame around whole button
    if (option->state & State_On)
    {
        QStyleOption tool(0);
        tool.palette = toolbutton->palette;
        tool.rect = RECT;
        tool.state = bflags;
        drawToolButtonShape(&tool, painter, widget);
    }

    if ((toolbutton->subControls & SC_ToolButtonMenu) &&
        !(bflags & State_Sunken)) // don't paint a dropdown arrow iff the real button is pressed
    {
        QRect menuarea = subControlRect(CC_ToolButton, toolbutton, SC_ToolButtonMenu, widget);
//         if (toolbutton->activeSubControls & SC_ToolButtonMenu) // pressed
//             painter->drawTiledPixmap(menuarea, Gradients::pix(FCOLOR(Window), menuarea.height(), Qt::Vertical, Gradients::Sunken));
        QPen oldPen = painter->pen();
        painter->setPen(Colors::mid(FCOLOR(Window), FCOLOR(WindowText), 8-step, step+3));
//          tool.rect = menuarea; tool.state = mflags;
        drawSolidArrow(Navi::S, menuarea, painter);
        painter->setPen(oldPen);
//         if (hover)
//         {
//             menuarea.setLeft(button.right()-shadows.line[1][Sunken].thickness()/2);
//             shadows.line[1][Sunken].render(menuarea, painter);
//         }
    }

   // label in the toolbutton area
   QStyleOptionToolButton label = *toolbutton;
   label.rect = button;
   drawToolButtonLabel(&label, painter, widget);
}

void
Style::drawToolButtonShape(const QStyleOption * option,
                                 QPainter * painter, const QWidget * widget) const
{
    OPT_ENABLED;

    if (!isEnabled)
        return;

    const bool isOn = option->state & State_On;
    const QColor &c = Colors::bg(PAL, widget);
    if (isOn)
    {
        masks.rect[true].render(RECT, painter, Gradients::Sunken, Qt::Vertical, c);
        shadows.sunken[true][true].render(RECT, painter);
    }
}

static QPixmap scaledIcon, emptyIcon;
qint64 lastIconPix = 0;
static QPixmap &
icon(QPixmap &pix, int step)
{
    if (pix.cacheKey() != lastIconPix)
    {
        scaledIcon = pix.scaledToHeight ( pix.height() + F(4), Qt::SmoothTransformation );
        if (emptyIcon.size() != scaledIcon.size())
            emptyIcon = QPixmap(scaledIcon.size());
        lastIconPix = pix.cacheKey();
    }
    emptyIcon.fill(Qt::transparent);
    float quote = step/6.0;
    if (quote >= 1.0)
        return scaledIcon;
#ifndef QT_NO_XRENDER
    OXRender::composite(pix, NULL, emptyIcon, 0, 0, 0, 0, F(2), F(2), pix.width(), pix.height(), PictOpOver);
    OXRender::blend(scaledIcon, emptyIcon, quote);
#else
    QPixmap tmp = scaledIcon.copy();
    QPainter p(&tmp);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(tmp.rect(), QColor(0,0,0, quote*255.0));
    p.end();
    p.begin(&emptyIcon);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawPixmap(F(2), F(2), pix);
    p.drawPixmap(0, 0, tmp);
    p.end();
#endif
    return emptyIcon;
}

void
Style::drawToolButtonLabel(const QStyleOption * option,
                                 QPainter * painter, const QWidget *widget) const
{
    ASSURE_OPTION(toolbutton, ToolButton);
    OPT_ENABLED OPT_SUNKEN

    // Arrow type always overrules and is always shown
    const bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
    const bool justText = (!hasArrow && toolbutton->icon.isNull()) &&
                            !toolbutton->text.isEmpty() ||
                            toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly;

    QPalette::ColorRole role = widget ? widget->foregroundRole() : QPalette::WindowText;
    if (justText)
    {   // the most simple way
        painter->setPen(Colors::mid(PAL.color(role), FCOLOR(Highlight), 6-step, step));
        QFont fnt = toolbutton->font;
        if (sunken) fnt.setBold(true);
        painter->setFont(fnt);
        drawItemText(painter, RECT, Qt::AlignCenter | BESPIN_MNEMONIC, PAL, isEnabled, toolbutton->text);
        return;
    }

    QPixmap pm;
    QSize pmSize = RECT.size() - QSize(F(4), F(4));
    pmSize = pmSize.boundedTo(toolbutton->iconSize);

    if (!toolbutton->icon.isNull())
    {
//         const QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
        pm = toolbutton->icon.pixmap(RECT.size().boundedTo(pmSize), QIcon::Normal, QIcon::Off);
        if (!isEnabled)
        {
            QImage img(pm.width() + F(4), pm.height() + F(4), QImage::Format_ARGB32);
            img.fill(Qt::transparent);
            QPainter p(&img);
            p.setOpacity(0.4);
            p.drawImage(F(2),F(2),pm.toImage());
            p.end();
            expblur<16,7>(img, F(4)/*pm.width()/4*/); // aligning blur kernel to icon size looks crap
            pm = QPixmap::fromImage(img);
        }
        else if (step && !sunken && !pm.isNull())
            pm = icon(pm, step);
        pmSize = pm.size();
    }

    if (!(toolbutton->text.isEmpty() || toolbutton->toolButtonStyle == Qt::ToolButtonIconOnly))
    {
        QColor c = PAL.color(role);
        if (pm.isNull())
            c = Colors::mid(c, FCOLOR(Highlight), 6-step, step);
        painter->setPen(c);
            
//       QFont fnt = toolbutton->font;
//       if (hover) fnt.setUnderline(true);
        painter->setFont(toolbutton->font);

        QRect pr = RECT, tr = RECT;
        int alignment = BESPIN_MNEMONIC;

        if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon)
        {
            int fh = painter->fontMetrics().height();
            pr.adjust(0, 0, 0, -fh - dpi.f2);
            tr.adjust(0, pr.bottom(), 0, -dpi.f3);
            if (!hasArrow)
                drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
            else
                drawSolidArrow(Navi::S, pr, painter);
            alignment |= Qt::AlignCenter;
        }
        else
        {
            pr.setWidth(toolbutton->iconSize.width() + F(4));

            if (!hasArrow)
                drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
            else
                drawSolidArrow(Navi::S, pr, painter);

            tr.adjust(pr.width() + F(4), 0, 0, 0);
            alignment |= Qt::AlignLeft | Qt::AlignVCenter;
        }
        drawItemText(painter, tr, alignment, PAL, isEnabled, toolbutton->text);
        return;
    }

    if (hasArrow)
    {
        const int f5 = F(5);
        drawSolidArrow(Navi::Direction(toolbutton->arrowType), RECT.adjusted(f5,f5,-f5,-f5), painter);
    }
    else
        drawItemPixmap(painter, RECT, Qt::AlignCenter, pm);
}

void
Style::drawToolBarHandle(const QStyleOption * option, QPainter * painter,
                               const QWidget *) const
{

    OPT_HOVER
//    if (!hover) return;

    painter->save();
    QRect rect = RECT; bool line = false; int dx(0), dy(0);
    if (RECT.width() > RECT.height())
    {
        line = (RECT.width() > 9*RECT.height()/2);
        if (line)
            { dx = 3*RECT.height()/2; dy = 0; }
        rect.setLeft(rect.left()+(rect.width()-rect.height())/2);
        rect.setWidth(rect.height());
    }
    else
    {
        line = (RECT.height() > 3*RECT.width());
        if (line)
            { dx = 0; dy = 3*RECT.width()/2; }
        rect.setTop(rect.top()+(rect.height()-rect.width())/2);
        rect.setHeight(rect.width());
    }
    QColor c = FCOLOR(Window);
    if (hover)
        c = Colors::mid(c, FCOLOR(Highlight), 3, 1);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Gradients::pix(c, rect.height(), Qt::Vertical, Gradients::Sunken));
    painter->setPen(Qt::NoPen);
    painter->setBrushOrigin(rect.topLeft());
    painter->drawEllipse(rect);
    if (line)
    {
        const int f1 = dpi.f1;
        rect.adjust(f1,f1,-f1,-f1);
        painter->setBrush(Gradients::pix(c, rect.height(), Qt::Vertical, Gradients::Sunken));
        rect.translate(-dx,-dy);
        painter->setBrushOrigin(rect.topLeft());
        painter->drawEllipse(rect);
        rect.translate( 2*dx, 2*dy);
        painter->setBrushOrigin(rect.topLeft());
        painter->drawEllipse(rect);
    }
    painter->restore();
}
