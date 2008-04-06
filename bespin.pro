
HEADERS = colors.h bespin.h tileset.h debug.h eventkiller.h \
          visualframe.h gradients.h styleanimator.h draw.h config.h types.h\
          hacks.h xproperty.h
SOURCES = colors.cpp bespin.cpp tileset.cpp stylehint.cpp \
          sizefromcontents.cpp qsubcmetrics.cpp \
          pixelmetric.cpp stdpix.cpp styleanimator.cpp \
          visualframe.cpp gradients.cpp init.cpp genpixmaps.cpp polish.cpp \
          buttons.cpp docks.cpp frames.cpp input.cpp menus.cpp progress.cpp \
          scrollareas.cpp shapes.cpp slider.cpp tabbing.cpp toolbars.cpp \
          views.cpp window.cpp hacks.cpp xproperty.cpp
unix {
   HEADERS += oxrender.h
   SOURCES += oxrender.cpp
}
TEMPLATE = lib
PLUGIN = true
CONFIG += qt x11 plugin
QT += qt3support
VERSION       = 0.1
target.path += $$[QT_INSTALL_PLUGINS]/styles
INSTALLS += target DATA
DEFINES += SHAPE_POPUP=0


DATA.files = bespin.themerc
DATA.path += /usr/local/kde4/share/apps/kstyle/themes/

