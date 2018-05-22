cmake_minimum_required(VERSION 3.10)

project(mart-common_testing)

include_directories(libs)
file(GLOB_RECURSE TEST_RCS tests_*.cpp)

# message(STATUS "Found test files: ${TEST_RCS}")

add_executable(testing_mart-common
	main.cpp
	${TEST_RCS}
)

find_package(Threads REQUIRED)

target_link_libraries(testing_mart-common PRIVATE Mart::common Threads::Threads)

if(MSVC)
	string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	target_compile_definitions(testing_mart-common
		PRIVATE
		_CRT_SECURE_NO_WARNINGS
		_SCL_SECURE_NO_WARNINGS
		_SILENCE_PARALLEL_ALGORITHMS_EXPERIMENTAL_WARNING
	)
	target_compile_options(testing_mart-common
		PRIVATE
			/permissive-
	)
else()
	target_compile_options(testing_mart-common PRIVATE -Wall -Wextra)
endif()

## Make ctest run build.
# idea taken from https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
# TODO: DOES NOT WORK with MSVC open folder (${CMAKE_COMMAND} seems to be the problem, but a plain "cmake" doesn't pass the correct incldue directories)

ADD_TEST(NAME ctest_build_test_code COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target testing_mart-common)

set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS ON)
set(PARSE_CATCH_TESTS_ADD_TARGET_IN_TEST_NAME OFF)

include(libs/catch2/ParseAndAddCatchTests.cmake)

ParseAndAddCatchTests(testing_mart-common)


