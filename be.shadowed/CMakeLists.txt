
PROJECT( be.shadowed )


INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}
	${QT_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${X11_INCLUDES} )
add_definitions ( -DBLIB_EXPORT=Q_DECL_IMPORT )
set(be.shadowed_SRCS be.shadowed.cpp )

kde4_add_executable(be.shadowed ${be.shadowed_SRCS})

target_link_libraries(be.shadowed QtBespin)

install(TARGETS be.shadowed DESTINATION ${BIN_INSTALL_DIR} )

# install( FILES be.shadowed.desktop  DESTINATION  ${AUTOSTART_INSTALL_DIR} )

