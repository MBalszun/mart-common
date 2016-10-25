/*
* ip.h
*
*  Created on: 2016-09-29
*      Author: Michael Balszun <michael.balszun@tum.de>
*
*      This file provides some basic types related to the ip protocol
*/

#ifndef LIBS_MART_COMMON_EXPERIMENTAL_NW_IP_H_
#define LIBS_MART_COMMON_EXPERIMENTAL_NW_IP_H_
#pragma once

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

/* Project Includes */
#include "basic_types.h"

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
private:
	uint32_net_t _addr{};

	static uint32_net_t _parseIpV4String(mart::StringView str)
	{
		uint32_net_t ret{};
		uint8_t * blocks = reinterpret_cast<uint8_t*>(&ret);
		int idx = 0;
		for (auto c : str) {
			if ('0' <= c && c <= '9') {
				blocks[idx] *= 10;
				blocks[idx] += c - '0';
			} else {
				//separator digit -> go to next block
				idx++;
				if (idx >= 4) {
					break;
				}
			}
		}
		return ret;
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
	explicit basic_endpoint_v4(mart::StringView str)
	{
		auto sepIdx = str.find(':');
		if (sepIdx == mart::StringView::npos) {
			throw std::invalid_argument("Addess must have format a.b.c.d:p - colon is missing");
		}
		address = address_v4(str.substr(0,sepIdx));
		port	= port_nr(mart::to_integral(str.substr(sepIdx+1)));
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