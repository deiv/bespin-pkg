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
#include <QGraphicsLinearLayout>
#include <QGraphicsScene>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include <QRectF>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>

#include <kwindowsystem.h>

#include <Plasma/Theme>

// #include "button.h"
#include "menubar.h"
#include "taskbar.h"
#include "xbar.h"
#include "dbus.h"

#include <QtDebug>

static XBar *instance = NULL;

class DummyWidget : public QWidget
{
public:
    DummyWidget( QWidget * parent = 0, Qt::WindowFlags f = 0 ) : QWidget(parent, Qt::X11BypassWindowManagerHint) {}
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
    d.taskbar = 0;
    d.currentBar = 0; // important!
    dummy = 0;
    if (instance)
    {
//         QMessageBox::warning ( 0, "Multiple XBar requests", "XBar shall be unique dummy text");
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
        instance = NULL;
        delete dummy; dummy = NULL;
        byeMenus();
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
    if (!view()->inherits("PanelView"))
    {
//         QMessageBox::warning ( 0, "XBar requires a Panel", "XBar shall be on panels dummy text");
        qWarning("XBar, Do NOT use XBar on Desktop widgets!");
        deleteLater();
        return;
    }
    dummy = new DummyWidget();
    dummy->setGeometry(5000,5000,1,1);
    dummy->show();
    Plasma::Applet::init();
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMaximumSize(INT_MAX, INT_MAX);
//     setFlag(ItemClipsChildrenToShape); setFlag(ItemClipsToShape);
    
    d.taskbar = new TaskBar(this, dummy);
    d.currentBar = d.taskbar;
    d.extraTitle = false;
    
    updatePalette();
    
    show(d.taskbar);
    
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
    d.taskbar->setPalette(pal);
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
    if (!d.currentBar || d.currentBar == d.taskbar)
        return; // nothing to raise...
    dbusAction(d.currentBar, -1, "raise");
}

void
XBar::registerMenu(const QString &service, qlonglong key, const QString &title, const QStringList &entries)
{
    MenuBar *newBar = new MenuBar(service, key, this, dummy);
    newBar->setAppTitle(title);
    newBar->setPalette(palette());
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
        d.currentBar = d.taskbar;
        show(d.taskbar);
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

void
XBar::requestFocus(qlonglong key)
{
    for (MenuMap::iterator i = d.menus.begin(); i != d.menus.end(); ++i)
    {
        if (i.key() == key)
        {
            hide(d.taskbar);
            show(i.value());
        }
        else
            hide(i.value());
    }
}

void
XBar::setOpenPopup(int idx)
{
    if (d.currentBar && d.currentBar != d.taskbar)
    {
        d.currentBar->setOpenPopup(idx + d.extraTitle);
        d.currentBar->update();
    }
}

void
XBar::show(MenuBar *item)
{
    d.currentBar = item;
    item->setPos(contentsRect().topLeft());
    item->show();
}

void
XBar::showTaskbar()
{
    foreach (MenuBar *menu, d.menus)
        hide(menu);

    d.currentBar = d.taskbar;
    show(d.taskbar);
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
    if (!d.currentBar || d.currentBar == d.taskbar)
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

    if (d.currentBar == d.taskbar)
    {
        hide(d.taskbar);
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
        show(d.taskbar);
    else
        show(n.value());
}

K_EXPORT_PLASMA_APPLET(xbar, XBar)

#include "dbus.moc"
#include "xbar.moc"
