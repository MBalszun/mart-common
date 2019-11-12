cmake_minimum_required(VERSION 3.13)
project(mat-common-nw-examples)

#add_compile_options(/LTCG /O2)
#add_link_options(/LTCG)
option(MART_COMMON_EXAMPLE_NW_USE_INSTALLED_LIBRARIES "Don't compile mart-common / mart-netlib as part of the example, but use system installed version" OFF)

if(${MART_COMMON_EXAMPLE_NW_USE_INSTALLED_LIBRARIES})

	find_package(MartCommon REQUIRED)
	message("[MART-COMMON][EXAMPLES] Found MartCommon at " ${MartCommon_CONFIG})

else()

	get_directory_property( hasParent PARENT_DIRECTORY )
	if( NOT hasParent )
		set(CMAKE_CXX_STANDARD 17)
		#add_link_options(-s -fno-rtti)
		#add_compile_options(-fno-rtti)
		add_subdirectory(../.. ${CMAKE_CURRENT_BINARY_DIR}/mart-common)
	endif()

endif()


find_package(Threads)

add_executable( mart-common_ex_udp_sender simple_udp_sender.cpp )
target_link_libraries( mart-common_ex_udp_sender PUBLIC Mart::netlib)

add_executable( mart-common_ex_udp_server udp_server.cpp )
target_link_libraries( mart-common_ex_udp_server PUBLIC Mart::netlib Threads::Threads Mart::im_str)