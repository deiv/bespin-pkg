; customizable part you may skip if you're on OS-X or M$ ------------------------------------
unix {
;    on unix systems (linux)
   CONFIG += x11
; this can talk to kwin
   HEADERS += xproperty.h
   SOURCES += xproperty.cpp
   
;    xrender is assumed on unix systems (linux) - if you don't want (stupid idea) or have (e.g. qtopia)
;    comment the 3 lines below with a ';' (compiling will fail otherwise)
   HEADERS += oxrender.h
   SOURCES += oxrender.cpp
   LIBS += -lXrender
   
;    not interested in a nice macalike menubar? comment the 3 lines below with a ';'
   HEADERS += macmenu.h macmenu-dbus.h
   SOURCES += macmenu.cpp
   QT += dbus
}

; no more editing after this line --------------------------------------

HEADERS = animator/basic.h animator/aprogress.h animator/hover.h \
          animator/hoverindex.h animator/hovercomplex.h animator/tab.h \
          colors.h bespin.h tileset.h debug.h \
          visualframe.h gradients.h draw.h config.h types.h\
          hacks.h

SOURCES = animator/basic.cpp animator/aprogress.cpp animator/hover.cpp \
          animator/hoverindex.cpp animator/hovercomplex.cpp animator/tab.cpp \
          colors.cpp bespin.cpp tileset.cpp stylehint.cpp \
          sizefromcontents.cpp qsubcmetrics.cpp \
          pixelmetric.cpp stdpix.cpp \
          visualframe.cpp gradients.cpp init.cpp genpixmaps.cpp polish.cpp \
          buttons.cpp docks.cpp frames.cpp input.cpp menus.cpp progress.cpp \
          scrollareas.cpp shapes.cpp slider.cpp tabbing.cpp toolbars.cpp \
          views.cpp window.cpp hacks.cpp

TEMPLATE = lib
PLUGIN = true
CONFIG += qt plugin

QT += qt3support


VERSION       = 0.1
target.path += $$[QT_INSTALL_PLUGINS]/styles
INSTALLS += target DATA
