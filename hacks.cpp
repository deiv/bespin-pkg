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

#include <QAbstractItemView>
#include <QCoreApplication>
#include <QDesktopWidget>
#include <QDial>
#include <QDockWidget>
#include <QEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QSplitter>
#include <QSlider>
#include <QStatusBar>
#include <QStyle>
#include <QStyleOption>
#include <QStyleOptionGroupBox>
#include <QStyleOptionSlider>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QWidgetAction>

#ifdef Q_WS_X11

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "fixx11h.h"
#include <QX11Info>

#include <cmath>

static Atom netMoveResize = XInternAtom(QX11Info::display(), "_NET_WM_MOVERESIZE", False);
#endif

#include <QtDebug>
#include "colors.h"
#include "gradients.h"
#include "hacks.h"

using namespace Bespin;

#define ENSURE_INSTANCE if (!bespinHacks) bespinHacks = new Hacks
static const int DT = 4000; // display duration
static const int FT = 500; // fade duration > 50!!!!

class ProxyDial : public QDial
{
public:
    ProxyDial(QAbstractSlider *slider, QWidget *parent = 0) : QDial(parent)
    {
        client = slider;
        setRange(slider->minimum(), slider->maximum()/*0, 100*/);
        setValue(slider->value());
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        connect (slider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
        connect (this, SIGNAL(valueChanged(int)), slider, SIGNAL(sliderMoved(int)));
    }
    QPointer<QAbstractSlider> client;
protected:
    void resizeEvent(QResizeEvent *re)
    {
        if (width() != height())
            setFixedWidth(height());
        else
            QDial::resizeEvent(re);
    }
};

class PrettyLabel : public QWidget
{
public:
    PrettyLabel( const QStringList & data, QWidget * parent = 0, Qt::WindowFlags f = 0) :
    QWidget(parent, f), time(0), index(0), animTimer(0), animated(true)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        QFont font; font.setBold(true); setFont(font);
        setMinimumHeight( QFontMetrics(font).height() + 4 );
        setData(data, false);
    }
    void setData(const QStringList &data, bool upd = true)
    {
        if (data == this->data)
            return;
        this->data = data.isEmpty() ? QStringList() << "" : data;
        time = 0; index = 0;
        if (data.count() > 1 && animated)
            { if (!animTimer) animTimer = startTimer(50); }
        else if (animTimer)
            { killTimer(animTimer); animTimer = 0; }
        if (upd && !animTimer)
            update();
    }
protected:
    void paintEvent( QPaintEvent * pe)
    {
        QPainter p(this);
        p.setClipRegion(pe->region());
        if (animTimer)
        {
            int level = -1;
            if (time < FT/50)
                level = time;
            else if (time > (DT-FT)/50)
                level = DT/50-time;
            if (level > -1)
            {
                QColor c(palette().color(foregroundRole()));
                c.setAlpha((255*level) / (FT/50));
                p.setPen(c);
            }
        }
        p.drawText(rect(), Qt::AlignCenter | Qt::TextSingleLine, data.at(index));
        p.end();
    }
    void mouseReleaseEvent( QMouseEvent * me )
    {
        if (me->button() == Qt::LeftButton)
        {
            animated = !animated;
            if (animated)
            {
                if (data.count() > 1)
                    { animTimer = startTimer(50); update(); }
            }
            else if (animTimer)
                { killTimer(animTimer); animTimer = 0; update(); }
        }
    }
    void timerEvent( QTimerEvent * te )
    {
        if (!isVisible() || te->timerId() != animTimer)
            return;
        if (time < FT/50 || time > (DT-FT)/50)
            update();
        ++time;
        if (time > DT/50)
        {
            time = 0; ++index;
            if (index >= data.count())
                index = 0;
        }
    }
    void wheelEvent ( QWheelEvent * we )
    {
        if (data.count() < 2)
            return;
        if (we->delta() < 0)
            { ++index; if (index >= data.count()) index = 0; }
        else
            { --index; if (index < 0) index = data.count()-1; }
        update();
    }
private:
    int time, index, animTimer;
    bool animated;
    QStringList data;
};

class ClickLabel : public QLabel {
public:
    ClickLabel(QWidget *p = 0, Qt::WindowFlags f = 0) : QLabel(p,f) { setAttribute(Qt::WA_PaintOnScreen); }
    void setPixmap(const QPixmap &pix, const QWidget *relative = 0)
    {
        QLabel::setPixmap(pix);
        resize(pix.size());
        QRect r = rect();
        QRect desktop = QDesktopWidget().availableGeometry();
        if (relative)
        {
            r.moveCenter(relative->mapToGlobal(relative->rect().center()));
            if (r.right() > desktop.right()) r.moveRight(desktop.right());
            if (r.bottom() > desktop.bottom()) r.moveBottom(desktop.bottom());
            if (r.left() < desktop.left()) r.moveLeft(desktop.left());
            if (r.top() < desktop.top()) r.moveTop(desktop.top());
        }
        else
            r.moveCenter(desktop.center());
        move(r.topLeft());
    }
protected:
    void mousePressEvent(QMouseEvent * me)
    {
        if (me->button() != Qt::LeftButton)
            return;
        hide(); deleteLater();
    }
};


class CoverLabel : public QLabel {
public:
    CoverLabel(QWidget *parent) : QLabel (parent)
    {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        full = 0; setMargin(3);
        setCursor(Qt::PointingHandCursor); hide();
    }
    void setUrl(const QString &url)
    {
        if (this->url == url)
            return;

        this->url = url;
        if (url.isEmpty())
            { setPixmap(QPixmap()); hide(); }
        else
        {
            QImage img(url);
            img = img.scaledToHeight(height()-6, Qt::SmoothTransformation);
            img = img.convertToFormat(QImage::Format_ARGB32);

            const uchar *bits = img.bits();
            QRgb *pixel = (QRgb*)(const_cast<uchar*>(bits));
            int n = img.width() * img.height();
            int r,g,b;
            palette().color(foregroundRole()).getRgb(&r,&g,&b);
            
            if (Colors::value(palette().color(foregroundRole())) > 128)
                for (int i = 0; i < n; ++i) pixel[i] = qRgba(r,g,b, (200*qGray(pixel[i]))>>8);
            else
                for (int i = 0; i < n; ++i) pixel[i] = qRgba(r,g,b, (200*(255-qGray(pixel[i])))>>8);

            QPainterPath pp;
            pp.moveTo(0,0);
            pp.lineTo(img.rect().bottomLeft());
            pp.quadTo(QPoint(img.width()/4, img.height()/4), img.rect().topRight());
            pp.closeSubpath();

            QPainter p(&img);
            p.setPen(Qt::NoPen); p.setBrush(QColor(255,255,255,45));
            p.setRenderHint(QPainter::Antialiasing);
            p.drawPath(pp);
#if QT_VERSION >= 0x040400
            pp = QPainterPath();
            pp.addRect(img.rect());
            pp.addRoundedRect(img.rect(),25,25,Qt::RelativeSize);
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.setBrush(Qt::black);
            p.drawPath(pp);
#endif
            p.end();

            setPixmap(QPixmap::fromImage(img));
            show();
        }
        updatePreview();
    }
protected:
    void mousePressEvent(QMouseEvent * me)
    {
        if (url.isEmpty() || full || me->button() != Qt::LeftButton)
            return;
        full = new ClickLabel(0, Qt::Window | Qt::X11BypassWindowManagerHint);
        updatePreview();
    }
    void resizeEvent (QResizeEvent *re)
    {
        QLabel::resizeEvent(re);
        if (re->size().height() == re->oldSize().height())
            return;
        QString oldUrl = url;
        url = QString();
        setUrl(oldUrl);
    }
private:
    void updatePreview()
    {
        if (!full) return;
        if (url.isEmpty())
            { full->hide(); delete full; full = 0; }
        else
        {
            full->setPixmap(QPixmap(url), this);
            full->show();
        }
    }
    QString url;
    QPointer<ClickLabel> full;
};

class AmarokData {
    public:
        AmarokData() :  context(0), player(0), lowerPart(0), cover(0), meta(0),
        status(0), displayBg(0), dial(0), toggleContext(0), toggleCompact(0), mainWindow(0) {}
        ~AmarokData()
        {
            if (context) context->show();
            if (lowerPart) lowerPart->show();
            if (dial && dial->client) dial->client->show();
            if (status) status->show();
            if (player)
            {
                player->setAttribute(Qt::WA_OpaquePaintEvent, false);
                player->setPalette(QPalette());
            }

            #define deleteDeferred(_thing_) if (_thing_) { _thing_->hide(); _thing_->deleteLater(); } //
            deleteDeferred(cover);
            deleteDeferred(meta);
            deleteDeferred(dial);
            deleteDeferred(toggleContext);
            deleteDeferred(toggleCompact);
            delete displayBg;
        }
        QList<QPointer<QDockWidget> > docks, visibleDocks;
        QPointer<QWidget> context, player, lowerPart;
        QPointer<CoverLabel> cover;
        QPointer<PrettyLabel> meta;
        QPointer<QStatusBar> status;
        QSize size;
        QPixmap *displayBg;
        QPointer<ProxyDial> dial;
        QPointer<QToolButton> toggleContext, toggleCompact;
        QPointer<QMainWindow> mainWindow;
};

static Hacks *bespinHacks = 0;
static Hacks::HackAppType *appType = 0;
const char *SMPlayerVideoWidget = "MplayerLayer" ;// MplayerWindow
const char *DragonVideoWidget = "Phonon::VideoWidget"; // Codeine::VideoWindow, Phonon::Xine::VideoWidget
static QPointer<QWidget> dragWidget = NULL;
static bool dragHadTrack = false;
static AmarokData *amarok = NULL;


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
            triggerWMMove(box, mev->globalPos());
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
isWindowDragWidget(QObject *o)
{
    return Hacks::config.windowMovement &&
           (qobject_cast<QDialog*>(o) ||
            qobject_cast<QMenuBar*>(o) && !static_cast<QMenuBar*>(o)->activeAction()||
            qobject_cast<QGroupBox*>(o) ||
        
            (o->inherits("QToolButton") && !static_cast<QWidget*>(o)->isEnabled()) ||
            qobject_cast<QToolBar*>(o) || qobject_cast<QDockWidget*>(o) ||
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
//         !w->rect().contains(w->mapFromGlobal(QCursor::pos()))) // KColorChooser etc., catched by mouseGrabber ?!
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
        (mev->pos().y() < 12 && qobject_cast<QDockWidget*>(w)))
        return false;

//     QMouseEvent rel(QEvent::MouseButtonRelease, mev->pos(), mev->button(),
//                     mev->buttons(), mev->modifiers());
//     QCoreApplication::sendEvent( w, &rel );
    triggerWMMove(w, mev->globalPos());
//     w->setWindowState ( w->windowState() | Qt::WindowActive );
    return true;
}

void
Hacks::swapAmarokPalette()
{
    if (!amarok->player)
        return;
    QPalette pal = amarok->player->palette();
    QColor c = pal.color(QPalette::Active, amarok->player->foregroundRole());
    int h,s,v; c.getHsv(&h,&s,&v);
    if (v < 60)
        c.setHsv(h,s,60);
    pal.setColor(amarok->player->foregroundRole(), pal.color(QPalette::Active, amarok->player->backgroundRole()));
    pal.setColor(amarok->player->backgroundRole(), c);
    //                 pal.setColor(QPalette::Button, c);
    //                 pal.setColor(QPalette::ButtonText, pal.color(QPalette::Active, frame->foregroundRole()));
    amarok->player->setPalette(pal);
#define UNSET(_W_) if (amarok->_W_) amarok->_W_->setPalette(QPalette())
    UNSET(meta);
    UNSET(dial);
    UNSET(toggleContext);
    UNSET(toggleCompact);
    UNSET(cover);
}

static void
hackAmarokPlayer(QWidget *frame)
{
//     const bool oldSchool = qobject_cast<QFrame*>(frame);
    ENSURE_INSTANCE;
    QLayout *layout = 0;
    if (qobject_cast<QBoxLayout*>(frame->layout()))
        layout = frame->layout();
    else
    {
        QList<QWidgetAction*> list = frame->findChildren<QWidgetAction*>();
        if (!list.isEmpty() && list.first()->defaultWidget())
        {
            frame = list.first()->defaultWidget();
            layout = frame->layout();
        }
    }
    amarok->player = frame;
    if (layout)
    {
        layout->setContentsMargins(0, 0, 0, 0);
        if (!amarok->dial)
        {
            QList<QSlider*> list = frame->findChildren<QSlider*>();
            foreach (QSlider *slider, list)
            {
                if (slider->inherits("Amarok::VolumeSlider"))
                {
                    if (slider->parentWidget() && slider->parentWidget()->inherits("VolumeWidget"))
                        slider->parentWidget()->hide();
                    else
                        slider->hide();
                    amarok->dial = new ProxyDial(slider, frame);
                    amarok->dial->setPageStep(10);
                    amarok->dial->setSingleStep(1);
                    if (QBoxLayout *box = qobject_cast<QBoxLayout*>(layout))
                    {
                        QLayoutItem *item = box->itemAt(box->count()-1);
                        if (item)
                        {
                            if (item->layout())
                                box->setStretchFactor(item->layout(), 100);
                            else if (item->widget())
                                box->setStretchFactor(item->widget(), 100);
                        }
                    }
                    layout->addWidget(amarok->dial);
                    layout->setAlignment(amarok->dial, Qt::AlignRight);
                    break;
                }
            }
        }
        if (!amarok->cover)
        {
            amarok->cover = new CoverLabel(frame);
            layout->addWidget(amarok->cover);
            layout->setAlignment(amarok->cover, Qt::AlignRight);
        }
    }
    
    frame->setAttribute(Qt::WA_OpaquePaintEvent);
    QList<QFrame*> list = frame->findChildren<QFrame*>();
    QObjectList oList;
    QBoxLayout *box = 0;
    QFrame *f = 0;
    
    foreach (QFrame *runner, list)
        if (runner->inherits("KVBox")) { f = runner; break; }
        
    if (f)
    {
        list = f->findChildren<QFrame*>();
        oList = f->children();
        f = 0;
        foreach (QFrame *runner, list)
            if (runner->inherits("KHBox")) { f = runner; break; }
            if (f)
                box = qobject_cast<QBoxLayout*>(f->layout());
    }

    QObject *o = 0;
    foreach (o, oList)
        if (o->inherits("ProgressWidget")) break;
    
    if (QWidget *pw = qobject_cast<QWidget*>(o))
    {
        if (!box && pw->parentWidget())
        {
            box = qobject_cast<QBoxLayout*>(pw->parentWidget()->layout());
            if ( box && box->itemAt(0)->widget() && box->itemAt(0)->widget() != o && box->itemAt(0)->widget()->layout() )
                box = qobject_cast<QHBoxLayout*>(box->itemAt(0)->widget()->layout());
            else
                box = 0; // ensure this for later injection!
        }
        
        QList<QLabel*> lList = pw->findChildren<QLabel*>();
        if (lList.size() == 2)
        {
            lList.at(0)->setFont(QFont());
            lList.at(1)->hide();
        }
        
        if (pw->layout())
            if (QBoxLayout *box2 = qobject_cast<QBoxLayout*>(pw->layout()))
            {
                QFont fnt;
                if (!amarok->toggleContext)
                {
                    amarok->toggleContext = new QToolButton(f);
                    fnt = amarok->toggleContext->font(); fnt.setBold(true);
                    amarok->toggleContext->setFont(fnt);
                    if (amarok->context)
                    {
                        amarok->toggleContext->setText(amarok->context->isVisible() ? "[|]" : "[||]");
                        amarok->toggleContext->setToolTip("Toggle ContextView");
                        bespinHacks->connect(amarok->toggleContext, SIGNAL(clicked(bool)), SLOT(toggleAmarokContext()));
                    }
                    box2->addWidget(amarok->toggleContext);
                }
                
                if (!amarok->toggleCompact)
                {
                    amarok->toggleCompact = new QToolButton(f);
                    amarok->toggleCompact->setFont(fnt);
                    amarok->toggleCompact->setText("-");
                    
                    amarok->toggleCompact->setToolTip("Toggle comapct mode");
                    box2->addWidget(amarok->toggleCompact);
                    bespinHacks->connect(amarok->toggleCompact, SIGNAL(clicked(bool)), SLOT(toggleAmarokCompact()));
                }
            }
    }
    
    if (box && !amarok->meta)
    {
        amarok->meta = new PrettyLabel(QStringList() << "Amarok² / Bespin edition" << "Click to toggle animation" << "Wheel to change item", f);
        box->addWidget(amarok->meta);
        amarok->meta->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        //                         amarok->meta->setAlignment(Qt::AlignCenter);
#ifdef Q_WS_X11
        QDBusConnection::sessionBus().connect( "org.kde.amarok", "/Player",
                                               "org.freedesktop.MediaPlayer", "CapsChange", bespinHacks, SLOT(setAmarokMetaInfo(int)) );
                                               box->addSpacing(22);
#endif
    }
    QTimer::singleShot(300, bespinHacks, SLOT(swapAmarokPalette()));
///     frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    frame->installEventFilter(bespinHacks);
}

inline static bool
paintAmarok(QWidget *w, QPaintEvent *pe)
{
    if (w == amarok->player)
    {
        if (!amarok->displayBg || amarok->displayBg->height() != w->height())
        {
            delete amarok->displayBg;
//             amarok->displayBg = new QPixmap(Gradients::pix(frame->palette().color(w->backgroundRole()), frame->height(), Qt::Vertical, Gradients::Glass));
            amarok->displayBg = new QPixmap(32, w->height());
            QLinearGradient lg( 1, 0, 1, w->height() );
            QColor c = w->palette().color(w->backgroundRole());
#if 0
            // gloss
            lg.setColorAt(0, Colors::mid(c, Qt::white, 6, 1));
            lg.setColorAt(.49, c);
            lg.setColorAt(.5, Colors::mid(c, Qt::black, 6, 1));
            lg.setColorAt(1, c);
#endif
            // smooth
            lg.setColorAt(0, Colors::mid(c, Qt::white, 6, 1));
            lg.setColorAt(.8, Colors::mid(c, Qt::black, 6, 1));
            QPainter p(amarok->displayBg);
            p.setBrush(lg); p.setPen(Qt::NoPen);
            p.drawRect(amarok->displayBg->rect());
            p.end();
        }
        QPainter p(w); p.setClipRegion(pe->region());
        p.drawTiledPixmap(w->rect(), *amarok->displayBg); p.end();
        return true;
    }
    
    if (QSlider *slider = qobject_cast<QSlider*>(w)) {
    if (slider->inherits("Amarok::Slider"))
    {
        QStyleOptionSlider opt; opt.initFrom(slider);
        opt.maximum = slider->maximum();
        opt.minimum = slider->minimum();
        // SIC! wrong set in Amarok::Slider ====== (TODO: maybe check w vs. h...)
        opt.state |= QStyle::State_Horizontal;
        opt.orientation = Qt::Horizontal;
        // ============
        opt.pageStep = slider->pageStep();
        opt.singleStep = slider->singleStep();
        opt.sliderPosition = slider->sliderPosition();
        opt.sliderValue = slider->value();
        QPainter p(slider); p.setClipRegion(pe->region());
        slider->style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, slider);
        p.end();
        return true;
    }
    return false;
    }
    
    if (QAbstractButton *btn = qobject_cast<QAbstractButton*>(w)) {
    if (btn->inherits("SideBarButton"))
    {
//         bool sunken = btn->isChecked() || btn->isDown();
        QStyleOptionToolButton opt;
        opt.initFrom(w);
        opt.text = btn->text();
        opt.toolButtonStyle = Qt::ToolButtonTextOnly;
        if (btn->isChecked())
            opt.state |= QStyle::State_On;
        if (btn->isDown())
            opt.state |= QStyle::State_Sunken;

        QPainter p(w);
        btn->style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, btn);
        // rotated text
        opt.rect.setRect(0, 0, btn->height(), btn->width());
        QMatrix m; m.translate(0, opt.rect.width()); m.rotate(-90);
        p.setMatrix(m, true);
        if (opt.state & (QStyle::State_On | QStyle::State_Sunken))
            { QFont fnt = p.font(); fnt.setBold(true); p.setFont(fnt); }
        p.setPen(opt.palette.color(opt.state & QStyle::State_MouseOver ? QPalette::Highlight : btn->foregroundRole()));
        
        p.drawText(opt.rect, Qt::AlignCenter | Qt::TextHideMnemonic, btn->text());
//         btn->style()->drawControl(QStyle::CE_ToolButtonLabel, &opt, &p, btn);

        p.end();
        return true;
    }
    return false;
    }
    return false;
}

void
Hacks::toggleAmarokContext()
{
    if (!amarok->context)
        return;
    amarok->context->setVisible(!amarok->context->isVisible());

    if (QToolButton *btn = qobject_cast<QToolButton*>(sender()))
        btn->setText(amarok->context->isVisibleTo(amarok->context->parentWidget()) ? "[|]" : "[||]");
}

void
Hacks::toggleAmarokCompact()
{
    QToolButton *btn = qobject_cast<QToolButton*>(sender());
    if (btn)
        btn->setText(amarok->size.isValid() ? "-" : "+");

    bool compact;

    if (amarok->mainWindow)
    {
        if (amarok->visibleDocks.isEmpty())
        {
            foreach (QDockWidget *dock, amarok->docks)
                if (dock && dock->isVisibleTo(amarok->mainWindow))
                {
                    amarok->visibleDocks << dock;
                    dock->hide();
                }
                compact = true;
        }
        else
        {
            foreach (QDockWidget *dock, amarok->visibleDocks)
                if (dock) dock->show();
                amarok->visibleDocks.clear();
            compact = false;
        }
    }
    else
    {
        if (!amarok->lowerPart)
        {
            if (!btn)
                return;
            if (QWidget *window = btn->window())
            {
                if (!amarok->size.isValid())
                {
                    amarok->size = window->size();
                    window->resize(QSize(600, 2));
                }
                else
                {
                    window->resize(amarok->size);
                    amarok->size = QSize();
                }
            }
            return;
        }
    
        amarok->lowerPart->setVisible(!amarok->lowerPart->isVisible());
        compact = amarok->lowerPart->isVisible();

    }

    QWidget *window = amarok->mainWindow;
    if (!window && amarok->lowerPart)
        window = amarok->lowerPart->window();
    if (window)
    {
        QWidget *statusContainer = amarok->status;
        if (statusContainer->parentWidget() && statusContainer->parentWidget()->inherits("KVBox"))
            statusContainer = statusContainer->parentWidget();
        
        if (compact)
        {
            amarok->size = window->size();
            if (statusContainer)
                statusContainer->hide();
            if (amarok->player)
            {
                int h = amarok->player->height() + 4;
                if (QMainWindow *mw = qobject_cast<QMainWindow*>(window))
                    if (mw->menuBar())
                        h += mw->menuBar()->height();
                    window->setFixedHeight(h);
            }
            window->resize(QSize(600, 2));
        }
        else
        {
            if (statusContainer) statusContainer->show();
            if (amarok->player)
                window->setMaximumHeight(0xffffff);
            window->resize(amarok->size);
        }
    }
}

void
Hacks::setAmarokMetaInfo(int)
{
#ifdef Q_WS_X11
    if (!amarok->meta)
        return;

    QDBusInterface org_kde_amarok( "org.kde.amarok", "/Player", "org.freedesktop.MediaPlayer");
    QDBusReply<QVariantMap> reply = org_kde_amarok.call("GetMetadata");
    QString toolTip;
    QStringList data;
    if (reply.isValid())
    {
        QString tmp = reply.value().value("title").toString();
        if (!tmp.isEmpty()) {
            if (tmp != "Unknown") data << tmp;
            toolTip += "<b>" + tmp + "</b>";
        }

        tmp = reply.value().value("artist").toString();
        if (!tmp.isEmpty() && tmp != "Unknown") {
            data << tmp;
            toolTip += " by " + tmp;
        }

        QString tmp2;
        tmp = reply.value().value("album").toString();
        if (!tmp.isEmpty() && tmp != "Unknown") {
            tmp2 = tmp;
            toolTip += "<br>on \"" + tmp + "\"";
        }

        tmp = reply.value().value("year").toString();
        if (!tmp.isEmpty() && tmp != "0") {
            tmp2 += " (" + tmp + ")";
            toolTip += "<br>Year: " + tmp;
        }
        if (!tmp2.isEmpty())
            data << tmp2;

        tmp = reply.value().value("genre").toString();
        if (!tmp.isEmpty() && tmp != "Unknown") {
            toolTip += "<br>Genre: " + tmp;
        }

        tmp = reply.value().value("time").toString();
        bool ok; int s = tmp.toInt(&ok);
        if (ok && s > 0) {
            int m = s/60; s = s - 60*m;
            toolTip += QString("<br>Legth: %1:%2").arg(m).arg(s);
        }

        tmp = reply.value().value("audio-bitrate").toString();
        if (!tmp.isEmpty() && tmp != "0") {
            toolTip += "<br>" + tmp + "kbps";
            tmp = reply.value().value("audio-samplerate").toString();
            if (!tmp.isEmpty() && tmp != "0") {
                toolTip += " / " + tmp + "Hz";
            }
        }
        else
        {
            tmp = reply.value().value("audio-samplerate").toString();
            if (!tmp.isEmpty() && tmp != "0") {
                toolTip += "<br>" + tmp + "Hz";
            }
        }

        if (amarok->cover)
            amarok->cover->setUrl(QUrl(reply.value().value("arturl").toString()).toLocalFile());
        
//         comment: Hans Zimmer & James Newton Howard
//         location: file:///home/music/ost/Hans%20Zimmer/The%20Dark%20Knight/01%20-%20Why%20So%20Serious.mp3
//         mtime: 554000
//         rating: 5
//         
//         tracknumber: 146059284
    }
    if (data.isEmpty())
        data << "Amarok² / Bespin edition" << "Click to toggle animation" << "Wheel to change item";
    amarok->meta->setData(data);
    if (!toolTip.isEmpty())
        amarok->meta->setToolTip(toolTip);
#endif
}

static QVector<QRect>
kmixRegion(const QProgressBar *pb)
{
    QVector<QRect> ret;
    const int n = pb->maximum() - pb->minimum();
    if (n < 1)
        return ret;

    const int chunks = pb->width()/8;
    const int last = lround((pb->value() * chunks) / (float)n);
    const float unit = 2.0 * pb->height() / (6 * chunks);
    int y;
    for (int i = 0; i < last; ++i)
    {
        y = (int)(chunks-i)*unit;
        ret << QRect(i*8, y, 5, pb->height() - 2*y);
    }
    return ret;
}

void
Hacks::setKmixMask(int)
{
    QProgressBar *pb = qobject_cast<QProgressBar*>(sender());
    if ( !(pb && pb->isWindow()) )
        return;
    QRegion mask; 
    if (pb->value() == pb->minimum())
    {
        const int sz = qMin(pb->width(), pb->height());
        mask = QRegion( 0/*(pb->width()-sz)/2*/, (pb->height()-sz)/2, sz, sz, QRegion::Ellipse );
    }
    else
    {
        QVector<QRect> rects = kmixRegion(pb);
        mask.setRects(rects.constData(), rects.count());
    }
    pb->setMask(mask);
}

bool
Hacks::eventFilter(QObject *o, QEvent *e)
{
    if (*appType == Amarok && amarok)
    {
        if (e->type() == QEvent::Paint)
            return paintAmarok(static_cast<QWidget*>(o), static_cast<QPaintEvent*>(e));

        if (e->type() == QEvent::PaletteChange)
        {
            if (o->objectName() == "MainToolbar")
            {
                delete amarok->displayBg; amarok->displayBg = 0;
//                 o->removeEventFilter(this);
//                 swapAmarokPalette(static_cast<QWidget*>(o));
//                 o->installEventFilter(this);
            }
            return false;
        }
#if 0
        if (e->type() == QEvent::MouseButtonPress)
        {
            qDebug() << "BESPIN" << o << o->parent();
            return false;
        }
#endif
        return false;
    }
    if (*appType == KMix)
    {
        if (e->type() == QEvent::Paint)
        if (QProgressBar *pb = qobject_cast<QProgressBar*>(o))
        if (pb->isWindow())
        {
            QPainter p(pb);
            p.setPen(QColor(230,230,230));
            p.setBrush(Qt::black);
            if (pb->value() == pb->minimum())
            {
                int sz = qMin(pb->width(), pb->height());
                QRect r(/*(pb->width()-sz)/2+*/1, (pb->height()-sz)/2+1, sz-1, sz-1);
                p.setRenderHint(QPainter::Antialiasing);
                p.drawEllipse(r);
                p.setBrush(QColor(255,255,255,128));
                p.setPen(Qt::NoPen);
                sz /= 5;
                QRect r2 = r.adjusted(sz, sz, -sz, -sz);
                sz = r2.width();
                QPainterPath shape(QPoint(r2.x(), r2.y()+sz/4));
                shape.lineTo(r2.x()+sz/2, r2.y()+sz/4);
                shape.lineTo(r2.right(), r2.y());
                shape.lineTo(r2.right(), r2.bottom());
                shape.lineTo(r2.x()+sz/2, r2.bottom()-sz/4);
                shape.lineTo(r2.x(), r2.bottom()-sz/4);
                shape.closeSubpath();
                p.drawPath(shape);
                p.setBrush(Qt::NoBrush);
                p.setPen(QPen(QColor(255,255,255,128), 2));
                p.drawLine(r.bottomLeft(), r.topRight());
            }
            else
            {
                QVector<QRect> rects = kmixRegion(pb);
                foreach (QRect r, rects)
                    p.drawRect(r.adjusted(0,0,-1,-1));
            }
            p.end();
            return true;
        }
    }

    if (QMessageBox* box = qobject_cast<QMessageBox*>(o))
        return hackMessageBox(box, e);

    if (e->type() == QEvent::MouseButtonPress && isWindowDragWidget(o))
    {
        QWidget *w = static_cast<QWidget*>(o);
        QMouseEvent *mev = static_cast<QMouseEvent*>(e);
        if ( w->mouseGrabber() || // someone else is more interested in this
            (mev->modifiers() != Qt::NoModifier) || // allow forcing e.g. ctrl + click
            (mev->button() != Qt::LeftButton)) // rmb shall not move, maybe resize?!
            return false;
        
        dragWidget = w;
        dragHadTrack = dragWidget->hasMouseTracking();
        dragWidget->setMouseTracking(true);
        return false;
    }
    else if (e->type() == QEvent::MouseButtonRelease)
    {
        if (dragWidget)
            dragWidget->setMouseTracking(dragHadTrack);
        dragWidget = NULL;
        return false;
    }
    else if (e->type() == QEvent::MouseMove && dragWidget)
    {
        dragWidget->setMouseTracking(dragHadTrack);
        bool ret = hackMoveWindow(dragWidget, e);
        dragWidget = NULL;
        return ret;
    }

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
        else if (QCoreApplication::applicationName() == "dragonplayer")
            *appType = Dragon;
        else if (!QCoreApplication::arguments().isEmpty() &&
                 QCoreApplication::arguments().at(0).endsWith("smplayer"))
            *appType = SMPlayer;
        else if (QCoreApplication::applicationName() == "amarok")
            *appType = Amarok;
        else if (QCoreApplication::applicationName() == "kmix")
            *appType = KMix;
    }

    if (*appType == Amarok)
    {
        if (!amarok) amarok = new AmarokData;
//         ENSURE_INSTANCE;
//         w->installEventFilter(bespinHacks);

        if (QSplitter *splitter = qobject_cast<QSplitter*>(w))
        {
            splitter->setChildrenCollapsible(true);
            if (splitter->parentWidget() && splitter->parentWidget()->parentWidget() &&
                splitter->parentWidget()->parentWidget()->inherits("MainWindow"))
                amarok->lowerPart = splitter;
        }
        else if (config.amarokDisplay && qobject_cast<QToolBar*>(w) && w->objectName() == "MainToolbar")
            hackAmarokPlayer(w);
        else if (QDockWidget *dock = qobject_cast<QDockWidget*>(w))
        {
            if (!amarok->mainWindow)
                amarok->mainWindow = qobject_cast<QMainWindow*>(dock->parentWidget());
            dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
            if (!amarok->docks.contains(dock))
                amarok->docks << dock;
        }
        else if (QFrame *frame = qobject_cast<QFrame*>(w))
        {
            if (config.amarokListView)
            if (QAbstractItemView *view = qobject_cast<QAbstractItemView*>(w))
            {
                view->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
                view->setPalette(QPalette());
                if (QWidget *viewport = view->viewport())
                {
                    viewport->setAutoFillBackground(true);
                    viewport->setPalette(QPalette());
                }
            }
            if (!(amarok->context || amarok->mainWindow) &&
                (config.amarokContext || config.amarokDisplay) &&
                w->inherits("Context::ContextView"))
            {
                QWidget *splitterKid = w;
                
                while (splitterKid->parentWidget() &&
                       !(qobject_cast<QSplitter*>(splitterKid->parentWidget()) || splitterKid->inherits("KVBox")) )
                    splitterKid = splitterKid->parentWidget();
                
                if (qobject_cast<QSplitter*>(splitterKid->parentWidget()) || splitterKid->inherits("KVBox"))
                {
//                     if (config.amarokDisplay)
                    amarok->context = splitterKid;
                    if (config.amarokContext)
                        splitterKid->hide();
                }
            }
            if (config.amarokDisplay && frame->objectName() == "MainToolbar")
                hackAmarokPlayer(frame);
            else if (config.amarokFrames)
            {
                QWidget *runner = w;
                while ((runner = runner->parentWidget()))
                {
                    if (qobject_cast<QSplitter*>(runner))
                    {
                        frame->setFrameStyle(QFrame::NoFrame);
                        break;
                    }
                }
            }
        }
        else if (config.amarokDisplay && qobject_cast<QAbstractButton*>(w))
        {
            if (w->inherits("SideBarButton"))
            {
                ENSURE_INSTANCE;
                w->setBackgroundRole(QPalette::Window);
                w->setForegroundRole(QPalette::WindowText);
                w->installEventFilter(bespinHacks);
            }
        }
//         else if (QToolBar *bar = qobject_cast<QToolBar*>(w))
//             if (bar->objectName() == "PlaylistToolBar")
//                 bar->setToolButtonStyle(Qt::ToolButtonTextOnly);
        else if (config.amarokDisplay && w->inherits("Amarok::Slider"))
        {
            ENSURE_INSTANCE;
//             QSizePolicy pol = w->sizePolicy();
//             pol.setHorizontalPolicy(QSizePolicy::Expanding);
//             pol.setHorizontalStretch(100);
//             w->setSizePolicy(pol);
            w->installEventFilter(bespinHacks);
        }
        else if (w->inherits("StatusBar"))
            amarok->status = qobject_cast<QStatusBar*>(w);
    }
    else if (*appType == KMix)
    {
        if (w->isWindow())
        if (QProgressBar *pb = qobject_cast<QProgressBar*>(w))
        {
            ENSURE_INSTANCE;
            pb->removeEventFilter(bespinHacks); // just to be sure
            pb->installEventFilter(bespinHacks);
            pb->setWindowOpacity(0.7);
            bespinHacks->disconnect(pb);
            bespinHacks->connect(pb, SIGNAL(valueChanged(int)), SLOT(setKmixMask(int)));
            return true;
        }
    }
    
    if (config.messages && qobject_cast<QMessageBox*>(w))
    {
        ENSURE_INSTANCE;
        w->setWindowFlags ( Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
        w->removeEventFilter(bespinHacks); // just to be sure
        w->installEventFilter(bespinHacks);
        return true;
    }
    
    if (config.KHTMLView)
    if (QFrame *frame = qobject_cast<QFrame*>(w))
    if (frame->inherits("KHTMLView"))
    {
        frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        return true;
    }

    if (isWindowDragWidget(w))
    {
        ENSURE_INSTANCE;
        w->removeEventFilter(bespinHacks); // just to be sure
        w->installEventFilter(bespinHacks);
        return true;
    }
#if 0
    ENSURE_INSTANCE;
    w->removeEventFilter(bespinHacks);
    w->installEventFilter(bespinHacks);
#endif

//    if (config.hack.konsole)
//    if (w->inherits("Konsole::TerminalDisplay")) {
//       w->setAttribute(Qt::WA_StyledBackground);
//       w->setAttribute(Qt::WA_MacBrushedMetal);
//       return true;
//    }
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
    delete bespinHacks; bespinHacks = 0L;
    delete amarok; amarok = 0L;
}
