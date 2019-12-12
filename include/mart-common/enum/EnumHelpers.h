#ifndef LIB_MART_COMMON_GUARD_ENUM_ENUM_HELPERS_H
#define LIB_MART_COMMON_GUARD_ENUM_ENUM_HELPERS_H
/**
 * EnumHelpers.h (mart-common/enum)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <array>
#include <optional>
#include <string_view>
#include <type_traits>

/* Proprietary Library Includes */

/* Project Includes */
#include "../experimental/Optional.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

/**
 * converts a enum class value to the corresponding value of the underlying type
 * @param e enum to convert
 * @return corresponding underlying vale
 */
template<class T>
constexpr auto toUType( T e ) noexcept -> std::underlying_type_t<T>
{
	return static_cast<std::underlying_type_t<T>>( e );
};

// this is the fallback implementation if the enum author doesn't provide  mart_enumCnt_impl himself
template<class Enum, int val = toUType( Enum::COUNT )>
constexpr int mart_enumCnt_impl( Enum* ) noexcept
{
	return val;
}

template<class Enum, class uType = std::underlying_type_t<Enum>>
constexpr Enum mart_idxToEnum_impl( size_t v )
{
	return static_cast<Enum>( v );
}

/*#### Enum query functions ####*/
/**
 * Implementation note
 *Those interface functions forward to mart_<name>_impl functions that are supposed to be defined in the same namespace
 * as the respective enum and are found via argument dependent lookup. One way to generate those definition functions is
 * to use the macro MART_UTILS_DEFINE_ENUM which is defined in EnumDefinitionMacro.h header but you can also just write
 * them on your own. In some cases, a fallback solution is provided here. e.g. mart_enumCnt_impl is defined in terms of
 * Enum::COUNT by default
 *
 */

// clang-format off
template <class Enum> constexpr auto enumCnt      ()               -> std::size_t                                                       { return mart_enumCnt_impl( static_cast<Enum*>( nullptr ) );}
template <class Enum> constexpr auto to_string_view( Enum id )     -> std::string_view                                                  { return mart_to_string_v_impl( id ); }

template <class Enum> constexpr auto idxToEnum    ( size_t value ) -> Enum                                                              { return mart_idxToEnum_impl<Enum>( value ); }
template <class Enum> constexpr auto getEnums     ()               -> decltype( mart_getEnums_impl    ( static_cast<Enum*>(nullptr) ) ) { return mart_getEnums_impl    (static_cast<Enum*>(nullptr)); }
template <class Enum> constexpr auto getEnumNames ()               -> decltype( mart_getEnumNames_impl( static_cast<Enum*>(nullptr) ) ) { return mart_getEnumNames_impl(static_cast<Enum*>(nullptr)); }

// clang-format on

template<class Enum>
[[deprecated("Please use to_enum instead")]] constexpr Optional<Enum> toEnum( std::string_view str ) noexcept
{
	std::size_t idx = 0;
	for( const auto& name : getEnumNames<Enum>() ) {
		if( str == name ) {
			return getEnums<Enum>()[idx];
		} else {
			++idx;
		}
	}
	return {};
}

template<class Enum>
constexpr std::optional<Enum> to_enum( std::string_view str ) noexcept
{
	std::size_t idx = 0;
	for( const auto& name : getEnumNames<Enum>() ) {
		if( str == name ) {
			return getEnums<Enum>()[idx];
		} else {
			idx++;
		}
	}
	return {};
}

} // namespace mart

#endif
