#ifndef LIB_MART_COMMON_GUARD_NW_IP_H
#define LIB_MART_COMMON_GUARD_NW_IP_H
/**
 * ip.h (mart-netlib)
 *
 * Copyright (C) 2015-2019: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:  Provides some basic types related to the ip protocol
 *
 */

/* ######## INCLUDES ######### */
#include "basic_types.hpp"
#include "port_layer.hpp"

#include "detail/ip_detail.hpp"
/* Project Includes */

/* Proprietary Library Includes */
#include <mart-common/ConstString.h>
#include <mart-common/utils.h> //narrow

/* Standard Library Includes */
#include <array>
#include <cassert>
#include <optional>
#include <string_view>
#include <type_traits>

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */


namespace mart::nw {
// classes related to the ip protocol in general
namespace ip {

// ##### IP_v4 Address

class address_v4 {
public:
	constexpr address_v4() = default;
	constexpr explicit address_v4( uint32_host_t addr_h ) noexcept
		: _addr( to_net_order( addr_h ) )
	{
	}
	constexpr explicit address_v4( uint32_net_t addr_h ) noexcept
		: _addr( addr_h )
	{
	}
	constexpr explicit address_v4( std::string_view str )
		: address_v4( _parseIpV4String( str ) )
	{
	}

	mart::ConstString asString() const;

	constexpr uint32_net_t  inNetOrder() const noexcept { return _addr; }
	constexpr uint32_host_t inHostOrder() const noexcept { return to_host_order( _addr ); }

	friend constexpr bool operator==( address_v4 l, address_v4 r ) noexcept { return l._addr == r._addr; }
	friend constexpr bool operator!=( address_v4 l, address_v4 r ) noexcept { return l._addr != r._addr; }
	friend constexpr bool operator<( address_v4 l, address_v4 r )
	{
		return to_host_order( l._addr ) < to_host_order( r._addr );
	}

private:
	uint32_net_t _addr {};

	[[noreturn]] static void _throwParseIpV4StringError( const std::string_view str );

	static constexpr uint32_host_t _parseIpV4String( const std::string_view str )
	{
		std::optional<uint32_host_t> res = _impl_addr_v4::parse_address( str );
		if( !res ) { _throwParseIpV4StringError( str ); }
		return *res;
	}
};

constexpr address_v4 address_any {};
constexpr address_v4 address_local_host( uint32_host_t {0x7F'00'00'01} );

constexpr std::optional<address_v4> try_parse_v4_address( const std::string_view string ) noexcept
{
	auto res = _impl_addr_v4::parse_address( string );
	if( res ) {
		return {address_v4 {*res}};
	} else {
		return {};
	}
}

constexpr bool is_valid_v4_address( const std::string_view string )
{
	return _impl_addr_v4::parse_address( string ).has_value();
}

// ##### Port

class port_nr {
public:
	constexpr port_nr() noexcept = default;
	constexpr explicit port_nr( uint16_host_t value ) noexcept
		: _p( to_net_order( value ) )
	{
	}
	constexpr explicit port_nr( uint16_net_t value ) noexcept
		: _p( value )
	{
	}

	constexpr uint16_net_t  inNetOrder() const noexcept { return _p; }
	constexpr uint16_host_t inHostOrder() const noexcept { return to_host_order( _p ); }

	friend constexpr bool operator==( port_nr l, port_nr r ) noexcept { return l._p == r._p; }
	friend constexpr bool operator<( port_nr l, port_nr r ) noexcept
	{
		return to_host_order( l._p ) < to_host_order( r._p );
	}

	static constexpr int max_port_nr = std::numeric_limits<uint16_t>::max();

private:
	uint16_net_t _p {};
};

constexpr std::optional<port_nr> try_parse_v4_port( const std::string_view string ) noexcept
{
	using namespace _impl_port_v4;
	if( has_wrong_length( string ) || has_invalid_char( string ) ) { // maximal 6 digits
		return {};
	}
	auto parsed = mart::to_integral_unsafe<std::uint32_t>( string );
	if( parsed > std::numeric_limits<std::uint16_t>::max() ) { return {}; }
	return port_nr( static_cast<std::uint16_t>( parsed ) );
}

// ##### Protocol

enum class TransportProtocol { Udp, Tcp };

namespace _impl_details_ip {

[[noreturn]] void _throw_ipv4_parse_fail_invalid_format( std::string_view str );
[[noreturn]] void _throw_ipv4_parse_fail_port( std::string_view str, std::string_view port );

struct basic_endpoint_v4_base {

	/* ####### State ############ */
	address_v4 address {};
	port_nr    port {};
	bool       valid = false;

	/* ####### constructors ############ */
	constexpr basic_endpoint_v4_base() noexcept = default;

	explicit basic_endpoint_v4_base( const mart::nw::socks::port_layer::SockaddrIn& native );

	constexpr basic_endpoint_v4_base( address_v4 address, port_nr port ) noexcept
		: address( address )
		, port( port )
		, valid {true}
	{
	}

	constexpr basic_endpoint_v4_base( uint32_host_t address, uint16_host_t port ) noexcept
		: address( address )
		, port( port )
		, valid {true}
	{
	}

	constexpr basic_endpoint_v4_base( std::string_view address, uint16_host_t port ) noexcept
		: address( address_v4( address ) )
		, port( port )
		, valid {true}
	{
	}

	// expects format XXX.XXX.XXX.XXX:pppp
	constexpr explicit basic_endpoint_v4_base( std::string_view str )
	{
		const auto addr_port_pair = [lstr = mart::StringView( str )] {
			auto ps = lstr.split( ':' );
			if( ps.second.size() < 1u ) { _throw_ipv4_parse_fail_invalid_format( lstr ); }
			return ps;
		}();

		address = address_v4( addr_port_pair.first );

		port = [addr_port_pair, str] {
			auto port = try_parse_v4_port( addr_port_pair.second );
			if( port ) {
				return port.value();
			} else {
				_throw_ipv4_parse_fail_port( str, addr_port_pair.second );
			}
		}();
		valid = true;
	}

	mart::nw::socks::port_layer::SockaddrIn toSockAddr_in() const noexcept
	{
		return mart::nw::socks::port_layer::SockaddrIn( address.inNetOrder(), port.inNetOrder() );
	}

	mart::ConstString toString() const;

protected:
	mart::ConstString toStringEx( TransportProtocol p ) const;
};

constexpr std::optional<basic_endpoint_v4_base> try_parse_basic_v4_endpoint( std::string_view str ) noexcept
{
	const auto ps = mart::StringView( str ).split( ':' );

	const auto o_address = try_parse_v4_address( ps.first );
	if( !o_address ) { return {}; }

	const auto o_port = try_parse_v4_port( ps.second );
	if( !o_port ) { return {}; }

	return basic_endpoint_v4_base {*o_address, *o_port};
}

} // namespace _impl_details_ip

template<TransportProtocol p>
struct basic_endpoint_v4 : _impl_details_ip::basic_endpoint_v4_base {

	using _impl_details_ip::basic_endpoint_v4_base::basic_endpoint_v4_base;

	mart::ConstString     toStringEx() const { return _impl_details_ip::basic_endpoint_v4_base::toStringEx( p ); }
	friend constexpr bool operator==( basic_endpoint_v4 l, basic_endpoint_v4 r ) noexcept
	{
		return l.address == r.address && l.port == r.port;
	}

	friend constexpr bool operator!=( basic_endpoint_v4 l, basic_endpoint_v4 r ) noexcept { return !( l == r ); }

	static constexpr std::optional<basic_endpoint_v4<p>> try_parse( std::string_view str )
	{
		auto rs = _impl_details_ip::try_parse_basic_v4_endpoint( str );
		if( rs.has_value() ) {
			return basic_endpoint_v4<p>( rs.value() );
		} else {
			return {};
		}
	}

private:
	constexpr basic_endpoint_v4( const _impl_details_ip::basic_endpoint_v4_base& other )
		: basic_endpoint_v4_base( other )
	{
	}
};



constexpr bool is_valid_v4_endpoint( std::string_view str )
{
	return _impl_details_ip::try_parse_basic_v4_endpoint( str ).has_value();
}

template<TransportProtocol p>
constexpr std::optional<basic_endpoint_v4<p>> try_parse_v4_endpoint( std::string_view str )
{
	return basic_endpoint_v4<p>::try_parse( str );
}

} // namespace ip
} // namespace mart::nw

#endif