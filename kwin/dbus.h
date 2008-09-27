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

#ifndef XBAR_ADAPTOR_H
#define XBAR_ADAPTOR_H

#include <QDBusAbstractAdaptor>
#include "factory.h"
namespace Bespin
{
class BespinDecoAdaptor : public QDBusAbstractAdaptor
{
   Q_OBJECT
   Q_CLASSINFO("D-Bus Interface", "org.kde.BespinDeco")

private:
    Factory *fac;

public:
    BespinDecoAdaptor(Factory *factory) : QDBusAbstractAdaptor(factory), fac(factory) { }

public slots:
    Q_NOREPLY void styleByPid(qint64 pid, uint bgInfo, uint activeColors, uint inactiveColors)
    { fac->learn(pid, bgInfo, activeColors, inactiveColors); }
    Q_NOREPLY void forget(qint64 pid) { fac->forget(pid); }
};
} //namespace
#endif //XBAR_ADAPTOR_H
