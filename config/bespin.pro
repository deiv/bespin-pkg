TEMPLATE    = app
HEADERS     = bconfig.h config.h dialog.h
FORMS       = config.ui uiDemo.ui
SOURCES     = main.cpp bconfig.cpp config.cpp
RESOURCES   = config.qrc
target.path = $$(PREFIX)/bin
INSTALLS = target
DEFINES += EXECUTABLE=1
unix {
HEADERS += bpp.h
SOURCES += bpp.cpp
DEFINES += PUSHER=1
}