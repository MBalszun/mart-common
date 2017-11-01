cmake_minimum_required(VERSION 2.8.12)
#TODO: Specify required language standard when CMAKE VERISION 2.8 (Ubuntu14.04) no longer has to be supported

PROJECT(mart-common)

if(TARGET ${PROJECT_NAME} )

	#TODO cmake 2.8.12 doesn't have standrdized way of querying the soiurce directory of the previously build target
	message(WARNING "-- Library \"${PROJECT_NAME}\" included multiple times --\n-- Only one directory will be built --")

else()
	# Header only library for consumption by other project
	add_library(${PROJECT_NAME} INTERFACE)
	target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

	# interface libraries don't create their own project in IDES, so we create an additional _dev library '
	file(GLOB_RECURSE SOURCES *.h *.hpp)
	add_library(${PROJECT_NAME}_dev ${SOURCES})
	set_target_properties(${PROJECT_NAME}_dev PROPERTIES LINKER_LANGUAGE CXX)
	target_include_directories(${PROJECT_NAME}_dev PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

endif()


