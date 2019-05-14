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
#include <string>
#include <array>
#include <cassert>
#include <type_traits>
#include <optional>

/* Proprietary Library Includes */
#include "../../utils.h"
#include "../../ConstString.h"
#include "../../ArrayView.h"
#include "../../algorithm.h"

/* Project Includes */
#include "basic_types.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace experimental {
namespace nw {

//classes related to the ip protocol in general
namespace ip {

namespace impl_addr_v4 {

inline bool has_wrong_length(mart::StringView str)
{
	return (str.size() > 4 * 3 + 3) || (str.size() < 4+3); // max length: aaa.bbb.ccc.dddd  (4blocks of 3 digits + 3 '.')
}

inline bool has_invalid_char(mart::StringView str)
{
	return str.end() != mart::find_if(str, [](char c) { return !(('0' <= c && c <= '9') || (c == '.')); });
}

inline bool has_wrong_block_count(mart::StringView str)
{
	return std::count(str.begin(), str.end(), '.') != 3;
}

inline bool is_malformed(mart::StringView str)
{
	return has_wrong_length(str) || has_invalid_char(str) || has_wrong_block_count(str);
}

inline std::optional<std::uint32_t> parse_block(mart::StringView block) {
	if (block.size() > 3) {
		return {};
	}
	const auto num = mart::to_integral_unsafe<std::uint32_t>(block);
	if (num > 255) {
		return {};
	}
	return num;
}

inline bool is_invalid_number(mart::StringView block) {
	return !parse_block(block);
}

inline std::array<mart::StringView, 4> split_blocks_unchecked(const mart::StringView str) {
	std::array<mart::StringView, 4> ret{};
	int cnt = 0;
	mart::StringView::size_type start = 0;
	for (auto pos = start; pos < str.size(); ++pos) {
		if (str[pos] == '.') {
			ret[cnt++] = str.substr(start, pos - start);
			start = pos + 1;
		}
	}
	ret[3] = str.substr(start /*, str.size()-start*/);
	return ret;
}

inline std::optional<uint32_host_t> parse_address(const mart::StringView string)
{
	if (is_malformed(string)) {
		return {};
	}
	std::uint32_t accum = 0;
	for (const auto block : split_blocks_unchecked(string)) {
		auto b = parse_block(block);
		if (!b) {
			return {};
		}
		auto v = *b;
		assert(v <= 255u);
		accum <<= 8;
		accum |= v;
	}
	return accum;
}

}




class address_v4 {
public:
	constexpr address_v4() = default;
	constexpr explicit address_v4(uint32_host_t addr_h) : _addr(to_net_order(addr_h)) {}
	constexpr explicit address_v4(uint32_net_t addr_h) : _addr(addr_h) {}
	explicit address_v4(const char* str)
	{
		in_addr addr{};

		if (1 == nw::ip::port_layer::inet_pres_to_net(AF_INET, str, &addr)) {
			_addr = uint32_net_t(addr.s_addr);
		}
	}
	explicit address_v4(mart::StringView str) :
		address_v4(_parseIpV4String(str))
	{}

	mart::ConstString asString() const
	{
		std::array<char, 24> ret{}; //17 is maximal length a ipv4 address can have in the dotted notation: XXX.XXX.XXX.XXX\0
		in_addr addr{};
		addr.s_addr = mart::toUType(_addr);
		nw::ip::port_layer::inet_net_to_pres(AF_INET, &addr, ret.data(), ret.size());// mart::ArrayView<char>(ret)));


		return mart::ConstString(mart::StringView::fromZString(ret.data()));
	}



	constexpr uint32_net_t inNetOrder() const { return _addr; }
	constexpr uint32_host_t inHostOrder() const { return to_host_order(_addr); }

	friend bool operator==(address_v4 l, address_v4 r) {
		return l._addr == r._addr;
	}

	friend bool operator<(address_v4 l, address_v4 r) {
		return l._addr < r._addr;
	}
private:
	uint32_net_t _addr{};

	static uint32_host_t _parseIpV4String( const mart::StringView str )
	{
		std::optional<uint32_host_t> res = impl_addr_v4::parse_address(str);
		if (!res) {
			throw std::invalid_argument(mart::concat_cpp_str("Could not parse string \"", str, "\" - IP-Addess must have format a.b.c.d. "));
		}
		return *res;
	}

	template<class ...ARGS>
	[[noreturn]] static void _throw_parse_error(mart::StringView str, ARGS&& ... args)
	{
		throw std::invalid_argument(mart::concat_cpp_str("Could not parse string \"", str, "\" - IP-Addess must have format a.b.c.d. ", args ...));
	}
};

constexpr address_v4 address_any{};
constexpr address_v4 address_local_host(uint32_host_t{ 0x7F000001 });

inline std::optional< address_v4> parse_v4_address(const mart::StringView string)
{
	auto res = impl_addr_v4::parse_address(string);
	if (res) {
		return { address_v4{*res} };
	} else {
		return {};
	}
}

inline bool is_valid_v4_address(const mart::StringView string)
{
	return impl_addr_v4::parse_address(string).has_value();
}

namespace impl_port_v4 {
	inline bool has_wrong_length(mart::StringView str)
	{
		return str.size() > (4 * 3 + 3) || str.empty();
	}

	inline bool has_invalid_char(mart::StringView str)
	{
		return str.end() != mart::find_if(str, [](char c) { return !('0' <= c && c <= '9') ; });
	}
}

class port_nr {
public:
	constexpr port_nr() = default;
	constexpr explicit port_nr(uint16_host_t value) :_p(to_net_order(value)) {}
	constexpr explicit port_nr(uint16_net_t value) :_p(value) {}

	constexpr uint16_net_t inNetOrder() const { return _p; }
	constexpr uint16_host_t inHostOrder() const { return to_host_order(_p); }
	friend bool operator==(port_nr l, port_nr r) {
		return l._p == r._p;
	}
	friend bool operator<(port_nr l, port_nr r) {
		return l._p < r._p;
	}
private:
	uint16_net_t _p{};
};

inline std::optional< port_nr> parse_v4_port(const mart::StringView string)
{
	using namespace impl_port_v4;
	if (has_wrong_length(string) || has_invalid_char(string)) { //maximal 6 digits
		return {};
	}
	auto parsed = mart::to_integral_unsafe<std::uint32_t>(string);
	if (parsed > std::numeric_limits<std::uint16_t>::max()) {
		return {};
	}
	return port_nr(static_cast<std::uint16_t>(parsed));
}

enum class TransportProtocol {
	UDP,
	TCP
};

namespace _impl_details_ip {

template<TransportProtocol p>
struct basic_endpoint_v4 {
	constexpr basic_endpoint_v4() = default;

	constexpr explicit basic_endpoint_v4(const sockaddr_in& native) :
		address(uint32_net_t(native.sin_addr.s_addr)),
		port(uint16_net_t(native.sin_port)),
		valid{ native.sin_family == AF_INET ? true : throw std::invalid_argument("Invalid sockaddr_in passed ") }
	{
	}
	constexpr basic_endpoint_v4(address_v4 address, port_nr port) :
		address(address),
		port(port),
		valid{ true }
	{}
	constexpr basic_endpoint_v4(uint32_host_t address, uint16_host_t port) :
		address(address),
		port(port),
		valid{ true }
	{}
	//expects format XXX.XXX.XXX.XXX:pppp
	explicit basic_endpoint_v4( mart::StringView str )
	{
		const auto addr_port_pair = [str] {
										auto ps = str.split( str.find( ':' ) );
										if( ps.second.size() < 1u ) {
											throw std::invalid_argument( mart::concat_cpp_str(
																			"Creating ipv4 endpoint from string \"", str,  "\" Failed. "
																			"Addess must have format a.b.c.d:p - colon or p is missing"
																		) );
										}
										return ps;
									}();

		address = address_v4( addr_port_pair.first );

		port = [addr_port_pair, str] {
								constexpr int max_port_nr = std::numeric_limits<uint16_t>::max();
								const int  parsed_port_nr = mart::to_integral( addr_port_pair.second );
								if(parsed_port_nr < 0 || parsed_port_nr > max_port_nr ) {
									throw std::invalid_argument( mart::concat_cpp_str(
																	"Creating ipv4 endpoint from string \"", str, "\" Failed. "
																	"Portnumber was parsed as <",  std::to_string( parsed_port_nr ), "> "
																	"which exeds the allowed range of [0..", std::to_string( max_port_nr ), "]"
																) );
								}
								return port_nr{ static_cast<uint16_t>( parsed_port_nr ) };
							}();
		valid = true;
	}

	sockaddr_in toSockAddr_in() const
	{
		sockaddr_in sockaddr{};
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_port = mart::toUType(port.inNetOrder());
		sockaddr.sin_addr.s_addr = mart::toUType(address.inNetOrder());
		return sockaddr;
	}

	mart::ConstString toString() const
	{
		return mart::concat(address.asString() , ":" , std::to_string(port.inHostOrder()));
	}
	mart::ConstString toStringEx() const
	{
		switch (p) {
		case TransportProtocol::UDP: return mart::concat(toString() , " (UDP)");
		case TransportProtocol::TCP: return mart::concat(toString() , " (TCP)");
		default:					 return mart::concat(toString() , " (Unknown)");
		}
	}
	/* ####### State ############ */
	address_v4 address{};
	port_nr port{};
	bool valid = false;
	friend bool operator==(basic_endpoint_v4 l, basic_endpoint_v4 r) {
		return l.address == r.address && l.port == r.port;
	}

	friend bool operator!=(basic_endpoint_v4 l, basic_endpoint_v4 r) {
		return !(l == r);
	}
};

template<TransportProtocol p>
std::optional<basic_endpoint_v4<p>> parse_v4_endpoint(mart::StringView str)
{
	auto ps = str.split(':');
	auto o_address = parse_v4_address(ps.first);
	auto o_port = parse_v4_port(ps.second);
	if (!o_address) {
		return {};
	}
	if (!o_port) {
		return {};
	}
	return basic_endpoint_v4<p>{ *o_address, *o_port };
}

template<TransportProtocol p>
inline bool is_valid_v4_endpoint(mart::StringView str)
{
	return parse_v4_endpoint<p>(str).has_value();
}

}//ns _impl_details_ip

inline bool is_valid_v4_endpoint(mart::StringView str)
{
	return _impl_details_ip::is_valid_v4_endpoint<TransportProtocol::TCP>(str);
}


}//ns ip
}//ns nw
}//experimental
}//ns mba


#endif