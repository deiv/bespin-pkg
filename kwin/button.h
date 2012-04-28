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
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include "../blib/gradients.h"

#include <QWidget>

namespace Bespin
{

class Client;

class Button : public QWidget
{
    Q_OBJECT
public:
    enum State { Normal, Hovered, Sunken };
    enum Type
    {
        Close = 0, Min, Max, Multi,
        Menu, Help, Above, Below, Stick, Shade, Exposee, Info, Special,
        // VertMax, HoriMax,
        Restore, Unstick, UnAboveBelow, Unshade, NumTypes
    };
    Button(Client *parent, Type type, bool left = false);
    static void init(bool leftMenu = false, bool fixedColors = false, int variant = 1);
    bool isEnabled() const;
    void setBg(const QPixmap &pix) { bgPix = pix; }
    inline bool type() {return myType;}
protected:
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void paintEvent(QPaintEvent*);
    void timerEvent(QTimerEvent*);
    void wheelEvent(QWheelEvent*);
private:
    Q_DISABLE_COPY(Button)
    QColor color( bool background = false ) const;
    bool hoverOut, left, iAmScrollable;
    Client *client;
    Type myType;
    int state, multiIdx, hoverTimer, hoverLevel;
    QPixmap bgPix;
    static QPainterPath shape[NumTypes];
    static QString tip[NumTypes];
    static bool fixedColors;
private slots:
    void clientStateChanged(bool);
    void maximizeChanged(bool);
};

} //namespace
#endif
