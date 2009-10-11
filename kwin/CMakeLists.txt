project (kwin3-bespin)

add_definitions (-DBESPIN_DECO)

set (kwin3_bespin_SOURCES button.cpp client.cpp resizecorner.cpp
../colors.cpp factory.cpp ../gradients.cpp ../xproperty.cpp ../paths.cpp ../oxrender.cpp)
# kde4_add_plugin automocs, so either use automoc or postcompile rename
# set (kwin3_bespin_MOC_HDRS button.h client.h resizecorner.h)
# qt4_wrap_cpp(kwin3_bespin_MOC_SRCS ${kwin3_bespin_MOC_HDRS})

kde4_add_plugin (kwin3_bespin ${kwin3_bespin_SOURCES})
target_link_libraries (kwin3_bespin ${KDE4_KDEUI_LIBS} kdecorations ${X11_LIBRARIES} ${X11_Xrender_LIB})
install (TARGETS kwin3_bespin DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES bespin.desktop DESTINATION ${DATA_INSTALL_DIR}/kwin)


project (kwin-bespin-config)
set (kwin_bespin_config_SOURCES ../config/bconfig.cpp config.cpp)
qt4_wrap_ui (kwin_bespin_config_FRM_HDRS configdialog.ui)
kde4_add_plugin (kwin_bespin_config ${kwin_bespin_config_SOURCES} ${kwin_bespin_config_FRM_HDRS})
target_link_libraries (kwin_bespin_config ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS})
install (TARGETS kwin_bespin_config DESTINATION ${PLUGIN_INSTALL_DIR})

