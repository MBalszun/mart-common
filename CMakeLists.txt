cmake_minimum_required(VERSION 3.13)

if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW) # allow to override options in submake files by setting the corresponding vairable locally
endif()

# guard against multiple inclusions in a single super project as we have sometimes in the mart project
if(TARGET mart-common )
	get_target_property(PREV_SOURCE_DIR mart-common SOURCE_DIR)
	message(WARNING "[MART-COMMON] -- \"${PROJECT_NAME}\" included multiple times --\n-- Only one directory will be built --\n-- Selected dir: ${PREV_SOURCE_DIR}")
	return()
endif()

project( MartCommon VERSION 3.0 LANGUAGES CXX)

option( MART_COMMON_INCLUDE_TESTS "Build tests" OFF )
option( MART_COMMON_INCLUDE_EXAMPLES "Build examples" OFF)
option( MART_COMMON_INCLUDE_NET_LIB "Also build netlib components (Those are not header only)" ON)

add_subdirectory(im_str)

add_library(Mart::im_str ALIAS mart_im_str)

####### create mart-common library
#
add_library( mart-common INTERFACE )
add_library( Mart::common ALIAS mart-common )
set_target_properties ( mart-common PROPERTIES EXPORT_NAME common )
target_link_libraries( mart-common INTERFACE Mart::im_str )

target_include_directories( mart-common
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)
target_compile_features( mart-common INTERFACE cxx_std_17 )


####### create mart-netlib library
#
if(MART_COMMON_INCLUDE_NET_LIB)

	add_library( mart-netlib )
	add_library( Mart::netlib ALIAS mart-netlib)
	set_target_properties ( mart-netlib PROPERTIES EXPORT_NAME netlib )

	# here, only interface properties are set, the build properties are set in src/mart-netlib
	target_link_libraries( mart-netlib PUBLIC Mart::common Mart::netlib-portlayer)

	target_compile_features( mart-netlib INTERFACE cxx_std_17 )
	target_include_directories( mart-netlib
		INTERFACE
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
			$<INSTALL_INTERFACE:include>
	)

	add_subdirectory( src/mart-netlib )

endif()


if( MART_COMMON_INCLUDE_TESTS )
	enable_testing()
	add_subdirectory( tests )
endif()

# support installation
get_directory_property( hasParent PARENT_DIRECTORY )
#if( False ) #Visual studio 2019 currently (16.2.0) fails when having an installation target
if( NOT hasParent )
	include( cmake/install_targets.cmake )

	set_target_properties ( mart_im_str PROPERTIES EXPORT_NAME im_str )
	install_targets( "mart_im_str"  Mart:: "${CMAKE_CURRENT_LIST_DIR}/im_str/include" "im_str") #todo: ugly hack - this should probably not work
	install_targets( "mart-common;mart-netlib;mart-netlib-portlayer" Mart:: "${CMAKE_CURRENT_LIST_DIR}/include"  "mart-common;mart-netlib")
endif()


if( MART_COMMON_INCLUDE_EXAMPLES )
	add_subdirectory(examples/nw)
endif()

