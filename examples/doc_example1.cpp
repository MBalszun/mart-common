#include <im_str/im_str.hpp>

#include <iostream>
#include <string_view>

struct AppConfig {
	mba::im_str log_file_name;
	mba::im_str greeting;
};


#if IM_STR_USE_CONSTEXPR_DESTRUCTOR
	// in c++17 this needs to be const instead of constexpr
	constexpr AppConfig default_config{ "log.txt", "Hello, World! How are you today?" };
#else
	const AppConfig default_config{ "log.txt", "Hello, World! How are you today?" };
#endif

AppConfig make_config( int argc, char** argv )
{
	if( argc >= 3 ) {
		return AppConfig{ mba::im_zstr::from_c_str( argv[1] ), mba::im_zstr::from_c_str( argv[2] ) };
	} else {
		return default_config;
	}
}

void run( const AppConfig& entry )
{
	std::cout << entry.greeting << std::endl;
}

int main( int argc, char** argv )
{
	auto config = make_config( argc, argv );
	run( config );
}




