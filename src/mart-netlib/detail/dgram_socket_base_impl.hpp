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

template<class EndpointT>
DgramSocket<EndpointT>::DgramSocket( EndpointT local, EndpointT remote )
	: DgramSocket()
{
	bind( local );
	connect( remote );
}


template<class EndpointT>
void DgramSocket<EndpointT>::connect( endpoint ep )
{
	auto result = _socket.connect( ep.toSockAddr() );
	if( !result.success() ) {
		throw generic_nw_error( make_error_message_with_appended_last_errno(
			result, "Could not connect datagram socket to address \"", ep.toStringEx(), "\"" ) );
	}

	_ep_remote = ep;
}

template<class EndpointT>
void DgramSocket<EndpointT>::bind( endpoint ep )
{
	auto result = _socket.bind( ep.toSockAddr() );
	if( !result.success() ) {
		throw generic_nw_error( make_error_message_with_appended_last_errno(
			result, "Could not bind datagram socket to address \"", ep.toStringEx(), "\"" ) );
	}

	_ep_local = ep;
}

template<class EndpointT>
socks::ErrorCode DgramSocket<EndpointT>::try_bind( endpoint ep ) noexcept
{
	auto result = _socket.bind( ep.toSockAddr() );
	if( result.success() ) { _ep_local = ep; }

	return result;
}

template<class EndpointT>
socks::ErrorCode DgramSocket<EndpointT>::try_connect( endpoint ep ) noexcept
{
	auto result = _socket.connect( ep.toSockAddr() );
	if( result.success() ) { _ep_remote = ep; }

	return result;
}

template<class EndpointT>
void DgramSocket<EndpointT>::sendto( mart::ConstMemoryView data, endpoint ep )
{
	const auto res = _socket.sendto( data, 0, ep.toSockAddr() );
	if( !_txWasSuccess( data, res ) ) {
		throw nw::generic_nw_error( make_error_message_with_appended_last_errno(
			res.result.error_code(), "Failed to send data to ", ep.toString(), ". Details:  " ) );
	}
}


namespace {
template<class T, T... Vals>
bool is_none_of( T v )
{
	return ( true && ... && ( v != Vals ) );
}
} // namespace

template<class EndpointT>
typename DgramSocket<EndpointT>::RecvfromResult DgramSocket<EndpointT>::recvfrom( mart::MemoryView buffer )
{
	using mart::nw::socks::ErrorCodeValues;
	using abi_addr = typename EndpointT::abi_endpoint_type;
	abi_addr addr{};

	auto res = _socket.recvfrom( buffer, 0, addr );
	if( !res.result
		&& is_none_of<ErrorCodeValues,
					  ErrorCodeValues::WouldBlock,
					  ErrorCodeValues::TryAgain,
					  ErrorCodeValues::Timeout,
					  ErrorCodeValues::WsaeConnReset>( res.result.error_code().value() ) ) {
		throw nw::generic_nw_error( make_error_message_with_appended_last_errno(
			res.result.error_code(), "Failed to receive data. Details:  " ) );
	}

	return { res.received_data, EndpointT( addr ) };
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

template<class EndpointT>
void DgramSocket<EndpointT>::clearRxBuff()
{
	BlockingRestorer r( _socket );
	auto             res = _socket.set_blocking( false );
	if( !res ) { throw mart::nw::generic_nw_error( "Failed to set socket in non-blocking mode for ex cleanup" ); }

	uint64_t buffer[8]{};
	auto     buffer_view = mart::view_bytes_mutable( buffer );

	while( _socket.recv( buffer_view, 0 ).result.success() ) {
		;
	}
}


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
//inline std::string_view errno_nr_as_string( mart::nw::socks::ErrorCode error, mart::ArrayView<char> buffer )
//{
//#if __has_include( <charconv> )
//	auto res = std::to_chars( buffer.begin(), buffer.end(), error.raw_value() );
//	return {buffer.begin(), static_cast<std::string_view::size_type>( res.ptr - buffer.begin() )};
//#else
//	auto res = std::to_string( error.raw_value() );
//	auto n   = std::min( res.size(), buffer.size() );
//
//	std::copy_n( res.begin(), n, buffer.begin() );
//	return {buffer.begin(), n};
//#endif
//}
//
//inline std::string_view errno_nr_as_string( mart::ArrayView<char> buffer )
//{
//	return errno_nr_as_string( mart::nw::socks::port_layer::get_last_socket_error(), buffer );
//}
//
//template<class T, T... Vals>
//bool is_none_of( T v )
//{
//	return ( true && ... && ( v != Vals ) );
//}
//
//template<class... Elements>
//mba::im_zstr make_error_message_with_appended_last_errno( mart::nw::socks::ErrorCode error, Elements&&... elements )
//{
//	std::array<char, 24> errno_buffer{};
//	return mba::concat( std::string_view( elements )...,
//						"| Error Code:",
//						errno_nr_as_string( error, errno_buffer ),
//						" Error Msg: ",
//						socks::to_text_rep( error ) );
//}
//
///* WARNING: This is meant as a convenience class around the generic RaiiSocket for udp communication. Its interface is
// * still very much in flux */
//
//inline DgramSocketBase::DgramSocketBase( mart::nw::socks::Domain domain )
//	: HighLevelSocketBase( domain, socks::TransportType::Datagram )
//{
//	if( !is_valid() ) {
//		throw mart::nw::generic_nw_error( make_error_message_with_appended_last_errno(
//			mart::nw::socks::port_layer::get_last_socket_error(), "Could not create socket." ) );
//	}
//}
//
//inline auto DgramSocketBase::send( mart::ConstMemoryView data ) -> mart::ConstMemoryView
//{
//	const auto res = _socket.send( data, 0 );
//	if( !res.result ) {
//		throw nw::generic_nw_error(
//			make_error_message_with_appended_last_errno( res.result.error_code(), "Failed to send data. Details:  " ) );
//	}
//	return res.remaining_data;
//}
//
//inline mart::MemoryView DgramSocketBase::recv( mart::MemoryView buffer )
//{
//	using mart::nw::socks::ErrorCodeValues;
//	const auto res = _socket.recv( buffer, 0 );
//	if( !res.result
//		&& is_none_of<ErrorCodeValues,
//					  ErrorCodeValues::WouldBlock,
//					  ErrorCodeValues::TryAgain,
//					  ErrorCodeValues::Timeout>( res.result.error_code().value() ) ) {
//		throw nw::generic_nw_error( make_error_message_with_appended_last_errno(
//			res.result.error_code(), "Failed to receive data. Details:  " ) );
//	}
//	return res.received_data;
//}
//
//inline void DgramSocketBase::clearRxBuff()
//{
//	BlockingRestorer r( _socket );
//	auto             res = _socket.set_blocking( false );
//	if( !res ) { throw ::mart::nw::generic_nw_error( "Failed to set socket in non-blocking mode for ex cleanup" ); }
//
//	uint64_t buffer[8]{};
//	auto     buffer_view = mart::view_bytes_mutable( buffer );
//
//	while( _socket.recv( buffer_view, 0 ).result.success() ) {
//		;
//	}
//}
//
//inline bool _txWasSuccess( mart::ConstMemoryView data, nw::socks::ReturnValue<mart::nw::socks::txrx_size_t> ret )
//{
//	return ret.success() && mart::narrow<nw::socks::txrx_size_t>( data.size() ) == ret.value();
//}
//
//template<class EndpointT>
//typename DgramSocket<EndpointT>::RecvfromResult DgramSocket<EndpointT>::recvfrom( mart::MemoryView buffer )
//{
//	using mart::nw::socks::ErrorCodeValues;
//	typename EndpointT::abi_endpoint_type addr{};
//
//	auto res = _socket.recvfrom( buffer, 0, addr );
//	if( !res.result
//		&& is_none_of<ErrorCodeValues,
//					  ErrorCodeValues::WouldBlock,
//					  ErrorCodeValues::TryAgain,
//					  ErrorCodeValues::Timeout>( res.result.error_code().value() ) ) {
//		throw nw::generic_nw_error( make_error_message_with_appended_last_errno(
//			res.result.error_code(), "Failed to receive data. Details:  " ) );
//	}
//
//	return {res.received_data, EndpointT( addr )};
//}
//
//template<class EndpointT>
//DgramSocket<EndpointT>::DgramSocket( const EndpointT& local, const EndpointT& remote )
//	: DgramSocket<EndpointT>::DgramSocket()
//{
//	bind( local );
//	connect( remote );
//}
//
//template<class EndpointT>
//void DgramSocket<EndpointT>::connect( endpoint ep )
//{
//	auto result = _socket.connect( ep.toSockAddr() );
//	if( !result.success() ) {
//		throw generic_nw_error( make_error_message_with_appended_last_errno(
//			result, "Could not connect socket to address ", ep.toStringEx() ) );
//	}
//
//	_ep_remote = ep;
//}
//
//template<class EndpointT>
//void DgramSocket<EndpointT>::bind( endpoint ep )
//{
//	auto result = _socket.bind( ep.toSockAddr() );
//	if( !result.success() ) {
//		throw generic_nw_error( make_error_message_with_appended_last_errno(
//			result, "Could not bind socket to address ", ep.toStringEx() ) );
//	}
//
//	_ep_local = ep;
//}
//
//template<class EndpointT>
//socks::ErrorCode DgramSocket<EndpointT>::try_bind( endpoint ep ) noexcept
//{
//	auto result = _socket.bind( ep.toSockAddr() );
//	if( result.success() ) { _ep_local = ep; }
//
//	return result;
//}
//
//template<class EndpointT>
//socks::ErrorCode DgramSocket<EndpointT>::try_connect( endpoint ep ) noexcept
//{
//	auto result = _socket.connect( ep.toSockAddr() );
//	if( result.success() ) { _ep_remote = ep; }
//
//	return result;
//}
//
//template<class EndpointT>
//auto DgramSocket<EndpointT>::sendto( mart::ConstMemoryView data, endpoint ep ) -> mart::ConstMemoryView
//{
//	const auto res = _socket.sendto( data, 0, ep.toSockAddr() );
//	if( !res.result ) {
//		throw nw::generic_nw_error( make_error_message_with_appended_last_errno(
//			res.result.error_code(), "Failed to send data to", ep.toStringEx(), " . Details:  " ) );
//	}
//	return res.remaining_data;
//}

} // namespace detail
} // namespace socks
} // namespace nw
} // namespace mart

#endif