cmake_minimum_required( VERSION 3.12...3.16 )

if(POLICY CMP0110)
    cmake_policy(SET CMP0110 NEW)
endif()

project(mart-common_testing LANGUAGES CXX)

option(MART_COMMON_RUN_BENCHMARKS "run benchmarks as part of unit_test source" OFF)
option(MART_COMMON_UPLOAD_COVERAGE "" OFF)

if(MSVC)
	string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

	add_compile_definitions(
		_CRT_SECURE_NO_WARNINGS
		_SCL_SECURE_NO_WARNINGS
	)
	add_compile_options(
			/W4
			/permissive-
	)
else()
	add_compile_options(-Wall -Wextra)
	add_compile_options(-Wno-deprecated-declarations) # we need to test deprecated APIs
endif()

# let catch print chrono duration values
add_compile_definitions(CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS)



if( TARGET Catch2::Catch2 )
	include( ParseAndAddCatchTests )
else()
	find_package(Catch2 CONFIG)

	set( PARSE_CATCH_TESTS_ADD_TARGET_IN_TEST_NAME ON )

	if(${Catch2_FOUND})
		message( STATUS "[MART-COMMON][TESTS] Using Catch2 from ${Catch2_CONFIG}" )

		include( ParseAndAddCatchTests )
	else()
		message( STATUS "[MART-COMMON][TESTS] Didn't find catch2 -- using local copy" )

		add_library( mart-common-catch2-dummy INTERFACE )
		add_library( Catch2::Catch2 ALIAS mart-common-catch2-dummy )
		target_include_directories( mart-common-catch2-dummy INTERFACE libs )

		include( libs/catch2/ParseAndAddCatchTests.cmake )
	endif()
endif()

file(GLOB_RECURSE TEST_SRC tests_*.cpp)
list(FILTER TEST_SRC EXCLUDE REGEX netlib)
add_executable(testing_mart-common
	main.cpp
	${TEST_SRC}
)

find_package(Threads REQUIRED QUIET)

target_link_libraries(testing_mart-common PRIVATE Mart::common Threads::Threads Catch2::Catch2)

if(MART_COMMON_RUN_BENCHMARKS)
	target_compile_definitions(testing_mart-common PRIVATE /DMART_COMMON_RUN_BENCHMARK)
endif()



## Make ctest run build.
# idea taken from https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
# TODO: DOES NOT WORK with MSVC open folder (${CMAKE_COMMAND} seems to be the problem, but a plain "cmake" doesn't pass the correct incldue directories)

ADD_TEST(NAME ctest_build_test_code COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target testing_mart-common)

ParseAndAddCatchTests(testing_mart-common)

if( MART_COMMON_UPLOAD_COVERAGE )
	if(CMAKE_COMPILER_IS_GNUCXX)
		include(libs/CodeCoverage.cmake)
		APPEND_COVERAGE_COMPILER_FLAGS()
		set(COVERAGE_EXCLUDES '/usr/include/*' '${PROJECT_SOURCE_DIR}/*')
		setup_target_for_coverage(NAME testing_mart-common_coverage EXECUTABLE testing_mart-common DEPENDENCIES testing_mart-common)
	endif()
endif()

if( MART_COMMON_INCLUDE_NET_LIB)
add_subdirectory(src/netlib)
endif()

