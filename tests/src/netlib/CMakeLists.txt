option(MART_NETLIB_TEST_UNIX_DOMAIN_SOCKS off)

file(GLOB_RECURSE TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/tests_*.cpp)

if(NOT MART_NETLIB_BUILD_UNIX_DOMAIN_SOCKET)
	message(STATUS "[MART-COMMON][NETLIB][TESTS] NOT Building tests for unix domain sockets (set MART_NETLIB_BUILD_UNIX_DOMAIN_SOCKET to enable them)")
	list(REMOVE_ITEM TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/tests_unix.cpp)
endif()

add_executable(testing_mart-netlib
	main.cpp
	${TEST_SRC}
)

find_package(Threads REQUIRED)

target_link_libraries(testing_mart-netlib PRIVATE Mart::netlib Threads::Threads Catch2::Catch2)

## Make ctest run build.
# idea taken from https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
# TODO: DOES NOT WORK with MSVC open folder (${CMAKE_COMMAND} seems to be the problem, but a plain "cmake" doesn't pass the correct incldue directories)
ADD_TEST(NAME ctest_build_netlib_test_code COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target testing_mart-netlib)

if(NOT MART_NETLIB_TEST_UNIX_DOMAIN_SOCKS)
	message(STATUS "[MART-COMMON][NETLIB][TESTS] NOT Running tests for unix domain sockets (set MART_NETLIB_TEST_UNIX_DOMAIN_SOCKS to enable them)")
	set(PARSE_CATCH_TESTS_NO_HIDDEN_TESTS ON)
endif()
ParseAndAddCatchTests(testing_mart-netlib)
