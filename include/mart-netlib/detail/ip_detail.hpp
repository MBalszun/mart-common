#ifndef LIB_MART_COMMON_GUARD_NW_DETAIL_IP_DETAIL_H
#define LIB_MART_COMMON_GUARD_NW_DETAIL_IP_DETAIL_H

#include "../basic_types.hpp"

#include <mart-common/StringView.h>

#include <array>
#include <cassert>
#include <optional>
#include <string_view>

namespace mart::nw::ip {

namespace _impl_addr_v4 {

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
	const auto num = mart::to_integral_unsafe<std::uint32_t>( block );
	if( num > 255 ) { return {}; }
	return num;
}

constexpr bool is_invalid_number( std::string_view block )
{
	return !parse_block( block );
}

// unchecked version of str.substr (which would throw exceptions)
constexpr std::string_view substr( std::string_view str, std::size_t start, std::size_t length )
{
	return std::string_view {str.data() + start, length};
}

// unchecked version of str.substr
constexpr std::string_view substr( std::string_view str, std::size_t start )
{
	return std::string_view {str.data() + start, str.size() - start};
}

constexpr std::array<std::string_view, 4> split_blocks_unchecked( const std::string_view str )
{
	std::array<std::string_view, 4> ret {};
	int                             cnt   = 0;
	std::string_view::size_type     start = 0;
	for( auto pos = start; pos < str.size(); ++pos ) {
		if( str[pos] == '.' ) {
			ret[cnt++] = substr( str, start, pos - start );
			start      = pos + 1;
		}
	}
	ret[3] = substr( str, start /*, str.size()-start*/ );
	return ret;
}

constexpr std::optional<uint32_host_t> parse_address( const std::string_view string )
{
	if( is_malformed( string ) ) { return {}; }
	std::uint32_t accum = 0;
	for( const auto block : split_blocks_unchecked( string ) ) {
		auto b = parse_block( block );
		if( !b ) { return {}; }
		std::uint32_t v = *b;
		assert( v <= 255u );
		accum <<= 8;
		accum |= v;
	}
	return accum;
}

} // namespace _impl_addr_v4

namespace _impl_port_v4 {
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
} // namespace _impl_port_v4

namespace _impl_details_ip {

[[noreturn]] void _throw_ipv4_parse_fail_invalid_format( std::string_view str );
[[noreturn]] void _throw_ipv4_parse_fail_port( std::string_view str, std::string_view port );

} // namespace _impl_details_ip

} // namespace mart::nw::ip

#endif