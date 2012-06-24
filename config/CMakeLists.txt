# the exec config tool

# ADD_DEPENDENCIES(bespin bespin_config)
# ADD_DEPENDENCIES(kstyle_bespin_config bespin_config)

# project (bespin-config)
# add_definitions ( -DB_EXPORT=Q_DECL_EXPORT )

set (bespin_config_SOURCES bconfig.cpp config.cpp kdeini.cpp)

qt4_add_resources (bespin_config_SOURCES)
qt4_wrap_ui (bespin_config_FRM_HDRS config.ui)

set (bespin_config_MOC_HDRS bconfig.h config.h dialog.h)
qt4_wrap_cpp(bespin_config_MOC_SRCS ${bespin_config_MOC_HDRS})

# add_library(bespin_config STATIC ${bespin_config_SOURCES} ${bespin_config_MOC_SRCS}  ${bespin_config_FRM_HDRS})

#--------------------------------------------------------

if(WIN32)
set(BESPIN_CONFIG bespin_cfg)
else()
set(BESPIN_CONFIG bespin)
endif()

project (${BESPIN_CONFIG})

# add_definitions ( -DB_EXPORT=Q_DECL_IMPORT )
qt4_wrap_ui (bespin_demo_FRM_HDRS uiDemo.ui)
add_executable (${BESPIN_CONFIG} main.cpp ${bespin_config_SOURCES} ${bespin_config_MOC_SRCS}  ${bespin_config_FRM_HDRS} ${bespin_demo_FRM_HDRS})
target_link_libraries (${BESPIN_CONFIG} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
if(WIN32)
    install (TARGETS ${BESPIN_CONFIG} DESTINATION ${QT_BINARY_DIR})
else(WIN32)
    SET_TARGET_PROPERTIES(${BESPIN_CONFIG} PROPERTIES OUTPUT_NAME "bespin")
    install (TARGETS ${BESPIN_CONFIG} DESTINATION bin)
endif(WIN32)

#--------------------------------------------------------

# the kcmstyle plugin, in case
if (KDE4_FOUND)

project (kstyle-bespin-config)
# add_definitions ( -DB_EXPORT=Q_DECL_IMPORT )
kde4_add_plugin (kstyle_bespin_config ${bespin_config_SOURCES} ${bespin_config_FRM_HDRS} )
target_link_libraries (kstyle_bespin_config ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
install (TARGETS kstyle_bespin_config DESTINATION ${PLUGIN_INSTALL_DIR})
   
endif (KDE4_FOUND)
