#include <QApplication>
#include <QX11Info>
#include <QtDebug>

#include "../blib/shadows.h"
#include "../blib/xproperty.h"

#include <X11/Xlib.h>

using namespace Bespin;

class Application : public QApplication
{
public:
    Application(int &argc , char **argv) : QApplication(argc, argv) {
        XSelectInput(QX11Info::display(), QX11Info::appRootWindow(), SubstructureNotifyMask);
    }
protected:
    bool x11EventFilter(XEvent *event) {
        if (event->type == MapNotify) {
            shadow(event->xmap.window);
        }
        return QApplication::x11EventFilter(event);
    }
private:
    void shadow(WId window) {
        if (!Shadows::areSet(window)) {
            Display *dpy = QX11Info::display();
            unsigned long n(0);
            static Atom noshadow = XInternAtom(dpy, "_KDE_SHADOW_OVERRIDE", False);
            Atom *blocked = XProperty::get<Atom>(window, noshadow, XProperty::ATOM, &n);
            if (n) { // this window does explicitly not want to be shadowed
                XFree(blocked);
                return;
            }

            static Atom wm_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
#define SUPPORTED_TYPES 4
            static Atom supportedTypes[SUPPORTED_TYPES] = {
                XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU", False),
                XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_POPUP_MENU", False),
                XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_TOOLTIP", False),
//             XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False),
//             XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_TOOLBAR", False),
//             XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_MENU", False),
//             XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_UTILITY", False),
//             XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_SPLASH", False),
//             XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False),
//             XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NOTIFICATION", False),
            XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_COMBO", False)
//                 XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NORMAL", False) // chromium case? - requires MapRequest...
            };

            Atom *types = XProperty::get<Atom>(window, wm_type, XProperty::ATOM, &n);

            // naked, untyped window - the OOo case :-(
            if (!n) {
                if (XGrabPointer(dpy, window, false, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) == Success) {
                    // not grabbed at all, unlikely a popup
                    XUngrabPointer(dpy, CurrentTime);
                    return;
                }
                // NOTICE: it's not sure that this is a popup!
                //         and not even sure that this window grabs the mouse - just some does.
                Shadows::set(window, Shadows::Small, true);
            }

            // check whether one of the set types matches a "likes to be shadowed" one.
            bool notDone = true;
            for (uint i = 0; i < n && notDone; ++i) {
                for (uint j = 0; j < SUPPORTED_TYPES && notDone; ++j) {
                    if (types[i] == supportedTypes[j]) {
                        notDone = false;
                        Shadows::set(window, Shadows::Small, true);
                    }
                }
            }
            if (n)
                XFree(types);
        }
    }
};

int main(int argc, char **argv) {
    Application a(argc, argv);
    return a.exec();
}
