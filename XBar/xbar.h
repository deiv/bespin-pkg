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

#ifndef XBAR_H
#define XBAR_H

class QGraphicsLinearLayout;

#include <QMap>
#include <QMenu>
#include <QWidget>

#include <Plasma/Applet>

class MenuBar;
class TaskBar;

class XBar : public Plasma::Applet
{
    Q_OBJECT
public:
    XBar(QObject *parent, const QVariantList &args);
    virtual ~XBar();
    void registerMenu(const QString &service, qlonglong key, const QString &title, const QStringList &entries);
    void unregisterMenu(qlonglong key);
    QPoint mapToGlobal(const QPointF &pt);
    void reparent(qlonglong oldKey, qlonglong newKey);
    void changeEntry(qlonglong key, int idx, const QString &entry = QString(), bool add = false);
    void setOpenPopup(int idx);
    void requestFocus(qlonglong key);
    void releaseFocus(qlonglong key);
public slots:
        void init();
protected:
    void wheelEvent(QGraphicsSceneWheelEvent *ev);
private:
    void hide(MenuBar *item);
    void show(MenuBar *item);
    bool dbusAction(const QObject *o, int idx, const QString &cmd);
private:
    typedef QMap<qlonglong, MenuBar*> MenuMap;
    friend class DummyWidget;
    struct {
        MenuMap menus;
        QMenu windowList;
        MenuBar *taskbar, *currentBar;
        bool extraTitle, showTaskBar;
    } d;
    static QTimer bodyCleaner;
private slots:
    void hover(int);
    void trigger(int);
    void updatePalette();
    void updateWindowlist();
    void unregisterCurrentMenu();
    void activateWin();
    void callMenus();
    void byeMenus();
    void raiseCurrentWindow();
    void showTaskbar();
    void cleanBodies();
};

#endif //XBAR_H
