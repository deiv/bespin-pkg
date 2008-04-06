
TEMPLATE    = lib
CONFIG     += plugin
HEADERS     = bconfig.h config.h
FORMS       = config.ui
SOURCES     = bconfig.cpp config.cpp
RESOURCES   = config.qrc
target.path += /usr/local/kde4/lib/kde4
target.prefix = 
INSTALLS += target
DEFINES += EXECUTABLE=0

