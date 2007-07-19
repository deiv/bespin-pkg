TEMPLATE    = app
HEADERS     = bconfig.h config.h
FORMS       = config.ui
SOURCES     = bconfig.cpp config.cpp
RESOURCES   = config.qrc
target.path += $$(PREFIX)/bin
INSTALLS += target
