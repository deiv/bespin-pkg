TEMPLATE    = app
HEADERS     = bconfig.h config.h dialog.h kdeini.h
FORMS       = config.ui uiDemo.ui
SOURCES     = main.cpp bconfig.cpp config.cpp kdeini.cpp
DEFINES += EXECUTABLE=1
TARGET = bespin
QMAKE_CXXFLAGS += -fPIC
