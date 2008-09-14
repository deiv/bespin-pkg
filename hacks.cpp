/* Bespin widget style for Qt4
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

#include <QCoreApplication>
#include <QEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QStyle>
#include <QStyleOption>
#include <QStyleOptionGroupBox>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "fixx11h.h"
#include <QX11Info>

static Atom netMoveResize = XInternAtom(QX11Info::display(), "_NET_WM_MOVERESIZE", False);
#endif

#include <QtDebug>
#include "colors.h"
#include "config.h"
#include "hacks.h"

using namespace Bespin;
extern Config config;

static Hacks *bespinHacks = new Hacks;
static Hacks::HackAppType *appType = 0;
const char *SMPlayerVideoWidget = "MplayerLayer" ;// MplayerWindow
const char *DragonVideoWidget = "Phonon::VideoWidget"; // Codeine::VideoWindow, Phonon::Xine::VideoWidget

static void
triggerWMMove(const QWidget *w, const QPoint &p)
{
#ifdef Q_WS_X11
   // stolen... errr "adapted!" from QSizeGrip
   QX11Info info;
   XEvent xev;
   xev.xclient.type = ClientMessage;
   xev.xclient.message_type = netMoveResize;
   xev.xclient.display = QX11Info::display();
   xev.xclient.window = w->window()->winId();
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = p.x();
   xev.xclient.data.l[1] = p.y();
   xev.xclient.data.l[2] = 8; // NET::Move
   xev.xclient.data.l[3] = Button1;
   xev.xclient.data.l[4] = 0;
   XUngrabPointer(QX11Info::display(), QX11Info::appTime());
   XSendEvent(QX11Info::display(), QX11Info::appRootWindow(info.screen()), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);
#endif // Q_WS_X11
}

static bool
hackMessageBox(QMessageBox* box, QEvent *e)
{
   switch (e->type()) {
   case QEvent::Paint: {
      int s = qMin(164, 3*box->height()/2);
      QStyleOption opt; opt.rect = QRect(0,0,s,s); opt.palette = box->palette();
      QStyle::StandardPixmap logo = (QStyle::StandardPixmap)0;
      switch (box->icon()){
      case QMessageBox::NoIcon:
      default:
         break;
      case QMessageBox::Question:
         logo = QStyle::SP_MessageBoxQuestion; break;
      case QMessageBox::Information:
         logo = QStyle::SP_MessageBoxInformation; break;
      case QMessageBox::Warning:
         logo = QStyle::SP_MessageBoxWarning; break;
      case QMessageBox::Critical:
         logo = QStyle::SP_MessageBoxCritical; break;
      }
      QPixmap icon = box->style()->standardPixmap ( logo, &opt, box );
      QPainter p(box);
      if (logo) {
         const int y = (box->height()-s)/2 - qMax(0,(box->height()-164)/3);
         p.drawPixmap(-s/3,y, icon);
      }
      p.setPen(Colors::mid(box->palette().color(QPalette::Window),
                           box->palette().color(QPalette::WindowText)));
      p.drawRect(box->rect().adjusted(0,0,-1,-1));
      p.end();
      return true;
   }
   case QEvent::MouseButtonPress: {
      QMouseEvent *mev = static_cast<QMouseEvent*>(e);
      if (mev->button() == Qt::LeftButton)
         triggerWMMove(box, mev->globalPos());
      return false;
   }
   case QEvent::Show: {
      QLabel *icon = box->findChild<QLabel*>("qt_msgboxex_icon_label");
      if (icon) {
         icon->setPixmap(QPixmap());
         icon->setFixedSize(box->height()/3,10);
      }
      QLabel *text = box->findChild<QLabel*>("qt_msgbox_label");
      if (text) {
         text->setAutoFillBackground(true);
         QPalette pal = text->palette();
         QColor c = pal.color(QPalette::Base);
         c.setAlpha(220);
         pal.setColor(QPalette::Base, c);
         text->setPalette(pal);
         text->setBackgroundRole(QPalette::Base);
         text->setForegroundRole(QPalette::Text);
         if (!text->text().contains("<h2>")) {
            QString head = "<qt><h2>" + box->windowTitle() + "</h2>";
//             switch (box->icon()){
//             case QMessageBox::NoIcon:
//             default:
//                break;
//             case QMessageBox::Question:
//                head = "<qt><h2>Question...</h2>"; break;
//             case QMessageBox::Information:
//                head = "<qt><h2>Info:</h2>"; break;
//             case QMessageBox::Warning:
//                head = "<qt><h2>Warning!</h2>"; break;
//             case QMessageBox::Critical:
//                head = "<qt><h2>Error!</h2>"; break;
//             }
            QString newText = text->text();
            newText.replace(QRegExp("^(<qt>)*"), head);
            if (!newText.endsWith("</qt>")) newText.append("</qt>");
            text->setText(newText);
//             text->setMargin(4);
         }
         text->setFrameStyle ( QFrame::StyledPanel | QFrame::Sunken );
         text->setLineWidth ( 0 );
      }
      QLabel *info = box->findChild<QLabel*>("qt_msgbox_informativelabel");
      if (info) {
         info->setAutoFillBackground(true);
         QPalette pal = info->palette();
         QColor c = pal.color(QPalette::Base);
         c.setAlpha(220);
         pal.setColor(QPalette::Base, c);
         c = Colors::mid(pal.color(QPalette::Base), pal.color(QPalette::Text),1,3);
         pal.setColor(QPalette::Text, c);
         info->setPalette(pal);
         info->setBackgroundRole(QPalette::Base);
         info->setForegroundRole(QPalette::Text);
         info->setMargin(4);
      }
      return false;
   }
   default:
      return false;
   }
   return false;
}

static bool
isWindowDragWidget(QObject *o)
{
    return config.hack.windowMovement && (
        qobject_cast<QDialog*>(o) ||
        qobject_cast<QMenuBar*>(o) ||
        qobject_cast<QGroupBox*>(o) ||
        
        (o->inherits("QToolButton") && !static_cast<QWidget*>(o)->isEnabled()) ||
        o->inherits("QToolBar") ||
        o->inherits("QDockWidget") ||
//         o->inherits("QMainWindow") || // this is mostly useles... PLUS triggers problems

        ((*appType == Hacks::SMPlayer) && o->inherits(SMPlayerVideoWidget)) ||
        ((*appType == Hacks::Dragon) && o->inherits(DragonVideoWidget)) ||

        o->inherits("QStatusBar") ||
        (o->inherits("QLabel") && o->parent() && o->parent()->inherits("QStatusBar")));
}

static bool
hackMoveWindow(QWidget* w, QEvent *e)
{
    // general validity ================================
    QMouseEvent *mev = static_cast<QMouseEvent*>(e);
    if ( w->mouseGrabber() || // someone else is more interested in this
        (mev->modifiers() != Qt::NoModifier) || // allow forcing e.g. ctrl + click
        (mev->button() != Qt::LeftButton)) // rmb shall not move, maybe resize?!
//         !w->rect().contains(w->mapFromGlobal(QCursor::pos()))) // KColorChooser etc., catched by mouseGrabber ?!
        return false;

    // avoid if we click a menu action ========================================
    if (QMenuBar *bar = qobject_cast<QMenuBar*>(w))
    if (bar->activeAction())
        return false;

    // avoid if we try to (un)check a groupbx ==============================
    if (QGroupBox *gb = qobject_cast<QGroupBox*>(w))
    if (gb->isCheckable())
    {
        // gather options, fucking protected functions... :-(
        QStyleOptionGroupBox opt;
        opt.initFrom(gb);
        if (gb->isFlat())
            opt.features |= QStyleOptionFrameV2::Flat;
        opt.lineWidth = 1; opt.midLineWidth = 0;
        
        opt.text = gb->title();
        opt.textAlignment = gb->alignment();

        opt.subControls = (QStyle::SC_GroupBoxFrame | QStyle::SC_GroupBoxCheckBox);
        if (!gb->title().isEmpty())
            opt.subControls |= QStyle::SC_GroupBoxLabel;

        opt.state |= (gb->isChecked() ? QStyle::State_On : QStyle::State_Off);
        
        if (gb->style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxCheckBox, gb).contains(mev->pos()))
            return false;
    }

    // preserve dock / toolbar internal move float trigger on dock titles =================
    if (w->cursor().shape() != Qt::ArrowCursor ||
        (mev->pos().y() < 12 && w->inherits("QDockWidget")))
        return false;

//     QMouseEvent rel(QEvent::MouseButtonRelease, mev->pos(), mev->button(),
//                     mev->buttons(), mev->modifiers());
//     QCoreApplication::sendEvent( w, &rel );
    triggerWMMove(w, mev->globalPos());
//     w->setWindowState ( w->windowState() | Qt::WindowActive );
    return true;
}

static bool
paintKrunner(QWidget *w, QPaintEvent *)
{
    // TODO: use paintevent clipping
    if (w->isWindow()) {
        QPainter p(w);
        QStyleOption opt;
        opt.initFrom ( w );
        w->style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, w);
        return true;
    }
    return false;
}

bool
Hacks::eventFilter(QObject *o, QEvent *e)
{
    if ((*appType == KRunner))
    {
        if (e->type() == QEvent::Paint)
            return paintKrunner(static_cast<QWidget*>(o), static_cast<QPaintEvent*>(e));
        if (e->type() == QEvent::Show)
        {
            static_cast<QWidget*>(o)->setWindowOpacity( 80.0 );
            return false;
        }
    }

    if (QMessageBox* box = qobject_cast<QMessageBox*>(o))
        return hackMessageBox(box, e);

    if (e->type() == QEvent::MouseButtonPress && isWindowDragWidget(o))
        return hackMoveWindow(static_cast<QWidget*>(o), e);

    return false;
}

bool
Hacks::add(QWidget *w)
{
    if (!appType)
    {
        appType = new HackAppType((HackAppType)Unknown);
        if (qApp->inherits("GreeterApp")) // KDM segfaults on QCoreApplication::arguments()...
            *appType = KDM;
        else if (config.hack.krunner && QCoreApplication::applicationName() == "krunner")
            *appType = KRunner;
        else if (QCoreApplication::applicationName() == "dragonplayer")
            *appType = Dragon;
        else if (QCoreApplication::arguments().at(0).endsWith("smplayer"))
            *appType = SMPlayer;
    }

    if (*appType == KRunner)
    {
        if (QPushButton *btn = qobject_cast<QPushButton*>(w))
            btn->setFlat ( true );
        else if (w->isWindow())
        {
            w->setAttribute(Qt::WA_MacBrushedMetal);
//             w->setAttribute(Qt::WA_NoSystemBackground);
            w->installEventFilter(bespinHacks);
        }
        return true;
    }
    
    if (config.hack.messages && qobject_cast<QMessageBox*>(w))
    {
        w->setWindowFlags ( Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
        w->removeEventFilter(bespinHacks); // just to be sure
        w->installEventFilter(bespinHacks);
        return true;
    }
    
    if (config.hack.KHTMLView)
    if (QFrame *frame = qobject_cast<QFrame*>(w))
    if (frame->inherits("KHTMLView"))
    {
        frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        return true;
    }

    if (isWindowDragWidget(w))
    {
        w->removeEventFilter(bespinHacks); // just to be sure
        w->installEventFilter(bespinHacks);
        return true;
    }

//    if (config.hack.konsole)
//    if (w->inherits("Konsole::TerminalDisplay")) {
//       w->setAttribute(Qt::WA_StyledBackground);
//       w->setAttribute(Qt::WA_MacBrushedMetal);
//       return true;
//    }
    return false;
}

void
Hacks::remove(QWidget *w) {
    w->removeEventFilter(bespinHacks);
    if (w->inherits("KHTMLView"))
        static_cast<QFrame*>(w)->setFrameStyle(QFrame::NoFrame);
}
