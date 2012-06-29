/*
 *   Bespin style for Qt4
 *   Copyright 2007-2012 by Thomas Lübking <thomas.luebking@gmail.com>
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

#include <QHash>
#include <QPixmap>
#include <QTime>
#include "basic.h"

class QStackedWidget;

namespace Animator {

enum Transition {Jump = 0, ScanlineBlend, SlideIn, SlideOut,
RollIn, RollOut, OpenVertically, CloseVertically, OpenHorizontally,
CloseHorizontally, CrossFade
};

class Curtain;
class Tab;

class TabInfo : public QObject
{
public:
    TabInfo(QObject* parent, QWidget *currentWidget = 0, int index = -1);
    bool proceed();
    void switchTab(QStackedWidget *sw, int index);
protected:
    QWeakPointer<Curtain> curtain;
    float progress;
    QWeakPointer<QWidget> currentWidget;
    friend class Tab;
    int index;
    uint duration;
    QTime clock;
protected:
    friend class Curtain;
    QPixmap tabPix[3];
private:
    void rewind();
    void updatePixmaps(Transition transition, uint ms = 0);
};

class Tab : public Basic {
    Q_OBJECT
public:
    static bool manage(QWidget *w);
    static void release(QWidget *w);
    static void setDuration(uint ms);
    static void setFPS(uint fps);
    static void setTransition(Transition t);
    typedef QWeakPointer<QStackedWidget> StackWidgetPtr;
protected:
    Tab();
    virtual bool _manage(QWidget *w);
    virtual void _release(QWidget *w);
    virtual void timerEvent(QTimerEvent * event);
    typedef QMap<StackWidgetPtr, TabInfo*> Items;
    Items items;
    int _activeTabs;
protected slots:
    void changed(int);
    void widgetRemoved(int);
private:
    Q_DISABLE_COPY(Tab)
};

inline bool operator< (const Tab::StackWidgetPtr &ptr1, const Tab::StackWidgetPtr &ptr2) {
    return ptr1.data() < ptr2.data();
}

} //namespace
