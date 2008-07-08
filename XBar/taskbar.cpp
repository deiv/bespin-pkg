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
#include <QMenu>

#include <kworkspace/kworkspace.h>

#include "taskbar.h"

#include <QtDebug>

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
    QString ret = task->visibleNameWithState();
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
        ret = task->visibleNameWithState(); // ...
    ret = "  [ " + ret + " ]  ";
    return ret;
}

void
TaskAction::update()
{
    setText(entry(task, isOnPopup));
}

TaskBar::TaskBar(QGraphicsItem *parent) : MenuBar( QString(), 0, parent), dirty(true)
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
                if (taskAction = qobject_cast<TaskAction*>(action))
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
    connect (task.data(), SIGNAL(changed()), taskAction, SLOT(update()));
    update();
//    void activated();
//     void deactivated();
//    task->isActive();
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
