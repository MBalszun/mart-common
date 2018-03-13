cmake_minimum_required(VERSION 3.9)

project(mart-common_testing)

include_directories(libs)
file(GLOB_RECURSE TEST_RCS tests_*.cpp)

message(STATUS "Found test files: ${TEST_RCS}")

add_executable(testing_mart-common
	main.cpp
	${TEST_RCS}
)

target_link_libraries(testing_mart-common PRIVATE Mart::common)

if(MSVC)
string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
target_compile_options(testing_mart-common PRIVATE -Wall -Wextra)
endif()


enable_testing()
include(libs/catch2/ParseAndAddCatchTests.cmake)
include(libs/catch2/Catch.cmake)

set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS ON)
ParseAndAddCatchTests(testing_mart-common)
catch_discover_tests(testing_mart-common)

