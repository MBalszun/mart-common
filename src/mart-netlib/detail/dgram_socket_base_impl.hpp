#ifndef LIB_MART_COMMON_GUARD_NW_DETAIL_DGRAM_SOCKET_BASE_IMPL_H
#define LIB_MART_COMMON_GUARD_NW_DETAIL_DGRAM_SOCKET_BASE_IMPL_H

/**
 * dgram_socket_base.hpp (mart-common/nw/detail)
 *
 * Copyright (C) 2019: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	BAse class for data gram sockets
 *
 */

/* ######## INCLUDES ######### */
/* Project Includes */
#include <mart-netlib/Socket.hpp>
#include <mart-netlib/detail/dgram_socket_base.hpp>
#include <mart-netlib/network_exceptions.hpp>

/* Proprietary Library Includes */
#include <mart-common/ArrayView.h>
#include <mart-common/ConstString.h>
#include <mart-common/StringViewOstream.h>
#include <mart-common/utils.h>

/* Standard Library Includes */
#include <chrono>
#include <optional>
#include <string_view>

#include <algorithm>
#include <cerrno>
#include <cstring>

#if __has_include( <charconv> )
#include <charconv>
#endif

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace nw {
namespace socks {
namespace detail {

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

inline std::string_view errno_nr_as_string( mart::nw::socks::ErrorCode error, mart::ArrayView<char> buffer )
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

inline std::string_view errno_nr_as_string( mart::ArrayView<char> buffer )
{
	return errno_nr_as_string( mart::nw::socks::port_layer::get_last_socket_error(), buffer );
}

template<class T, T... Vals>
bool is_none_of( T v )
{
	return ( true && ... && ( v != Vals ) );
}

/* WARNING: This is meant as a convenience class around the generic RaiiSocket for udp communication. Its interface is
 * still very much in flux */

inline DgramSocketBase::DgramSocketBase( mart::nw::socks::Domain domain )
	: _socket_handle( domain, socks::TransportType::Stream )
{
	if( !is_valid() ) {
		char errno_buffer[24] {};
		throw generic_nw_error(
			mart::concat( "Could not create socket | Errnor:",
						  errno_nr_as_string( errno_buffer ),
						  " msg: ",
						  socks::to_text_rep( mart::nw::socks::port_layer::get_last_socket_error() ) ) );
	}
}

inline auto DgramSocketBase::send( mart::ConstMemoryView data ) -> mart::ConstMemoryView
{
	const auto res = _socket_handle.send( data, 0 );
	if( !res.result ) { throw nw::generic_nw_error( mart::concat( "Failed to send data. Details:  " ) ); }
	return res.remaining_data;
}

inline mart::MemoryView DgramSocketBase::recv( mart::MemoryView buffer )
{
	using mart::nw::socks::ErrorCodeValues;
	const auto res = _socket_handle.recv( buffer, 0 );
	if( !res.result
		&& is_none_of<ErrorCodeValues,
					  ErrorCodeValues::WouldBlock,
					  ErrorCodeValues::TryAgain,
					  ErrorCodeValues::Timeout>( res.result.error_code().value() ) ) {
		throw nw::generic_nw_error( mart::concat( "Failed to receive data from socket. Details:  " ) );
	}
	return res.received_data;
}

inline void DgramSocketBase::clearRxBuff()
{
	BlockingRestorer r( _socket_handle );
	auto             res = _socket_handle.set_blocking( false );
	if( !res ) { throw ::mart::nw::generic_nw_error( "Failed to set socket in non-blocking mode for ex cleanup" ); }

	uint64_t buffer[8] {};
	auto     buffer_view = mart::view_bytes_mutable( buffer );

	while( _socket_handle.recv( buffer_view, 0 ).result.success() ) {
		;
	}
}

inline bool _txWasSuccess( mart::ConstMemoryView data, nw::socks::ReturnValue<mart::nw::socks::txrx_size_t> ret )
{
	return ret.success() && mart::narrow<nw::socks::txrx_size_t>( data.size() ) == ret.value();
}


template<class EndpointT>
typename DgramSocket<EndpointT>::RecvfromResult DgramSocket<EndpointT>::recvfrom( mart::MemoryView buffer )
{
	using mart::nw::socks::ErrorCodeValues;
	typename EndpointT::abi_endpoint_type addr {};

	auto res = _socket_handle.recvfrom( buffer, 0, addr );
	if( !res.result
		&& is_none_of<ErrorCodeValues,
					  ErrorCodeValues::WouldBlock,
					  ErrorCodeValues::TryAgain,
					  ErrorCodeValues::Timeout>( res.result.error_code().value() ) ) {
		throw nw::generic_nw_error( mart::concat( "Failed to receive data from socket. Details:  " ) );
	}

	return {res.received_data, EndpointT( addr )};
}

template<class EndpointT>
DgramSocket<EndpointT>::DgramSocket( const EndpointT& local, const EndpointT& remote )
	: DgramSocket<EndpointT>::DgramSocket()
{
	bind( local );
	connect( remote );
}

template<class EndpointT>
void DgramSocket<EndpointT>::connect( endpoint ep )
{
	auto result = _socket_handle.connect( ep.toSockAddr() );
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

template<class EndpointT>
void DgramSocket<EndpointT>::bind( endpoint ep )
{
	auto result = _socket_handle.bind( ep.toSockAddr() );
	if( !result.success() ) {
		char errno_buffer[24] {};
		throw generic_nw_error( mart::concat( "Could not bind socket to address ",
											  ep.toStringEx(),
											  "| Errno:",
											  errno_nr_as_string( errno_buffer ),
											  " msg: ",
											  socks::to_text_rep( result ) ) );
	}

	_ep_local = ep;
}

template<class EndpointT>
socks::ErrorCode DgramSocket<EndpointT>::try_bind( endpoint ep ) noexcept
{
	auto result = _socket_handle.bind( ep.toSockAddr() );
	if( result.success() ) { _ep_local = ep; }

	return result;
}

template<class EndpointT>
socks::ErrorCode DgramSocket<EndpointT>::try_connect( endpoint ep ) noexcept
{
	auto result = _socket_handle.connect( ep.toSockAddr() );
	if( result.success() ) { _ep_remote = ep; }

	return result;
}

template<class EndpointT>
auto DgramSocket<EndpointT>::sendto( mart::ConstMemoryView data, endpoint ep ) -> mart::ConstMemoryView
{
	const auto res = _socket_handle.sendto( data, 0, ep.toSockAddr() );
	if( !res.result ) { throw nw::generic_nw_error( mart::concat( "Failed to send data. Details:  " ) ); }
	return res.remaining_data;
}

} // namespace detail
} // namespace socks
} // namespace nw
} // namespace mart

#endif