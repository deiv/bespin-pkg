project(bespin)
cmake_minimum_required(VERSION 2.4)

add_definitions( ${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} )

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )

option(ENABLE_KDE "Enable Bespin support for KDE4" ON)
option(ENABLE_KWIN "Build Bespin KWin decoration" ON)
option(ENABLE_XBAR "Enable the XBar plasmoid (Mac-Alike-Menubar)" ON)

if (ENABLE_KDE)
    find_package(KDE4)
endif (ENABLE_KDE)

find_package(X11)

# add_definitions (-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)

if (KDE4_FOUND)
   include (KDE4Defaults)
   include (MacroLibrary)
   add_definitions ( ${KDE4_DEFINITIONS} )
   include_directories (${KDE4_INCLUDES})
else (KDE4_FOUND)
   message (STATUS "WARNING: *** KDE4 not found, just the style will be built ***")
   set (QT_MIN_VERSION "4.3.0")
   find_package (Qt4 REQUIRED)
   include (UseQt4)
   set (CMAKE_INCLUDE_CURRENT_DIR ON)
endif (KDE4_FOUND)

set (bespin_SOURCES animator/basic.cpp animator/aprogress.cpp animator/hover.cpp
animator/hoverindex.cpp animator/hovercomplex.cpp animator/tab.cpp
colors.cpp bespin.cpp tileset.cpp stylehint.cpp
sizefromcontents.cpp qsubcmetrics.cpp
pixelmetric.cpp stdpix.cpp
visualframe.cpp gradients.cpp init.cpp genpixmaps.cpp polish.cpp
buttons.cpp docks.cpp frames.cpp input.cpp menus.cpp progress.cpp
scrollareas.cpp shapes.cpp slider.cpp tabbing.cpp toolbars.cpp
views.cpp window.cpp hacks.cpp macmenu.cpp)

if ( X11_FOUND )
   set (bespin_SOURCES ${bespin_SOURCES} xproperty.cpp)
endif ( X11_FOUND )
if (X11_Xrender_FOUND)
   message (STATUS "INFO: XRender was found - kwin deco & FX via GPU available!")
   set (bespin_SOURCES ${bespin_SOURCES} oxrender.cpp)
endif (X11_Xrender_FOUND)

set (bespin_MOC_HDRS animator/basic.h animator/aprogress.h animator/hover.h
animator/hoverindex.h animator/tab.h
bespin.h visualframe.h hacks.h macmenu.h macmenu-dbus.h)
qt4_wrap_cpp(bespin_MOC_SRCS ${bespin_MOC_HDRS})

add_library (bespin SHARED ${bespin_SOURCES} ${bespin_MOC_SRCS})


target_link_libraries(bespin  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
                              ${QT_QTDBUS_LIBRARY} ${QT_QT3SUPPORT_LIBRARY})
if ( X11_FOUND )
   target_link_libraries(bespin ${X11_LIBRARIES})
endif( X11_FOUND )
if (X11_Xrender_FOUND)
   target_link_libraries(bespin ${X11_Xrender_LIB})
endif (X11_Xrender_FOUND)

install (TARGETS bespin LIBRARY DESTINATION ${QT_PLUGINS_DIR}/styles)

add_subdirectory (config)

if (KDE4_FOUND)
    install (FILES bespin.themerc DESTINATION ${DATA_INSTALL_DIR}/kstyle/themes)
    if (X11_Xrender_FOUND)
        if(ENABLE_KWIN)
            add_subdirectory (kwin)
        else(ENABLE_KWIN)
            message (STATUS "WARNING: *** KWin decoration will not be built ***")
        endif(ENABLE_KWIN)
    endif (X11_Xrender_FOUND)
    if ( X11_FOUND )
        if(ENABLE_XBAR)
            add_subdirectory (XBar)
        else(ENABLE_XBAR)
            message (STATUS "WARNING: *** XBar plasmoid will not be built ***")
        endif(ENABLE_XBAR)
    endif ( X11_FOUND )
endif (KDE4_FOUND)
