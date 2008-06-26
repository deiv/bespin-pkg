project(plasma-xbar)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include(MacroOptionalAddSubdirectory)

find_package(Plasma REQUIRED)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${PLASMA_INCLUDE_DIR})

set(xbar_SRCS menubar.cpp xbar.cpp taskbar.cpp)

kde4_add_plugin(plasma_applet_xbar ${xbar_SRCS})
target_link_libraries(plasma_applet_xbar kworkspace ${KDE4_KDEUI_LIBS} ${PLASMA_LIBS} )

install(TARGETS plasma_applet_xbar DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-xbar.desktop DESTINATION ${SERVICES_INSTALL_DIR})
