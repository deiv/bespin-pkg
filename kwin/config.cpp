//////////////////////////////////////////////////////////////////////////////
//
// -------------------
// Bespin window decoration for KDE.
// -------------------
// Copyright (c) 2008/2009 Thomas Lübking <baghira-style@gmx.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include <QSettings>
#include <QtDebug>

class KConfig;

extern "C"
{
    Q_DECL_EXPORT QObject* allocate_config(KConfig*, QWidget* parent) {
        return (new Config(parent));
    }
}

enum ConfigRole { ActiveGradient = Qt::UserRole, ActiveGradient2,
                  InactiveGradient, InactiveGradient2,
                  ActiveColor, ActiveColor2, InactiveColor, InactiveColor2,
                  ActiveText, ActiveButtons, InactiveText, InactiveButtons,
                  Classes, Types };

Config::Config(QWidget* parent) : BConfig(parent)
{
    ui.setupUi(this);
    // designer can't do this atm.
    ui.actGrad->setItemData( 8, -1);
    ui.actGrad->setItemData( 9, -3);
    ui.actGrad->setItemData( 10, -4);
    ui.inactGrad->setItemData( 8, -1);
    ui.inactGrad->setItemData( 9, -3);
    ui.inactGrad->setItemData( 10, -4);
    connect (ui.actGrad, SIGNAL(currentIndexChanged(int)), this, SLOT(watchBgMode()));
    connect (ui.inactGrad, SIGNAL(currentIndexChanged(int)), this, SLOT(watchBgMode()));

    connect (ui.actGrad2, SIGNAL(currentIndexChanged(int)), this, SLOT(watchDecoGradient()));
    connect (ui.inactGrad2, SIGNAL(currentIndexChanged(int)), this, SLOT(watchDecoGradient()));
    
    ui.onlinehelp->setOpenExternalLinks( true ); /** i've an internet link here */
    ui.onlinehelp->viewport()->setAutoFillBackground(false);
    const QPalette::ColorGroup groups[3] = { QPalette::Active, QPalette::Inactive, QPalette::Disabled };
    QPalette pal = ui.onlinehelp->palette();
    for (int i = 0; i < 3; ++i)
    {
        pal.setColor(groups[i], QPalette::Base, pal.color(groups[i], QPalette::Window));
        pal.setColor(groups[i], QPalette::Text, pal.color(groups[i], QPalette::WindowText));
    }
    ui.onlinehelp->setPalette(pal);

    /** 1. name the info browser, you'll need it to show up context help
    Can be any QTextBrowser on your UI form */
    setInfoBrowser(ui.onlinehelp);
    /** 2. Define a context info that is displayed when no other context help is
    demanded */
    setDefaultContextInfo("<h1 align=\"center\">Bespin KWin Client Config</h1>");

    /** handleSettings(.) tells BConfig to take care (save/load) of a widget
    In this case "ui.bgMode" is the widget on the form,
    "BackgroundMode" specifies the entry in the ini style config file and
    "3" is the default value for this entry*/
    handleSettings(ui.resizeCorner, "ResizeCorner", false);

    /** setContextHelp(.) attaches a context help string to a widget on your form */
    setContextHelp(ui.resizeCorner, "<b>Resize Corner</b><hr>\
    Displays a small sizeGrip in the lower right corner of every window, this is especially\
    usefull if you select \"Tiny\" border size (i.e. no border at all)<br>\
    If it should ever cover some important area of the window content, you can allways hide it\
    for 5 seconds by rightclicking it.<br>\
    <b>Notice:<b/> This is a little hackish and might not work on all systems!");

    handleSettings(ui.trimmTitle, "TrimmCaption", true);
    setContextHelp(ui.trimmTitle, "<b>Trimm Title</b><hr>\
    Some windows tend to have ridiculusly looong captions, e.g. Konqueror if a website title\
    is long (many newspages)<br>\
    Check this to carve out the (hopefully) interesting part of the title for a slicker look.");

    handleSettings(ui.forceUserColors, "ForceUserColors", false);
    setContextHelp(ui.forceUserColors, "<b>Ignore the styles color hints</b><hr>\
    The Bespin style informs the decoration about wishes on the used gradients and colors<br>\
    This is fully configurable in the style setup and per preset, but if you want, you can\
    completely ignore these hints and use the colors configured for KWin and the gradients setup\
    below.");

    handleSettings(ui.actGrad, "ActiveGradient", 2);
    setContextHelp(ui.actGrad, "<b>Active base gradient</b><hr>\
    The BASE gradient of ACTIVE windows.<br>\
    If set to \"Flat\", \"Vertical gradient\" or \"Horizontal gradient\", the inactive variant will\
    use the same value");

    handleSettings(ui.inactGrad, "InactiveGradient", 0);
    setContextHelp(ui.inactGrad, "<b>Inctive base gradient</b><hr>\
    The BASE gradient of INACTIVE windows.<br>\
    If set to \"Flat\", \"Vertical gradient\" or \"Horizontal gradient\", the active variant will\
    use the same value");

    handleSettings(ui.actGrad2, "ActiveGradient2", 0);
    setContextHelp(ui.actGrad2, "<b>Second active gradient</b><hr>\
    Accessoire gradient in the titlebar center of ACTIVE windows.");
    
    handleSettings(ui.inactGrad2, "InactiveGradient2", 0);
    setContextHelp(ui.inactGrad2, "<b>Second inactive gradient</b><hr>\
    Accessoire gradient in the titlebar center of INACTIVE windows.");

    handleSettings(ui.multibutton, "MultiButtonOrder", "MHFBS");
    setContextHelp(ui.multibutton, "<b>The 'Multibutton'</b><hr>\
    KWin supports a lot of different button type with more or less usefull functions.<br>\
    Though this is really a nice feature, it will make the titlebar look cluttered and dull.<br>\
    Thus the Bespin decoration will add only the first button beyond (NOT behind ;) Close/Min/Max\
    and stack all extra buttons into it.<br>\
    You can simply and literally scroll through the different extra buttons with your MouseWheel then.<br>\
    Below you can setup an order for the extra buttons. Only the letters described below are taken\
    into account, case doesn't matter and you can type as much blanks or jerk between them as you want.<br><hr>\
    <b>M</b>: Window Menu<br>\
    <b>S</b>: Toggle Window on all Desktops<br>\
    <b>H</b>: Help ('What's this')<br>\
    <b>F</b>: Toggle 'Keep Above'<br>\
    <b>B</b>: Toggle 'Keep Below'<br>\
    <b>L</b>: Toggle Shade (The window is shrinked into the titlebar)<br>\
    <b>!</b>: Window Info<br>\
    <b>E</b>: Window List");

    handleSettings(ui.iconVariant, "IconVariant", 1);
    
    handleSettings(ui.slickButtons, "SlickButtons", 0);
    setContextHelp(ui.slickButtons, "The appereance of unhovered buttons. Morphs to icon on hover<br>\
    Dots and bricks look slick, but may be considered less usable, as unhovered buttons look all the same");

    handleSettings(ui.titlePadding, "TitlePadding", 0);
    setContextHelp(ui.titlePadding, "<b>Titlebar padding</b><hr>\
    How much additional space you want above and below the title text");

    handleSettings(ui.inactiveButtons, "InactiveButtons", false);
    setContextHelp(ui.inactiveButtons, "<b>Show inactive Buttons</b><hr>\
    By default no buttons are shown on inactive windows but fade in and out.<br>You can force them to be visible here.");

    QButtonGroup *btngrp = new QButtonGroup(this);
    btngrp->addButton(ui.titleLeft, Qt::AlignLeft);
    btngrp->addButton(ui.titleCenter, Qt::AlignHCenter);
    btngrp->addButton(ui.titleRight, Qt::AlignRight);
    handleSettings(btngrp, "TitleAlign", Qt::AlignHCenter);

    handleSettings(ui.smallTitleClasses, "SmallTitleClasses", "");
    setContextHelp(ui.smallTitleClasses, "<b>Small Title classes</b><hr>\
    Windows with the NET_WM types \"NET::Utility\", \"NET::Menu\" and \"NET::Toolbar\" get\
    a smaller titlebar<br>\
    You can enter a comma separated list of window classes to enforce such small titlebar for all\
    window types.<br>\
    The window class is usually near the application name and can determined by adding the Info button\
    (\"!\") to the multibutton order and pressing it.");

    /** if you call setContextHelp(.) with a combobox and pass a stringlist,
    the strings are attached to the combo entries and shown on select/hover */

    /** setQSetting(.) tells BConfig to store values at
    "Company, Application, Group" - these strings are passed to QSettings */
    setQSetting("Bespin", "Style", "Deco");

    /** you can call loadSettings() whenever you want, but (obviously)
    only items that have been propagated with handleSettings(.) are handled !!*/
    loadSettings();

    /** ===========================================
    You're pretty much done here - simple eh? ;) **/

    /* setup the presets UI */
    QListWidgetItem *item = new QListWidgetItem("Default");
    item->setData(ActiveGradient, variant(ui.actGrad));
    item->setData(ActiveGradient2, variant(ui.actGrad2));
    item->setData(InactiveGradient, variant(ui.inactGrad));
    item->setData(InactiveGradient2, variant(ui.inactGrad2));
    ui.presets->addItem(item);
    connect (ui.newPreset, SIGNAL(clicked()), this, SLOT(createNewPreset()));
    connect (ui.deletePreset, SIGNAL(clicked()), this, SLOT(deleteCurrentPreset()));
    connect (ui.presets, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
              this, SLOT(presetChanged(QListWidgetItem*, QListWidgetItem*)));

    setContextHelp(ui.presets, "<b>Presets</b><hr>\
    The presets are meant to define the look of <i>some</i> <b>exceptional</b> windows <b>not</b>\
    controlled by the widget style (whether native or through gtk-qt).<br>\
    First the class will be matched, then the type. The preset that fits best is chosen. If no match\
    is found, the default preset will be used.<br>\
    <i>Preset changes are <b>not</b> applied to running clients!</i><br>\
    10 presets are no big deal, 1000 will cause a major performance hit.");

    setContextHelp(ui.wmClasses, "<b>Classes</b><hr>\
    Comma separated list of window classes to define which windows the preset will be applied to.<br>\
    If this and the type (below) are <i>both</i> empty, the preset will be ignored.<br>\
    The window class is usually near the application name and can determined by adding the Info button\
    (\"!\") to the multibutton order and pressing it.");

    setContextHelp(ui.wmTypes, "<b>Types</b><hr>\
    Comma separated list of window types to define which windows the preset will be applied to.<br>\
    If this and the class (above) are <i>both</i> empty, the preset will be ignored.<br>\
    Valid entries (atm, case insensitive): \"normal\", \"dialog\" and \"utility\".");

    loadPresets();
    /* ------------------------ */
}

// void Config::load(KConfigGroup) {load();}
void Config::save(KConfigGroup&)
{
    ui.presets->setCurrentRow(0);
    BConfig::save();
    savePresets();
}

void Config::catchClones(QListWidgetItem *item)
{
    bool isClone = false;
    for (int i = 0; i < ui.presets->count(); ++i)
    {
        QListWidgetItem *other = ui.presets->item(i);
        if (isClone = (item != other && item->text() == other->text()))
            break;
    }
    if (isClone)
    {
        item->setText("Allready taken!");
        ui.presets->editItem(item);
    }
    else
        disconnect (ui.presets, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(catchClones(QListWidgetItem*)));
}

void Config::createNewPreset()
{
    QListWidgetItem *item;
    if (ui.presets->currentItem())
    {
        item = ui.presets->currentItem()->clone();
        item->setText("Enter a name");
    }
    else
        item = new QListWidgetItem("Enter a name");
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui.presets->addItem(item);

    connect (ui.presets, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(catchClones(QListWidgetItem*)));
    ui.presets->editItem(item);
}

void Config::presetChanged(QListWidgetItem *item, QListWidgetItem *prev)
{
    if (prev) // store data
    {
        prev->setData(ActiveGradient, variant(ui.actGrad));
        prev->setData(ActiveGradient2, variant(ui.actGrad2));
        prev->setData(InactiveGradient, variant(ui.inactGrad));
        prev->setData(InactiveGradient2, variant(ui.inactGrad2));
        if (ui.presets->row(prev)) // not for the default preset
        {
            prev->setData(ActiveColor, ui.actColor->color().rgba());
            prev->setData(ActiveColor2, ui.actColor2->color().rgba());
            prev->setData(ActiveText, ui.actText->color().rgba());
            prev->setData(ActiveButtons, ui.actButtons->color().rgba());
            prev->setData(InactiveColor, ui.inactColor->color().rgba());
            prev->setData(InactiveColor2, ui.inactColor2->color().rgba());
            prev->setData(InactiveText, ui.inactText->color().rgba());
            prev->setData(InactiveButtons, ui.inactButtons->color().rgba());
            prev->setData(Classes, ui.wmClasses->text());
            prev->setData(Types, ui.wmTypes->text());
        }
    }
    bool enabled = false;
    if (item)
    {
        setVariant(ui.actGrad, item->data(ActiveGradient));
        setVariant(ui.actGrad2, item->data(ActiveGradient2));
        setVariant(ui.inactGrad, item->data(InactiveGradient));
        setVariant(ui.inactGrad2, item->data(InactiveGradient2));
        if (ui.presets->row(item)) // not for the default preset
        {
            enabled = true;
            ui.actColor->setColor(item->data(ActiveColor).toUInt());
            ui.actColor2->setColor(item->data(ActiveColor2).toUInt());
            ui.actText->setColor(item->data(ActiveText).toUInt());
            ui.actButtons->setColor(item->data(ActiveButtons).toUInt());
            ui.inactColor->setColor(item->data(InactiveColor).toUInt());
            ui.inactColor2->setColor(item->data(InactiveColor2).toUInt());
            ui.inactText->setColor(item->data(InactiveText).toUInt());
            ui.inactButtons->setColor(item->data(InactiveButtons).toUInt());
            ui.wmClasses->setText(item->data(Classes).toString());
            ui.wmTypes->setText(item->data(Types).toString());
        }
        else
        {
            ui.wmClasses->setText(QString());
            ui.wmTypes->setText(QString());
        }
    }
    ui.deletePreset->setEnabled(enabled);
    ui.wmClasses->setEnabled(enabled);
    ui.wmTypes->setEnabled(enabled);
    ui.actColor->setEnabled(enabled);
    ui.actColor2->setEnabled(enabled && ui.actGrad2->currentIndex());
    ui.actText->setEnabled(enabled);
    ui.actButtons->setEnabled(enabled);
    ui.inactColor->setEnabled(enabled);
    ui.inactColor2->setEnabled(enabled && ui.inactGrad2->currentIndex());
    ui.inactText->setEnabled(enabled);
    ui.inactButtons->setEnabled(enabled);
}

void Config::deleteCurrentPreset()
{
    if (ui.presets->currentRow() > 0)
        delete ui.presets->currentItem();
}

void Config::loadPresets()
{
    QSettings settings("Bespin", "Style");
    settings.beginGroup("Deco");
    QStringList presets = settings.childGroups();
    foreach (QString preset, presets)
    {
        settings.beginGroup(preset);
        QListWidgetItem *item = new QListWidgetItem(preset);
        item->setData(ActiveGradient, settings.value("ActiveGradient", 0));
        item->setData(ActiveGradient2, settings.value("ActiveGradient2", 0));
        item->setData(ActiveColor, settings.value("ActiveColor", 0));
        item->setData(ActiveColor2, settings.value("ActiveColor2", 0));
        item->setData(ActiveText, settings.value("ActiveText", 0));
        item->setData(ActiveButtons, settings.value("ActiveButtons", 0));

        item->setData(InactiveGradient, settings.value("InactiveGradient", 0));
        item->setData(InactiveGradient2, settings.value("InactiveGradient2", 0));
        item->setData(InactiveColor, settings.value("InactiveColor", 0));
        item->setData(InactiveColor2, settings.value("InactiveColor2", 0));
        item->setData(InactiveText, settings.value("InactiveText", 0));
        item->setData(InactiveButtons, settings.value("InactiveButtons", 0));
        item->setData(Classes, settings.value("Classes", QString()));
        item->setData(Types, settings.value("Types", QString()));
        ui.presets->addItem(item);
        settings.endGroup();
    }
    settings.endGroup();
}

void Config::savePresets()
{
    QSettings settings("Bespin", "Style");
    settings.beginGroup("Deco");
    QStringList presets = settings.childGroups();
    foreach (QString preset, presets)
    {
        settings.beginGroup(preset);
        settings.remove("");
        settings.endGroup();
    }
    for (int i=1; i < ui.presets->count(); ++i)
    {
        QListWidgetItem *item = ui.presets->item(i);
        settings.beginGroup(item->text());
        settings.setValue("ActiveGradient", item->data(ActiveGradient).toInt());
        int gradient = item->data(ActiveGradient2).toInt();
        settings.setValue("ActiveGradient2", gradient);
        QRgb color = item->data(ActiveColor).toUInt();
        settings.setValue("ActiveColor", color);
        settings.setValue("ActiveColor2", gradient ? item->data(ActiveColor2).toUInt() : color);
        settings.setValue("ActiveText", item->data(ActiveText).toUInt());
        settings.setValue("ActiveButtons", item->data(ActiveButtons).toUInt());

        settings.setValue("InactiveGradient", item->data(InactiveGradient).toInt());
        gradient = item->data(InactiveGradient2).toInt();
        settings.setValue("InactiveGradient2", gradient);
        color = item->data(InactiveColor).toUInt();
        settings.setValue("InactiveColor", color);
        settings.setValue("InactiveColor2", gradient ? item->data(InactiveColor2).toUInt() : color);
        settings.setValue("InactiveText", item->data(InactiveText).toUInt());
        settings.setValue("InactiveButtons", item->data(InactiveButtons).toUInt());
        settings.setValue("Classes", item->data(Classes).toString());
        settings.setValue("Types", item->data(Types).toString());
        settings.endGroup();
    }
    settings.endGroup();
}

void Config::watchBgMode()
{
    QWidget *other = 0;
    if (sender() == ui.actGrad)
        other = ui.inactGrad;
    else if (sender() == ui.inactGrad)
        other = ui.actGrad;
    if (other)
        other->setEnabled(variant(sender()).toInt() >= 0);
}

void Config::watchDecoGradient()
{
    if (ui.presets->currentRow() < 1)
        return;
    QWidget *other = 0;
    if (sender() == ui.actGrad2)
        other = ui.actColor2;
    else if (sender() == ui.inactGrad2)
        other = ui.inactColor2;
    if (other)
        other->setEnabled(variant(sender()).toInt() != 0);
}
