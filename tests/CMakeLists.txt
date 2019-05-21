cmake_minimum_required( VERSION 3.9 )
project( ImStrTests LANGUAGES CXX )

option( IM_STR_RUN_SANITIZERS "Run tests with ubsan and address sanitizer" OFF )
option( IM_STR_COVERAGE "Generate coverage data" OFF )

if(MSVC)
	string( REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
	add_compile_options( /permissive- /W4 )
	add_definitions( -D_SCL_SECURE_NO_WARNINGS )
else()
	add_compile_options( -Wall -Wextra )
endif()

find_package( Catch2 CONFIG REQUIRED )
find_package( Threads REQUIRED )

add_executable(im_str_test
	main.cpp
	test_ref_cnt_buf.cpp
	test_split.cpp
	test_substr.cpp
	tests.cpp
)

target_link_libraries( im_str_test PUBLIC ImStr::im_str Catch2::Catch2 Threads::Threads )

if( NOT MSVC )
	if( ${IM_STR_RUN_SANITIZERS} )
		target_compile_options(im_str_test
			PUBLIC
				-g
				-O1
				-fno-omit-frame-pointer
				-fsanitize=undefined
				-fsanitize=address
		)
		target_link_libraries(im_str_test
			PUBLIC
				-fsanitize=undefined
				-fsanitize=address
		)
	endif()
	if(${IM_STR_COVERAGE})
		target_compile_options(im_str_test
			PUBLIC
				-fprofile-arcs
				-ftest-coverage
				-fno-omit-frame-pointer
		)
		target_link_libraries(im_str_test
			PUBLIC
				-lgcov --coverage
		)
	endif()
endif()

include( ParseAndAddCatchTests )
ParseAndAddCatchTests( im_str_test )

add_executable( im_str_benchmark benchmark_split.cpp )
target_link_libraries( im_str_benchmark PUBLIC ImStr::im_str  Threads::Threads )
