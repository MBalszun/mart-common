cmake_minimum_required(VERSION 3.9)

PROJECT(mart-common)

option(MART_COMMON_INCLUDE_TESTS "Build tests" OFF)

if(TARGET mart-common )
	get_target_property(PREV_SOURCE_DIR mart-common SOURCE_DIR)
	message(WARNING "-- Library \"${PROJECT_NAME}\" included multiple times --\n-- Only one directory will be built --\n-- Selected dir: ${PREV_SOURCE_DIR}")

else()
	message("Dinary Dir: ${CMAKE_BINARY_DIR}")
	file(GLOB_RECURSE SOURCES *.h *.hpp)

	# Header only library for consumption by other project
	add_library(mart-common INTERFACE)

	# interface libraries don't create their own project in IDES, so we create an additional _dev library
	add_library(mart-common_dev ${SOURCES})
	set_target_properties(mart-common_dev PROPERTIES LINKER_LANGUAGE CXX)

	target_include_directories(mart-common		INTERFACE	${CMAKE_CURRENT_SOURCE_DIR}/include)
	target_include_directories(mart-common_dev	PUBLIC		${CMAKE_CURRENT_SOURCE_DIR}/include)

	target_compile_features(mart-common		INTERFACE cxx_std_17)
	target_compile_features(mart-common_dev PUBLIC cxx_std_17)

	add_library(Mart::common ALIAS mart-common)
	add_library(Mart::common_dev ALIAS mart-common_dev)

	if(MART_COMMON_INCLUDE_TESTS)
		enable_testing()
		add_subdirectory(tests)
	endif()

endif()


