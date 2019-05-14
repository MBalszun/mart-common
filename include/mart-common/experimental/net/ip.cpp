#include "ip.hpp"

#ifdef MBA_UTILS_USE_WINSOCKS
#define NOMINMAX
#include <WinSock2.h>
#else
#include <netinet/ip.h>
#endif

namespace mart::nw::ip {

mart::ConstString address_v4::asString() const
{
	// 17 is maximal length a ipv4 address can have in the dotted notation: XXX.XXX.XXX.XXX\0
	std::array<char, 24> ret {};
	in_addr addr {};
	addr.s_addr = mart::toUType( _addr );
	nw::ip::port_layer::inet_net_to_pres( AF_INET, &addr, ret.data(), ret.size() ); // mart::ArrayView<char>(ret)));

	return mart::ConstString( mart::StringView::fromZString( ret.data() ) );
}

[[noreturn]] void address_v4::_throwParseIpV4StringError( const std::string_view str )
{
	throw std::invalid_argument(
		mart::concat( "Could not parse string \"", str, "\" - IP-Addess must have format a.b.c.d. " ).c_str() );
}

namespace _impl_details_ip {

[[noreturn]] void _throw_ipv4_parse_fail_invalid_format( std::string_view str )
{
	throw std::invalid_argument( mart::concat( "Creating ipv4 endpoint from string \"",
											   str,
											   "\" Failed. "
											   "Addess must have format a.b.c.d:p - colon or p is missing" )
									 .c_str() );
}
[[noreturn]] void _throw_ipv4_parse_fail_port( std::string_view str, std::string_view port )
{
	throw std::invalid_argument( mart::concat( "Parsing port <",
											   port,
											   "> from ipv4 endpoint string"
											   "\"",
											   str,
											   "\""
											   " failed."
											   "Note, port number has to be in the interval [0..65535]" )
									 .c_str() );
}

basic_endpoint_v4_base::basic_endpoint_v4_base( const sockaddr_in& native )
	: address( uint32_net_t( native.sin_addr.s_addr ) )
	, port( uint16_net_t( native.sin_port ) )
	, valid {native.sin_family == AF_INET ? true : throw std::invalid_argument( "Invalid sockaddr_in passed " )}
{
}

mart::ConstString basic_endpoint_v4_base::toString() const
{
	return mart::concat( address.asString(), ":", std::to_string( port.inHostOrder() ) );
}

mart::ConstString basic_endpoint_v4_base::toStringEx( TransportProtocol p ) const
{
	switch( p ) {
		case TransportProtocol::UDP: return mart::concat( toString(), " (UDP)" );
		case TransportProtocol::TCP: return mart::concat( toString(), " (TCP)" );
		default: return mart::concat( toString(), " (Unknown)" );
	}
}

sockaddr_in basic_endpoint_v4_base::toSockAddr_in() const noexcept
{
	sockaddr_in sockaddr {};
	sockaddr.sin_family      = AF_INET;
	sockaddr.sin_port        = mart::toUType( port.inNetOrder() );
	sockaddr.sin_addr.s_addr = mart::toUType( address.inNetOrder() );
	return sockaddr;
}

} // namespace _impl_details_ip

} // namespace mart::nw::ip
