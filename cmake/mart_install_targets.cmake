
# https://cmake.org/cmake/help/latest/command/find_package.html

function(mart_install_targets)
cmake_parse_arguments(PARSE_ARGV 0 install_param "" "PACKAGE_NAME;NAMESPACE" "TARGETS;HEADER_DIRS")

set( INSTALL_DIR_CMAKE share/${install_param_PACKAGE_NAME} )
set( CONFIG_STEM_NAME ${install_param_PACKAGE_NAME}Config )

include(GNUInstallDirs)

# install results from compiling the targets and export the
# relevant information into file
install( TARGETS ${install_param_TARGETS} EXPORT ${install_param_PACKAGE_NAME}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

foreach( dir IN LISTS install_param_HEADER_DIRS)
	install( DIRECTORY ${dir} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )
endforeach()


# install the CmakeConfig file holding the infromation about the isntalled targets
install( EXPORT ${install_param_PACKAGE_NAME}
	FILE
		${CONFIG_STEM_NAME}.cmake
	NAMESPACE
		${install_param_NAMESPACE}
	DESTINATION
		${INSTALL_DIR_CMAKE}
)

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
		${INSTALL_DIR_CMAKE}
)

endfunction()
