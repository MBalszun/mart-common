#ifndef LIB_MART_COMMON_GUARD_NW_BASIC_TYPES_H
#define LIB_MART_COMMON_GUARD_NW_BASIC_TYPES_H
/**
 * basic_types.hpp (mart-common/nw)
 *
 * Copyright (C) 2015-2019: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:  This file provides various basic types used in networking
 *          Many of them are platform independent replacements for
 *          native types related to the platform socket api
 *
 */

/* ######## INCLUDES ######### */
/* Project Includes */
/* Proprietary Library Includes */

/* Standard Library Includes */
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

#ifndef MBA_UTILS_USE_WINSOCKS
#ifdef _MSC_VER
#define MBA_UTILS_USE_WINSOCKS 1
#endif
#endif

#ifdef _MSC_VER
// assume little endian for windows
#define MBA_ORDER_LITTLE_ENDIAN 1
#define MBA_BYTE_ORDER MBA_ORDER_LITTLE_ENDIAN
#else
// use gcc primitives
#define MBA_ORDER_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define MBA_BYTE_ORDER __BYTE_ORDER__
#endif

// forward declaration of system specific sockaddr structure
struct sockaddr;
struct sockaddr_in;
struct sockaddr_in6;
struct sockaddr_un;

namespace mart {
namespace nw {

// Make unsigned interger in host and network order distinct types
enum class uint64_net_t : std::uint64_t {};
enum class uint32_net_t : std::uint32_t {};
enum class uint16_net_t : std::uint16_t {};

using uint64_host_t = std::uint64_t;
using uint32_host_t = std::uint32_t;
using uint16_host_t = std::uint16_t;

/*#### implement portable byte swap function that compiles down to a single bswap instruction on any platform I've
 * tested so far #########*/
namespace _impl_detail_bs {
// create a bitmasks of type t, where either bits [0...N/2) or [N/2...N) are set to one

template<class T>
constexpr T lb_mask( unsigned int N = sizeof( T ) )
{
	return ( T( 0x1 ) << ( N * 8 / 2 ) ) - T( 0x1 );
}
template<class T>
constexpr T hb_mask( unsigned int N = sizeof( T ) )
{
	return lb_mask<T>( N ) << N * 8 / 2;
}

template<class T, size_t N = sizeof( T )>
struct mbswap {
	static_assert( N % 2 == 0, "bswap Works only for types whose size is a power of 2" );
	static constexpr T calc( T d )
	{
		return mbswap<T, N / 2>::calc( d >> ( N * 8 / 2 ) & lb_mask<T>( N ) )
			   | ( mbswap<T, N / 2>::calc( d ) << ( N * 8 / 2 ) & hb_mask<T>( N ) );
	}
};

template<class T>
struct mbswap<T, 1> {
	static constexpr T calc( T d ) { return d; }
};

} // namespace _impl_detail_bs

template<class T, size_t N = sizeof( T )>
constexpr T bswap( T d )
{
	return _impl_detail_bs::mbswap<T, N>::calc( d );
}

// clang-format off
#if MBA_BYTE_ORDER == MBA_ORDER_LITTLE_ENDIAN //use whatever compiletime mechanism available to determine, if net byte order (big endian) is the same / or different than machine order

constexpr uint16_net_t  to_net_order(uint16_host_t host_rep) { return uint16_net_t(bswap(host_rep)); }
constexpr uint32_net_t  to_net_order(uint32_host_t host_rep) { return uint32_net_t(bswap(host_rep)); }
constexpr uint64_net_t  to_net_order(uint64_host_t host_rep) { return uint64_net_t(bswap(host_rep)); }

constexpr uint16_host_t to_host_order(uint16_net_t net_rep) { return uint16_host_t(bswap(static_cast<uint16_t>(net_rep))); }
constexpr uint32_host_t to_host_order(uint32_net_t net_rep) { return uint32_host_t(bswap(static_cast<uint32_t>(net_rep))); }
constexpr uint64_host_t to_host_order(uint64_net_t net_rep) { return uint64_host_t(bswap(static_cast<uint64_t>(net_rep))); }

#else

constexpr uint16_net_t  to_net_order(uint16_host_t host_rep) { return uint16_net_t(host_rep); }
constexpr uint32_net_t  to_net_order(uint32_host_t host_rep) { return uint32_net_t(host_rep); }
constexpr uint64_net_t  to_net_order(uint64_host_t host_rep) { return uint64_net_t(host_rep); }

constexpr uint16_host_t to_host_order(uint16_net_t net_rep) { return uint16_host_t(net_rep); }
constexpr uint32_host_t to_host_order(uint32_net_t net_rep) { return uint32_host_t(net_rep); }
constexpr uint64_host_t to_host_order(uint64_net_t net_rep) { return uint64_host_t(net_rep); }

#endif

// clang-format on

using raw_ip_address = uint32_net_t;
using raw_port       = uint16_net_t;

// very lightweight span-like type that abstracts a range of bytes
struct byte_range {
	const unsigned char* _data;
	std::uintptr_t       _size;

	constexpr std::size_t          size() const noexcept { return _size; }
	constexpr const unsigned char* begin() const noexcept { return _data; }
	constexpr const unsigned char* end() const noexcept { return _data + _size; }
	constexpr const unsigned char* data() const noexcept { return _data; }

	const char* char_ptr() const noexcept { return reinterpret_cast<const char*>( _data ); }
};

struct byte_range_mut {
	unsigned char* _data;
	std::uintptr_t _size;

	constexpr operator byte_range() const noexcept { return byte_range{_data, _size}; }

	constexpr std::size_t    size() const noexcept { return _size; }
	constexpr unsigned char* begin() const noexcept { return _data; }
	constexpr unsigned char* end() const noexcept { return _data + _size; }
	constexpr unsigned char* data() const noexcept { return _data; }

	char* char_ptr() const noexcept { return reinterpret_cast<char*>( _data ); }
};

template<class T>
byte_range byte_range_from_pod( const T& pod )
{
	return byte_range{reinterpret_cast<const unsigned char*>( &pod ), sizeof( pod )};
}

template<class T>
byte_range_mut byte_range_from_pod( T& pod )
{
	return byte_range_mut{reinterpret_cast<unsigned char*>( &pod ), sizeof( pod )};
}

template<class T>
byte_range_mut byte_range_mut_rom_pod( T& pod )
{
	return byte_range_mut{reinterpret_cast<unsigned char*>( &pod ), sizeof( pod )};
}

namespace socks {

enum class Domain {
	Invalid,
	Local,
	Inet,
	Inet6,
	Unspec,
};

enum class TransportType {
	Invalid,
	Stream,
	Datagram,
	Seqpacket,
};

enum class Protocol { Default, Udp, Tcp };

enum class SocketOptionLevel { Socket };

enum class SocketOption { so_rcvtimeo, so_sndtimeo };

enum class Direction { Tx, Rx };

using txrx_size_t = int;

struct Sockaddr {
	Domain domain() const noexcept { return _domain; }
	bool   is_valid() const noexcept { return _raw_storage_range.data() != nullptr; }
	// byte_range        raw_storage() const noexcept { return _raw_storage_range; }
	// byte_range_mut    raw_storage() noexcept { return _raw_storage_range; };
	::sockaddr*       to_native_ptr() noexcept { return reinterpret_cast<::sockaddr*>( _raw_storage_range.data() ); }
	const ::sockaddr* to_native_ptr() const noexcept
	{
		return reinterpret_cast<const ::sockaddr*>( _raw_storage_range.data() );
	}

	std::size_t size() const noexcept { return _raw_storage_range.size(); }
	void        set_valid_data_range( std::size_t s ) noexcept { _raw_storage_range._size = s; }

protected:
	const Domain   _domain;
	byte_range_mut _raw_storage_range;

	Sockaddr( Domain domain )
		: _domain( domain )
		, _raw_storage_range{} {};

	Sockaddr( Domain domain, byte_range_mut data )
		: _domain( domain )
		, _raw_storage_range( data ){};

	// default behavior is most likely wrong, as _raw_storage_range usually points into the datastructure
	Sockaddr( const Sockaddr& ) = delete;
	Sockaddr& operator=( const Sockaddr& ) = delete;
	~Sockaddr()                            = default;
};

enum class ErrorCodeValues : int {
	NoError         = 0,
	TryAgain        = EAGAIN,
	InvalidArgument = EINVAL,
	WouldBlock      = EWOULDBLOCK,
	Timeout         = 10060,     // Windows
	WsaeConnReset   = 0x00002746 // Windows WSAECONNRESET ECONNRESET
};

struct ErrorCode {
	// TODO list more error codes
	using Value_t = ErrorCodeValues;
	Value_t                    _value;
	static constexpr ErrorCode Ok() noexcept { return {ErrorCodeValues::NoError}; }

	constexpr int      raw_value() const noexcept { return static_cast<int>( _value ); }
	constexpr Value_t  value() const noexcept { return _value; }
	constexpr explicit operator bool() const noexcept { return _value == Value_t::NoError; }
	constexpr bool     success() const noexcept { return _value == Value_t::NoError; }
};

template<class T = int>
struct ReturnValue {
	ReturnValue() = default;
	constexpr explicit ReturnValue( const T& value ) noexcept
		: _value{value}
		, _success{true}
	{
	}

	constexpr explicit ReturnValue( ErrorCode errc ) noexcept
		: _errc( errc )
		, _success{false}
	{
	}

	constexpr explicit ReturnValue( ErrorCodeValues errc ) noexcept
		: _errc{errc}
		, _success{false}
	{
	}

	constexpr bool     success() const noexcept { return _success; }
	constexpr explicit operator bool() const noexcept { return success(); }

	constexpr T value() const noexcept { return _value; }
	constexpr T value_or( const T& default_value ) const { return _success ? value() : default_value; }

	constexpr ErrorCode error_code() const noexcept
	{
		return _success ? ErrorCode{ErrorCode::Value_t::NoError} : _errc;
	}
	constexpr int raw() const noexcept { return static_cast<int>( _errc.raw_value() ); }

private:
	union {
		ErrorCode _errc;
		T         _value;
	};
	bool _success = false;
};

template<class T = int>
struct NonTrivialReturnValue {
	NonTrivialReturnValue() = default;
	NonTrivialReturnValue( NonTrivialReturnValue&& other )
	{
		if( _success ) { _value.~T(); }
		if( other._success ) {
			_value = std::move( other._value );
		} else {
			_errc = other._errc;
		}
		_success = other._success;
	}

	constexpr explicit NonTrivialReturnValue( const T& value ) noexcept
		: _value{value}
		, _success{true}
	{
	}

	constexpr explicit NonTrivialReturnValue( T&& value ) noexcept
		: _value{std::move( value )}
		, _success{true}
	{
	}

	constexpr explicit NonTrivialReturnValue( ErrorCode errc ) noexcept
		: _errc( errc )
		, _success{false}
	{
	}

	constexpr bool     success() const noexcept { return _success; }
	constexpr explicit operator bool() const noexcept { return success(); }

	constexpr const T& value() const noexcept { return _value; }
	T&                 value() noexcept { return _value; } // can't be constexpr in c++11

	constexpr ErrorCode error_code() const noexcept
	{
		return _success ? ErrorCode{ErrorCode::Value_t::NoError} : _errc;
	}
	constexpr int raw() const noexcept { return static_cast<int>( _errc.raw_value() ); }

	~NonTrivialReturnValue()
	{
		if( _success ) { _value.~T(); }
	}

private:
	union {
		ErrorCode _errc;
		T         _value;
	};
	bool _success = false;
};

struct ISockaddrPolyWrapper {
	virtual ~ISockaddrPolyWrapper() = default;

	virtual const Sockaddr& to_Sockaddr() const = 0;

	operator const Sockaddr&() const { return to_Sockaddr(); };
};

template<class SockAddrT>
struct SockaddrPolyWrapper : ISockaddrPolyWrapper {
	SockaddrPolyWrapper( const SockAddrT& sock_addr )
		: addr( sock_addr )
	{
	}

	const Sockaddr& to_Sockaddr() const override { return addr; }

	SockAddrT addr;
};

struct AddrInfo {
	int                                   flags;
	Domain                                family;
	TransportType                         socktype;
	Protocol                              protocol;
	std::unique_ptr<ISockaddrPolyWrapper> addr;
	std::string                           canonname;
};

struct AddrInfoHints {
	int           flags;
	Domain        family;
	TransportType socktype;
	Protocol      protocol;
};

} // namespace socks

} // namespace nw

namespace net = nw;
} // namespace mart

#endif