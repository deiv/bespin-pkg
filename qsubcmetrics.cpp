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

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QTabBar>
#include <QStyleOption>
#include <QStyleOptionTab>
#include <limits.h>
#include "bespin.h"
#include "draw.h"


using namespace Bespin;

inline static bool
verticalTabs(QTabBar::Shape shape)
{
    return  shape == QTabBar::RoundedEast || shape == QTabBar::TriangularEast ||
            shape == QTabBar::RoundedWest || shape == QTabBar::TriangularWest;
}

QRect
Style::subControlRect (   ComplexControl control, const QStyleOptionComplex * option,
                                SubControl subControl, const QWidget * widget) const
{
    QRect ret;
    switch (control)
    {
    case CC_SpinBox:
    {   // A spinbox, like QSpinBox
        ASSURE_OPTION(spinbox, SpinBox) ret;

        int w = spinbox->rect.width(), h = spinbox->rect.height();
        h /= 2;
        // 1.6 -approximate golden mean
        w = qMax(dpi.f18, qMin(h, w / 4));
//          bs = bs.expandedTo(QApplication::globalStrut());
        int x = spinbox->rect.width() - w;
        switch (subControl)
        {
        case SC_SpinBoxUp:
            ret = QRect(x, 0, w, h);
            break;
        case SC_SpinBoxDown:
            ret = QRect(x, spinbox->rect.bottom()-h, w, h);
            break;
        case SC_SpinBoxEditField:
            w = h = 0; // becomes framesizes
            if (spinbox->frame)
                { w = dpi.f4; h = dpi.f1; }
            ret = QRect(w, h, x-dpi.f1, spinbox->rect.height() - 2*h);
            break;
        case SC_SpinBoxFrame:
            ret = spinbox->rect;
        default:
            break;
        }
        ret = visualRect(config.leftHanded, spinbox->rect, ret);
        break;
    }
    case CC_ComboBox:
    {   // A combobox, like QComboBox
        ASSURE_OPTION(cb, ComboBox) ret;
        int x,y,wi,he;
        cb->rect.getRect(&x,&y,&wi,&he);
        int margin = cb->editable ? 1 : config.btn.fullHover ? dpi.f2 : dpi.f4;

        switch (subControl)
        {
        case SC_ComboBoxFrame:
            ret = cb->rect;
            break;
         case SC_ComboBoxArrow:
            he = cb->fontMetrics.ascent() + dpi.f2;
//             he -= 2*margin;
            x += wi; wi = (int)(he*1.1); //1.618
            x -= margin + wi;
//             y += margin;
            y += (cb->rect.height()-he)/2;
            ret.setRect(x, y, wi, he);
            break;
        case SC_ComboBoxEditField:
            wi -= (int)((he - 2*margin)/1.1) + 3*margin;
            ret.setRect(x+margin, y+margin, wi, he - 2*margin);
            break;
        case SC_ComboBoxListBoxPopup:
            ret = cb->rect;
            if (!cb->editable)
            { // shorten for the arrow
                wi -= (int)((he - 2*margin)/1.1) + 3*margin;
                ret.setRect(x+margin, y, wi, he);
            }
            break;
        default:
            break;
        }
        ret = visualRect(config.leftHanded, cb->rect, ret);
      break;
    }
    case CC_GroupBox:
    {
        ASSURE_OPTION(groupBox, GroupBox) ret;

        switch (subControl)
        {
        case SC_GroupBoxFrame:
            ret = groupBox->rect;
            if (config.sunkenGroups && !groupBox->text.isEmpty())
                ret.setTop(ret.top() + groupBox->fontMetrics.height());
            break;
        case SC_GroupBoxContents:
        {
            int top = (groupBox->features & QStyleOptionFrameV2::Flat) ? F(3) : F(6);
            if (!groupBox->text.isEmpty())
                top += groupBox->fontMetrics.height();
            ret = groupBox->rect.adjusted(F(3), top, -F(3), -F(7));
            break;
        }
        case SC_GroupBoxCheckBox:
        {
            if (!config.sunkenGroups)
                ret = groupBox->rect.adjusted(F(7), F(7), 0, 0);
            ret.setWidth(dpi.ExclusiveIndicator);
            ret.setHeight(dpi.ExclusiveIndicator);
            break;
        }
        case SC_GroupBoxLabel:
        {
            QFontMetrics fontMetrics = groupBox->fontMetrics;
            const bool flat = groupBox->features & QStyleOptionFrameV2::Flat;
            int h = fontMetrics.height() + dpi.f4;
            int tw = fontMetrics.size(BESPIN_MNEMONIC, groupBox->text + QLatin1Char(' ')).width();
            int marg = flat ? 0 : F(4);
            Qt::Alignment align;
        
            if (flat || config.sunkenGroups)
            {
                int left = marg;
                if (groupBox->subControls & SC_GroupBoxCheckBox)
                    left += dpi.ExclusiveIndicator;
                ret = groupBox->rect.adjusted(left, 0, -marg, 0);
                align = Qt::AlignLeft | Qt::AlignVCenter;
            }
            else
            {
                ret = groupBox->rect.adjusted(marg, marg, -marg, 0);
                align = Qt::AlignCenter;
            }
            ret.setHeight(h);

            // Adjusted rect for label + indicatorWidth + indicatorSpace
            ret = alignedRect(groupBox->direction, align, QSize(tw, h), ret);
            break;
        }
        default:
            break;
        }
        break;
    }
   case CC_ScrollBar: // A scroll bar, like QScrollBar
      if (const QStyleOptionSlider *scrollbar =
          qstyleoption_cast<const QStyleOptionSlider *>(option)) {
         int sbextent = pixelMetric(PM_ScrollBarExtent, scrollbar, widget);
         int buttonSpace = config.scroll.showButtons * sbextent * 2;
         int maxlen = ((scrollbar->orientation == Qt::Horizontal) ?
                       scrollbar->rect.width() : scrollbar->rect.height()) - buttonSpace;
         
         int sliderlen;
            // calculate slider length
         if (scrollbar->maximum != scrollbar->minimum) {
            uint range = scrollbar->maximum - scrollbar->minimum;
            sliderlen = (qint64(scrollbar->pageStep) * maxlen) / (range + scrollbar->pageStep);
            
            int slidermin = pixelMetric(PM_ScrollBarSliderMin, scrollbar, widget);
            if (sliderlen < slidermin || range > INT_MAX / 2)
               sliderlen = slidermin;
            if (sliderlen > maxlen)
               sliderlen = maxlen;
         }
         else
            sliderlen = maxlen;
        
         int sliderstart = sliderPositionFromValue(scrollbar->minimum,
            scrollbar->maximum, scrollbar->sliderPosition, maxlen - sliderlen,
            scrollbar->upsideDown);
         switch (subControl) {
         case SC_ScrollBarSubLine:            // top/left button
            if (!config.scroll.showButtons)
               ret = QRect();
            else if (scrollbar->orientation == Qt::Horizontal) {
               int buttonWidth = qMin(scrollbar->rect.width() / 2, sbextent);
               ret.setRect(scrollbar->rect.width() - 2*buttonWidth, 0, buttonWidth, sbextent);
            }
            else {
               int buttonHeight = qMin(scrollbar->rect.height() / 2, sbextent);
               ret.setRect(0, scrollbar->rect.height() - 2*buttonHeight, sbextent, buttonHeight);
            }
            break;
         case SC_ScrollBarAddLine:            // bottom/right button
            if (!config.scroll.showButtons)
               ret = QRect();
            else if (scrollbar->orientation == Qt::Horizontal) {
               int buttonWidth = qMin(scrollbar->rect.width()/2, sbextent);
               ret.setRect(scrollbar->rect.width() - buttonWidth, 0, buttonWidth, sbextent);
            }
            else {
               int buttonHeight = qMin(scrollbar->rect.height()/2, sbextent);
               ret.setRect(0, scrollbar->rect.height() - buttonHeight, sbextent, buttonHeight);
            }
            break;
         case SC_ScrollBarSubPage:            // between top/left button and slider
            if (scrollbar->orientation == Qt::Horizontal)
               ret.setRect(dpi.f2, 0, sliderstart, sbextent);
            else
               ret.setRect(0, dpi.f2, sbextent, sliderstart);
            break;
         case SC_ScrollBarAddPage:            // between bottom/right button and slider
            if (scrollbar->orientation == Qt::Horizontal)
               ret.setRect(sliderstart + sliderlen - dpi.f2, 0,
                           maxlen - sliderstart - sliderlen, sbextent);
            else
               ret.setRect(0, sliderstart + sliderlen - dpi.f3,
                           sbextent, maxlen - sliderstart - sliderlen);
            break;
         case SC_ScrollBarGroove: {
            if (!config.scroll.groove) {
               int off = dpi.f4;
               ret = scrollbar->rect.adjusted(off,off,-off,-off);
               break;
            }
            int off = 0, d = 0;
            if (scrollbar->orientation == Qt::Horizontal) {
               if (config.scroll.groove == Groove::Groove) {
                  off = dpi.f2; d = scrollbar->rect.height()/3;
               }
               ret.setRect(off, d,
                           scrollbar->rect.width() - (buttonSpace + 2*off),
                           scrollbar->rect.height()-(2*d+off));
            }
            else {
               if (config.scroll.groove == Groove::Groove) {
                  off = dpi.f2; d = scrollbar->rect.width()/3;
               }
               ret.setRect(d, off, scrollbar->rect.width()-2*d,
                           scrollbar->rect.height() - (buttonSpace + 2*off));
            }
            break;
         }
         case SC_ScrollBarSlider:
            if (scrollbar->orientation == Qt::Horizontal)
               ret.setRect(sliderstart, 0, sliderlen, sbextent);
            else
               ret.setRect(0, sliderstart, sbextent, sliderlen);
            break;
         default:
            break;
         }
         ret = visualRect(scrollbar->direction, scrollbar->rect, ret);
      }
      break;
      
   case CC_Slider:
      if (const QStyleOptionSlider *slider =
          qstyleoption_cast<const QStyleOptionSlider *>(option)) {
         int tickOffset = pixelMetric(PM_SliderTickmarkOffset, slider, widget);
         int thickness = pixelMetric(PM_SliderControlThickness, slider, widget);
         
         switch (subControl) {
         case SC_SliderHandle: {
            int sliderPos = 0;
            int len = pixelMetric(PM_SliderLength, slider, widget);
            bool horizontal = slider->orientation == Qt::Horizontal;
            sliderPos =
               sliderPositionFromValue(slider->minimum, slider->maximum,
                                       slider->sliderPosition,
                                       (horizontal ? slider->rect.width() :
                                        slider->rect.height()) - len,
                                       slider->upsideDown);
            if (horizontal)
               ret.setRect(slider->rect.x() + sliderPos, slider->rect.y() + tickOffset, len, thickness);
            else
               ret.setRect(slider->rect.x() + tickOffset, slider->rect.y() + sliderPos, thickness, len);
            break;
         }
         case SC_SliderGroove: {
            const int d = (thickness-dpi.f2)/3;
            if (slider->orientation == Qt::Horizontal)
               ret.setRect(slider->rect.x(), slider->rect.y() + tickOffset + d,
                           slider->rect.width(), thickness-(2*d+dpi.f2));
            else
               ret.setRect(slider->rect.x() + tickOffset + d + dpi.f1,
                           slider->rect.y(), thickness-(2*d+dpi.f1),
                           slider->rect.height());
            break;
         }
         default:
            break;
         ret = visualRect(slider->direction, slider->rect, ret);
         }
      }
      break;
      
    case CC_ToolButton: // A tool button, like QToolButton
        if HAVE_OPTION(tb, ToolButton)
        {
//             int fw = pixelMetric(PM_DefaultFrameWidth, tb, widget);
//             ret = tb->rect.adjusted(fw,fw,-fw,0);
            ret = tb->rect;
            if ((tb->features & // has an arrow
                (QStyleOptionToolButton::Menu | QStyleOptionToolButton::PopupDelay)) ==
                QStyleOptionToolButton::Menu)
            {
                int x = ret.right() - pixelMetric(PM_MenuButtonIndicator, tb, widget);
                if (subControl == SC_ToolButton)
                    ret.setRight(x);
                else if (subControl == SC_ToolButtonMenu)
                    ret.setLeft(x);
            }
            return visualRect(tb->direction, tb->rect, ret);
        }
        break;
   case CC_TitleBar: // A Title bar, like what is used in Q3Workspace
      if (const QStyleOptionTitleBar *tb =
          qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
         const int controlMargin = dpi.f3;
         const int controlHeight = tb->rect.height() - controlMargin*2;
         const int delta = controlHeight + 2*controlMargin;
         int offset = 0;
         
         bool isMinimized = tb->titleBarState & Qt::WindowMinimized;
         bool isMaximized = tb->titleBarState & Qt::WindowMaximized;
             
         SubControl sc = subControl;
         if (sc == SC_TitleBarNormalButton) { // check what it's good for
            if (isMinimized)
               sc = SC_TitleBarMinButton; // unminimize
            else if (isMaximized)
               sc = SC_TitleBarMaxButton; // unmaximize
            else
               break;
         }
         
         switch (sc) {
         case SC_TitleBarLabel:
            if (tb->titleBarFlags &
                (Qt::WindowTitleHint | Qt::WindowSystemMenuHint)) {
               ret = tb->rect;
//                ret.adjust(delta, 0, 0, 0);
               if (tb->titleBarFlags & Qt::WindowSystemMenuHint)
                  ret.adjust(delta, 0, -delta, 0);
               if (tb->titleBarFlags & Qt::WindowMinimizeButtonHint)
                  ret.adjust(delta, 0, 0, 0);
               if (tb->titleBarFlags & Qt::WindowMaximizeButtonHint)
                  ret.adjust(delta, 0, 0, 0);
               if (tb->titleBarFlags & Qt::WindowShadeButtonHint)
                  ret.adjust(0, 0, -delta, 0);
               if (tb->titleBarFlags & Qt::WindowContextHelpButtonHint)
                  ret.adjust(0, 0, -delta, 0);
            }
            break;
         case SC_TitleBarMaxButton:
            if (isMaximized && subControl == SC_TitleBarMaxButton)
               break;
            if (tb->titleBarFlags & Qt::WindowMaximizeButtonHint)
               offset += delta;
            else if (sc == SC_TitleBarMaxButton) // true...
               break;
         case SC_TitleBarMinButton:
            if (isMinimized && subControl == SC_TitleBarMinButton)
               break;
            if (tb->titleBarFlags & Qt::WindowMinimizeButtonHint)
               offset += delta;
            else if (sc == SC_TitleBarMinButton)
               break;
         case SC_TitleBarCloseButton:
            if (tb->titleBarFlags & Qt::WindowSystemMenuHint)
               offset += controlMargin;
            else if (sc == SC_TitleBarCloseButton)
               break;
            ret.setRect(tb->rect.left() + offset, tb->rect.top() + controlMargin,
                        controlHeight, controlHeight);
            break;
            
         case SC_TitleBarContextHelpButton:
            if (tb->titleBarFlags & Qt::WindowContextHelpButtonHint)
               offset += delta;
         case SC_TitleBarShadeButton:
            if (!isMinimized && (tb->titleBarFlags & Qt::WindowShadeButtonHint))
               offset += delta;
            else if (sc == SC_TitleBarShadeButton)
               break;
         case SC_TitleBarUnshadeButton:
            if (isMinimized && (tb->titleBarFlags & Qt::WindowShadeButtonHint))
               offset += delta;
            else if (sc == SC_TitleBarUnshadeButton)
               break;
         case SC_TitleBarSysMenu:
            if (tb->titleBarFlags & Qt::WindowSystemMenuHint)
               offset += delta + controlMargin;
            else if (sc == SC_TitleBarSysMenu)
               break;
            ret.setRect(tb->rect.right() - offset,
                        tb->rect.top() + controlMargin, controlHeight, controlHeight);
            break;
         default:
            break;
         }
         ret = visualRect(tb->direction, tb->rect, ret);
      }
      break;
   case CC_Q3ListView: // Used for drawing the Q3ListView class
   case CC_Dial: // A dial, like QDial
   default:
      ret = QCommonStyle::subControlRect ( control, option, subControl, widget);
   }
   return ret;
}

QRect
Style::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element)
    {
    case SE_PushButtonContents: // Area containing the label (icon with text or pixmap)
        return visualRect(option->direction, RECT, RECT.adjusted(dpi.f4,dpi.f4,-dpi.f4,-dpi.f4));
//    case SE_PushButtonFocusRect: // Area for the focus rect (usually larger than the contents rect)
    case SE_CheckBoxContents: // Area for the state label
    case SE_ViewItemCheckIndicator: // Area for a view item's check mark
    case SE_CheckBoxIndicator: // Area for the state indicator (e.g., check mark)
    {
        int h = dpi.Indicator;
        QRect r = RECT;
        if (config.btn.layer)
            r.setRect(r.x()+F(1), r.y() + ((r.height() - h) / 2), h-dpi.f2, h);
        else
            r.setRect(r.x(), r.y() + ((r.height() - h) / 2), h, h);
        if (element != SE_CheckBoxContents)
            return visualRect(option->direction, RECT, r);
        int spacing = dpi.f5;
        r.setRect(r.right() + spacing, RECT.y(), RECT.width() - r.width() - spacing, RECT.height());
        return visualRect(option->direction, RECT, r);
    }
    case SE_RadioButtonIndicator: // Area for the state indicator
    case SE_RadioButtonContents: // Area for the label
    {
        int h = dpi.ExclusiveIndicator;
        QRect r = RECT;
        r.setRect(r.x()+F(1), r.y() + ((r.height() - h) / 2), h, h);

        if (element == SE_RadioButtonIndicator)
            return visualRect(option->direction, RECT, r);
            
        int spacing = dpi.f5;
        r.setRect(r.right() + spacing, RECT.y(), RECT.width() - r.width() - spacing, RECT.height());
        return visualRect(option->direction, RECT, r);
    }
    case SE_CheckBoxFocusRect: // Area for the focus indicator
    case SE_CheckBoxClickRect: // Clickable area, defaults to SE_CheckBoxFocusRect
    case SE_RadioButtonFocusRect: // Area for the focus indicator
    case SE_RadioButtonClickRect: // Clickable area, defaults to SE_RadioButtonFocusRect
        return RECT;
   
//    case SE_ComboBoxFocusRect: // Area for the focus indicator
//    case SE_SliderFocusRect: // Area for the focus indicator
//    case SE_Q3DockWindowHandleRect: // Area for the tear-off handle
    case SE_ProgressBarGroove: // Area for the groove
    case SE_ProgressBarContents: // Area for the progress indicator
    case SE_ProgressBarLabel: // Area for the text label
        return RECT;
//    case SE_DialogButtonAccept: // Area for a dialog's accept button
//    case SE_DialogButtonReject: // Area for a dialog's reject button
//    case SE_DialogButtonApply: // Area for a dialog's apply button
//    case SE_DialogButtonHelp: // Area for a dialog's help button
//    case SE_DialogButtonAll: // Area for a dialog's all button
//    case SE_DialogButtonRetry: // Area for a dialog's retry button
//    case SE_DialogButtonAbort: // Area for a dialog's abort button
//    case SE_DialogButtonIgnore: // Area for a dialog's ignore button
//    case SE_DialogButtonCustom: // Area for a dialog's custom widget area (in the button row)
    case SE_HeaderArrow:
    {
        int x,y,w,h;
        if (widget)
            option->rect.intersected(widget->rect()).getRect(&x,&y,&w,&h);
        else
            option->rect.getRect(&x,&y,&w,&h);
        int margin = dpi.f2;// ;) pixelMetric(QStyle::PM_HeaderMargin, opt, widget);
        QRect r;
        if (option->state & State_Horizontal)
        {
            bool up = false;
            if HAVE_OPTION(hdr, Header)
                up = hdr->sortIndicator == QStyleOptionHeader::SortUp;
            const int h_3 = h / 3;
            r.setRect(x + (w - h_3)/2, up ? y+h-h_3 : y, h_3, h_3);
//          r.setRect(x + w - 2*margin - (h / 2), y + h/4 + margin, h / 2, h/2);
        }
        else
        {
            r.setRect(x + dpi.f5, y, h / 2, h / 2 - margin * 2);
            r = visualRect(option->direction, option->rect, r);
        }
        return r;
    }
//    case SE_HeaderLabel: //  
    case SE_TabWidgetLeftCorner:
    case SE_TabWidgetRightCorner:
        if HAVE_OPTION(twf, TabWidgetFrame)
        {
            int off = 0, x = 0, y = 0;
            switch (twf->shape)
            {
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                y = twf->rect.bottom() - twf->tabBarSize.height();
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                off = twf->tabBarSize.height()/4;
                if (element == SE_TabWidgetRightCorner)
                x = twf->rect.right() - twf->tabBarSize.height();
                break;
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                x = twf->rect.right() - twf->tabBarSize.width();
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                off = twf->tabBarSize.width()/4;
                if (element == SE_TabWidgetRightCorner)
                y = twf->rect.bottom() - twf->tabBarSize.width();
            default:
                break;
            }
            off -= dpi.f2; x +=off; y += off;
            off = 2*(off+dpi.f4);
            return QRect(x,y,off,off);
        }
    case SE_TabWidgetTabBar:
    {
        if HAVE_OPTION(twf, TabWidgetFrame)
        {
            QRect r = QCommonStyle::subElementRect(SE_TabWidgetTabBar, option, widget);
            if (verticalTabs(twf->shape))
            {
                r.translate(0, F(4));
                if (r.bottom() > option->rect.bottom())
                    r.setBottom(option->rect.bottom());
            }
            else if (option->direction == Qt::LeftToRight)
            {
                r.translate(F(4), 0);
                if (r.right() > option->rect.right())
                    r.setRight(option->rect.right());
            }
            else // rtl support
            {
                r.translate(-F(4), 0);
                if (r.left() > option->rect.left())
                    r.setLeft(option->rect.left());
            }
            return r;
        }
    }
    case SE_TabWidgetTabContents:
        if HAVE_OPTION(twf, TabWidgetFrame)
        {
            QRect r = RECT; //subElementRect ( SE_TabWidgetTabPane, option, widget);
//          QStyleOptionTab tabopt;
//          tabopt.shape = twf->shape;
            const int margin = F(4);
//          int baseHeight = pixelMetric(PM_TabBarBaseHeight, &tabopt, widget);
            switch (twf->shape)
            {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                r.adjust(margin, margin+twf->tabBarSize.height(), -margin, -margin);
            break;
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                r.adjust(margin, margin, -margin, -margin-twf->tabBarSize.height());
            break;
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                r.adjust(margin, margin, -margin-twf->tabBarSize.width(), -margin);
            break;
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                r.adjust(margin+twf->tabBarSize.width(), margin, -margin, -margin);
            }
            return r;
        }
    case SE_TabWidgetTabPane: //
        return RECT;//.adjusted(-dpi.f8, 0, dpi.f8, 0);
    case SE_ToolBoxTabContents: // Area for a toolbox tab's icon and label
        return RECT;
//    case SE_TabBarTearIndicator: // Area for the tear indicator on a tab bar with scroll arrows.
    default:
        return QCommonStyle::subElementRect ( element, option, widget);
   }
}
