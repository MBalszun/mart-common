#ifndef LIB_MART_COMMON_GUARD_NW_IP_H
#define LIB_MART_COMMON_GUARD_NW_IP_H
/**
 * ip.h (mart-common/experimental/nw)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
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
/* Standard Library Includes */

#include <array>
#include <cassert>
#include <optional>
#include <string_view>
#include <type_traits>

/* Proprietary Library Includes */
#include <mart-common/ArrayView.h>
#include <mart-common/ConstString.h>
#include <mart-common/algorithm.h>
#include <mart-common/utils.h>

/* Project Includes */
#include "basic_types.hpp"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

//#ifdef MBA_UTILS_USE_WINSOCKS
//#define NOMINMAX
//#include <WinSock2.h>
//#else
//#include <netinet/ip.h>
//#endif

namespace mart::nw {
// classes related to the ip protocol in general
namespace ip {

namespace impl_addr_v4 {

constexpr bool has_wrong_length( std::string_view str ) noexcept
{
	return ( str.size() > 4 * 3 + 3 )
		   || ( str.size() < 4 + 3 ); // max length: aaa.bbb.ccc.dddd  (4blocks of 3 digits + 3 '.')
}

constexpr bool has_invalid_char( std::string_view str ) noexcept
{
	for( auto c : str ) {
		bool is_valid = ( ( '0' <= c && c <= '9' ) || ( c == '.' ) );
		if( !is_valid ) { return true; }
	}
	return false;
}

constexpr bool has_wrong_block_count( std::string_view str ) noexcept
{
	int cnt = 0;
	for( auto c : str ) {
		cnt += c == '.';
	}
	return cnt != 3;
	// not constexpr yet: return std::count( str.begin(), str.end(), '.' ) != 3;
}

constexpr bool is_malformed( std::string_view str ) noexcept
{
	return has_wrong_length( str ) || has_invalid_char( str ) || has_wrong_block_count( str );
}

constexpr std::optional<std::uint32_t> parse_block( std::string_view block )
{
	if( block.size() > 3 ) { return {}; }
	const auto num = mart::to_integral_unsafe<std::uint32_t>( mart::StringView( block ) );
	if( num > 255 ) { return {}; }
	return num;
}

constexpr bool is_invalid_number( std::string_view block )
{
	return !parse_block( block );
}

constexpr std::array<std::string_view, 4> split_blocks_unchecked( const std::string_view str )
{
	std::array<std::string_view, 4> ret {};
	int                             cnt   = 0;
	std::string_view::size_type     start = 0;
	for( auto pos = start; pos < str.size(); ++pos ) {
		if( str[pos] == '.' ) {
			ret[cnt++] = str.substr( start, pos - start );
			start      = pos + 1;
		}
	}
	ret[3] = str.substr( start /*, str.size()-start*/ );
	return ret;
}

constexpr std::optional<uint32_host_t> parse_address( const std::string_view string )
{
	if( is_malformed( string ) ) { return {}; }
	std::uint32_t accum = 0;
	for( const auto block : split_blocks_unchecked( string ) ) {
		auto b = parse_block( block );
		if( !b ) { return {}; }
		auto v = *b;
		assert( v <= 255u );
		accum <<= 8;
		accum |= v;
	}
	return accum;
}

} // namespace impl_addr_v4

class address_v4 {
public:
	constexpr address_v4() = default;
	constexpr explicit address_v4( uint32_host_t addr_h )
		: _addr( to_net_order( addr_h ) )
	{
	}
	constexpr explicit address_v4( uint32_net_t addr_h )
		: _addr( addr_h )
	{
	}
	constexpr explicit address_v4( std::string_view str )
		: address_v4( _parseIpV4String( str ) )
	{
	}

	mart::ConstString asString() const;

	constexpr uint32_net_t  inNetOrder() const { return _addr; }
	constexpr uint32_host_t inHostOrder() const { return to_host_order( _addr ); }

	friend constexpr bool operator==( address_v4 l, address_v4 r ) { return l._addr == r._addr; }
	friend constexpr bool operator!=( address_v4 l, address_v4 r ) { return l._addr != r._addr; }
	friend constexpr bool operator<( address_v4 l, address_v4 r ) { return l._addr < r._addr; }

private:
	uint32_net_t _addr {};

	[[noreturn]] static void _throwParseIpV4StringError( const std::string_view str );

	static constexpr uint32_host_t _parseIpV4String( const std::string_view str )
	{
		std::optional<uint32_host_t> res = impl_addr_v4::parse_address( str );
		if( !res ) { _throwParseIpV4StringError( str ); }
		return *res;
	}
};

constexpr address_v4 address_any {};
constexpr address_v4 address_local_host( uint32_host_t {0x7F000001} );

constexpr std::optional<address_v4> parse_v4_address( const std::string_view string )
{
	auto res = impl_addr_v4::parse_address( string );
	if( res ) {
		return {address_v4 {*res}};
	} else {
		return {};
	}
}

constexpr bool is_valid_v4_address( const std::string_view string )
{
	return impl_addr_v4::parse_address( string ).has_value();
}

namespace impl_port_v4 {
constexpr bool has_wrong_length( std::string_view str )
{
	return str.size() > 6 || str.empty(); // 16 bit number 65XXX
}

constexpr bool has_invalid_char( std::string_view str )
{
	for( auto c : str ) {
		bool is_valid = ( ( '0' <= c && c <= '9' ) );
		if( !is_valid ) { return true; }
	}
	return false;
}
} // namespace impl_port_v4

class port_nr {
public:
	constexpr port_nr() = default;
	constexpr explicit port_nr( uint16_host_t value )
		: _p( to_net_order( value ) )
	{
	}
	constexpr explicit port_nr( uint16_net_t value )
		: _p( value )
	{
	}

	constexpr uint16_net_t  inNetOrder() const { return _p; }
	constexpr uint16_host_t inHostOrder() const { return to_host_order( _p ); }
	friend constexpr bool   operator==( port_nr l, port_nr r ) { return l._p == r._p; }
	friend constexpr bool   operator<( port_nr l, port_nr r ) { return l._p < r._p; }

	static constexpr int max_port_nr = std::numeric_limits<uint16_t>::max();

private:
	uint16_net_t _p {};
};

constexpr std::optional<port_nr> parse_v4_port( const std::string_view string )
{
	using namespace impl_port_v4;
	if( has_wrong_length( string ) || has_invalid_char( string ) ) { // maximal 6 digits
		return {};
	}
	auto parsed = mart::to_integral_unsafe<std::uint32_t>( mart::StringView( string ) );
	if( parsed > std::numeric_limits<std::uint16_t>::max() ) { return {}; }
	return port_nr( static_cast<std::uint16_t>( parsed ) );
}

enum class TransportProtocol { UDP, TCP };

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

	explicit basic_endpoint_v4_base( const sockaddr_in& native );

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

	// expects format XXX.XXX.XXX.XXX:pppp
	constexpr explicit basic_endpoint_v4_base( std::string_view str )
	{
		const auto addr_port_pair = [lstr =  mart::StringView( str )] {
			auto ps = lstr.split( ':' );
			if( ps.second.size() < 1u ) { _throw_ipv4_parse_fail_invalid_format( lstr ); }
			return ps;
		}();

		address = address_v4( addr_port_pair.first );

		port = [addr_port_pair, str] {
			auto port = parse_v4_port( addr_port_pair.second );
			if( port ) {
				return port.value();
			} else {
				_throw_ipv4_parse_fail_port( str, addr_port_pair.second );
			}
		}();
		valid = true;
	}

	mart::ConstString toString() const;
	mart::ConstString toStringEx( TransportProtocol p ) const;
	sockaddr_in toSockAddr_in() const noexcept;
};

constexpr std::optional<basic_endpoint_v4_base> parse_basic_v4_endpoint( std::string_view str )
{
	auto ps        = mart::StringView( str ).split( ':' );
	auto o_address = parse_v4_address( ps.first );
	auto o_port    = parse_v4_port( ps.second );
	if( !o_address ) { return {}; }
	if( !o_port ) { return {}; }
	return basic_endpoint_v4_base {*o_address, *o_port};
}

template<TransportProtocol p>
struct basic_endpoint_v4 : basic_endpoint_v4_base {

	using basic_endpoint_v4_base::basic_endpoint_v4_base;

	mart::ConstString     toStringEx() const { return basic_endpoint_v4_base::toStringEx( p ); }
	friend constexpr bool operator==( basic_endpoint_v4 l, basic_endpoint_v4 r )
	{
		return l.address == r.address && l.port == r.port;
	}

	friend constexpr bool operator!=( basic_endpoint_v4 l, basic_endpoint_v4 r ) { return !( l == r ); }

	static constexpr std::optional<basic_endpoint_v4<p>> parse_v4_endpoint( std::string_view str )
	{
		auto rs = parse_basic_v4_endpoint( str );
		if( rs.has_value() ) {
			return basic_endpoint_v4( rs.value() );
		} else {
			return {};
		}
	}

private:
	constexpr basic_endpoint_v4( const basic_endpoint_v4_base& other )
		: basic_endpoint_v4_base( other )
	{
	}
};

} // namespace _impl_details_ip

constexpr bool is_valid_v4_endpoint( std::string_view str )
{
	return _impl_details_ip::parse_basic_v4_endpoint( str ).has_value();
}

template<TransportProtocol p>
constexpr std::optional<_impl_details_ip::basic_endpoint_v4<p>> parse_v4_endpoint( std::string_view str )
{
	return _impl_details_ip::basic_endpoint_v4<p>::parse_v4_endpoint( str );
}

} // namespace ip
} // namespace mart::nw

#endif