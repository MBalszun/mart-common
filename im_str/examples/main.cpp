#include <im_str/im_str.hpp>

#include <cstring>
#include <string>
#include <iostream>

int c_api_func(const char* str) {
	return std::strlen( str );
}

int main() {
	using namespace std::string_literals;

	mba::im_str str1 = "Hello ";

	auto greeting = mba::concat( str1, "World"s, "!" );

	std::cout << greeting << std::endl;
}