TEMPLATE    = app
HEADERS     = bconfig.h config.h dialog.h
FORMS       = config.ui uiDemo.ui
SOURCES     = bconfig.cpp config.cpp
RESOURCES   = config.qrc
target.path += $$(PREFIX)/bin
INSTALLS += target
DEFINES += EXECUTABLE=1
