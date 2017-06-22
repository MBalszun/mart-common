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
		_addr(_parseIpV4String(str))
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

	static uint32_net_t _parseIpV4String( const mart::StringView str )
	{
		_throwOnInvalidChar( str );
		_throwOnWrongBlockCount( str );

		uint32_net_t ret{};
		auto remaining = str;
		for( uint8_t& b : mart::viewMemory(ret)) {
			mart::StringView sblock;
			std::tie(sblock, remaining) = remaining.split(remaining.find('.'));

			b = static_cast<uint8_t>( _parseBlockChecked(sblock, str) );
		}
		return ret;
	}

	static int _parseBlockChecked(mart::StringView block, mart::StringView str )
	{
		if (block.size() < 4) {
			auto num = mart::to_integral_unsafe<int>(block);
			if (num <= 255) {
				return num;
			}
		}
		_throw_parse_error(str, "Block ", block, " exeeds allowed maximum of 255.");
		//return 0;
	}
	static void _throwOnWrongBlockCount(mart::StringView str)
	{
		if (std::count(str.begin(), str.end(), '.') != 3) {
			_throw_parse_error(str, "String doesn't have 4 number blocks separated by '.'");
		}
	}
	static void _throwOnInvalidChar(mart::StringView str)
	{
		if (auto it = mart::find_if_ex(str, [](char c) { return (c < '0' || '9' < c) && (c != '.'); })) {
			_throw_parse_error(str, "Invalid char: '", *it, "'");
		}
	}
	template<class ...ARGS>
	[[noreturn]] static void _throw_parse_error(mart::StringView str, ARGS&& ... args)
	{
		throw std::invalid_argument(mart::concat_cpp_str("Could not parse string \"", str, "\" - IP-Addess must have format a.b.c.d. ", args ...));
	}
};

constexpr address_v4 address_any{};
constexpr address_v4 address_local_host(uint32_host_t{ 0x7F000001 });

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

}//ns _impl_details_ip
}//ns ip
}//ns nw
}//experimental
}//ns mba


#endif