
# https://cmake.org/cmake/help/latest/command/find_package.html

function(install_targets targets tl_include_dirs namespace)

set( INSTALL_DIR_CMAKE share/MartCommon )
set( CONFIG_STEM_NAME MartCommonConfig )

include(GNUInstallDirs)

# install results from compiling the targets and export the
# relevant information into file
install( TARGETS ${targets} EXPORT ${CONFIG_STEM_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# install the CmakeConfig file holding the infromation about the isntalled targets
install( EXPORT ${CONFIG_STEM_NAME}
	FILE
		${CONFIG_STEM_NAME}.cmake
	NAMESPACE
		${namespace}
	DESTINATION
		${INSTALL_DIR_CMAKE}
)


foreach(dir IN LISTS tl_include_dirs)
	install( DIRECTORY include/${dir} DESTINATION include/ )
endforeach()



include(CMakePackageConfigHelpers)

write_basic_package_version_file("${CONFIG_STEM_NAME}Version.cmake"
	VERSION ${${PROJECT_NAME}_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_STEM_NAME}Version.cmake"
	DESTINATION
		${INSTALL_DIR_CMAKE}
)

install(
	FILES
		"LICENSE.md"
	DESTINATION
		share
)

endfunction()
