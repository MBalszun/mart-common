#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_IP_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_NW_IP_H
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
#include <string>
#include <string_view>
#include <type_traits>

/* Proprietary Library Includes */
#include "../../ArrayView.h"
#include "../../algorithm.h"
#include "../../utils.h"

#include <im_str/im_str.hpp>

/* Project Includes */
#include "basic_types.h"
#include "exceptions.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace experimental {
namespace nw {

// classes related to the ip protocol in general
namespace ip {

namespace impl_addr_v4 {

constexpr bool has_wrong_length( const std::string_view str )
{
	return ( str.size() > 4 * 3 + 3 )
		   || ( str.size() < 4 + 3 ); // max length: aaa.bbb.ccc.dddd  (4blocks of 3 digits + 3 '.')
}

constexpr bool has_invalid_char( const std::string_view str )
{
	return str.end() != mart::find_if( str, []( char c ) { return !( ( '0' <= c && c <= '9' ) || ( c == '.' ) ); } );
}

constexpr bool has_wrong_block_count( const std::string_view str )
{
	return mart::count( str, '.' ) != 3;
}

constexpr bool is_malformed( const std::string_view str )
{
	return has_wrong_length( str ) || has_invalid_char( str ) || has_wrong_block_count( str );
}

constexpr std::optional<std::uint32_t> parse_block( const std::string_view block )
{
	if( block.size() > 3 ) { return {}; }
	const auto num = mart::to_integral_unsafe<std::uint32_t>( block );
	if( num > 255 ) { return {}; }
	return num;
}

inline bool is_invalid_number( std::string_view block )
{
	return !parse_block( block );
}

constexpr std::array<std::string_view, 4> split_blocks_unchecked( const std::string_view str )
{
	std::array<std::string_view, 4> ret{};
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

class [[deprecated]] address_v4 {
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
	explicit address_v4( const char* str )
	{
		in_addr addr{};

		if( 1 == nw::ip::port_layer::inet_pres_to_net( AF_INET, str, &addr ) ) { _addr = uint32_net_t( addr.s_addr ); }
	}
	constexpr explicit address_v4( std::string_view str )
		: address_v4( _parseIpV4String( str ) )
	{
	}

	mba::im_zstr asString() const
	{
		std::array<char, 24>
				ret{}; // 17 is maximal length a ipv4 address can have in the dotted notation: XXX.XXX.XXX.XXX\0
		in_addr addr{};
		addr.s_addr = mart::toUType( _addr );
		nw::ip::port_layer::inet_net_to_pres( AF_INET, &addr, ret.data(), ret.size() ); // mart::ArrayView<char>(ret)));

		return mba::im_zstr( std::string_view( ret.data() ) );
	}

	constexpr uint32_net_t  inNetOrder() const { return _addr; }
	constexpr uint32_host_t inHostOrder() const { return to_host_order( _addr ); }

	friend bool operator==( address_v4 l, address_v4 r ) { return l._addr == r._addr; }

	friend bool operator<( address_v4 l, address_v4 r ) { return l._addr < r._addr; }

private:
	uint32_net_t _addr{};

	constexpr static uint32_host_t _parseIpV4String( const std::string_view str )
	{
		std::optional<uint32_host_t> res = impl_addr_v4::parse_address( str );
		if( !res ) {
			throw InvalidArgument(
				mart::concat( "Could not parse string \"", str, "\" - IP-Addess must have format a.b.c.d. " ) );
		}
		return *res;
	}

	template<class... ARGS>
	[[noreturn]] static void _throw_parse_error( std::string_view str, ARGS&&... args )
	{
		throw InvalidArgument(
			mart::concat( "Could not parse string \"", str, "\" - IP-Addess must have format a.b.c.d. ", args... ) );
	}
};

constexpr address_v4 address_any{};
constexpr address_v4 address_local_host( uint32_host_t{0x7F000001} );

inline std::optional<address_v4> parse_v4_address( const std::string_view string )
{
	auto res = impl_addr_v4::parse_address( string );
	if( res ) {
		return {address_v4{*res}};
	} else {
		return {};
	}
}

inline bool is_valid_v4_address( const std::string_view string )
{
	return impl_addr_v4::parse_address( string ).has_value();
}

namespace impl_port_v4 {
inline bool has_wrong_length( const std::string_view str )
{
	return str.size() > ( 4 * 3 + 3 ) || str.empty();
}

inline bool has_invalid_char( const std::string_view str )
{
	return str.end() != mart::find_if( str, []( char c ) { return !( '0' <= c && c <= '9' ); } );
}
} // namespace impl_port_v4

class [[deprecated]] port_nr
{
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
	friend bool             operator==( port_nr l, port_nr r ) { return l._p == r._p; }
	friend bool             operator<( port_nr l, port_nr r ) { return l._p < r._p; }

private:
	uint16_net_t _p{};
};

inline std::optional<port_nr> parse_v4_port( const std::string_view string )
{
	using namespace impl_port_v4;
	if( has_wrong_length( string ) || has_invalid_char( string ) ) { // maximal 6 digits
		return {};
	}
	auto parsed = mart::to_integral_unsafe<std::uint32_t>( string );
	if( parsed > std::numeric_limits<std::uint16_t>::max() ) { return {}; }
	return port_nr( static_cast<std::uint16_t>( parsed ) );
}

enum class TransportProtocol { UDP, TCP };

namespace _impl_details_ip {

template<TransportProtocol p>
struct [[deprecated]] basic_endpoint_v4
{
	constexpr basic_endpoint_v4() = default;

	constexpr explicit basic_endpoint_v4( const sockaddr_in& native )
		: address( uint32_net_t( native.sin_addr.s_addr ) )
		, port( uint16_net_t( native.sin_port ) )
		, valid{native.sin_family == AF_INET ? true : throw std::invalid_argument( "Invalid sockaddr_in passed " )}
	{
	}
	constexpr basic_endpoint_v4( address_v4 address, port_nr port )
		: address( address )
		, port( port )
		, valid{true}
	{
	}
	constexpr basic_endpoint_v4( uint32_host_t address, uint16_host_t port )
		: address( address )
		, port( port )
		, valid{true}
	{
	}
	// expects format XXX.XXX.XXX.XXX:pppp
	constexpr explicit basic_endpoint_v4( std::string_view strv )
	{
		const auto addr_port_pair = [str = mart::StringView( strv )] {
			auto ps = str.split( str.find( ':' ) );
			if( ps.second.size() < 1u ) {
				throw std::invalid_argument( mart::concat( "Creating ipv4 endpoint from string \"",
														   str,
														   "\" Failed. "
														   "Addess must have format a.b.c.d:p - colon or p is missing" )
												 .c_str() );
			}
			return ps;
		}();

		address = address_v4( addr_port_pair.first );

		port = [addr_port_pair, strv] {
			constexpr int max_port_nr    = std::numeric_limits<uint16_t>::max();
			const int     parsed_port_nr = mart::to_integral( addr_port_pair.second );
			if( parsed_port_nr < 0 || parsed_port_nr > max_port_nr ) {
				throw std::invalid_argument( mart::concat_cpp_str( "Creating ipv4 endpoint from string \"",
																   strv,
																   "\" Failed. "
																   "Portnumber was parsed as <",
																   std::to_string( parsed_port_nr ),
																   "> "
																   "which exeds the allowed range of [0..",
																   std::to_string( max_port_nr ),
																   "]" ) );
			}
			return port_nr{static_cast<uint16_t>( parsed_port_nr )};
		}();
		valid = true;
	}

	sockaddr_in toSockAddr_in() const
	{
		sockaddr_in sockaddr{};
		sockaddr.sin_family      = AF_INET;
		sockaddr.sin_port        = mart::toUType( port.inNetOrder() );
		sockaddr.sin_addr.s_addr = mart::toUType( address.inNetOrder() );
		return sockaddr;
	}

	mba::im_zstr toString() const
	{
		return mba::concat( address.asString(), ":", std::to_string( port.inHostOrder() ) );
	}
	mba::im_zstr toStringEx() const
	{
		std::string_view suffix;
		switch( p ) {
			case TransportProtocol::UDP: suffix = " (UDP)"; break;
			case TransportProtocol::TCP: suffix = " (TCP)"; break;
			default: suffix = " (Unknown)";
		}
		return mba::concat( address.asString(), ":", std::to_string( port.inHostOrder() ), suffix );
	}
	/* ####### State ############ */
	address_v4  address{};
	port_nr     port{};
	bool        valid = false;
	friend bool operator==( basic_endpoint_v4 l, basic_endpoint_v4 r )
	{
		return l.address == r.address && l.port == r.port;
	}

	friend bool operator!=( basic_endpoint_v4 l, basic_endpoint_v4 r ) { return !( l == r ); }
};

template<TransportProtocol p>
std::optional<basic_endpoint_v4<p>> parse_v4_endpoint( std::string_view str )
{
	auto ps        = mart::StringView( str ).split( ':' );
	auto o_address = parse_v4_address( ps.first );
	auto o_port    = parse_v4_port( ps.second );
	if( !o_address ) { return {}; }
	if( !o_port ) { return {}; }
	return basic_endpoint_v4<p>{*o_address, *o_port};
}

template<TransportProtocol p>
inline bool is_valid_v4_endpoint( std::string_view str )
{
	return parse_v4_endpoint<p>( str ).has_value();
}

} // namespace _impl_details_ip

inline bool is_valid_v4_endpoint( std::string_view str )
{
	return _impl_details_ip::is_valid_v4_endpoint<TransportProtocol::TCP>( str );
}

} // namespace ip
} // namespace nw
} // namespace experimental
} // namespace mart

#endif