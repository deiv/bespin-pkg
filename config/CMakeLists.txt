# the exec config tool
project (bespin-config)

set (bespin_config_SOURCES main.cpp bconfig.cpp config.cpp kdeini.cpp)
qt4_add_resources (bespin_config_SOURCES)
qt4_wrap_ui (bespin_config_FRM_HDRS config.ui uiDemo.ui)

set (bespin_config_MOC_HDRS bconfig.h config.h dialog.h)
qt4_wrap_cpp(bespin_config_MOC_SRCS ${bespin_config_MOC_HDRS})

add_executable (bespin ${bespin_config_SOURCES} ${bespin_config_MOC_SRCS} ${bespin_config_FRM_HDRS})
target_link_libraries (bespin ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
install (TARGETS bespin DESTINATION bin)

# the kcmstyle plugin, in case
if (KDE4_FOUND)
project (kstyle-bespin-config)
set (kstyle_bespin_config_SOURCES bconfig.cpp config.cpp kdeini.cpp)
qt4_add_resources (kstyle_bespin_config_SOURCES)
qt4_wrap_ui (kstyle_bespin_config_FRM_HDRS config.ui)

# set (kstyle_bespin_config_MOC_HDRS bconfig.h config.h)
# kde4_add_plugin automocs, so either use automoc or postcompile rename
# qt4_wrap_cpp(kstyle_bespin_config_MOC_SRCS ${kstyle_bespin_config_MOC_HDRS})

kde4_add_plugin (kstyle_bespin_config ${kstyle_bespin_config_SOURCES} ${kstyle_bespin_config_FRM_HDRS})
target_link_libraries (kstyle_bespin_config ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
install (TARGETS kstyle_bespin_config DESTINATION ${PLUGIN_INSTALL_DIR})
   
endif (KDE4_FOUND)
