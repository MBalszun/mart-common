cmake_minimum_required(VERSION 3.9 )

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(.. im_str)

add_executable(im_str_example main.cpp)
target_link_libraries(im_str_example PRIVATE ImStr::im_str)