/* Bespin mac-a-like XBar KDE4
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
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDesktopWidget>
#include <QDomElement>
#include <QFile>
#include <QGraphicsLinearLayout>
#include <QGraphicsScene>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsView>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include <QRectF>
#include <QSettings>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>

#include <kglobalsettings.h>
#include <kwindowsystem.h>
#include <KDirWatch>
#include <KIcon>
#include <KStandardDirs>
#include <KUriFilterData>
#include <KRun>

#include <Plasma/Containment>
#include <Plasma/Theme>

#include <limits.h>

// #include "button.h"
#include "menubar.h"
#include "xbar.h"
#include "dbus.h"

#include <QtDebug>

static XBar *instance = NULL;

class DummyWidget : public QWidget
{
public:
    DummyWidget( QWidget * parent = 0, Qt::WindowFlags f = 0) : QWidget(parent, Qt::X11BypassWindowManagerHint)
    {
        Q_UNUSED(f);
    }
protected:
    void paintEvent(QPaintEvent *)
    {
        if (instance && instance->d.currentBar)
            instance->d.currentBar->update();
    }
};

static DummyWidget *dummy = NULL;

QTimer XBar::bodyCleaner;

XBar::XBar(QObject *parent, const QVariantList &args) : Plasma::Applet(parent, args)
{
    myMainMenu = 0;
    myMainMenuDefWatcher = 0;
    d.currentBar = 0; // important!
    dummy = 0;
    if (instance)
    {
        QMessageBox::warning ( 0, "Multiple XBar requests", "XBar shall be unique dummy text");
        qWarning("XBar, Do NOT load XBar more than once!");
        deleteLater();
    }
    else
    {
        instance = this;
    }
}

XBar::~XBar()
{
    if (instance == this)
    {
        byeMenus();
        instance = NULL;
        delete dummy; dummy = NULL;
    }
}

void
XBar::init()
{
    if (this != instance)
        return;
    if (!view())
    {
        QTimer::singleShot(100, this, SLOT(init()));
        return;
    }

    if (QGraphicsLinearLayout *lLayout = dynamic_cast<QGraphicsLinearLayout*>(containment()->layout()))
    {
        lLayout->setStretchFactor(this, 1000);
        lLayout->setAlignment( this, Qt::AlignLeft|Qt::AlignVCenter );
    }
    
//     if (!view()->inherits("PanelView"))
//     {
//         QMessageBox::warning ( 0, "XBar requires a Panel", "XBar shall be on panels dummy text");
//         qWarning("XBar, Do NOT use XBar on Desktop widgets!");
//         deleteLater();
//         return;
//     }
    dummy = new DummyWidget();
    dummy->setGeometry(5000,5000,1,1);
    dummy->show();
    
//     Plasma::Applet::init();
    //TODO : Qt's bug??
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding));
    setMaximumSize(INT_MAX, INT_MAX);

//     setFlag(ItemClipsChildrenToShape); setFlag(ItemClipsToShape);
    
    // TODO: use plasmoid popup and make this dynamic -> update all menubars...
    QSettings settings("Bespin", "XBar");
    settings.beginGroup("XBar");
    float scale = settings.value("FontScale", 1.0f).toFloat();
    if (scale > 0.0 && scale != 1.0)
    {
        myFont = KGlobalSettings::menuFont();
        myFont.setPointSize(scale*myFont.pointSize());
        setFont(myFont);
    }
    d.extraTitle = false; //settings.value("WindowList", false).toBool();

    repopulateMainMenu();
    
    d.currentBar = myMainMenu;

    updatePalette();
    
    show(myMainMenu);
    
    new XBarAdaptor(this);
    QDBusConnection::sessionBus().registerService("org.kde.XBar");
    QDBusConnection::sessionBus().registerObject("/XBar", this);
    
    connect (this, SIGNAL(destroyed()), this, SLOT(byeMenus()));
    connect (qApp, SIGNAL(aboutToQuit()), this, SLOT(byeMenus()));
    connect (&d.windowList, SIGNAL(aboutToShow()), this, SLOT(updateWindowlist()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updatePalette()));
    connect (&bodyCleaner, SIGNAL(timeout()), this, SLOT(cleanBodies()));
    bodyCleaner.start(30000); // every 5 minutes - it's just to clean menus from crashed windows, so users won't constantly scroll them
    callMenus();
}

void
XBar::updatePalette()
{
    QColor fg = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QColor bg = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    QPalette pal(fg, bg, Qt::white, Qt::black, Qt::gray, fg, fg, bg, bg );
    pal.setColor(QPalette::ButtonText, fg);
    setPalette(pal);
    myMainMenu->setPalette(pal);
    foreach (MenuBar *menu, d.menus)
        menu->setPalette(pal);
    d.windowList.setPalette(QApplication::palette());
}

void
XBar::activateWin()
{
    if (QAction *act = qobject_cast<QAction*>(sender()))
    {
        bool ok;
        int id = act->data().toUInt(&ok);
        if (ok)
            KWindowSystem::activateWindow( id );
    }
}

void
XBar::byeMenus()
{
   QDBusConnectionInterface *session = QDBusConnection::sessionBus().interface();
   QStringList services = session->registeredServiceNames();
   foreach (QString service, services) {
      if (service.startsWith("org.kde.XBar-")) {
         QDBusInterface interface( service, "/XBarClient", "org.kde.XBarClient" );
         if (interface.isValid())
            interface.call("deactivate");
      }
   }
}

void
XBar::callMenus()
{
   QDBusConnectionInterface *session = QDBusConnection::sessionBus().interface();
   QStringList services = session->registeredServiceNames();

   foreach (QString service, services) {
      if (service.startsWith("org.kde.XBar-")) {
         QDBusInterface interface( service, "/XBarClient", "org.kde.XBarClient" );
         if (interface.isValid()) {
            interface.call("activate");
         }
      }
   }
}

void
XBar::changeEntry(qlonglong key, int idx, const QString &entry, bool add)
{
    MenuMap::iterator i = d.menus.find( key );
    if (i == d.menus.end())
        return;

    MenuBar *bar = i.value();
    if (entry.isNull())
    {   // delete
        if (idx < 0)
            return;
        bar->removeAction(idx + d.extraTitle);
    }
    else if (add)
        bar->addAction(entry, idx < 0 ? -1 : idx + d.extraTitle);
    else
    {
        if (idx < 0)
            return;
        bar->changeAction(idx + d.extraTitle, entry);
    }
}

void
XBar::cleanBodies()
{
    QDBusConnectionInterface *session = QDBusConnection::sessionBus().interface();
    QStringList services = session->registeredServiceNames();
    services = services.filter(QRegExp("^org\\.kde\\.XBar-"));
    MenuMap::iterator i = d.menus.begin();
    MenuBar *mBar = 0;
    while (i != d.menus.end())
    {
        if (services.contains(i.value()->service()))
            ++i;
        else
        {
            mBar = i.value();
            i = d.menus.erase(i);
            delete mBar;
        }
    }
}

bool
XBar::dbusAction(const QObject *o, int idx, const QString &cmd)
{
    const MenuBar *mBar = qobject_cast<const MenuBar*>(o);
    if (!mBar)
        return false; // that's not our business!
    if (idx > -1)
    {
        QAction *act = mBar->action(idx);
        if (!act || act->menu())
            return false; // that's not our business!
    }

    QPoint pt = mapToGlobal(mBar->actionGeometry(idx).bottomLeft() + mBar->pos());

    QDBusInterface interface( mBar->service(), "/XBarClient", "org.kde.XBarClient" );
    if (interface.isValid())
    {
        if (idx < 0)
            interface.call(cmd, mBar->key());
        else
            interface.call(cmd, mBar->key(), idx - d.extraTitle, pt.x(), pt.y());
    }
    return true;
}

void
XBar::hide(MenuBar *item)
{
    item->hide();
}

void
XBar::hover(int idx)
{
    dbusAction(sender(), idx, "hover");
}

QPoint
XBar::mapToGlobal(const QPointF &pt)
{
    return view()->mapToGlobal(view()->mapFromScene(mapToScene(pt)));
}

void
XBar::raiseCurrentWindow()
{
    if (!d.currentBar || d.currentBar == myMainMenu)
        return; // nothing to raise...
    dbusAction(d.currentBar, -1, "raise");
}

void
XBar::registerMenu(const QString &service, qlonglong key, const QString &title, const QStringList &entries)
{
    MenuBar *newBar = new MenuBar(service, key, this, dummy);
    newBar->setAppTitle(title);
    newBar->setPalette(palette());
    newBar->setFont(myFont);
    connect (newBar, SIGNAL(hovered(int)), this, SLOT(hover(int)));
    connect (newBar, SIGNAL(triggered(int)), this, SLOT(trigger(int)));

    if (d.extraTitle)
    {   // the windowlist entry
        QAction *act = newBar->addAction( title );
        act->setMenu(&d.windowList);
    }

    // the demanded menu entries
    foreach (QString entry, entries)
        newBar->addAction(entry);


    // replace older versions - in case
    delete d.menus.take( key );
    d.menus.insert( key, newBar );

    // add hidden
    newBar->hide();
}

void
XBar::releaseFocus(qlonglong key)
{
    int n = 0;
    for (MenuMap::iterator i = d.menus.begin(); i != d.menus.end(); ++i)
    {
        if (i.key() == key)
            hide(i.value());
        else
            n += i.value()->isVisible();
    }
    if (!n)
    {
        d.currentBar = myMainMenu;
        show(myMainMenu);
    }
}

void
XBar::reparent(qlonglong oldKey, qlonglong newKey)
{
    MenuMap::iterator i = d.menus.find( oldKey );
    if (i == d.menus.end())
        return;
    MenuBar *bar = i.value();
    d.menus.erase(i);
    d.menus.insert(newKey, bar);
}

#define LABEL_ERROR "missing \"label\" attribute"
#define MENU_FUNC(_FUNC_) menu ? menu->_FUNC_ : menubar->_FUNC_

void
XBar::runFromAction()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
        return;
    const QString &command = action->data().toString();
    KUriFilterData execLineData( command );
    KUriFilter::self()->filterUri( execLineData, QStringList() << "kurisearchfilter" << "kshorturifilter" );
    QString cmd = ( execLineData.uri().isLocalFile() ? execLineData.uri().path() : execLineData.uri().url() );
    
    if ( cmd.isEmpty() )
        return;
    
    switch( execLineData.uriType() )
    {
        case KUriFilterData::LocalFile:
        case KUriFilterData::LocalDir:
        case KUriFilterData::NetProtocol:
        case KUriFilterData::Help:
        {
            new KRun( execLineData.uri(), 0 );
            break;
        }
        case KUriFilterData::Executable:
        case KUriFilterData::Shell:
        {
            QString args = cmd;
            if( execLineData.hasArgsAndOptions() )
                cmd += execLineData.argsAndOptions();
            KRun::runCommand( cmd, args, "", 0 );
            break;
        }
        case KUriFilterData::Unknown:
        case KUriFilterData::Error:
        default:
            break;
    }
}

void
XBar::callFromAction()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
        return;
    const QString &instruction = action->data().toString();
    QStringList list = instruction.split(';');
    if (list.count() < 5)
    {
        qWarning("invalid dbus chain, must be: \"bus;service;path;interface;method[;arg1;arg2;...]\", bus is \"session\" or \"system\"");
        return;
    }
    
    QDBusInterface *caller = 0;
    if (list.at(0) == "session")
        caller = new QDBusInterface( list.at(1), list.at(2), list.at(3), QDBusConnection::sessionBus() );
    else if (list.at(0) == "system")
        caller = new QDBusInterface( list.at(1), list.at(2), list.at(3), QDBusConnection::systemBus() );
    else
    {
        qWarning("unknown bus, must be \"session\" or \"system\"");
        return;
    }
    
    QList<QVariant> args;
    if (list.count() > 5)
    {
        for (int i = 5; i < list.count(); ++i)
        {
            bool ok = false;
            short Short = list.at(i).toShort(&ok);
            if (ok) { args << Short; continue; }
            unsigned short UShort = list.at(i).toUShort(&ok);
            if (ok) { args << UShort; continue; }
            int Int = list.at(i).toInt(&ok);
            if (ok) { args << Int; continue; }
            uint UInt = list.at(i).toUInt(&ok);
            if (ok) { args << UInt; continue; }
            double Double = list.at(i).toDouble(&ok);
            if (ok) { args << Double; continue; }
            
            args << list.at(i);
        }
    }
    caller->asyncCallWithArgumentList(list.at(4), args);
    delete caller;
}

void
XBar::rBuildMenu(const QDomElement &node, QObject *widget)
{
    MenuBar *menubar = 0;
    QMenu *menu = qobject_cast<QMenu*>(widget);
    if (!menu)
    {
        menubar = qobject_cast<MenuBar*>(widget);
        if (!menubar)
            return;
    }
    
    QDomNode kid = node.firstChild();
    while(!kid.isNull())
    {
        QDomElement e = kid.toElement(); // try to convert the node to an element.
        if(!e.isNull())
        {
            if (e.tagName() == "menu")
            {
                QString type = e.attribute("menu");
                if (!type.isEmpty())
                    buildMenu(type, widget, "submenu");
                else
                {
                    QMenu *newMenu = MENU_FUNC(addMenu(e.attribute("label", LABEL_ERROR)));
                    rBuildMenu(e, newMenu);
                }
            }
            else if (e.tagName() == "action")
            {
                QAction *action = new QAction(widget);
                QString cmd = e.attribute("dbus");
                if (!cmd.isEmpty())
                    connect ( action, SIGNAL(triggered()), SLOT(callFromAction()) );
                else
                {
                    cmd = e.attribute("exec");
                    if (cmd.isEmpty())
                    {
                        cmd = KGlobal::dirs()->locate("services", e.attribute("service") + ".desktop");
                        if (!cmd.isEmpty())
                        {
                            KService kservice(cmd);
                            action->setIcon(KIcon(kservice.icon()));
                            action->setText(kservice.name());
                            cmd = kservice.desktopEntryName();
                        }
                    }
                    if (!cmd.isEmpty())
                        connect ( action, SIGNAL(triggered()), SLOT(runFromAction()) );
                    else
                        qWarning("MainMenu action without effect, add \"dbus\" or \"exec\" attribute!");
                }
                action->setData(cmd);
                if (action->text().isEmpty())
                    action->setText(e.attribute("label", LABEL_ERROR));
                QString icn = e.attribute("icon");
                if (!icn.isEmpty())
                    action->setIcon(KIcon(icn));
                MENU_FUNC(addAction(action));
            }
            else if (e.tagName() == "separator")
                MENU_FUNC(addSeparator());
        }
        kid = kid.nextSibling();
    }
}


void
XBar::buildMenu(const QString &name, QObject *widget, const QString &type)
{
    if (!instance)
        return;
    
    QDomDocument menu(name);
    QFile file(KGlobal::dirs()->locate("data", "XBar/" + name + ".xml"));
    if (!file.open(QIODevice::ReadOnly))
        return;
    if (!menu.setContent(&file))
    {
        file.close();
        return;
    }
    file.close();
    
    QDomElement element = menu.documentElement();
    if (!element.isNull() /*&& element.tagName() == type*/)
        instance->rBuildMenu(element, widget);
}

void
XBar::repopulateMainMenu()
{
    if (d.currentBar == myMainMenu)
        d.currentBar = 0;
    delete myMainMenu;
    myMainMenu = new MenuBar("", 0, this, dummy);
    myMainMenu->setFont(myFont);
    myMainMenu->setAppTitle("Plasma");
    myMainMenu->addAction("Plasma",-1, &d.windowList);

    delete myMainMenuDefWatcher;

    buildMenu("MainMenu", myMainMenu, "menubar");

    myMainMenuDefWatcher = new KDirWatch(this);
    myMainMenuDefWatcher->addFile(KGlobal::dirs()->locate("data", "XBar/MainMenu.xml"));
    connect( myMainMenuDefWatcher, SIGNAL(created(const QString &)), this, SLOT(repopulateMainMenu()) );
    connect( myMainMenuDefWatcher, SIGNAL(deleted(const QString &)), this, SLOT(repopulateMainMenu()) );
    connect( myMainMenuDefWatcher, SIGNAL(dirty(const QString &)), this, SLOT(repopulateMainMenu()) );

    if (d.currentBar)
        myMainMenu->hide();
    else
        d.currentBar = myMainMenu;
}

void
XBar::requestFocus(qlonglong key)
{
    for (MenuMap::iterator i = d.menus.begin(); i != d.menus.end(); ++i)
    {
        if (i.key() == key)
        {
            hide(myMainMenu);
            show(i.value());
        }
        else
            hide(i.value());
    }
}

void
XBar::setOpenPopup(int idx)
{
    if (d.currentBar && d.currentBar != myMainMenu)
    {
        d.currentBar->setOpenPopup(idx + d.extraTitle);
        d.currentBar->update();
    }
}

QSizeF
XBar::sizeHint ( Qt::SizeHint which, const QSizeF & constraint ) const
{
    if (d.currentBar)
        return d.currentBar->sizeHint(which, constraint);
    return QSizeF(1,1);
}

void
XBar::show(MenuBar *item)
{
    d.currentBar = item;
    int dy = (contentsRect().height() - item->rect().height())/2;
    item->setPos(contentsRect().x(), contentsRect().y()+dy);
    item->show();
}

void
XBar::showMainMenu()
{
    foreach (MenuBar *menu, d.menus)
        hide(menu);

    d.currentBar = myMainMenu;
    show(myMainMenu);
    if (view())
        view()->activateWindow();
    update();
}


void
XBar::trigger(int idx)
{
    dbusAction(sender(), idx, "popup");
}

void
XBar::unregisterMenu(qlonglong key)
{
    releaseFocus(key);
    delete d.menus.take( key );
}

void
XBar::unregisterCurrentMenu()
{
    if (!d.currentBar || d.currentBar == myMainMenu)
        return;
    qlonglong key = d.menus.key(d.currentBar, 0);
    if (key)
    {
        QDBusInterface interface( d.currentBar->service(), "/XBarClient", "org.kde.XBarClient" );
        if (interface.isValid())
            interface.call("deactivate");
        unregisterMenu(key);
    }
}


void
XBar::updateWindowlist()
{
    d.windowList.clear();

    d.windowList.addAction ( "Raise Window", this, SLOT(raiseCurrentWindow()) );
    d.windowList.addSeparator();
//    d.windowList.addAction ( "Show Taskbar", this, SLOT(showTaskbar()) );
//    d.windowList.addSeparator();
   
    const QList<WId>& windows = KWindowSystem::windows();
    QAction *act = 0;
    KWindowInfo info; QString title;
    #define NET_FLAGS NET::WMVisibleIconName | NET::WMWindowType | NET::WMDesktop | NET::WMState | NET::XAWMState

    foreach (WId id, windows)
    {
        info = KWindowInfo(id, NET_FLAGS, 0);
        if (info.windowType( NET::NormalMask | NET::DialogMask | NET::UtilityMask ) != -1) {
            title = info.visibleIconName();
            if (info.isMinimized())
                title = "( " + title + " )";
            if (!info.isOnCurrentDesktop())
                title = "< " + title + " >";
            if (title.length() > 52)
                title = title.left(22) + "..." + title.right(22);
            act = d.windowList.addAction ( title, this, SLOT(activateWin()) );
            act->setData((uint)id);
            act->setDisabled(id == KWindowSystem::activeWindow());
        }
    }
    d.windowList.setTitle("Windows");
    d.windowList.addSeparator();
    d.windowList.addAction ( "Embed menu in window", this, SLOT(unregisterCurrentMenu()) );
}

void
XBar::wheelEvent(QGraphicsSceneWheelEvent *ev)
{
    if (d.menus.isEmpty())
        return;

    if (view())
        view()->activateWindow();

    MenuMap::iterator n;

    if (d.currentBar == myMainMenu)
    {
        hide(myMainMenu);
        if (ev->delta() < 0)
            n = d.menus.begin();
        else
            { n = d.menus.end(); --n; }
    }
    else {
        n = d.menus.end();
        MenuMap::iterator i = d.menus.end();
        for (i = d.menus.begin(); i != d.menus.end(); ++i)
        {
            hide(i.value());
            if (i.value() == d.currentBar)
            {
                if (ev->delta() < 0)
                    n = i+1;
                else if (i == d.menus.begin())
                    n = d.menus.end();
                else
                    n = i-1;
            }
        }
    }
    if (n == d.menus.end())
        show(myMainMenu);
    else
        show(n.value());
}

K_EXPORT_PLASMA_APPLET(xbar, XBar)

#include "dbus.moc"
#include "xbar.moc"
