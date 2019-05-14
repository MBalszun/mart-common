#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_OPTIONAL_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_OPTIONAL_H
/**
 * Optional.h (mart-common/experimental)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	PRovides mart::Optional, a type with similar purpose as std::optional, but vastly different semantics
 *
 */

#include <cstdint>
#include <stdexcept>

namespace mart {
template<class T>
class Optional {
	enum class OPT_FLAG : std::uint8_t { Invalid = 0, Valid };
	T        data;
	OPT_FLAG flag = OPT_FLAG::Invalid;

	constexpr void throwIfInvalid() const
	{
		if( flag != OPT_FLAG::Valid ) { throw std::invalid_argument( "Access to invalid data of Optional class" ); }
	}

public:
	constexpr Optional() = default;
	constexpr Optional( const T& data )
		: data( data )
		, flag( OPT_FLAG::Valid ) {};
	constexpr Optional( T&& data )
		: data( std::move( data ) )
		, flag( OPT_FLAG::Valid ) {};

	constexpr Optional( const Optional<T>& other ) = default;
	constexpr Optional( Optional<T>&& other )      = default;
	constexpr Optional& operator=( const Optional<T>& other ) = default;
	constexpr Optional& operator=( Optional<T>&& other ) = default;

	constexpr const T& getData() const
	{
		throwIfInvalid();
		return data;
	}

	constexpr const T& value() const
	{
		throwIfInvalid();
		return data;
	}

	constexpr T& value()
	{
		throwIfInvalid();
		return data;
	}

	constexpr void setData( const T& data )
	{
		this->data = data;
		this->flag = OPT_FLAG::Valid;
	}

	constexpr const T& operator*() const { return data; }
	constexpr       T& operator*() { return data; }
	constexpr const T* operator->() const { return &data; }
	constexpr       T* operator->() { return &data; }

	constexpr explicit operator bool() const { return isValid(); }
	constexpr OPT_FLAG getState() const { return flag; }
	constexpr bool     isValid() const { return flag == OPT_FLAG::Valid; }
	constexpr bool     has_value() const { return flag == OPT_FLAG::Valid; }
};
} // namespace mart

#endif /* LIBS_C2_API_LIBS_COMMON_OPTIONAL_H_ */
