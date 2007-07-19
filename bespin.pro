HEADERS = oxrender.h bespin.h tileset.h debug.h eventkiller.h \
          visualframe.h gradients.h styleanimator.h
SOURCES = oxrender.cpp bespin.cpp tileset.cpp stylehint.cpp \
          sizefromcontents.cpp qsubcmetrics.cpp \
          pixelmetric.cpp stdpix.cpp styleanimator.cpp \
          drawcomplexcontrol.cpp drawcontrol.cpp \
          drawprimitive.cpp visualframe.cpp gradients.cpp
TEMPLATE = lib
PLUGIN = true
CONFIG += qt x11 plugin
QT += qt3support
VERSION       = 0.1
target.path += $$[QT_INSTALL_PLUGINS]/styles
DATA.files = bespin.themerc
DATA.path += $$(KDEDIR)/share/apps/kstyle/themes/
INSTALLS += target DATA