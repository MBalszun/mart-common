cmake_minimum_required(VERSION 2.8)

PROJECT(mart-common)

#create library
if(TARGET ${PROJECT_NAME} )
	message(WARNING "-- Library \"${PROJECT_NAME}\" included multiple times --\n-- Only one directory will be built --")
else()
	file(GLOB_RECURSE SOURCES *.h *.hpp)

	add_library(${PROJECT_NAME} ${SOURCES})
	set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

	target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

#TODO: Specify required language standard if CMAKE VERISION 2.8 no longer has to be supported

