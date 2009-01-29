/*
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

#include <QDBusInterface>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QStyle>
#include <QStyleOptionMenuItem>

#include <kworkspace/kworkspace.h>

#include "taskbar.h"

#include <QtDebug>

enum TaskTask
{
    toggleMaximizeTask = 0, toggleMinimizeTask, moveTask, resizeTask, closeTask,
    toggleTaskAlwaysOnTop, toggleTaskKeptBelowOthers, toggleTaskFullScreen, toggleTaskShaded,
    taskToDesktop, taskToCurrentDesktop
};

static bool
isBrowser(const QString &s)
{
    return !s.compare("konqueror", Qt::CaseInsensitive) ||
    !s.compare("opera", Qt::CaseInsensitive) ||
    !s.compare("firefox", Qt::CaseInsensitive) ||
    !s.compare("mozilla", Qt::CaseInsensitive) ||
    !s.compare("safari", Qt::CaseInsensitive); // just in case ;)
}

static QString
entry(TaskPtr task, bool popup = true)
{
    if (popup)
    {
        QString ret = task->visibleNameWithState();
        if (!ret.contains(" - "))
            return ret.trimmed();
        ret.remove(task->classClass(), Qt::CaseInsensitive);
        if (ret.endsWith(" - ")) // usually yes...
            ret.remove(ret.length()-4, 3);
        if (ret.isEmpty())
            ret = task->visibleNameWithState(); // stupid, ehh??
        return ret.trimmed();
    }
    // see ../kwin/client.cpp for more comments
    QString ret = task->visibleName();
    QString appName = task->className();
    if (ret.contains(" - "))
        ret = ret.section(" - ", 0, -2, QString::SectionSkipEmpty );
    if (isBrowser(appName))
    {
        int n = qMin(2, ret.count(" - "));
        if (n--) // select last two if 3 or more sects, prelast otherwise
            ret = ret.section(" - ", -2, n-2, QString::SectionSkipEmpty);
    }
    if (ret.contains(": "))
        ret = ret.section(": ", 1, -1, QString::SectionSkipEmpty );
    if (ret.contains("http://"))
        ret = ret.remove("http://", Qt::CaseInsensitive)/*.section()*/;
    else
    {
        int i = ret.indexOf(appName, 0, Qt::CaseInsensitive);
        if (i > -1)
            ret = ret.mid(i, appName.length());
    }
    ret = ret.trimmed();
    if (ret.isEmpty())
        ret = task->visibleName(); // ...

    ret.replace("[modified]", "*");

    if (task->isActive())
        ret = "  >>  " + ret + "  <<  ";
    else if (task->isMinimized())
        ret = "  ( " + ret + " )  ";
    else if (task->isOnCurrentDesktop())
        ret = "  [ " + ret + " ]  ";
    else
        ret = "  { " + ret + " }  ";
    return ret;
}

void
TaskAction::update()
{
    QFont fnt = font();
    fnt.setBold(task->isActive());
    setFont(fnt);
    setText(entry(task, isOnPopup));
    emit changed();
}

TaskBar::TaskBar(QGraphicsItem *parent, const QWidget *dummy) : MenuBar( QString(), 0, parent, dummy),
dirty(true), isSqueezed(false)
{
    QMenu *sm = new QMenu;
    sm->addAction("Lock Screen", this, SLOT(lock()));
    sm->addAction("Leave...", this, SLOT(logout()));

    QAction *act = addAction("KDE 4");
    QFont fnt = act->font();
    fnt.setWeight(QFont::Black);
    fnt.setPointSize(fnt.pointSize()*1.2);
    act->setFont(fnt);
    act->setMenu(sm);

    connect (TaskManager::TaskManager::self(), SIGNAL(taskAdded(TaskPtr)),
                this, SLOT(addTask(TaskPtr)));
    connect (TaskManager::TaskManager::self(), SIGNAL(taskRemoved(TaskPtr)),
                this, SLOT(removeTask(TaskPtr)));

        taskTasks = new QMenu();
}

void
TaskBar::show()
{
    if (dirty)
    {
        // first tidy up
        QAction *entry;
        while (actions().count() > 1)
        {
            entry = actions().last();
            if (entry->menu())
            {
                delete entry->menu();
                entry->setMenu(NULL);
            }
            removeAction(actions().count()-1);
        }

        // then refill from current list
        TaskDict::const_iterator i = TaskManager::TaskManager::self()->tasks().constBegin();
        while (i != TaskManager::TaskManager::self()->tasks().constEnd())
        {
            addTask(i.value());
            ++i;
        }
        dirty = false;
    }
    validateSize();
    MenuBar::show();
}

void
TaskBar::addTask(TaskPtr task)
{
    if ( sender() && !(isVisible() && task->showInTaskbar()) )
    {
        dirty = true;
        return; // only if we're visible or function called by "show()"
    }
    
    bool newEntry = true;
    TaskAction *taskAction;
    QAction *action;
    for (int i = 1; i < actions().count(); ++i)
    {
        action = actions().at(i);
        taskAction = qobject_cast<TaskAction*>(action);
        if (taskAction && taskAction->task &&
             taskAction->task->classClass() == task->classClass())
        {
            if (taskAction->task == task) // should not happen
                return;
            newEntry = false; // nope, we allready had one
            QMenu *popup = action->menu();
            if (!popup)
            {
                // we'll need one NOW to group them
                popup = new QMenu();
                // remove the current action from bar... (it's actually an TaskAction)
                action = takeAction(i);
                // change the label to details...
                action->setText(entry(taskAction->task));
                // append it to the menu...
                if ((taskAction = qobject_cast<TaskAction*>(action)))
                    taskAction->isOnPopup = true;
                popup->addAction(action);
                // and add an action for the popup to the bar instead!
                addAction(task->classClass(), i, popup);
            }
            // anyway the (old or new) popup gets a new entry for this task appended
            taskAction = new TaskAction(entry(task), this, task);
            taskAction->isOnPopup = true;
            popup->addAction(taskAction);
            // we found a match and can leave loop now
            break;
        }
        else if (action->text() == task->classClass())
        {
            newEntry = false; // nope, we allready had one ...somehow
            QMenu *popup = action->menu();
            if (!popup)
                action->setMenu(popup = new QMenu());
            taskAction = new TaskAction(entry(task), this, task);
            popup->addAction(taskAction);
            break;
        }
    }
    if (newEntry) {
        taskAction = new TaskAction(entry(task, false), this, task);
        addAction(taskAction);
    }
    connect (taskAction, SIGNAL(triggered(bool)), task.data(), SLOT(activateRaiseOrIconify()));
    connect (task.data(), SIGNAL(changed()), taskAction, SLOT(validateSize()));
    connect (task.data(), SIGNAL(changed()), taskAction, SLOT(update()));
    validateSize();
    update();
//    void activated();
//     void deactivated();
//    task->isActive();
}

TaskPtr popupTask;

void
TaskBar::performTaskTask()
{
    if (!popupTask)
        return;
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
        return;
    bool ok;
    int value = action->data().toUInt(&ok);
    if (!ok)
        return;
    TaskTask task = (TaskTask)(value & 0xff);
    switch (task)
    {
    case toggleMaximizeTask:
        popupTask->toggleMaximized(); break;
    case toggleMinimizeTask:
        popupTask->toggleIconified(); break;

    case moveTask:
        popupTask->move(); break;
    case resizeTask:
        popupTask->resize(); break;
    case closeTask:
        popupTask->close(); break;

    case toggleTaskAlwaysOnTop:
        popupTask->toggleAlwaysOnTop(); break;
    case toggleTaskKeptBelowOthers:
        popupTask->toggleKeptBelowOthers(); break;

    case toggleTaskFullScreen:
        popupTask->toggleFullScreen(); break;
    case toggleTaskShaded:
        popupTask->toggleShaded(); break;
        
    case taskToDesktop:
        popupTask->toDesktop(value>>8); break;
    case taskToCurrentDesktop:
        popupTask->toCurrentDesktop(); break;
    default:
        break;
    }
}

void
TaskBar::removeTask(TaskPtr task)
{
    if (!isVisible()) {
        dirty = true;
        return;
    }

    TaskAction *taskAction;
    for (int i = 1; i < actions().count(); ++i)
    {
        taskAction = qobject_cast<TaskAction*>(actions().at(i));
        if (taskAction)
        {
            if (taskAction->task == task) // that's it
            {
                removeAction(i);
                validateSize();
                update();
                return;
            }
        }
        else if (QMenu *popup = actions().at(i)->menu())
        { // maybe in a submenu?
            foreach (QAction *action, popup->actions())
            {
                taskAction = qobject_cast<TaskAction*>(action);
                if (taskAction && taskAction->task == task)
                {
                    popup->removeAction(taskAction);
                    if (popup->actions().count() == 1) // there's only one item left - move it to the bar
                    {
                        action = popup->actions().takeFirst();
                        taskAction = qobject_cast<TaskAction*>(action);
                        if (taskAction && taskAction->task)
                        {
                            delete popup;
                            removeAction(i);
                            taskAction->setText(entry(taskAction->task, false));
                            taskAction->isOnPopup = false;
                            addAction(taskAction, i);
                            validateSize();
                            update();
                        }
                    }
                    return;
                }
            }
        }
    }
}

void
TaskBar::rightMouseButtonEvent(int idx, QGraphicsSceneMouseEvent *ev)
{
    ev->accept();
    TaskAction *taskAction = qobject_cast<TaskAction*>(actions().at(idx));
    if (!taskAction)
        return;
    popupTask = taskAction->task;

    taskTasks->hide();
    taskTasks->clear();
    
    QAction *action;
    QMenu *sub;

    // --------------------------------
    sub = taskTasks->addMenu("To Desktop");
    action = sub->addAction( "Current", this, SLOT(performTaskTask()));
    action->setData(taskToCurrentDesktop);
    action->setEnabled ( !popupTask->isOnCurrentDesktop() );
//     action = sub->addAction( popupTask->isOnAllDesktops() ? "All", this, SLOT(performTaskTask()));
//     action->setData(taskToCurrentDesktop);

    sub->addSeparator();

    for (int i = 0; i < TaskManager::TaskManager::self()->numberOfDesktops(); ++i)
    {
        action = sub->addAction(TaskManager::TaskManager::self()->desktopName(i), this, SLOT(performTaskTask()));
        action->setData(taskToDesktop | (i<<8));
        action->setEnabled ( i != popupTask->desktop() );
    }

//     bool isActive() const;
//     bool isOnTop() const;
//     QRect geometry() const;

    
    // ---------------------------------

    sub = taskTasks->addMenu("Advanced");
    action = sub->addAction( popupTask->isAlwaysOnTop() ? "Stack freely" : "Keep on Top",
                             this, SLOT(performTaskTask()));
    action->setData(toggleTaskAlwaysOnTop);
    action = sub->addAction( popupTask->isKeptBelowOthers() ? "Stack freely" : "Keep Below",
                             this, SLOT(performTaskTask()));
    action->setData(toggleTaskKeptBelowOthers);
    action = sub->addAction( popupTask->isFullScreen() ? "Windowed" : "Fullscreen",
                             this, SLOT(performTaskTask()));
    action->setData(toggleTaskFullScreen);
    action = sub->addAction( popupTask->isShaded() ? "Unshade" : "Shade",
                             this, SLOT(performTaskTask()));
    action->setData(toggleTaskShaded);

    // ---------------------------------

    taskTasks->addSeparator();

    action = taskTasks->addAction( popupTask->isMinimized() ? "Restore" : "Minimize",
                                   this, SLOT(performTaskTask()));
    action->setData(toggleMinimizeTask);
    action = taskTasks->addAction( popupTask->isMaximized() ? "Restore" : "Maximize",
                                   this, SLOT(performTaskTask()));
    action->setData(toggleMaximizeTask);

    taskTasks->addSeparator();

    action = taskTasks->addAction( "Move", this, SLOT(performTaskTask()));
    action->setData(moveTask);
    action = taskTasks->addAction( "Resize", this, SLOT(performTaskTask()));
    action->setData(resizeTask);

    taskTasks->addSeparator();

    action = taskTasks->addAction( "!!! Close !!!", this, SLOT(performTaskTask()));
    action->setData(closeTask);

    // ---------------------------------
    
    taskTasks->popup(mapToGlobal(ev->pos()));
}

void
TaskBar::validateSize()
{
    if (actions().count() < 2) // forced, 1st is "KDE 4"
        return;
    
    bool skip = true;
    if (parentWidget() && parentWidget()->isVisible() &&
        size().width() > parentWidget()->size().width())
        skip = false;
    if (isSqueezed && size().width() < parentWidget()->size().width()*0.9)
        skip = false;
    if (skip)
        return;

    int completeSize = parentWidget()->size().width() - actionGeometry(0).width();
    int average = completeSize /  (actions().count() - 1);

    int n = 0;

    TaskAction *taskAction = 0;
    QAction *action = 0;

    // first pass, query size demands
    for (int i = 1; i < actions().count(); ++i)
    {
        action = actions().at(i);
        taskAction = qobject_cast<TaskAction*>(action);
        if (taskAction)
        {
            if (taskAction->isSqueezed)
                taskAction->setText(entry(taskAction->task, false));
            if (actionGeometry(i).width() > average)
            {
                taskAction->isSqueezed = true;
                ++n;
            }
            else
            {
                taskAction->isSqueezed = false;
                completeSize -= actionGeometry(i).width();
            }
        }
        else
            completeSize -= actionGeometry(i).width();
    }

    isSqueezed = n;
    if (!isSqueezed)
        return;
    
    // second pass: fix text lenghts
    average = completeSize / n;
    
    QStyleOptionMenuItem opt;
    QRect r;
    int width;
    for (int i = 1; i < actions().count(); ++i)
    {
        if ((taskAction = qobject_cast<TaskAction*>(actions().at(i))))
        {
            if (taskAction->isSqueezed)
            {
                // real demand is style dependend...
                QFontMetrics fm(action->font());
                initStyleOption(&opt, i);
                r = fm.boundingRect(action->text());
                width = average * r.width();
                width /= style()->sizeFromContents(QStyle::CT_MenuBarItem, &opt, r.size(), 0).width();
                
                isSqueezed = true;
                QString string = fm.elidedText( taskAction->text(), Qt::ElideMiddle, width );
                taskAction->setText(string);
            }
        }
    }
}

void
TaskBar::lock()
{
   QDBusInterface interface( "org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver" );
   if (interface.isValid())
      interface.call("lock");
}

void
TaskBar::logout()
{
   KWorkSpace::requestShutDown(  KWorkSpace::ShutdownConfirmYes,
                                 KWorkSpace::ShutdownTypeNone,
                                 KWorkSpace::ShutdownModeInteractive );
}

#include "taskbar.moc"
