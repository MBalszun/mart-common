cmake_minimum_required(VERSION 3.10)


add_library( mart-common_net port_layer.cpp Socket.cpp udp.cpp ip.cpp)
add_library( Mart::common_net ALIAS mart-common_net)
target_link_libraries( mart-common_net PUBLIC Mart::common )
target_compile_features( mart-common_net PUBLIC cxx_std_17 )
target_compile_definitions( mart-common_net PRIVATE _CRT_SECURE_NO_WARNINGS )