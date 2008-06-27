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

#include <QAction>
#include <QDBusInterface>
#include <QMenu>

#include <kworkspace/kworkspace.h>

#include "taskbar.h"

#include <QtDebug>

TaskBar::TaskBar(QGraphicsItem *parent) : MenuBar( QString(), 0, parent)
{
   
   QMenu *sm = new QMenu;
   sm->addAction("Lock Screen", this, SLOT(lock()));
   sm->addAction("Leave...", this, SLOT(logout()));
   
   QAction *act = addAction("Plasma");
   QFont fnt = act->font();  fnt.setWeight(QFont::Black); act->setFont(fnt);
   act->setMenu(sm);
   
   addAction("A window");
   addAction("another window");
   addAction("windows???");
   addAction("... nahhh!");
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
