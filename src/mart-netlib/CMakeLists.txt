if(POLICY CMP0092)
  cmake_policy(SET CMP0092 NEW)
else()
if(MSVC)
	string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

endif()

if(MSVC)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	add_compile_options( /W4 )
else()
	add_compile_options( -Wall -Wextra )
endif()

## netlib-portlayer library - minimal abstraction over os-specific bsd-socket-api
# NOTE: the port-layer is supposed to be a c++11 library
set(CMAKE_CXX_STANDARD 11)
add_library( mart-netlib-portlayer STATIC ${CMAKE_CURRENT_SOURCE_DIR}/port_layer.cpp )
add_library( Mart::netlib-portlayer ALIAS mart-netlib-portlayer )
set_target_properties ( mart-netlib-portlayer PROPERTIES EXPORT_NAME netlib-portlayer )
target_include_directories( mart-netlib-portlayer
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
	INTERFACE
		$<INSTALL_INTERFACE:include>
)

## netlib library - network library build ontop of mart-commmon and netlib-portlayer
target_sources(mart-netlib PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/ip.cpp ${CMAKE_CURRENT_SOURCE_DIR}/udp.cpp)
set_target_properties ( mart-netlib PROPERTIES EXPORT_NAME netlib )
target_link_libraries( mart-netlib PRIVATE Mart::common Mart::netlib-portlayer)
target_include_directories( mart-netlib
	PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
)