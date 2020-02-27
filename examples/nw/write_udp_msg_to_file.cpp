#include <mart-common/MartTime.h>
#include <mart-common/algorithm.h>
#include <mart-netlib/udp.hpp>

#include <atomic>
#include <fstream>
#include <iostream>
#include <optional>
#include <string_view>
#include <vector>

#include <im_str/im_str.hpp>

namespace udp = mart::nw::ip::udp;
using namespace std::chrono_literals;

std::vector<std::string_view> make_arglist( int argc, char** argv )
{
	if( argc <= 0 ) { return {}; }

	std::vector<std::string_view> ret( argc );
	for( int i = 0; i < argc; ++i ) {
		ret[i] = std::string_view( argv[i] );
	}
	return ret;
}

std::optional<udp::endpoint> get_local_address( mart::ArrayView<const std::string_view> )
{
	// TODO actually esxtract from args
	return udp::try_parse_v4_endpoint( "127.0.0.1:3435" );
}

std::optional<std::string> get_filename( mart::ArrayView<const std::string_view> )
{
	// TODO actually esxtract from args
	return std::string{"Testfile"};
}

std::string_view to_stringview( mart::ConstMemoryView data )
{
	return std::string_view{data.asConstCharPtr(), data.size()};
}

int main( int argc, char** argv )
{
	const auto arglist = make_arglist( argc, argv );

	const udp::endpoint local_ep  = get_local_address( arglist ).value();
	const auto          file_name = get_filename( arglist ).value();

	udp::Socket sock;
	sock.set_rx_timeout( 1000ms );
	sock.bind( local_ep );

	std::cout << "Listening on " << local_ep.toStringEx() << " and writing to" << file_name << std::endl;

	std::string buffer( 1000, '\0' );

	while( true ) {
		auto res = sock.try_recv( mart::view_elements_mutable( buffer ).asBytes() );

		if( res.isValid() ) {
			auto msg = to_stringview( res );
			if( msg == "EXIT" ) { return 0; }
			std::ofstream file( file_name, std::ios_base::out | std::ios_base::trunc );
			file.seekp( 0 );
			file << msg << "\n";
			std::cout << "\"" << msg << "\"" << std::endl;
		}
	}
	std::cout << "Server shutting down\n" << std::flush;
}