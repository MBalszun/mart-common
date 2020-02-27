
if(MSVC)
	string( REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	add_compile_options( /W4 )
else()
	add_compile_options( -Wall -Wextra )
endif()

######
# netlib-portlayer library
#
# A minimal abstraction over os-specific bsd-socket-api
#
# NOTE: The port-layer is supposed to be a c++11 library, while the rest is c++17.
#		In order do make sure that we don't accidentially use c++14/17 features
#		this library is explicitly compiled in c++11 mode
#

add_library( mart-netlib-portlayer STATIC ${CMAKE_CURRENT_SOURCE_DIR}/port_layer.cpp )
target_include_directories( mart-netlib-portlayer
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)
add_library( Mart::netlib-portlayer ALIAS mart-netlib-portlayer )
set_target_properties( mart-netlib-portlayer PROPERTIES EXPORT_NAME netlib-portlayer )

#windows.h doesn't yet work with the new preprocessor
target_compile_options( mart-netlib-portlayer PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/experimental:preprocessor->)

######
# netlib library
#
#  network library build ontop of mart-commmon and netlib-portlayer
#

target_sources(mart-netlib
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/ip.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/udp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/unix.cpp
)
set_target_properties ( mart-netlib PROPERTIES EXPORT_NAME netlib )
target_include_directories( mart-netlib
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)