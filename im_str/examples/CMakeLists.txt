cmake_minimum_required( VERSION 3.9 )
project( im_str_examples )

set(CMAKE_CXX_STANDARD 20)
if(MSVC)
	add_compile_options(/std:c++latest)
endif()

get_directory_property( hasParent PARENT_DIRECTORY )

if( NOT hasParent )
	add_subdirectory(.. im_str)
endif()


add_executable( im_str_example1 main.cpp )
target_link_libraries(im_str_example1 PRIVATE ImStr::im_str)

add_executable( im_str_doc_example1 doc_example1.cpp )
target_link_libraries(im_str_doc_example1 PRIVATE ImStr::im_str)

add_executable(im_str_doc_example2 doc_example2.cpp)
target_link_libraries(im_str_doc_example2 PRIVATE ImStr::im_str)