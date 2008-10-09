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

// #include <QComboBox>
#include <QtDebug>
#include <QEvent>
#include <QFrame>
#include <QMenuBar>
#include <QStyleOptionComboBox>
#include "colors.h"
#include "bespin.h"

using namespace Bespin;

static const QStyle::StyleHint SH_KCustomPrimitiveElement = (QStyle::StyleHint)0xff000001;
static const QStyle::StyleHint SH_KCustomControlElement = (QStyle::StyleHint)0xff000002;
static const QStyle::StyleHint SH_KCustomComplexControl = (QStyle::StyleHint)0xff000003;
static const QStyle::StyleHint SH_KCustomSubElement = (QStyle::StyleHint)0xff000004;
static const QStyle::StyleHint SH_KCustomStyleHint = (QStyle::StyleHint)0xff000005;
static const QStyle::StyleHint SH_KCustomSubControl = (QStyle::StyleHint)0xff000006;

int Style::styleHint( StyleHint hint, const QStyleOption * option, const QWidget * widget,
                            QStyleHintReturn * returnData ) const
{
    switch (hint)
    {
    case SH_EtchDisabledText:
    case SH_DitherDisabledText: //
        return false; // Both look CRAP! (win has etching)
#ifdef QT3_SUPPORT
    case SH_GUIStyle:
        return Qt::MacStyle;
#endif
    case SH_ScrollBar_MiddleClickAbsolutePosition:
        return true; // support middle click jumping
    case SH_ScrollBar_LeftClickAbsolutePosition:
        return false; // annoying
    case SH_ScrollBar_ScrollWhenPointerLeavesControl:
        return true; // UIs are no ego shooters...
    case SH_TabBar_Alignment:
//       return Qt::AlignRight;
        return Qt::AlignLeft;
//       return Qt::AlignCenter;
    case SH_Header_ArrowAlignment:
        return Qt::AlignLeft; // we move it to text center though...
    case SH_Slider_SnapToValue:
        return true; // yes, better then snapping after releasing the slider!
    case SH_Slider_SloppyKeyEvents:
        return true; // allow left/right & top/bottom on both orientations
    case SH_ProgressDialog_CenterCancelButton:
        return false; // looks strange
    case SH_ProgressDialog_TextLabelAlignment:
        return Qt::AlignCenter; // is this used anywhere?
    case SH_PrintDialog_RightAlignButtons:
        return true; // ok/cancel just belong there
    case SH_MainWindow_SpaceBelowMenuBar:
        if (const QMenuBar *menubar = qobject_cast<const QMenuBar*>(widget))
        if (menubar->height() == 0)
        if (!menubar->actions().isEmpty())
        { // we trick menubars if we use macmenus - hehehe...
            // the final result needs to be > "0" (i.e. "1") to avoid side effects...
            return -menubar->actionGeometry(menubar->actions().first()).height() + 1;
        }
        return 0; // no space between menus and docks (we need padding rather than margin)
///    case SH_FontDialog_SelectAssociatedText: // Select the text in the line edit, or when selecting an item from the listbox, or when the line edit receives focus, as done on Windows.
///    case SH_Menu_AllowActiveAndDisabled: // Allows disabled menu items to be active.
    case SH_Menu_SpaceActivatesItem:
        return true; // yes
    case SH_Menu_SubMenuPopupDelay:
        return 96; // motif value - don't have time...
    case SH_Menu_Scrollable:
        return true; // better scroll than fold around covering the desktop!
    case SH_Menu_SloppySubMenus:
        return true; // don't hide submenus immediately please
    case SH_ScrollView_FrameOnlyAroundContents: // YES - period.
        return (!(widget && widget->inherits("QComboBoxListView")));
    case SH_MenuBar_AltKeyNavigation:
        return true;
    case SH_ComboBox_ListMouseTracking:
    case SH_Menu_MouseTracking:
    case SH_MenuBar_MouseTracking:
        return true; // feedback to the user please!
    case SH_Menu_FillScreenWithScroll:
        return false; // don't trash the desktop
    case SH_ItemView_ChangeHighlightOnFocus:
        return config.fadeInactive; // Gray out selected items when losing focus.
///    case SH_Widget_ShareActivation: // Turn on sharing activation with floating modeless dialogs.
    case SH_TabBar_SelectMouseType:
        return QEvent::MouseButtonRelease; // QEvent::MouseButtonPress?
    case SH_Q3ListViewExpand_SelectMouseType:
        return QEvent::MouseButtonPress;
    case SH_TabBar_PreferNoArrows:
        return false; // the can grow horribly big...
    case SH_ComboBox_Popup: // Allows popups as a combobox drop-down menu.
//       if (const QComboBox *cmb = qobject_cast<const QComboBox *>(widget))
//          return cmb->count() < 11; // maybe depend on item count?!
        return false;
    case SH_ComboBox_PopupFrameStyle:
        return QFrame::StyledPanel | QFrame::Plain;
///    case SH_Workspace_FillSpaceOnMaximize: // The workspace should maximize the client area.
    case SH_TitleBar_NoBorder:
        return true; // The title bar has no border.
    case SH_ScrollBar_StopMouseOverSlider:
        return false; // Stops auto-repeat when the slider reaches the mouse position. (h�hh?)
    case SH_BlinkCursorWhenTextSelected:
        return false; // that's annoying
    case SH_RichText_FullWidthSelection:
        return true; /// (hähh?)
    case SH_GroupBox_TextLabelVerticalAlignment:
        return Qt::AlignTop; // we've no halfheight frame
    case SH_GroupBox_TextLabelColor:
        return QPalette::WindowText;
///    case SH_DialogButtons_DefaultButton: // Which button gets the default status in a dialog's button widget.
    case SH_DialogButtonBox_ButtonsHaveIcons:
        return false; // no way
    case SH_ToolBox_SelectedPageTitleBold:
        return true; // yes please
    case SH_LineEdit_PasswordCharacter:
    { // configurable...
        const QFontMetrics &fm =
        option ? option->fontMetrics :
        (widget ? widget->fontMetrics() : QFontMetrics(QFont()));
        if (fm.inFont(QChar(config.input.pwEchoChar)))
            return config.input.pwEchoChar;
        return '|';
        }
    case SH_Table_GridLineColor:
        if (option)
            return Colors::mid(option->palette.color(QPalette::Base),
                                option->palette.color(QPalette::Text),6,1).rgb();
        return -1;
    case SH_UnderlineShortcut:
        return true; // means the alt+<x> menu thing
    case SH_SpinBox_AnimateButton:
        return true; // feedback to the user, please
    case SH_SpinBox_ClickAutoRepeatRate:
        return 150;
    case SH_SpinBox_KeyPressAutoRepeatRate:
        return 75;
    case SH_ToolTipLabel_Opacity:
        return 204; // ~ 80%
    case SH_DrawMenuBarSeparator:
        return false; // NO WAY!!!
    case SH_TitleBar_ModifyNotification:
        return true; // add '*' to window title (or however WM manages it)
    case SH_Button_FocusPolicy:
        return Qt::StrongFocus;
    case SH_MenuBar_DismissOnSecondClick:
        return true; // simple close popups
    case SH_MessageBox_UseBorderForButtonSpacing:
        return false; // h�hh?
    case SH_TitleBar_AutoRaise:
        return true; // hover titlebar buttons in MDI
    case SH_ToolButton_PopupDelay:
        return 200; // everyone can do a click in 200ms - yesno?
///    case SH_FocusFrame_Mask: // The mask of the focus frame.
    case SH_RubberBand_Mask: // The mask of the rubber band.
        return false; // we an opaque one
///    case SH_WindowFrame_Mask: // The mask of the window frame.
    case SH_SpinControls_DisableOnBounds:
        return true; // yeah - don't trick the user
    case SH_Dial_BackgroundRole:
        return QPalette::Window;
    case SH_ComboBox_LayoutDirection:
        return config.leftHanded ? Qt::RightToLeft : Qt::LeftToRight;
    case SH_ItemView_EllipsisLocation:
        return Qt::AlignTrailing;
    case SH_ItemView_ShowDecorationSelected:
        return true; // full width selection
    case SH_ItemView_ActivateItemOnSingleClick:
        return config.macStyle;
    case SH_WizardStyle:
        return config.macStyle ? 2 : 1; // QWizard::MacStyle / QWizard::ModernStyle
#if QT_VERSION >= 0x040400
    case SH_FormLayoutWrapPolicy:
        return 0; //  QFormLayout::RowWrapPolicy
    case SH_FormLayoutFieldGrowthPolicy:
        return 0; // QFormLayout::FieldsStayAtSizeHint / ExpandingFieldsGrow / AllNonFixedFieldsGrow
    case SH_FormLayoutFormAlignment:
        return (Qt::AlignHCenter | Qt::AlignTop); // (Qt::AlignLeft | Qt::AlignTop)
    case SH_FormLayoutLabelAlignment:
        return Qt::AlignRight;
#endif
    case SH_ItemView_PaintAlternatingRowColorsForEmptyArea:
        return true;
    case SH_KCustomPrimitiveElement:
    case SH_KCustomControlElement:
    case SH_KCustomComplexControl:
    case SH_KCustomSubElement:
    case SH_KCustomStyleHint:
    case SH_KCustomSubControl:
    {
        if (!widget)
            return 0;
        int id = elementId(widget->objectName());
        if (id)
            const_cast<QWidget*>(widget)->setObjectName("KDE_CustomStyleElementID");
        else
            qDebug() << "Unsupported KCustomStyleElement requested:" << widget->objectName();
        return id;
    }
    default:
        return QCommonStyle::styleHint(hint, option, widget, returnData);
    } // switch
}
