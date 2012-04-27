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

#ifndef RESIZECORNER_H
#define RESIZECORNER_H

#include <QWidget>

namespace Bespin {

class Client;

class ResizeCorner : public QWidget
{
   Q_OBJECT
public:
    ResizeCorner(Client *parent);
    void move ( int x, int y );
    void setColor(const QColor &c);
public slots:
    void raise();
protected:
    bool eventFilter(QObject *obj, QEvent *e);
    void mouseMoveEvent ( QMouseEvent * );
    void mousePressEvent ( QMouseEvent * );
    void mouseReleaseEvent ( QMouseEvent * );
    void paintEvent ( QPaintEvent * );
private:
    bool imCompiz;
    Client *client;
};

} // namespace

#endif //RESIZECORNER_H
