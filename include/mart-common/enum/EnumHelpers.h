#ifndef LIB_MART_COMMON_GUARD_ENUM_ENUM_HELPERS_H
#define LIB_MART_COMMON_GUARD_ENUM_ENUM_HELPERS_H
/**
* EnumIdxArray.h (mart-common/enum)
*
* Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
*
* This software may be modified and distributed under the terms
* of the MIT license. See either the LICENSE file in the library's root
* directory or http://opensource.org/licenses/MIT for details.
*
* @author: Michael Balszun <michael.balszun@mytum.de>
* @brief: array that can directly be indexed by an enum
*
*/

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <array>

/* Proprietary Library Includes */

/* Project Includes */
#include "EnumIdxArray.h"
#include "../StringView.h"
#include "../experimental/Optional.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */


#include <string>

namespace mart {

/**
 * converts a enum class value to the corresponding value of the underlying type
 * @param e enum to convert
 * @return corresponding underlying vale
 */
template <class T>
constexpr auto toUType( T e ) -> mart::underlying_type_t<T>
{
	return static_cast<mart::underlying_type_t<T>>( e );
};

// this is the fallback implementation
template <class Enum, int val = toUType( Enum::COUNT )>
constexpr int mart_enumCnt_impl( Enum* )
{
	return val;
}

template <class Enum, class uType = mart::underlying_type_t<Enum>>
constexpr Enum mart_idxToEnum_impl( size_t v )
{
	return static_cast<Enum>( v );
}

/*#### Enum query functions ####*/
/**
 * Implementation note
 *Those interface functions forward to mart_<name>_impl functions that are supposed to be defined in the same namespace
 * as the respective enum an are found via argument dependent lookup. On way to generate those definition functions is
 * to use the macro MART_UTILS_DEFINE_ENUM which is defined in EnumDefinitionMacro.h header but you can also just write them
 * on your own. In some cases, a fallback solution is provided here. e.g. mart_enumCnt_impl is defined in terms of Enum::COUNT by default
 *
 */

// clang-format off
template <class Enum> constexpr auto enumCnt      ()               -> int                                                               { return mart_enumCnt_impl( static_cast<Enum*>( nullptr ) );}
template <class Enum> constexpr auto to_string_v  ( Enum id )      -> mart::StringView                                                  { return mart_to_string_v_impl( id ); }
//FIXME if mart_to_string_impl( id ) returns by value we should do the same
template <class Enum> inline    auto to_string    ( Enum id )      -> const std::string&                                                { return mart_to_string_impl( id ); }
template <class Enum> constexpr auto idxToEnum    ( size_t value ) -> Enum                                                              { return mart_idxToEnum_impl<Enum>( value ); }
template <class Enum> constexpr auto getEnums     ()               -> decltype( mart_getEnums_impl    ( static_cast<Enum*>(nullptr) ) ) { return mart_getEnums_impl    (static_cast<Enum*>(nullptr)); }
template <class Enum> constexpr auto getEnumNames ()               -> decltype( mart_getEnumNames_impl( static_cast<Enum*>(nullptr) ) ) { return mart_getEnumNames_impl(static_cast<Enum*>(nullptr)); }

// clang-format on

template <class Enum> Optional<Enum> toEnum( mart::StringView str )
{
	constexpr auto enums = getEnumNames<Enum>();
	const auto	 idx   = std::find( enums.begin(), enums.end(), str ) - enums.begin();
	if( (size_t)idx < enumCnt<Enum>() ) {
		return getEnums<Enum>()[idx];
	} else {
		return {};
	}
}


}

#endif

