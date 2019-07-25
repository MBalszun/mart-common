
file(GLOB TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/tests_*.cpp)
add_executable(testing_mart-netlib
	main.cpp
	${TEST_SRC}
)

message("Tests: ${TEST_SRC}")

find_package(Threads REQUIRED)

target_link_libraries(testing_mart-netlib PRIVATE Mart::netlib Threads::Threads Catch2::Catch2)


## Make ctest run build.
# idea taken from https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
# TODO: DOES NOT WORK with MSVC open folder (${CMAKE_COMMAND} seems to be the problem, but a plain "cmake" doesn't pass the correct incldue directories)

ADD_TEST(NAME ctest_build_netlib_test_code COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target testing_mart-netlib)

ParseAndAddCatchTests(testing_mart-netlib)
