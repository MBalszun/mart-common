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
#include <mart-netlib/RaiiSocket.hpp>
#include <mart-netlib/network_exceptions.hpp>

#include <mart-netlib/detail/socket_base.hpp>

/* Proprietary Library Includes */
#include <mart-common/ArrayView.h>
#include <mart-common/utils.h>

#include <im_str/im_str.hpp>

/* Standard Library Includes */
#include <chrono>
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

namespace {

inline std::string_view errno_nr_as_string( mart::nw::socks::ErrorCode error, mart::ArrayView<char> buffer )
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

inline std::string_view errno_nr_as_string( mart::ArrayView<char> buffer )
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

// TODO: look for more efficient solution
std::string to_string( std::chrono::microseconds timeout )
{
	return std::to_string( timeout.count() ) + "us";
}

void HighLevelSocketBase::set_tx_timeout( std::chrono::microseconds timeout )
{
	if( !try_set_tx_timeout( timeout ) ) {
		throw generic_nw_error(
			make_error_message_with_appended_last_errno( mart::nw::socks::port_layer::get_last_socket_error(),
														 "Could  not set tx_timeout to ",
														 to_string( timeout ),
														 " on socket" ) );
	}
}

void HighLevelSocketBase::set_rx_timeout( std::chrono::microseconds timeout )
{
	if( !try_set_rx_timeout( timeout ) ) {
		throw generic_nw_error(
			make_error_message_with_appended_last_errno( mart::nw::socks::port_layer::get_last_socket_error(),
														 "Could not set rx_timeout to ",
														 to_string( timeout ),
														 " on socket" ) );
	}
}

std::chrono::microseconds HighLevelSocketBase::get_tx_timeout() const
{
	auto t = _socket.get_tx_timeout();
	if( t == RaiiSocket::invalid_timeout_v ) {
		throw generic_nw_error(
			make_error_message_with_appended_last_errno( mart::nw::socks::port_layer::get_last_socket_error(),
														 "Could not determine tx_timeout on socket" ) );
	}
	return t;
}

std::chrono::microseconds HighLevelSocketBase::get_rx_timeout() const
{
	auto t = _socket.get_rx_timeout();
	if( t == RaiiSocket::invalid_timeout_v ) {
		throw generic_nw_error(
			make_error_message_with_appended_last_errno( mart::nw::socks::port_layer::get_last_socket_error(),
														 "Could not determine rx_timeout on socket" ) );
	}
	return t;
}


void HighLevelSocketBase::close() {
	if( !try_close() ) {
		throw generic_nw_error( make_error_message_with_appended_last_errno(
			mart::nw::socks::port_layer::get_last_socket_error(), "Could not close socket" ) );
	}
}

template<class T, T... Vals>
bool is_none_of( T v )
{
	return ( true && ... && ( v != Vals ) );
}
//
//struct BlockingRestorer {
//	BlockingRestorer( nw::socks::RaiiSocket& socket )
//		: _socket( socket )
//		, _was_blocking( socket.is_blocking() )
//	{
//	}
//
//	~BlockingRestorer() { _socket.set_blocking( _was_blocking ); }
//
//	nw::socks::RaiiSocket& _socket;
//	const bool             _was_blocking;
//};
//
/* WARNING: This is meant as a convenience class around the generic RaiiSocket for udp communication. Its interface
 * is still very much in flux */
DgramSocketBase::DgramSocketBase( mart::nw::socks::Domain domain )
	: HighLevelSocketBase( domain, socks::TransportType::Datagram )
{
	if( !is_valid() ) {
		throw generic_nw_error( make_error_message_with_appended_last_errno(
			mart::nw::socks::port_layer::get_last_socket_error(), "Could not create socket." ) );
	}
}

void DgramSocketBase::send( mart::ConstMemoryView data )
{
	const auto res = _socket.send( data, 0 );
	if( !res.result ) {
		throw nw::generic_nw_error(
			make_error_message_with_appended_last_errno( res.result.error_code(), "Failed to send data. Details:  " ) );
	}
}

mart::MemoryView DgramSocketBase::recv( mart::MemoryView buffer )
{
	using mart::nw::socks::ErrorCodeValues;
	const auto res = _socket.recv( buffer, 0 );
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





//Socket::Socket( endpoint local, endpoint remote )
//	: Socket::Socket()
//{
//	bind( local );
//	connect( remote );
//}






} // namespace detail
} // namespace socks
} // namespace nw
} // namespace mart

#endif