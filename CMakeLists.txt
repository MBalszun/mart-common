cmake_minimum_required( VERSION 3.9 )
project( ImStr LANGUAGES CXX VERSION 0.1 )

option( IM_STR_INCLUDE_TESTS "Build tests for ImStr library" OFF )

get_directory_property( hasParent PARENT_DIRECTORY )

if( NOT hasParent )
	set( CMAKE_CXX_STANDARD 17)
	set( CMAKE_CXX_STANDARD_REQUIRED ON )
	set( CMAKE_CXX_EXTENSIONS OFF )
endif()

add_library( mart_im_str INTERFACE )
add_library( ImStr::im_str ALIAS mart_im_str )

target_include_directories( mart_im_str
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

if( ${IM_STR_INCLUDE_TESTS} )
	enable_testing()
	add_subdirectory( tests )
endif()
