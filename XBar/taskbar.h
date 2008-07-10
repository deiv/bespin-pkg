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

#ifndef TASKBAR_H
#define TASKBAR_H

#include <QAction>
#include <taskmanager/taskmanager.h>
#include "menubar.h"

class XBar;
class QStyleOptionMenuItem;
class QGraphicsView;

using TaskManager::Task;
using TaskManager::TaskDict;
using TaskManager::TaskPtr;

class TaskAction : public QAction
{
    Q_OBJECT
public:
    TaskAction(const QString &text, QObject *parent, const TaskPtr task) :
    QAction(text, parent)
    {
        this->task = task;
        isOnPopup = false;
    }
    TaskPtr task;
    bool isOnPopup;
public slots:
    void update();
};

class TaskBar : public MenuBar
{
    Q_OBJECT
public:
    TaskBar(QGraphicsItem *parent = 0);
    void show();
protected:
    void rightMouseButtonEvent(int idx, QGraphicsSceneMouseEvent *ev);
private:
    bool dirty;
    QMenu *taskTasks;
private slots:
    void lock();
    void logout();
    void addTask(TaskPtr);
    void removeTask(TaskPtr);
    void performTaskTask();
};

#endif //XBAR_H
