#include <mart-netlib/RaiiSocket.hpp>

#include <mart-common/enum/EnumHelpers.h>

#include <catch2/catch.hpp>

#include <string>

namespace Catch {
template<>
struct StringMaker<mart::nw::socks::ErrorCodeValues> {
	static std::string convert( mart::nw::socks::ErrorCodeValues const& value )
	{
		return std::string( (const char*)std::strerror( mart::toUType( value ) ) );
	}
};
} // namespace Catch

TEST_CASE( "net_socket_is_blocking_after_construction", "[net]" )
{
	using namespace mart::nw;

	socks::RaiiSocket socket( socks::Domain::Inet6, socks::TransportType::Datagram );
	CHECK( socket.is_valid() );
	CHECK( socket.is_blocking() );
	// NOTE: explicit comparison needed, because otherwise catch doesn't print
	// the error codes correctly when a check fails
	CHECK( socket.set_blocking( true ).value() == mart::nw::socks::ErrorCodeValues::NoError );
	CHECK( socket.is_blocking() );
	CHECK( socket.set_blocking( false ).value() == mart::nw::socks::ErrorCodeValues::NoError );
	CHECK( !socket.is_blocking() );
	CHECK( socket.set_blocking( true ).value() == mart::nw::socks::ErrorCodeValues::NoError );
	CHECK( socket.is_blocking() );
	CHECK( socket.set_blocking( false ).value() == mart::nw::socks::ErrorCodeValues::NoError );
	CHECK( !socket.is_blocking() );

	const auto old_handle = socket.get_handle();

	socket = socks::RaiiSocket( socks::Domain::Inet, socks::TransportType::Datagram );

	CHECK( old_handle != socket.get_handle() );
	CHECK( socket.is_valid() );

	CHECK( socket.is_blocking() );
	CHECK( socket.set_blocking( false ).value() == mart::nw::socks::ErrorCodeValues::NoError );
	CHECK( !socket.is_blocking() );
	CHECK( socket.set_blocking( true ).value() == mart::nw::socks::ErrorCodeValues::NoError );
	CHECK( socket.is_blocking() );
	CHECK( socket.set_blocking( false ).value() == mart::nw::socks::ErrorCodeValues::NoError );
	CHECK( !socket.is_blocking() );
	CHECK( socket.set_blocking( true ).value() == mart::nw::socks::ErrorCodeValues::NoError );
	CHECK( socket.is_blocking() );
	CHECK( socket.set_blocking( false ).value() == mart::nw::socks::ErrorCodeValues::NoError );
	CHECK( !socket.is_blocking() );
}