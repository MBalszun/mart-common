#include "udp.hpp"

#include "network_exceptions.hpp"

#include <mart-common/ConstString.h>

#ifdef MBA_UTILS_USE_WINSOCKS
#include <WinSock2.h>
#else
#include <netinet/in.h>
#endif

#include <cerrno>
#include <charconv>
#include <cstring>
#include <string_view>

namespace mart::nw::ip::udp {

namespace {
std::string_view errno_nr_as_string( mart::ArrayView<char> buffer )
{
	auto res = std::to_chars( buffer.begin(), buffer.end(), errno );
	return {buffer.begin(), static_cast<std::string_view::size_type>( res.ptr - buffer.begin() )};
}
} // namespace

Socket::Socket()
	: _socket_handle( socks::Domain::inet, socks::TransportType::datagram )
{
	if( !isValid() ) {
		char errno_buffer[24] {};
		throw generic_nw_error( mart::concat( "Could not create udp socket | Errnor:",
											  errno_nr_as_string( errno_buffer ),
											  " msg: ",
											  std::string_view( std::strerror( errno ) ) ) );
	}
}

Socket::Socket( endpoint local, endpoint remote )
	: Socket::Socket()
{
	bind( local );
	connect( remote );
}

mart::MemoryView Socket::recvfrom( mart::MemoryView buffer, endpoint& src_addr ) noexcept
{
	::sockaddr_in src {};
	auto          tmp = _socket_handle.recvfrom( buffer, 0, src );
	if( tmp.first.isValid() && src.sin_family == to_native( nw::socks::Domain::inet ) ) {
		src_addr = endpoint( src );
		return tmp.first;
	} else {
		return mart::MemoryView {};
	}
}

void Socket::connect( endpoint ep )
{
	auto result = _socket_handle.connect( ep.toSockAddr_in() );
	if( result == mart::nw::socks::port_layer::socket_error_value ) {
		char errno_buffer[24] {};
		throw generic_nw_error( mart::concat( "Could not connect socket to address ",
											  ep.toStringEx(),
											  "| Errnor:",
											  errno_nr_as_string( errno_buffer ),
											  " msg: ",
											  std::string_view( std::strerror( errno ) ) ) );
	}

	_ep_remote = ep;
}

void Socket::bind( endpoint ep )
{
	auto result = _socket_handle.bind( ep.toSockAddr_in() );
	if( result == mart::nw::socks::port_layer::socket_error_value ) {
		char errno_buffer[24] {};
		throw generic_nw_error( mart::concat( "Could not bind udp socket to address ",
											  ep.toStringEx(),
											  "| Errno:",
											  errno_nr_as_string( errno_buffer ),
											  " msg: ",
											  std::string_view( std::strerror( errno ) ) ) );
	}

	_ep_local = ep;
}

bool Socket::try_bind( endpoint ep ) noexcept
{
	auto result = _socket_handle.bind( ep.toSockAddr_in() );
	if( result == mart::nw::socks::port_layer::socket_error_value ) { return false; }
	_ep_local = ep;
	return true;
}

bool Socket::try_connect( endpoint ep ) noexcept
{
	auto result = _socket_handle.connect( ep.toSockAddr_in() );
	if( result == mart::nw::socks::port_layer::socket_error_value ) { return false; }
	_ep_remote = ep;
	return true;
}

bool Socket::sendto( mart::ConstMemoryView data, endpoint ep ) noexcept
{
	return _txWasSuccess( data, _socket_handle.sendto( data, 0, ep.toSockAddr_in() ) );
}

namespace {
struct BlockingRestorer {
	BlockingRestorer( nw::socks::Socket& socket )
		: _socket( socket )
		, _was_blocking( socket.isBlocking() )
	{
	}

	~BlockingRestorer() { _socket.setBlocking( _was_blocking ); }

	nw::socks::Socket& _socket;
	const bool         _was_blocking;
};
} // namespace

void Socket::clearRxBuff()
{
	BlockingRestorer r( _socket_handle );

	uint64_t buffer[8] {};
	auto     buffer_view = mart::view_bytes_mutable( buffer );

	while( _socket_handle.recv( buffer_view, 0 ).first.isValid() ) {
		;
	}
}

} // namespace mart::nw::ip::udp