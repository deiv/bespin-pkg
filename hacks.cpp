/*
 *   Bespin style for Qt4
 *   Copyright 2007-2012 by Thomas Lübking <thomas.luebking@gmail.com>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QCoreApplication>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QElapsedTimer>
#include <QEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QWindowStateChangeEvent>
#include <QSlider>
#include <QStatusBar>
#include <QStyle>
#include <QStyleOption>
#include <QStyleOptionGroupBox>
#include <QTabBar>
#include <QTextDocument>
#include <QToolBar>
#include <QToolButton>
#include <QWeakPointer>

#ifdef Q_WS_X11

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <QX11Info>
//#include "blib/fixx11h.h"
#include "blib/xproperty.h"

static Atom netMoveResize = XInternAtom(QX11Info::display(), "_NET_WM_MOVERESIZE", False);
#endif

#include <QtDebug>
#include "blib/colors.h"
#include "blib/gradients.h"
#include "blib/WM.h"
//#include "blib/xproperty.h"
#include "hacks.h"

namespace Bespin {
class Panner;
static Panner *s_pannerInstance = 0;

class Panner : public QObject
{
public:
    static void manage(QWidget *w) {
        if (!s_pannerInstance)
            s_pannerInstance = new Panner;
        else
            w->removeEventFilter(s_pannerInstance);
        w->installEventFilter(s_pannerInstance);
    }
protected:
    bool eventFilter(QObject *o, QEvent *e) {
        switch (e->type()) {
        case QEvent::MouseMove: {
            if (notRelevant(e))
                return false;
            if (m_panning) {
                const QPoint pos = static_cast<QMouseEvent*>(e)->pos();
                bool noClick = !m_click;
                if (noClick) {
                    const int dx = pos.x() - m_lastPos.x();
                    const int dy = pos.y() - m_lastPos.y();
                    if (QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea*>(o->parent())) {
                        // dolphin stacks a graphicsview into a view inside a view ...
                        QAbstractScrollArea *runner = area;
                        while ((runner = qobject_cast<QAbstractScrollArea*>(runner->parent())))
                            area = runner;

                        if (dx && area->horizontalScrollBar())
                            area->horizontalScrollBar()->setValue(area->horizontalScrollBar()->value() - dx);
                        if (dy && area->verticalScrollBar()) {
                            area->verticalScrollBar()->setValue(area->verticalScrollBar()->value() - dy);
                        }
                    } else { // mostly QWebView
                        int factor[2] = {1, 1};
                        if (o->inherits("QWebView")) {
                            foreach (const QObject *o2, o->children()) {
                                if (o2->inherits("QWebPage")) {
                                    foreach (const QObject *o3, o2->children()) {
                                        if (o3->inherits("QWebFrame")) {
                                            const QSize sz = o3->property("contentsSize").toSize();
                                            if (sz.isValid()) {
                                                const QSize wsz = static_cast<QWidget*>(o)->size();
                                                factor[0] = qMin(6, qRound(float(sz.width()) / wsz.width()));
                                                factor[1] = qMin(6, qRound(float(sz.height()) / wsz.height()));
                                            }
                                            break;
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        if (dy) {
                            QWheelEvent wev(pos, dy*factor[1], Qt::NoButton, Qt::NoModifier, Qt::Vertical);
                            QApplication::sendEvent(o, &wev);
                        }
                        if (dx) {
                            QWheelEvent weh(pos, dx*factor[0], Qt::NoButton, Qt::NoModifier, Qt::Horizontal);
                            QApplication::sendEvent(o, &weh); // "oi wehh"
                        }
                    }
                }
                m_lastPos = pos;
                m_click = m_click && qAbs(QPoint(m_startPoint - m_lastPos).manhattanLength()) <  QApplication::startDragDistance();
                return noClick;
            }
            return false;
        }
        case QEvent::MouseButtonPress:
            if (notRelevant(e))
                return false;
            if (m_panning) // fake event from us, let it pass
                return false;
            if (o == m_lastTarget && m_deadTime.restart() < 333)
                return false;

            m_lastTarget = o;
            m_startPoint = m_lastPos = static_cast<QMouseEvent*>(e)->pos();
            m_click = m_panning = true;
            m_deadTime.start();
            return true;
        case QEvent::MouseButtonRelease: {
            if (notRelevant(e))
                return false;
            if (m_panning && m_click) {
                QMouseEvent mp(QEvent::MouseButtonPress, m_startPoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(o, &mp);
            }
            m_deadTime.start();
            m_panning = false;
            return false;
        }
        default:
            return false;
        }
    }
private:
    bool notRelevant(QEvent *e) {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        return me->modifiers() != Qt::NoModifier || (me->button() != Qt::LeftButton && me->button() != Qt::NoButton);
    }
    QPoint m_startPoint, m_lastPos;
    bool m_panning, m_click;
    QObject *m_lastTarget;
    QElapsedTimer m_deadTime;
    Panner() : QObject(), m_panning(false), m_click(false), m_lastTarget(0) {}
};
} // namepsace

using namespace Bespin;

#define ENSURE_INSTANCE if (!bespinHacks) bespinHacks = new Hacks
#define FILTER_EVENTS(_WIDGET_) { _WIDGET_->removeEventFilter(bespinHacks); _WIDGET_->installEventFilter(bespinHacks); } // skip semicolon


static Hacks *bespinHacks = 0L;
static Hacks::HackAppType *appType = 0L;
static QWeakPointer<QWidget> s_dragCandidate;
static QWeakPointer<QWidget> s_dragWidget;
static bool dragWidgetHadTrack = false;
static QMenu *lockToggleMenu = 0L;
static QToolBar *lockToggleBar = 0L;
static QAction *lockToggleAction = 0L;
static int autoSlideTimer = 0;

inline static bool
hackMessageBox(QMessageBox* box, QEvent *e)
{
    switch (e->type())
    {
    case QEvent::Paint:
    {
        int s = qMin(164, 3*box->height()/2);
        QStyleOption opt; opt.rect = QRect(0,0,s,s); opt.palette = box->palette();
        QStyle::StandardPixmap logo = (QStyle::StandardPixmap)0;
        switch (box->icon())
        {
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
        if (logo)
        {
            const int y = (box->height()-s)/2 - qMax(0,(box->height()-164)/3);
            p.drawPixmap(-s/3,y, icon);
        }
        p.setPen(Colors::mid(box->palette().color(QPalette::Window), box->palette().color(QPalette::WindowText)));
        p.drawRect(box->rect().adjusted(0,0,-1,-1));
        p.end();
        return true;
    }
    case QEvent::MouseButtonPress:
    {
        QMouseEvent *mev = static_cast<QMouseEvent*>(e);
        if (mev->button() == Qt::LeftButton)
            WM::triggerMove(box->window()->winId(), mev->globalPos());
        return false;
    }
    case QEvent::Show:
    {
        if (box->layout())
            box->layout()->setSpacing(8);
        if (QLabel *icon = box->findChild<QLabel*>("qt_msgboxex_icon_label"))
        {
            icon->setPixmap(QPixmap());
            icon->setFixedSize(box->height()/3,10);
        }
        if (QLabel *text = box->findChild<QLabel*>("qt_msgbox_label"))
        {
            text->setAutoFillBackground(true);
            QPalette pal = text->palette();
            QColor c = pal.color(QPalette::Base);
            c.setAlpha(220);
            pal.setColor(QPalette::Base, c);
            text->setPalette(pal);
            text->setBackgroundRole(QPalette::Base);
            text->setForegroundRole(QPalette::Text);
//             text->setContentsMargins(16, 8, 16, 8);
            text->setMargin(8);
            text->setFrameStyle ( QFrame::StyledPanel | QFrame::Sunken );
            text->setLineWidth ( 0 );
            if (!text->text().contains("<h2>"))
            {
                if (box->windowTitle().isEmpty())
                {
                    QFont bold = text->font(); bold.setBold(true); text->setFont(bold);
                }
                else
                {
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
                    if (newText.contains("<qt>"))
                    {
                        newText.replace(QRegExp("^(<qt>)*"), head);
                        if (!newText.endsWith("</qt>"))
                            newText.append("</qt>");
                    }
                    else
                        newText.prepend(head);
                    text->setText(newText);
                }
            }
        }
        if (QLabel *info = box->findChild<QLabel*>("qt_msgbox_informativelabel"))
        {
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
isWindowDragWidget(QObject *o, const QPoint *pt = 0L)
{
    if (!Hacks::config.windowMovement)
        return false;

    if ( qobject_cast<QDialog*>(o) ||
         (qobject_cast<QMenuBar*>(o) && !static_cast<QMenuBar*>(o)->activeAction()) ||
         qobject_cast<QGroupBox*>(o) ||

         (qobject_cast<QToolButton*>(o) && !static_cast<QWidget*>(o)->isEnabled()) ||
         qobject_cast<QToolBar*>(o) || qobject_cast<QDockWidget*>(o) ||

         qobject_cast<QStatusBar*>(o) || o->inherits("QMainWindow") || o->inherits("MplayerLayer") )
        return true;

    if ( QLabel *label = qobject_cast<QLabel*>(o) )
    if (!(label->textInteractionFlags() & Qt::TextSelectableByMouse))
    if (qobject_cast<QStatusBar*>(o->parent()))
        return true;

    if ( QTabBar *bar = qobject_cast<QTabBar*>(o) )
    if ( !pt || (bar->tabAt(*pt) < 0 && !bar->childAt(*pt)) )
        return true;

    return false;
}

static bool
hackMoveWindow(QWidget* w, QEvent *e)
{
    if (w->mouseGrabber())
        return false;
    if (w->window()->isFullScreen())
        return false;
    // general validity ================================
    QMouseEvent *mev = static_cast<QMouseEvent*>(e);
    if (!(mev->buttons() & Qt::LeftButton)) {
        s_dragWidget.clear();
        s_dragCandidate.clear();
        return false; // hanging move - we did not receive a release
    }
//         !w->rect().contains(w->mapFromGlobal(QCursor::pos()))) // KColorChooser etc., catched by mouseGrabber ?!
    // avoid if we click a menu action ========================================
    if (QMenuBar *bar = qobject_cast<QMenuBar*>(w))
    if (bar->activeAction() || bar->inherits("QDesignerMenuBar")) // ....
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
        (mev->pos().y() < w->fontMetrics().height()+4 && qobject_cast<QDockWidget*>(w)))
        return false;

    WM::triggerMove(w->window()->winId(), mev->globalPos());

    return true;
}


// obviously gwenview is completely incapable to keep the view position when watching an image
// -> we need a beter image browser :(
static int gwenview_position = 0;
static QTextDocument html2text;
static QWeakPointer<QSlider> s_videoTimerSlider;
static QWeakPointer<QSlider> s_videoVolumeSlider;
static QPoint s_mousePressPos, s_lastMovePos;
bool
Hacks::eventFilter(QObject *o, QEvent *e)
{
    QWidget *dragWidget = s_dragWidget.data();
    if (dragWidget && e->type() == QEvent::MouseMove)
    {
        qApp->removeEventFilter(this);
        dragWidget->setMouseTracking(dragWidgetHadTrack);
        // the widget needs an leave/enter to update the mouse state
        // sending events doesn't work, so we generate a wink-of-an-eye cursor repositioning ;-P
        const QPoint cursor = QCursor::pos();
        QWidget *window = dragWidget->window();
        QCursor::setPos(window->mapToGlobal( window->rect().topRight() ) + QPoint(2, 0) );
        QCursor::setPos(cursor);
        s_dragWidget.clear();
        dragWidget = 0;
        s_dragCandidate.clear();
        return false;
    }

    if (e->type() == QEvent::Timer || e->type() == QEvent::Move)
        return false;

    if (QMessageBox* box = qobject_cast<QMessageBox*>(o))
        return hackMessageBox(box, e);

    if ( e->type() == QEvent::Paint )
    {
        if (config.titleWidgets)
        if (QLabel *label = qobject_cast<QLabel*>(o)) {
        if (label->parentWidget() && label->parentWidget()->parentWidget() &&
            label->parentWidget()->parentWidget()->inherits("KTitleWidget"))
        {
            QWidget *container = label->parentWidget();

            QString string = label->text();
            if (string.contains('<'))
                { html2text.setHtml(string); string = html2text.toPlainText(); }
            QStringList strings = string.split('\n', QString::SkipEmptyParts);
            if (strings.isEmpty())
                return false;

//             qDebug() << strings;

            QRect r = label->contentsRect();
            const int fh = r.height()/(strings.count()+1)/* - 1*/;
            QPainter p(label);
            p.setPen(container->palette().color(container->foregroundRole()));
            QFont fnt(container->font());
            fnt.setPointSize/*F*/(float(2*fh*72)/label->logicalDpiY());
//             fnt.setPixelSize(2*fh);
            r.setBottom(r.top()+2*fh);
            p.setFont(fnt);
            p.drawText( r, Qt::AlignCenter|Qt::TextSingleLine, strings.at(0) );

            if (strings.count() < 2)
                return true;

            fnt.setPixelSize(fh-2);
            p.setFont(fnt);
            r.setTop(r.top()+fh);

            for (int i = 1; i < strings.count(); ++i)
            {
                r.translate(0, fh);
                p.drawText( r, Qt::AlignCenter|Qt::TextSingleLine, strings.at(i) );
            }
            return true;
        }
        }
        return false;
    }

    if (e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mev = static_cast<QMouseEvent*>(e);
        QWidget *w = qobject_cast<QWidget*>(o);
        if (config.lockToolBars &&  (mev->modifiers() & Qt::ControlModifier) && qobject_cast<QToolBar*>(w) && !w->inherits("KToolBar"))
        {
            lockToggleBar = static_cast<QToolBar*>(w);
            lockToggleAction->setChecked(!lockToggleBar->isMovable());
            lockToggleMenu->popup(QCursor::pos());
            return true;
        }
        if ( *appType == Okular && config.panning  && w->isWindow() && w->objectName() == "presentationWidget") {
            QRect r(w->rect());
            r.setSize(r.size()/3);
            if (r.contains(mev->pos())) {
                QWheelEvent wev(mev->pos(), 120, Qt::NoButton, Qt::NoModifier, Qt::Vertical);
                QApplication::sendEvent(w, &wev);
            } else {
                r.moveBottomRight(w->rect().bottomRight());
                if (r.contains(mev->pos())) {
                    QWheelEvent wev(mev->pos(), -120, Qt::NoButton, Qt::NoModifier, Qt::Vertical);
                    QApplication::sendEvent(w, &wev);
                }
            }
            return true;
        }
        if ( !w || w->mouseGrabber() || // someone else is more interested in this
             (mev->modifiers() != Qt::NoModifier) || // allow forcing e.g. ctrl + click
             (mev->button() != Qt::LeftButton)) // rmb shall not move, maybe resize?!
                return false;
        if (!s_dragCandidate.data() && isWindowDragWidget(o, &mev->pos()))
        {
            s_dragCandidate = w;
            s_mousePressPos = mev->pos();
            s_lastMovePos = mev->pos();
            qApp->installEventFilter(this);
        }
        return false;
    }

    QWidget *dragCandidate = s_dragCandidate.data();
    if (o == dragCandidate && e->type() == QEvent::MouseButtonRelease)
    {   // was just a click
        qApp->removeEventFilter(this);
        if (*appType == SMPlayer && dragCandidate->window()->isFullScreen() && !s_videoTimerSlider.isNull()) {
            s_videoTimerSlider.data()->setSliderDown(false);
        }
        killTimer(autoSlideTimer);
        autoSlideTimer = 0;
        s_dragCandidate.clear();
        dragCandidate = 0;
        return false;
    }

    if (o == dragCandidate && e->type() == QEvent::MouseMove) // gonna be draged
    {   // we perhaps want to drag
        const bool wmDrag = hackMoveWindow(dragCandidate, e);
        if ( wmDrag )
        {
            s_dragWidget = dragWidget = dragCandidate;

            // the release would set "dragCandidate = 0L;", therfore it cannot be done in hackMoveWindow
            // it's probably not required either
//             QMouseEvent *mev = static_cast<QMouseEvent*>(e);
//             QMouseEvent mbr(QEvent::MouseButtonRelease, mev->pos(), mev->button(), mev->buttons(), mev->modifiers());
//             QCoreApplication::sendEvent( dragWidget, &mbr );
            dragWidgetHadTrack = dragWidget->hasMouseTracking();
            dragWidget->setMouseTracking(true);
        }
        else if (*appType == SMPlayer && dragCandidate->window()->isFullScreen())
        {
            QMouseEvent *mev = static_cast<QMouseEvent*>(e);
            QPoint diff = mev->pos() - s_mousePressPos;
            int dx = qAbs(diff.x()), dy = qAbs(diff.y());
            const int w = 64, h = 64;
            if (dx > w && dy < h) {
                if (QSlider *slider = s_videoTimerSlider.data()) {
                    dx = ((mev->pos().x() - s_lastMovePos.x()) * (slider->maximum() - slider->minimum())) / (dragCandidate->window()->width());
                    if (dx) {
                        slider->setSliderDown(true);
                        slider->setSliderPosition(slider->sliderPosition() + dx);
                        s_lastMovePos = mev->pos();
                        killTimer(autoSlideTimer);
                        autoSlideTimer = startTimer(250);
                    }
                }
            }
            else if (dx < w && dy > h) {
                if (QSlider *slider = s_videoVolumeSlider.data()) {
                    dy = ((mev->pos().y() - s_lastMovePos.y()) * (slider->maximum() - slider->minimum())) / (dragCandidate->window()->height());
                    if (dy) {
                        slider->setValue(slider->value() - dy);
                        s_lastMovePos = mev->pos();
                    }
                }
            }
        }
        else {
            s_dragCandidate.clear();
            dragCandidate = 0L;
        }
        return wmDrag;
    }

    if ( *appType == Gwenview && config.fixGwenview )
    {
        if ( ( e->type() == QEvent::Wheel || e->type() == QEvent::Show || e->type() == QEvent::Hide ) && o->objectName() == "qt_scrollarea_viewport" )
        {
            if ( QAbstractScrollArea *list = qobject_cast<QAbstractScrollArea*>(o->parent()) )
            {
                QScrollBar *bar = list->verticalScrollBar();
                if ( !bar )
                    return false;
                // Gwenview scrolls three rows at once, what drives me crazy because you loose track on images
                // and waste time to find yourself back into context.
                if ( e->type() == QEvent::Wheel )
                {
                    int step = bar->singleStep();
                    bar->setSingleStep(step/3);
    //                 list->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
                    QCoreApplication::sendEvent(bar, e); // tell the scrollbar to do this ;-P
                    bar->setSingleStep(step);
                    return true; // eat it
                }
                // another funny crap is that it scrolls around whenever you start looking a an image
                // so we save the position and re-set it when the thumbview comes back
                // i'd  make a bugreport but it's been like this since KDE4 times, this cannot be a bug but
                // must be stubborness :-(
                if ( e->type() == QEvent::Hide )
                    gwenview_position = bar->value();
                else if ( gwenview_position )
                {
                    bar->setValue( gwenview_position );
                    connect( bar, SIGNAL(valueChanged(int)), this, SLOT(fixGwenviewPosition()) );
                }
            }
            return false;
        }
        // a) abusing stylesheets because we're not able to just set the proper QWidget properties, are we? cpp is sooooo hard
        // b) if you don''t understand css or basic layering concepts: JUST STAY AWAY, FOOLS
        // c) yes i know - there's the bottom border. but you've got a frame in a widget in another widget being a frame .... *sigh*
        if (e->type() == QEvent::StyleChange && o->objectName() == "saveBarWidget" )
        {
            QWidget *w = static_cast<QWidget*>(o);
            if (w->styleSheet().isEmpty())
                return false;

            w->removeEventFilter(this);
            w->setStyleSheet(QString());
            w->installEventFilter(this);

            QPalette pal = QApplication::palette();

            QWidget *window = w->window();
            if ( window && window->isFullScreen() )
            {
                pal.setColor(QPalette::Window, QColor(48,48,48));
                pal.setColor(QPalette::WindowText, QColor(224,224,224));
            }
            else
            {
                pal.setColor(QPalette::Window, pal.color(QPalette::ToolTipBase));
                pal.setColor(QPalette::WindowText, pal.color(QPalette::ToolTipText));
            }

            w->setAutoFillBackground(true);
            w->setBackgroundRole(QPalette::Window);
            w->setForegroundRole(QPalette::WindowText);
            w->setPalette(pal);

            QList<QWidget*> kids = w->findChildren<QWidget*>();
            foreach (QWidget *w2, kids)
            {
                w2->setBackgroundRole(QPalette::Window);
                w2->setForegroundRole(QPalette::WindowText);
                w2->setAutoFillBackground(false);
                w2->setPalette(pal); // w2->setPalette(QPalette()); should be sufficient but fails. possibly related to qss abuse...
            }
        }
        return false;
    }
#ifdef Q_WS_X11
    if (e->type() == QEvent::WindowStateChange)
    {
        if (config.suspendFullscreenPlayers)
        if (QWidget *w = qobject_cast<QWidget*>(o))
        if (w->isWindow())
        {
            if (w->windowState() & Qt::WindowFullScreen)
                XProperty::setAtom( w->winId(), XProperty::blockCompositing );
            else if (static_cast<QWindowStateChangeEvent*>(e)->oldState() & Qt::WindowFullScreen)
                XProperty::remove(w->winId(), XProperty::blockCompositing);
        }
    }
#endif
    if (e->type() == QEvent::Show)
    {
        FILTER_EVENTS(o);
        return false;
    }  // >-)

    return false;
}

void
Hacks::fixGwenviewPosition()
{
    QScrollBar *bar = qobject_cast<QScrollBar*>(sender());
    if ( !bar )
        return;
    disconnect( bar, SIGNAL(valueChanged(int)), this, SLOT(fixGwenviewPosition()) );
    bar->setValue( gwenview_position );
}

void
Hacks::toggleToolBarLock()
{
    if (lockToggleBar)
        lockToggleBar->setMovable(!lockToggleBar->isMovable());
    lockToggleBar = 0;
}

bool
Hacks::add(QWidget *w)
{
    if (!appType)
    {
        appType = new HackAppType((HackAppType)Unknown);
        if (qApp->inherits("GreeterApp")) // KDM segfaults on QCoreApplication::arguments()...
            *appType = KDM;
        else if (QCoreApplication::applicationName() == "okular")
            *appType = Okular;
        else if (QCoreApplication::applicationName() == "gwenview")
            *appType = Gwenview;
        else if (QCoreApplication::applicationName() == "smplayer" ||
            (QCoreApplication::arguments().count() && QCoreApplication::arguments().at(0).endsWith("smplayer")) ) {
            *appType = SMPlayer;
        }
    }

    if (config.suspendFullscreenPlayers)
    if (w->isWindow())
    {
        ENSURE_INSTANCE;
        FILTER_EVENTS(w);
    }

    if (config.messages && qobject_cast<QMessageBox*>(w))
    {
        ENSURE_INSTANCE;
        w->setWindowFlags ( Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
        FILTER_EVENTS(w);
        return true;
    }

    if (config.lockToolBars && qobject_cast<QToolBar*>(w) && !w->inherits("KToolBar"))
    {
        ENSURE_INSTANCE;
        if (!lockToggleMenu)
        {
            lockToggleMenu = new QMenu();
            lockToggleAction = lockToggleMenu->addAction( "Lock Toolbar Position", bespinHacks, SLOT(toggleToolBarLock()) );
            lockToggleAction->setCheckable(true);
        }
        static_cast<QToolBar*>(w)->setMovable(false);
        FILTER_EVENTS(w);
    }

    if ( *appType == Gwenview && config.fixGwenview )
    {
        if ( QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea*>(w) )
        if ( area->inherits("Gwenview::ThumbnailView") )
        {
            ENSURE_INSTANCE;
            FILTER_EVENTS(area->viewport());
        }
        if (w->objectName() == "saveBarWidget") // they broke it again or never really fixed it. now we fix it. i hope canonical dies soon.
        {
            ENSURE_INSTANCE;
            FILTER_EVENTS(w);
        }
    }

    if ( *appType == SMPlayer && config.panning  )
    if (QSlider *slider = qobject_cast<QSlider*>(w)) {
    if (slider->parentWidget() && slider->parentWidget()->objectName() == "controlwidget")
    {
        if (slider->inherits("TimeSlider"))
            s_videoTimerSlider = slider;
        else if (slider->inherits("MySlider"))
            s_videoVolumeSlider = slider;
    }
    }

    if ( *appType == Okular && config.panning  && w->isWindow() && w->objectName() == "presentationWidget") {
        ENSURE_INSTANCE;
        FILTER_EVENTS(w);
    }

//     if ( w->objectName() == "qt_scrollarea_viewport" )

    if (config.KHTMLView)
    if (QFrame *frame = qobject_cast<QFrame*>(w))
    if (frame->inherits("KHTMLView"))
    {
        frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        return true;
    }

    if ( config.killThrobber )
    if ( QMenuBar *mbar = qobject_cast<QMenuBar*>(w->parentWidget()) )
    if ( w->inherits("KAnimatedButton") )
    {   // this is konquerors throbber...
        w->hide();
        w->setParent(mbar->parentWidget());
        mbar->setCornerWidget(NULL);
        return true;
    }

    if (isWindowDragWidget(w))
    {
        ENSURE_INSTANCE;
        FILTER_EVENTS(w);
        return true;
    }

    if (config.titleWidgets)
    if (QLabel *label = qobject_cast<QLabel*>(w))
    if (QFrame *frame = qobject_cast<QFrame*>(label->parentWidget()))
    if (frame->parentWidget() && frame->parentWidget()->inherits("KTitleWidget"))
    {
        ENSURE_INSTANCE;
        frame->setFrameShadow(QFrame::Sunken);
        frame->setAutoFillBackground(true);
        FILTER_EVENTS(label);
    }

    if (config.panning) {
        if ( QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea*>(w) ) {
            if (QAbstractItemView *view = qobject_cast<QAbstractItemView*>(area)) {
            if (!view->inherits("QHeaderView")) {
                view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
                view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
                Panner::manage(area->viewport());
            }
            } else if (area->inherits("QTextEdit"))
                Panner::manage(area->viewport());
            else if (area->parentWidget() && area->parentWidget()->parentWidget() &&
                QString(area->parentWidget()->parentWidget()->metaObject()->className()).startsWith("Dolphin"))
                Panner::manage(area->viewport());
//             else
//                 qDebug() << area << area->parentWidget();
        } else if (w->inherits("QWebView"))
            Panner::manage(w);
    }
#if 0
    ENSURE_INSTANCE;
    FILTER_EVENTS(w);
#endif
    return false;
}

void
Hacks::remove(QWidget *w)
{
    w->removeEventFilter(bespinHacks);
    if (w->inherits("KHTMLView"))
        static_cast<QFrame*>(w)->setFrameStyle(QFrame::NoFrame);
}

void
Hacks::releaseApp()
{
    bespinHacks->deleteLater(); bespinHacks = 0L;
    lockToggleMenu->deleteLater(); lockToggleMenu = 0L;
}

void
Hacks::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == autoSlideTimer) {
        if (*appType == SMPlayer && s_dragCandidate.data() && s_dragCandidate.data()->window()->isFullScreen()) {
            if (QSlider *slider = s_videoTimerSlider.data()) {
                slider->setSliderDown(false);
                slider->setSliderDown(true);
            }
        }
        killTimer(autoSlideTimer);
        autoSlideTimer = 0;
    }
}

#undef FILTER_EVENTS
#undef ENSURE_INSTANCE
