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
#include <QDesktopWidget>
#include <QDial>
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
#include <QPushButton>
#include <QSplitter>
#include <QSlider>
#include <QStatusBar>
#include <QStyle>
#include <QStyleOption>
#include <QStyleOptionGroupBox>
#include <QStyleOptionSlider>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "fixx11h.h"
#include <QX11Info>

static Atom netMoveResize = XInternAtom(QX11Info::display(), "_NET_WM_MOVERESIZE", False);
#endif

#include <QtDebug>
#include "colors.h"
#include "gradients.h"
#include "hacks.h"

using namespace Bespin;

static const int DT = 4000; // display duration
static const int FT = 500; // fade duration > 50!!!!

class ProxyDial : public QDial
{
public:
    ProxyDial(QAbstractSlider *slider, QWidget *parent = 0) : QDial(parent)
    {
        setRange(slider->minimum(), slider->maximum()/*0, 100*/);
        setValue(slider->value());
        connect (slider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
        connect (this, SIGNAL(valueChanged(int)), slider, SIGNAL(sliderMoved(int)));
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
        p.drawText(rect(), Qt::AlignCenter | Qt::TextHideMnemonic | Qt::TextSingleLine, data.at(index));
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
        AmarokData() : context(0), player(0), lowerPart(0), cover(0), meta(0), status(0), displayBg(0) {}
        QPointer<QWidget> context, player, lowerPart;
        QPointer<CoverLabel> cover;
        QPointer<PrettyLabel> meta;
        QPointer<QStatusBar> status;
        QSize size;
        QPixmap *displayBg;
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
    return Hacks::config.windowMovement &&
           (qobject_cast<QDialog*>(o) ||
            qobject_cast<QMenuBar*>(o) && !static_cast<QMenuBar*>(o)->activeAction()||
            qobject_cast<QGroupBox*>(o) ||
        
            (o->inherits("QToolButton") && !static_cast<QWidget*>(o)->isEnabled()) ||
            qobject_cast<QToolBar*>(o) || o->inherits("QDockWidget") ||
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
        (mev->pos().y() < 12 && w->inherits("QDockWidget")))
        return false;

//     QMouseEvent rel(QEvent::MouseButtonRelease, mev->pos(), mev->button(),
//                     mev->buttons(), mev->modifiers());
//     QCoreApplication::sendEvent( w, &rel );
    triggerWMMove(w, mev->globalPos());
//     w->setWindowState ( w->windowState() | Qt::WindowActive );
    return true;
}

inline static bool
paintAmarok(QWidget *w, QPaintEvent *pe)
{
    if (QFrame *frame = qobject_cast<QFrame*>(w)) {
    if (frame == amarok->player)
    {
        if (!amarok->displayBg || amarok->displayBg->height() != frame->height())
        {
            delete amarok->displayBg;
//             amarok->displayBg = new QPixmap(Gradients::pix(frame->palette().color(w->backgroundRole()), frame->height(), Qt::Vertical, Gradients::Glass));
            amarok->displayBg = new QPixmap(32, frame->height());
            QLinearGradient lg( 1, 0, 1, frame->height() );
            QColor c = frame->palette().color(w->backgroundRole());
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
    return false;
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
#if 0 // we have a space conserving dial now...
        if (slider->inherits("Amarok::VolumeSlider"))
        {
            //TODO: maybe volume icon...
            if (slider->testAttribute(Qt::WA_UnderMouse))
            {
                const QRect rect( opt.rect.right()-40, 0, 40, slider->height() );
                p.drawText( rect, Qt::AlignRight | Qt::AlignVCenter, QString::number( slider->value() ) + '%' );
            }
            QRect r = opt.rect;
            opt.rect.setWidth(r.height()*.877);
            const bool hover = opt.state & QStyle::State_MouseOver;
            opt.state |= QStyle::State_MouseOver;
            QPixmap icon = slider->style()->standardPixmap(opt.sliderValue ? QStyle::SP_MediaVolume : QStyle::SP_MediaVolumeMuted, &opt, slider);
            if (!hover)
                opt.state &= ~QStyle::State_MouseOver;
            p.drawPixmap(opt.rect.topLeft(), icon);
            opt.rect = r;
            opt.rect.adjust(icon.width()+4, 0, -44, 0);
        }
#endif
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
    if (!amarok->lowerPart)
        return;
    amarok->lowerPart->setVisible(!amarok->lowerPart->isVisible());

    if (QToolButton *btn = qobject_cast<QToolButton*>(sender()))
        btn->setText(amarok->lowerPart->isVisible() ? "-" : "+");

    if (QWidget *window = amarok->lowerPart->window())
    {
        QWidget *statusContainer = amarok->status;
        if (statusContainer->parentWidget() && statusContainer->parentWidget()->inherits("KVBox"))
            statusContainer = statusContainer->parentWidget();
        
        if (amarok->lowerPart->isVisible())
        {
            if (statusContainer) statusContainer->show();
            if (amarok->player)
                window->setMaximumHeight(0xffffff);
            window->resize(amarok->size);
        }
        else
        {
            amarok->size = window->size();
            if (statusContainer) statusContainer->hide();
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
    }
}

void
Hacks::setAmarokMetaInfo(int)
{
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
            toolTip += "<br>by " + tmp;
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
}

bool
Hacks::eventFilter(QObject *o, QEvent *e)
{
    if (*appType == Amarok)
    {
        if (e->type() == QEvent::Paint)
            return paintAmarok(static_cast<QWidget*>(o), static_cast<QPaintEvent*>(e));

        if (e->type() == QEvent::PaletteChange)
        {
            if (o->objectName() == "MainToolbar")
                { delete amarok->displayBg; amarok->displayBg = 0; }
            return false;
        }
        return false;
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
#define ENSURE_INSTANCE if (!bespinHacks) bespinHacks = new Hacks
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
    }

    if (*appType == Amarok)
    {
        if (!amarok) amarok = new AmarokData;
        if (QSplitter *splitter = qobject_cast<QSplitter*>(w))
        {
            splitter->setChildrenCollapsible(true);
            if (splitter->parentWidget() && splitter->parentWidget()->parentWidget() &&
                splitter->parentWidget()->parentWidget()->inherits("MainWindow"))
                amarok->lowerPart = splitter;
        }
        else if (QFrame *frame = qobject_cast<QFrame*>(w))
        {
            if ((config.amarokContext || config.amarokDisplay)
                && w->inherits("Context::ContextView"))
            {
                QWidget *splitterKid = w;
                
                while (splitterKid->parentWidget() && !qobject_cast<QSplitter*>(splitterKid->parentWidget()))
                    splitterKid = splitterKid->parentWidget();
                
                if (qobject_cast<QSplitter*>(splitterKid->parentWidget()))
                {
                    if (config.amarokDisplay)
                        amarok->context = splitterKid;
                    if (config.amarokContext)
                        splitterKid->hide();
                }
            }
            if (config.amarokDisplay && frame->objectName() == "MainToolbar")
            {
                ENSURE_INSTANCE;
                amarok->player = frame;
                if (QBoxLayout *box = qobject_cast<QBoxLayout*>(frame->layout()))
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
                            ProxyDial *dial = new ProxyDial(slider, frame);
                            dial->setPageStep(10);
                            dial->setSingleStep(1);
                            box->addWidget(dial);
                            break;
                        }
                    }
                    amarok->cover = new CoverLabel(frame);
                    box->addWidget(amarok->cover);
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
                        box = qobject_cast<QBoxLayout*>(pw->parentWidget()->layout());
                    if ( box && box->itemAt(0)->widget() && box->itemAt(0)->widget() != o && box->itemAt(0)->widget()->layout() )
                        box = qobject_cast<QHBoxLayout*>(box->itemAt(0)->widget()->layout());
                    else
                        box = 0; // ensure this for later injection!

                    QList<QLabel*> lList = pw->findChildren<QLabel*>();
                    if (lList.size() == 2)
                    {
                        lList.at(0)->setFont(QFont());
                        lList.at(1)->hide();
                    }
                    
                    if (pw->layout())
                    if (QBoxLayout *box = qobject_cast<QBoxLayout*>(pw->layout()))
                    {
                        QToolButton *btn = new QToolButton(f);
                        QFont fnt = btn->font(); fnt.setBold(true);
                        btn->setFont(fnt);
                        btn->setText(amarok->context && !amarok->context->isVisible() ? "[||]" : "[|]");

                        btn->setToolTip("Toggle ContextView");
                        box->addWidget(btn);
                        connect (btn, SIGNAL(clicked(bool)), bespinHacks, SLOT(toggleAmarokContext())); // TODO: bind toggle?

                        btn = new QToolButton(f);
                        btn->setFont(fnt);
                        btn->setText("-");

                        btn->setToolTip("Toggle comapct mode");
                        box->addWidget(btn);
                        connect (btn, SIGNAL(clicked(bool)), bespinHacks, SLOT(toggleAmarokCompact())); // TODO: bind toggle?
                    }
                }

                if (box)
                {
                    amarok->meta = new PrettyLabel(QStringList() << "Amarok² / Bespin edition" << "Click to toggle animation" << "Wheel to change item", f);
                    box->addWidget(amarok->meta);
                    amarok->meta->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                    //                         amarok->meta->setAlignment(Qt::AlignCenter);
                    QDBusConnection::sessionBus().connect( "org.kde.amarok", "/Player",
                                                           "org.freedesktop.MediaPlayer", "CapsChange", bespinHacks, SLOT(setAmarokMetaInfo(int)) );
                    box->addSpacing(22);
                }
                
                frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
                QPalette pal = frame->palette();
                QColor c = pal.color(QPalette::Active, frame->foregroundRole());
                int h,s,v; c.getHsv(&h,&s,&v);
                if (v < 60)
                    c.setHsv(h,s,60);
                pal.setColor(frame->foregroundRole(), pal.color(QPalette::Active, frame->backgroundRole()));
                pal.setColor(frame->backgroundRole(), c);
//                 pal.setColor(QPalette::Button, c);
//                 pal.setColor(QPalette::ButtonText, pal.color(QPalette::Active, frame->foregroundRole()));
                frame->setPalette(pal);
                frame->installEventFilter(bespinHacks);
            }
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
            { ENSURE_INSTANCE; w->installEventFilter(bespinHacks); }
        else if (w->inherits("StatusBar"))
            amarok->status = qobject_cast<QStatusBar*>(w);
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
