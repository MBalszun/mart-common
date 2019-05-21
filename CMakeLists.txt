cmake_minimum_required(VERSION 3.12)

# guard against multiple inclusions in a single super project as we have sometimes in the mart project
if(TARGET mart-common )
	get_target_property(PREV_SOURCE_DIR mart-common SOURCE_DIR)
	message(WARNING "[MART-COMMON] -- \"${PROJECT_NAME}\" included multiple times --\n-- Only one directory will be built --\n-- Selected dir: ${PREV_SOURCE_DIR}")
	return()
endif()

project( MartCommon VERSION 2.0 )

option( MART_COMMON_INCLUDE_TESTS "Build tests" OFF )
option( MART_COMMON_INCLUDE_EXAMPLES "Build examples" OFF)

add_library( mart-common INTERFACE )
add_library( Mart::common ALIAS mart-common )
add_subdirectory(./include/mart-common/experimental/net)

target_include_directories( mart-common
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)
target_compile_features( mart-common INTERFACE cxx_std_17 )


if( MART_COMMON_INCLUDE_TESTS )
	enable_testing()
	add_subdirectory( tests )
endif()

# support installation
get_directory_property( hasParent PARENT_DIRECTORY )
if( NOT hasParent )
	include( cmake/install_targets.cmake )
	set_target_properties ( mart-common PROPERTIES EXPORT_NAME common )
	install_targets( mart-common Mart:: )
endif()


if( MART_COMMON_INCLUDE_EXAMPLES )
	add_subdirectory(examples/nw)
endif()

