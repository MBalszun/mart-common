#include <mart-netlib/ip.hpp>

#include <im_str/im_str.hpp>


#include <mart-netlib/network_exceptions.hpp>

namespace mart::nw::ip {


mba::im_zstr address_v4::asString() const
{
	// 17 is maximal length a ipv4 address can have in the dotted notation: XXX.XXX.XXX.XXX\0
	std::array<char, 24> ret {};

	nw::socks::port_layer::inet_net_to_pres( socks::Domain::Inet, &(this->_addr), ret.data(), ret.size() ); // mart::ArrayView<char>(ret)));

	return mba::im_zstr( std::string_view( ret.data() ) );
}

[[noreturn]] void address_v4::_throwParseIpV4StringError( const std::string_view str )
{
	throw mart::nw::invalid_address_string(
		mba::concat( "Could not parse string \"", str, "\" - IP-Addess must have format a.b.c.d. " ) );
}

namespace _impl_details_ip {

[[noreturn]] void _throw_ipv4_parse_fail_invalid_format( std::string_view str )
{
	throw mart::nw::invalid_address_string(
		mba::concat( "Creating ipv4 endpoint from string \"",
											   str,
											   "\" Failed. "
											   "Addess must have format a.b.c.d:p - colon or p is missing" )
									);
}
[[noreturn]] void _throw_ipv4_parse_fail_port( std::string_view str, std::string_view port )
{
	throw mart::nw::invalid_address_string( mba::concat( "Parsing port <",
											   port,
											   "> from ipv4 endpoint string"
											   "\"",
											   str,
											   "\""
											   " failed."
											   "Note, port number has to be in the interval [0..65535]" )
									 );
}

basic_endpoint_v4_base::basic_endpoint_v4_base( const mart::nw::socks::port_layer::SockaddrIn& addr )
	: address( addr.address() )
	, port( addr.port() )
	, valid { addr.is_valid()
				 ? true : throw mart::nw::invalid_address_string( "Invalid sockaddr_in passed " )}
{
}

mba::im_zstr basic_endpoint_v4_base::toString() const
{
	return mba::concat( address.asString(), ":", std::to_string( port.inHostOrder() ) );
}

mba::im_zstr basic_endpoint_v4_base::toStringEx( TransportProtocol p ) const
{
	switch( p ) {
		case TransportProtocol::Udp: return mba::concat( toString(), " (UDP)" );
		case TransportProtocol::Tcp: return mba::concat( toString(), " (TCP)" );
		default: return mba::concat( toString(), " (Unknown)" );
	}
}

} // namespace _impl_details_ip

} // namespace mart::nw::ip
