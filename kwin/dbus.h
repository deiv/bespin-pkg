/*
 *   Bespin window decoration for KWin
 *   Copyright 2008-2012 by Thomas Lübking <thomas.luebking@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
    Q_NOREPLY void styleByPid(qint64 pid, QByteArray data) { fac->learn(pid, data); }
    Q_NOREPLY void forget(qint64 pid) { fac->forget(pid); }
    Q_NOREPLY void setNetbookMode(bool b) { fac->setNetbookMode(b); }
    Q_NOREPLY void updateDeco(uint wid) { fac->updateDeco((WId)wid); }
};
} //namespace
#endif //XBAR_ADAPTOR_H
