/* Bespin widget style for Qt4
Copyright (C); 2007 Thomas Luebking <thomas.luebking@web.de>

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

class QPainterPath;
class QRectF;

namespace Bespin
{
namespace Shapes
{
    QPainterPath close(const QRectF &bound, bool round = true);
    QPainterPath min(const QRectF &bound, bool round = true);
    QPainterPath max(const QRectF &bound, bool round = true);
    QPainterPath dockControl(const QRectF &bound, bool floating, bool round = true);
    QPainterPath restore(const QRectF &bound, bool round = true);
    QPainterPath stick(const QRectF &bound, bool round = true);
    QPainterPath unstick(const QRectF &bound, bool round = true);
    QPainterPath keepAbove(const QRectF &bound, bool round = true);
    QPainterPath keepBelow(const QRectF &bound, bool round = true);
    QPainterPath unAboveBelow(const QRectF &bound, bool round = true);
    QPainterPath menu(const QRectF &bound, bool leftSide, bool round = true);
    QPainterPath help(const QRectF &bound, bool round = true);
    QPainterPath shade(const QRectF &bound, bool round = true);
    QPainterPath unshade(const QRectF &bound, bool round = true);
    QPainterPath exposee(const QRectF &bound, bool round = true);
    QPainterPath info(const QRectF &bound, bool round = true);
    QPainterPath logo(const QRectF &bound);
}
}
