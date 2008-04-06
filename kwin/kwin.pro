
HEADERS = button.h client.h ../colors.h factory.h ../gradients.h ../xproperty.h
SOURCES = button.cpp client.cpp ../colors.cpp factory.cpp ../gradients.cpp ../xproperty.cpp
INCLUDEPATH += /usr/local/kde4/include /usr/local/kde4/include/KDE
LIBS += -L/usr/local/kde4/lib -lkdecorations
TARGET = kwin3_bespin
TEMPLATE = lib
PLUGIN = true
CONFIG += qt x11 plugin
VERSION       = 0.1
target.path += /usr/local/kde4/lib/kde4/
DATA.files = bespin.desktop
DATA.path += /usr/local/kde4/share/apps/kwin
INSTALLS += target DATA
DEFINES += BESPIN_DECO

