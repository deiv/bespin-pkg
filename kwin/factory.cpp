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

#include <QDBusConnection>
#include <QFontMetrics>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QSettings>
#include <QWidgetAction>
#include <QStyleOptionHeader>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <kwindowsystem.h>
// #include "button.h"
#include "client.h"
#include "factory.h"
#include "dbus.h"

#include <QtDebug>

extern "C"
{
KDE_EXPORT KDecorationFactory* create_factory()
{
   return new Bespin::Factory();
}
}

using namespace Bespin;

bool Factory::initialized_ = false;
Config Factory::_config = { false, false, false, {{Gradients::None, Gradients::Button}, {Gradients::None, Gradients::None}} };
int Factory::buttonSize_ = -1;
int Factory::borderSize_ = 4;
int Factory::titleSize_[2] = {18,16};
QVector<Button::Type> Factory::multiButton_(0);
QMenu *Factory::desktopMenu_ = 0;
QMenu *Factory::_windowList = 0;
QTextBrowser *Factory::_windowInfo = 0;

Factory::Factory() : QObject()
{
    readConfig();
    Gradients::init();
    initialized_ = true;
    new BespinDecoAdaptor(this);
//     QDBusConnection::sessionBus().registerService("org.kde.XBar");
    QDBusConnection::sessionBus().registerObject("/BespinDeco", this);
}

Factory::~Factory() { initialized_ = false; Gradients::wipe(); }

KDecoration* Factory::createDecoration(KDecorationBridge* b)
{
    return new Client(b, this);
}

//////////////////////////////////////////////////////////////////////////////
// Reset the handler. Returns true if decorations need to be remade, false if
// only a repaint is necessary

bool Factory::reset(unsigned long changed)
{
	initialized_ = false;
	const bool configChanged = readConfig();
   initialized_ = true;

	if (configChanged ||
		(changed & (SettingDecoration | SettingButtons | SettingBorder))) {
		return true;
	}
	else {
		resetDecorations(changed);
		return false;
	}
}

static void
multiVector(const QString & string, QVector<Button::Type> &vector)
{
   Button::Type type; vector.clear();
   for (int i = 0; i < string.length(); ++i) {
      switch (string.at(i).toAscii()) {
         case 'M': type = Button::Menu; break;
         case 'S': type = Button::Stick; break;
         case 'H': type = Button::Help; break;
         case 'F': type = Button::Above; break;
         case 'B': type = Button::Below; break;
         case 'L': type = Button::Shade; break;
         case 'E': type = Button::Exposee; break;
         case '!': type = Button::Info; break;
         default: continue;
      }
      vector.append(type);
   }
}

static QString
multiString(const QVector<Button::Type> &vector)
{
   QString string; char c;
   for (int i = 0; i < vector.size(); ++i) {
      switch (vector.at(i)) {
         case Button::Menu: c = 'M'; break;
         case Button::Stick: c = 'S'; break;
         case Button::Help: c = 'H'; break;
         case Button::Above: c = 'F'; break;
         case Button::Below: c = 'B'; break;
         case Button::Shade: c = 'L'; break;
         case Button::Exposee: c = 'E'; break;
         case Button::Info: c = '!'; break;
         default: continue;
      }
      string.append(c);
   }
   return string;
}

bool Factory::readConfig()
{
    bool ret = false;
    bool oldBool;
    Gradients::Type oldgradient;
    
    QSettings settings("Bespin", "Style");
    settings.beginGroup("Deco");

    oldBool = _config.forceUserColors;
    _config.forceUserColors = settings.value("ForceUserColors", false).toBool();
    if (oldBool != _config.forceUserColors) ret = true;

    oldBool = _config.trimmCaption;
    _config.trimmCaption = settings.value("TrimmCaption", true).toBool();
    if (oldBool != _config.trimmCaption) ret = true;

    oldBool = _config.resizeCorner;
    _config.resizeCorner = settings.value("ResizeCorner", false).toBool();
    if (oldBool != _config.resizeCorner) ret = true;

    oldgradient = _config.gradient[0][0];
    _config.gradient[0][0] = (Gradients::Type)(settings.value("InactiveGradient", 0).toInt());
    if (oldgradient != _config.gradient[0][0]) ret = true;

    oldgradient = _config.gradient[0][1];
    _config.gradient[0][1] = (Gradients::Type)(settings.value("ActiveGradient", 2).toInt());
    if (oldgradient != _config.gradient[0][1]) ret = true;

    oldgradient = _config.gradient[1][0];
    _config.gradient[1][0] = Gradients::fromInfo(settings.value("InactiveGradient2", 0).toInt());
    if (oldgradient != _config.gradient[1][0]) ret = true;
    
    oldgradient = _config.gradient[1][1];
    _config.gradient[1][1] = Gradients::fromInfo(settings.value("ActiveGradient2", 0).toInt());
    if (oldgradient != _config.gradient[1][1]) ret = true;

    QString oldmultiorder = multiString(multiButton_);
    QString newmultiorder = settings.value("MultiButtonOrder", "MHFBSLE!").toString();
    if (oldmultiorder != newmultiorder)
    {
        ret = true;
        multiVector(newmultiorder, multiButton_);
    }

    int oldbordersize = borderSize_;
    switch (options()->preferredBorderSize(this))
    {
        case BorderTiny: borderSize_ = 0; break;
        default:
        case BorderNormal: borderSize_ = 6; break;
        case BorderLarge: borderSize_ = 8; break;
        case BorderVeryLarge: borderSize_ = 11; break;
        case BorderHuge: borderSize_ = 16; break;
        case BorderVeryHuge: borderSize_ = 20; break;
        case BorderOversized: borderSize_ = 30; break;
    }
    if (oldbordersize != borderSize_) ret = true;

    int oldtitlesize = titleSize_[1];
    QFontMetrics fm(options()->font());
    titleSize_[1] = fm.height() + 2;
    if (oldtitlesize != titleSize_[1]) ret = true;
    oldtitlesize = titleSize_[0];
    titleSize_[0] = qMax(titleSize_[1] + 2, borderSize_);
    if (oldtitlesize != titleSize_[0]) ret = true;

    if (buttonSize_ != titleSize_[1])
    {
        buttonSize_ = titleSize_[1]-4; // for the moment
        Button::init( buttonSize_, options()->titleButtonsLeft().contains(QRegExp("(M|S|H|F|B|L)")),
                                                settings.value("IAmMyLittleSister", false).toBool());
    }

    return ret;
}

class Header : public QLabel
{
public:
    Header(const QString & title, QWidget *parent = 0) : QLabel(title, parent)
    {
        QFont font; font.setBold(false); setFont(font);
    }
protected:
    void paintEvent(QPaintEvent *pe)
    {
        QStyleOptionHeader opt; opt.initFrom(this);
        opt.textAlignment = Qt::AlignCenter;
        opt.text = text();
        QPainter p(this);
        style()->drawControl(QStyle::CE_Header, &opt, &p, this );
        p.end();
    }
private:
};

void
Factory::showDesktopMenu(const QPoint &p, Client *client)
{
//    static void KWindowSystem::setCurrentDesktop( int desktop );
    if (!client) return;
    if (!desktopMenu_)
        desktopMenu_ = new QMenu();
    else
        desktopMenu_->clear();

    QWidgetAction *headerAct = new QWidgetAction(desktopMenu_);
    headerAct->setDefaultWidget(new Header("Throw on:"));
    desktopMenu_->addAction(headerAct);

    QAction *act = 0;
    for (int i = 1; i <= KWindowSystem::numberOfDesktops(); ++i)
    {
        act = desktopMenu_->addAction ( "Desktop #" + QString::number(i), client, SLOT(throwOnDesktop()) );
        act->setData(i);
        act->setDisabled(i == KWindowSystem::currentDesktop());
    }
    desktopMenu_->popup(p);
}

void
Factory::showWindowList(const QPoint &p, Client *client)
{
    if (!_windowList)
        _windowList = new QMenu();
    else
        _windowList->clear();

    QWidgetAction *headerAct = new QWidgetAction(_windowList);
    headerAct->setDefaultWidget(new Header("Windows"));
    _windowList->addAction(headerAct);

    const QList<WId>& windows = KWindowSystem::windows();

    QAction *act = 0;
    KWindowInfo info; QString title;
#define NET_FLAGS NET::WMVisibleName | NET::WMWindowType | NET::WMDesktop | NET::WMState | NET::XAWMState
    foreach (WId id, windows)
    {
        info = KWindowInfo(id, NET_FLAGS, 0);
        if (info.windowType( NET::NormalMask | NET::DialogMask | NET::UtilityMask ) != -1)
        {
            title = info.visibleIconName();
            if (info.isMinimized())
                title = "( " + title + " )";
            if (!info.isOnCurrentDesktop())
                title = "< " + title + " >";
            if (title.length() > 52)
                title = title.left(22) + "..." + title.right(22);
            act = _windowList->addAction ( title, client, SLOT(activate()) );
            act->setData((uint)id);
            act->setDisabled(id == KWindowSystem::activeWindow());
        }
    }
    _windowList->popup(p);
}


static QString
winType2string(NET::WindowType type)
{
   switch (type) {
   default:
   case NET::Unknown: return "Unknown";
   case NET::Normal: return "Normal";
   case NET::Desktop: return "Desktop";
   case NET::Dock: return "Dock";
   case NET::Toolbar: return "Toolbar";
   case NET::Menu: return "Menu";
   case NET::Dialog: return "Dialog";
   case NET::Override: return "Override";
   case NET::TopMenu: return "TopMenu";
   case NET::Utility: return "Utility";
   case NET::Splash: return "Splash";
   case NET::DropdownMenu: return "DropdownMenu";
   case NET::PopupMenu: return "PopupMenu";
   case NET::Tooltip: return "Tooltip";
   case NET::Notification: return "Notification";
   case NET::ComboBox: return "ComboBox";
   case NET::DNDIcon: return "DNDIcon";
   }
}

void
Factory::showInfo(const QPoint &p, WId id) {

   // build info widget - in case
   if (!_windowInfo) {
      QWidget *window = new QWidget(0, Qt::Popup);
      QVBoxLayout *l = new QVBoxLayout(window);
      l->setContentsMargins ( 6, 2, 6, 10 );
      l->setSpacing(0);
      Header *header = new Header("Window Information", window);
      l->addWidget(header);
      _windowInfo = new QTextBrowser(window);

      _windowInfo->viewport()->setAutoFillBackground(false);
      _windowInfo->viewport()->setBackgroundRole(QPalette::Window);
      _windowInfo->viewport()->setForegroundRole(QPalette::WindowText);
      _windowInfo->setFrameStyle(QFrame::NoFrame);

      _windowInfo->setFontFamily ( "fixed" );
      QString css("h1 { font-size:large; margin-top:10px; margin-bottom:4px; }");
      _windowInfo->document()->setDefaultStyleSheet ( css );

      l->addWidget(_windowInfo);
      window->resize(255, 453);
   }
   else
      _windowInfo->clear();

   // fill with info
   KWindowInfo info( id,
                     NET::WMState | NET::WMWindowType | NET::WMVisibleName | NET::WMName |
                     NET::WMVisibleIconName | NET::WMIconName | NET::WMDesktop | NET::WMGeometry |
                     NET::WMFrameExtents,
                     NET::WM2TransientFor | NET::WM2GroupLeader | NET::WM2WindowClass |
                     NET::WM2WindowRole | NET::WM2ClientMachine | NET::WM2AllowedActions );
/*
   unsigned long state() const;
   WId transientFor() const; //the mainwindow for this window.
   WId groupLeader() const;
   QByteArray windowRole() const;
   bool actionSupported( NET::Action action ) const;
*/
   QString text("\
<h1 align=center>Identification</h1>\
WId      : <b>%1 %2 %3</b><br/>\
Name     : <b>%4</b>" );
   text = text.arg(QString::number(ulong(info.win()))).
               arg(QString::number(long(info.win()))).
               arg(QString::number(ulong(info.win()), 16)).
               arg(info.name());

   if (info.visibleName() != info.name())
      text.append("<br/>Name V  : <b>%1</b>").arg(info.visibleName());
   if (info.iconName() != info.name())
      text.append("<br/>Iconic  : <b>%1</b>").arg(info.iconName());
   if (info.iconName() != info.visibleIconName())
      text.append("<br/>Iconic V: <b>%1</b>").arg(info.visibleIconName());

   text.append("\
<hr><h1 align=center>Geometry</h1>\
    X    : <b>%1 (%5)</b><br/>\
    Y    : <b>%2 (%6)</b><br/>\
Width    : <b>%3 (%7)</b><br/>\
Height   : <b>%4 (%8)</b>");
   text = text.arg(info.geometry().x()).
               arg(info.geometry().y()).
               arg(info.geometry().width()).
               arg(info.geometry().height()).
               arg(info.frameGeometry().x()).
               arg(info.frameGeometry().y()).
               arg(info.frameGeometry().width()).
               arg(info.frameGeometry().height());

   text.append("\
<hr><h1 align=center>Location</h1>\
Desktop  : <b>%1 %2</b><br/>\
Machine  : <b>%3</b>\
<hr><h1 align=center>Properties</h1>\
Type     : <b>%4 (%5)</b><br/>\
Class    : <b>%6</b><br/>\
ClassName: <b>%7</b><br/>\
");
   NET::WindowType type = info.windowType( NET::AllTypesMask );
   text = text.arg(info.desktop()).
               arg(info.onAllDesktops() ? "(Sticked)" : (info.isOnCurrentDesktop() ? "(Current)" : "")).
               arg(QString(info.clientMachine())).
               arg(winType2string(type)).arg(type).
               arg(QString(info.windowClassClass())).
               arg(QString(info.windowClassName()));

   _windowInfo->setHtml( text );

   // and show up
   QWidget *win = _windowInfo->parentWidget();
//    QPoint ip = p;
//    if (ip.x() + 640 > )
   win->move(p);
   win->show();
}

bool
Factory::supports( Ability ability ) const
{
	switch( ability ) {
	// announce
	case AbilityAnnounceButtons: ///< decoration supports AbilityButton* values (always use)
	case AbilityAnnounceColors: ///< decoration supports AbilityColor* values (always use)
	// buttons
	case AbilityButtonMenu:   ///< decoration supports the menu button
	case AbilityButtonOnAllDesktops: ///< decoration supports the on all desktops button
	case AbilityButtonSpacer: ///< decoration supports inserting spacers between buttons
	case AbilityButtonHelp:   ///< decoration supports what's this help button
	case AbilityButtonMinimize:  ///< decoration supports a minimize button
	case AbilityButtonMaximize: ///< decoration supports a maximize button
	case AbilityButtonClose: ///< decoration supports a close button
	case AbilityButtonAboveOthers: ///< decoration supports an above button
	case AbilityButtonBelowOthers: ///< decoration supports a below button
   case AbilityButtonShade: ///< decoration supports a shade button

	// colors
	case AbilityColorTitleBack: ///< decoration supports titlebar background color
	case AbilityColorTitleFore: ///< decoration supports titlebar foreground color
	case AbilityColorTitleBlend: ///< decoration supports second titlebar background color
	case AbilityColorButtonBack: ///< decoration supports button background color
		return true;
   case AbilityColorButtonFore: ///< decoration supports button foreground color
   case AbilityColorFrame: ///< decoration supports frame color
   case AbilityButtonResize: ///< decoration supports a resize button
   case AbilityColorHandle: ///< decoration supports resize handle color
	default:
		return false;
	};
}

void
Factory::learn(qint64 pid, uint bgColors, uint activeColors, uint inactiveColors)
{
    forget(pid);
    DecoInfo *info = new DecoInfo;
    info->bgColors = bgColors;
    info->activeColors = activeColors;
    info->inactiveColors = inactiveColors;
    _decoInfos.insert(pid, info);
}

void
Factory::forget(qint64 pid)
{
    QMap<qint64, DecoInfo*>::iterator i = _decoInfos.find(pid);
    if (i == _decoInfos.end())
        return;
    
    delete i.value(); i.value() = 0;
    _decoInfos.erase(i);
}

bool
Factory::hasDecoInfo(qint64 pid, uint &bgColors, uint &activeColors, uint &inactiveColors)
{
    QMap<qint64, DecoInfo*>::iterator i = _decoInfos.find(pid);
    if (i == _decoInfos.end())
        return false;
    bgColors = i.value()->bgColors;
    activeColors = i.value()->activeColors;
    inactiveColors = i.value()->inactiveColors;
    return true;
}

#include "dbus.moc"
