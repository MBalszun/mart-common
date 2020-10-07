#include <mart-netlib/udp.hpp>

#include <mart-netlib/network_exceptions.hpp>

//#include <mart-common/StringViewOstream.h>

#include <im_str/im_str.hpp>

#include <algorithm>
#include <cerrno>
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
	return { buffer.begin(), static_cast<std::string_view::size_type>( res.ptr - buffer.begin() ) };
#else
	auto res = std::to_string( error.raw_value() );
	auto n   = std::min( res.size(), buffer.size() );

	std::copy_n( res.begin(), n, buffer.begin() );
	return { buffer.begin(), n };
#endif
}

std::string_view errno_nr_as_string( mart::ArrayView<char> buffer )
{
	return errno_nr_as_string( mart::nw::socks::port_layer::get_last_socket_error(), buffer );
}

template<class... Elements>
mba::im_zstr make_error_message_with_appended_last_errno( mart::nw::socks::ErrorCode error, Elements&&... elements )
{
	std::array<char, 24> errno_buffer{};
	return mba::concat( std::string_view( elements )...,
						"| Error Code:",
						errno_nr_as_string( error, errno_buffer ),
						" Error Msg: ",
						socks::to_text_rep( error ) );
}

} // namespace

Socket::Socket()
	: _socket_handle( socks::Domain::Inet, socks::TransportType::Datagram )
{
	if( !is_valid() ) {
		// TODO: The creation of this exception message seems to be pretty costly in terms of binary size - have to
		// investigate NOTE: Preliminary tests suggest, that the dynamic memory allocation for the message text could be
		// the main problem, but that is just a possibility
		throw generic_nw_error( make_error_message_with_appended_last_errno(
			mart::nw::socks::port_layer::get_last_socket_error(), "Could not create udp socket." ) );
	}
}

Socket::Socket( endpoint local, endpoint remote )
	: Socket::Socket()
{
	bind( local );
	connect( remote );
}

void Socket::connect( endpoint ep )
{
	auto result = _socket_handle.connect( ep.toSockAddr_in() );
	if( !result.success() ) {

		throw generic_nw_error( make_error_message_with_appended_last_errno(
			result, "Could not connect socket to address ", ep.toStringEx() ) );
	}

	_ep_remote = ep;
}

void Socket::bind( endpoint ep )
{
	auto result = _socket_handle.bind( ep.toSockAddr_in() );
	if( !result.success() ) {
		throw generic_nw_error( make_error_message_with_appended_last_errno(
			result, "Could not bind udp socket to address ", ep.toStringEx() ) );
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
	if( !_txWasSuccess( data, res ) ) {
		throw nw::generic_nw_error( make_error_message_with_appended_last_errno(
			res.result.error_code(), "Failed to send data to ", ep.toString(), ". Details:  " ) );
	}
}

void Socket::send( mart::ConstMemoryView data )
{
	const auto res = _socket_handle.send( data, 0 );
	if( !_txWasSuccess( data, res ) ) {
		throw nw::generic_nw_error(
			make_error_message_with_appended_last_errno( res.result.error_code(), "Failed to send data. Details:  " ) );
	}
}

namespace {
template<class T, T... Vals>
bool is_none_of( T v )
{
	return ( true && ... && ( v != Vals ) );
}
} // namespace

Socket::RecvfromResult Socket::recvfrom( mart::MemoryView buffer )
{
	using mart::nw::socks::ErrorCodeValues;
	mart::nw::socks::port_layer::SockaddrIn addr{};

	auto res = _socket_handle.recvfrom( buffer, 0, addr );
	if( !res.result
		&& is_none_of<ErrorCodeValues,
					  ErrorCodeValues::WouldBlock,
					  ErrorCodeValues::TryAgain,
					  ErrorCodeValues::Timeout,
					  ErrorCodeValues::WsaeConnReset>( res.result.error_code().value() ) ) {
		throw nw::generic_nw_error( make_error_message_with_appended_last_errno(
			res.result.error_code(), "Failed to receive data. Details:  " ) );
	}

	return { res.received_data, udp::endpoint( addr ) };
}

mart::MemoryView Socket::recv( mart::MemoryView buffer )
{
	using mart::nw::socks::ErrorCodeValues;
	const auto res = _socket_handle.recv( buffer, 0 );
	if( !res.result
		&& is_none_of<ErrorCodeValues,
					  ErrorCodeValues::WouldBlock,
					  ErrorCodeValues::TryAgain,
					  ErrorCodeValues::Timeout,
					  ErrorCodeValues::WsaeConnReset>( res.result.error_code().value() ) ) {
		throw nw::generic_nw_error( make_error_message_with_appended_last_errno(
			res.result.error_code(), "Failed to receive data. Details:  " ) );
	}
	return res.received_data;
}

namespace {
struct BlockingRestorer {
	BlockingRestorer( nw::socks::RaiiSocket& socket )
		: _socket( socket )
		, _was_blocking( socket.is_blocking() )
	{
	}

	~BlockingRestorer() { _socket.set_blocking( _was_blocking ); }

	nw::socks::RaiiSocket& _socket;
	const bool             _was_blocking;
};
} // namespace

void Socket::clearRxBuff()
{
	BlockingRestorer r( _socket_handle );
	auto             res = _socket_handle.set_blocking( false );
	if( !res ) { throw mart::nw::generic_nw_error( "Failed to set socket in non-blocking mode for ex cleanup" ); }

	uint64_t buffer[8]{};
	auto     buffer_view = mart::view_bytes_mutable( buffer );

	while( _socket_handle.recv( buffer_view, 0 ).result.success() ) {
		;
	}
}

} // namespace mart::nw::ip::udp