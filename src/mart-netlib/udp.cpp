#include <mart-netlib/udp.hpp>

#include <mart-netlib/network_exceptions.hpp>

#include <mart-common/ConstString.h>
#include <mart-common/StringViewOstream.h>

#include <cerrno>
#include <algorithm>
#include <cstring>
#include <string_view>

#if __has_include( <charconv> )
#include <charconv>
#endif

namespace mart::nw::ip::udp {

namespace {
std::string_view errno_nr_as_string( mart::nw::socks::ErrorCode error, mart::ArrayView<char> buffer )
{
#if __has_include( <charconv> )
	auto res = std::to_chars( buffer.begin(), buffer.end(), error.raw_value() );
	return {buffer.begin(), static_cast<std::string_view::size_type>( res.ptr - buffer.begin() )};
#else
	auto res = std::to_string( error.raw_value() );
	auto n   = std::min( res.size(), buffer.size() );

	std::copy_n( res.begin(), n, buffer.begin() );
	return {buffer.begin(), n};
#endif
}

std::string_view errno_nr_as_string( mart::ArrayView<char> buffer )
{
	return errno_nr_as_string( mart::nw::socks::port_layer::get_last_socket_error(), buffer );
}

} // namespace

Socket::Socket()
	: _socket_handle( socks::Domain::Inet, socks::TransportType::Datagram )
{
	if( !isValid() ) {
		char errno_buffer[24] {};
		throw generic_nw_error(
			mart::concat( "Could not create udp socket | Errnor:",
						  errno_nr_as_string( errno_buffer ),
						  " msg: ",
						  socks::to_text_rep( mart::nw::socks::port_layer::get_last_socket_error() ) ) );
	}
}

Socket::Socket( endpoint local, endpoint remote )
	: Socket::Socket()
{
	bind( local );
	connect( remote );
}

mart::MemoryView Socket::recvfrom( mart::MemoryView buffer, endpoint& src_addr )
{
	mart::nw::socks::port_layer::SockaddrIn src {};

	auto tmp = _socket_handle.recvfrom( buffer, 0, src );
	if( tmp.result.success() && src.is_valid() ) {
		src_addr = endpoint( src );
		return tmp.received_data;
	} else {
		return mart::MemoryView {};
	}
}

void Socket::connect( endpoint ep )
{
	auto result = _socket_handle.connect( ep.toSockAddr_in() );
	if( !result.success() ) {
		std::array<char, 24> errno_buffer {};
		throw generic_nw_error( mart::concat( "Could not connect socket to address ",
											  ep.toStringEx(),
											  "| Errnor:",
											  errno_nr_as_string( result, errno_buffer ),
											  " msg: ",
											  socks::to_text_rep( result ) ) );
	}

	_ep_remote = ep;
}

void Socket::bind( endpoint ep )
{
	auto result = _socket_handle.bind( ep.toSockAddr_in() );
	if( !result.success() ) {
		char errno_buffer[24] {};
		throw generic_nw_error( mart::concat( "Could not bind udp socket to address ",
											  ep.toStringEx(),
											  "| Errno:",
											  errno_nr_as_string( errno_buffer ),
											  " msg: ",
											  socks::to_text_rep( result ) ) );
	}

	_ep_local = ep;
}

socks::ErrorCode Socket::try_bind( endpoint ep ) noexcept
{
	auto result = _socket_handle.bind( ep.toSockAddr_in() );
	if( result.success() ) { _ep_local = ep; }

	return result;
}

socks::ErrorCode Socket::try_connect( endpoint ep ) noexcept
{
	auto result = _socket_handle.connect( ep.toSockAddr_in() );
	if( result.success() ) { _ep_remote = ep; }

	return result;
}

void Socket::sendto( mart::ConstMemoryView data, endpoint ep )
{
	const auto res = _socket_handle.sendto( data, 0, ep.toSockAddr_in() );
	if (!res.result) {
		throw nw::generic_nw_error( mart::concat( "Failed to send data. Details:  " ) );
	}
}

void Socket::send( mart::ConstMemoryView data )
{
	const auto res = _socket_handle.send( data, 0 );
	if( !res.result ) { throw nw::generic_nw_error( mart::concat( "Failed to send data. Details:  " ) ); }
}

namespace {
template<class T, T ... Vals>
bool is_none_of( T v )
{
		return ( true && ... && ( v != Vals ) );

}
}

mart::MemoryView Socket::rec( mart::MemoryView buffer )
{
	using mart::nw::socks::ErrorCodeValues;
	const auto res = _socket_handle.recv( buffer, 0 );
	if( !res.result
		&& is_none_of<ErrorCodeValues, ErrorCodeValues::WouldBlock, ErrorCodeValues::TryAgain, ErrorCodeValues::Timeout>(
			   res.result.value() )
			    ) {
		throw nw::generic_nw_error( mart::concat("Failed to receive data from socket. Details:  "));
	}
	return res.received_data;
}

namespace {
struct BlockingRestorer {
	BlockingRestorer( nw::socks::Socket& socket )
		: _socket( socket )
		, _was_blocking( socket.is_blocking() )
	{
	}

	~BlockingRestorer() { _socket.set_blocking( _was_blocking ); }

	nw::socks::Socket& _socket;
	const bool         _was_blocking;
};
} // namespace

void Socket::clearRxBuff()
{
	BlockingRestorer r( _socket_handle );
	auto res = _socket_handle.set_blocking( false );
	if( !res ) { throw mart::nw::generic_nw_error( "Failed to set socket in non-blocking mode for ex cleanup" );
	}

	uint64_t buffer[8] {};
	auto     buffer_view = mart::view_bytes_mutable( buffer );

	while( _socket_handle.recv( buffer_view, 0 ).result.success() ) {
		;
	}
}

} // namespace mart::nw::ip::udp