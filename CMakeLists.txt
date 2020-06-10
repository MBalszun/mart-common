cmake_minimum_required( VERSION 3.9 )
project( ImStr LANGUAGES CXX VERSION 0.1 )

get_directory_property( hasParent PARENT_DIRECTORY )

if( NOT hasParent )
	set( IM_STR_INCLUDE_TESTS_DEFAULT ON )
else()
	set( IM_STR_INCLUDE_TESTS_DEFAULT OFF )
endif()

option( IM_STR_INCLUDE_TESTS "Build tests for ImStr library" ${IM_STR_INCLUDE_TESTS_DEFAULT} )

add_library( mart_im_str INTERFACE )
add_library( ImStr::im_str ALIAS mart_im_str )

target_include_directories( mart_im_str
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_compile_features( mart_im_str INTERFACE cxx_std_17)

if( ${IM_STR_INCLUDE_TESTS} )
	enable_testing()
	add_subdirectory( tests )
endif()
