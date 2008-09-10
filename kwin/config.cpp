//////////////////////////////////////////////////////////////////////////////
//
// -------------------
// Bespin window decoration for KDE.
// -------------------
// Copyright (c) 2008 Thomas Lübking <baghira-style@gmx.net>
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
#include <QtDebug>

class KConfig;

extern "C"
{
    Q_DECL_EXPORT QObject* allocate_config(KConfig* config, QWidget* parent) {
        return (new Config(parent));
    }
}

Config::Config(QWidget* parent) : BConfig(parent)
{
    ui.setupUi(this);
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

    handleSettings(ui.actGrad, "ActiveGradient", 1);
    setContextHelp(ui.actGrad, "<b>Active Gradient</b><hr>\
    The gradient in the titlebar center of ACTIVE windows.");

    handleSettings(ui.inactGrad, "InactiveGradient", 0);
    setContextHelp(ui.inactGrad, "<b>Inactive Gradient</b><hr>\
    The gradient in the titlebar center of INACTIVE windows.");

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
    <b>_</b>: Some space");

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
}

// void Config::load(KConfigGroup) {load();}
void Config::save(KConfigGroup&) {BConfig::save();}
