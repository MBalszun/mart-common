#pragma once

#include "EnumHelpers.h"
#include "EnumIdxArray.h"
#include <array>
#include <string_view>

// Necessary to work around MSVC's decision to not expand __VA_ARGS__ when drectly passing it to a macro
#define MART_UTILS_IMPL_EXPAND( X ) X

#if !defined( _MSC_VER ) || ( defined( _MSVC_TRADITIONAL ) && !_MSVC_TRADITIONAL )

// Make a FOREACH macro
// apply WHAT on the second parameter, pop it off and recurse on the remaining parameteres
#define MART_UTILS_IMPL_FE_1( WHAT, X ) WHAT( X )
#define MART_UTILS_IMPL_FE_2( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_1( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_3( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_2( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_4( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_3( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_5( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_4( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_6( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_5( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_7( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_6( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_8( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_7( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_9( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_8( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_10( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_9( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_11( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_10( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_12( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_11( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_13( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_12( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_14( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_13( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_15( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_14( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_16( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_15( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_17( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_16( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_18( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_17( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_19( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_18( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_20( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_19( WHAT, __VA_ARGS__ )

// Make a PARAMETERIZED FOREACH macro
// apply WHAT on the second and third parameter, pop of the third parameter and recurse on the remaining parameteres

#define MART_UTILS_IMPL_FE_1_P( WHAT, P, X ) WHAT( P, X )
#define MART_UTILS_IMPL_FE_2_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_1_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_3_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_2_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_4_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_3_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_5_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_4_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_6_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_5_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_7_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_6_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_8_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_7_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_9_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_8_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_10_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_9_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_11_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_10_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_12_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_11_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_13_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_12_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_14_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_13_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_15_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_14_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_16_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_15_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_17_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_16_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_18_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_17_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_19_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_18_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_20_P( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_19_P( WHAT, P, __VA_ARGS__ )

#else // workaround for broken msvc preprocessor

// Make a FOREACH macro
// apply WHAT on the second parameter, pop it off and recurse on the remaining parameteres
#define MART_UTILS_IMPL_FE_1_IMPL( WHAT, X ) WHAT( X )
#define MART_UTILS_IMPL_FE_1( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_1_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_2_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_1( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_2( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_2_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_3_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_2( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_3( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_3_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_4_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_3( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_4( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_4_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_5_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_4( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_5( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_5_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_6_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_5( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_6( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_6_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_7_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_6( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_7( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_7_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_8_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_7( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_8( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_8_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_9_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_8( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_9( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_9_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_10_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_9( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_10( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_10_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_11_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_10( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_11( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_11_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_12_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_11( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_12( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_12_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_13_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_12( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_13( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_13_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_14_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_13( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_14( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_14_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_15_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_14( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_15( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_15_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_16_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_15( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_16( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_16_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_17_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_16( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_17( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_17_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_18_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_17( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_18( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_18_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_19_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_18( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_19( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_19_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_20_IMPL( WHAT, X, ... ) WHAT( X ) MART_UTILS_IMPL_FE_19( WHAT, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_20( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_20_IMPL( __VA_ARGS__ ) )
//... repeat as needed

// Make a PARAMETERIZED FOREACH macro
// apply WHAT on the second and third parameter, pop of the third parameter and recurse on the remaining parameteres
#define MART_UTILS_IMPL_FE_1_P_IMPL( WHAT, P, X ) WHAT( P, X )
#define MART_UTILS_IMPL_FE_1_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_1_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_2_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_1_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_2_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_2_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_3_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_2_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_3_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_3_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_4_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_3_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_4_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_4_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_5_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_4_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_5_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_5_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_6_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_5_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_6_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_6_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_7_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_6_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_7_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_7_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_8_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_7_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_8_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_8_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_9_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_8_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_9_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_9_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_10_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_9_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_10_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_10_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_11_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_10_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_11_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_11_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_12_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_11_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_12_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_12_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_13_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_12_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_13_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_13_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_14_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_13_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_14_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_14_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_15_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_14_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_15_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_15_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_16_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_15_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_16_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_16_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_17_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_16_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_17_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_17_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_18_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_17_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_18_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_18_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_19_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_18_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_19_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_19_P_IMPL( __VA_ARGS__ ) )
#define MART_UTILS_IMPL_FE_20_P_IMPL( WHAT, P, X, ... ) WHAT( P, X ) MART_UTILS_IMPL_FE_19_P( WHAT, P, __VA_ARGS__ )
#define MART_UTILS_IMPL_FE_20_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FE_20_P_IMPL( __VA_ARGS__ ) )
//... repeat as needed

#endif

#define MART_UTILS_IMPL_GET_21ST_ARG(                                                                                  \
	_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, NAME, ... )             \
	NAME

// IMPLEMENTATION NOTE:  let <N> be the number of Variadic arguments, then this applies the macro MART_UTILS_IMPL_FE_<N>
// on action and the variadic arguments
#define MART_UTILS_IMPL_FOR_EACH_IMPL( action, ... )                                                                   \
	MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_GET_21ST_ARG( __VA_ARGS__,                                                 \
														  MART_UTILS_IMPL_FE_20,                                       \
														  MART_UTILS_IMPL_FE_19,                                       \
														  MART_UTILS_IMPL_FE_18,                                       \
														  MART_UTILS_IMPL_FE_17,                                       \
														  MART_UTILS_IMPL_FE_16,                                       \
														  MART_UTILS_IMPL_FE_15,                                       \
														  MART_UTILS_IMPL_FE_14,                                       \
														  MART_UTILS_IMPL_FE_13,                                       \
														  MART_UTILS_IMPL_FE_12,                                       \
														  MART_UTILS_IMPL_FE_11,                                       \
														  MART_UTILS_IMPL_FE_10,                                       \
														  MART_UTILS_IMPL_FE_9,                                        \
														  MART_UTILS_IMPL_FE_8,                                        \
														  MART_UTILS_IMPL_FE_7,                                        \
														  MART_UTILS_IMPL_FE_6,                                        \
														  MART_UTILS_IMPL_FE_5,                                        \
														  MART_UTILS_IMPL_FE_4,                                        \
														  MART_UTILS_IMPL_FE_3,                                        \
														  MART_UTILS_IMPL_FE_2,                                        \
														  MART_UTILS_IMPL_FE_1 ) )                                     \
	( action, __VA_ARGS__ )

#define MART_UTILS_IMPL_FOR_EACH( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FOR_EACH_IMPL( __VA_ARGS__ ) )

// IMPLEMENTATION NOTE: let <N> be the number of Variadic arguments, then this applies the macro
// MART_UTILS_IMPL_FE_<N>_P on action, param and the variadic arguments
#define MART_UTILS_IMPL_FOR_EACH_P_IMPL( action, param, ... )                                                          \
	MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_GET_21ST_ARG( __VA_ARGS__,                                                 \
														  MART_UTILS_IMPL_FE_20_P,                                     \
														  MART_UTILS_IMPL_FE_19_P,                                     \
														  MART_UTILS_IMPL_FE_18_P,                                     \
														  MART_UTILS_IMPL_FE_17_P,                                     \
														  MART_UTILS_IMPL_FE_16_P,                                     \
														  MART_UTILS_IMPL_FE_15_P,                                     \
														  MART_UTILS_IMPL_FE_14_P,                                     \
														  MART_UTILS_IMPL_FE_13_P,                                     \
														  MART_UTILS_IMPL_FE_12_P,                                     \
														  MART_UTILS_IMPL_FE_11_P,                                     \
														  MART_UTILS_IMPL_FE_10_P,                                     \
														  MART_UTILS_IMPL_FE_9_P,                                      \
														  MART_UTILS_IMPL_FE_8_P,                                      \
														  MART_UTILS_IMPL_FE_7_P,                                      \
														  MART_UTILS_IMPL_FE_6_P,                                      \
														  MART_UTILS_IMPL_FE_5_P,                                      \
														  MART_UTILS_IMPL_FE_4_P,                                      \
														  MART_UTILS_IMPL_FE_3_P,                                      \
														  MART_UTILS_IMPL_FE_2_P,                                      \
														  MART_UTILS_IMPL_FE_1_P ) )                                   \
	( action, param, __VA_ARGS__ )

#define MART_UTILS_IMPL_FOR_EACH_P( ... ) MART_UTILS_IMPL_EXPAND( MART_UTILS_IMPL_FOR_EACH_P_IMPL( __VA_ARGS__ ) )

#define MART_UTILS_IMPL_DEFINE_ENUM_MEMBER( P, X ) P::X,
#define MART_UTILS_IMPL_DEFINE_ENUM_MEMBER_STRING_VIEW( X ) std::string_view( #X ),
#define MART_UTILS_IMPL_DEFINE_ENUM_MEMBER_C_STRING( X ) #X,

// Actual macro we will use in code to define enums
#define MART_UTILS_DEFINE_ENUM( NAME, BASE_TYPE, MEMBER_CNT, ... )                                                     \
	enum class NAME : BASE_TYPE { __VA_ARGS__ };                                                                       \
                                                                                                                       \
	/* Array containing all enums */                                                                                   \
	[[maybe_unused]] constexpr std::array<NAME, MEMBER_CNT> NAME##_Array{                                              \
		{MART_UTILS_IMPL_FOR_EACH_P( MART_UTILS_IMPL_DEFINE_ENUM_MEMBER, NAME, __VA_ARGS__ )}};                        \
                                                                                                                       \
	/* Arrays containing all enum names as std::string c-string */                                                     \
	[[maybe_unused]] constexpr mart::EnumIdxArray<std::string_view, NAME, MEMBER_CNT> NAME##_StringViews{              \
		MART_UTILS_IMPL_FOR_EACH( MART_UTILS_IMPL_DEFINE_ENUM_MEMBER_STRING_VIEW, __VA_ARGS__ )};                      \
                                                                                                                       \
	[[maybe_unused]] constexpr mart::EnumIdxArray<const char*, NAME, MEMBER_CNT> NAME##_CStrings{                      \
		MART_UTILS_IMPL_FOR_EACH( MART_UTILS_IMPL_DEFINE_ENUM_MEMBER_C_STRING, __VA_ARGS__ )};                         \
                                                                                                                       \
	[[maybe_unused]] constexpr std::size_t      mart_enumCnt_impl( const NAME* ) { return MEMBER_CNT; }                \
	[[maybe_unused]] constexpr std::string_view mart_to_string_v_impl( NAME id ) { return NAME##_StringViews[id]; };   \
                                                                                                                       \
	[[maybe_unused]] constexpr const mart::EnumIdxArray<std::string_view, NAME, MEMBER_CNT>& mart_getEnumNames_impl(   \
		NAME* )                                                                                                        \
	{                                                                                                                  \
		return NAME##_StringViews;                                                                                     \
	}                                                                                                                  \
	[[maybe_unused]] constexpr const std::array<NAME, MEMBER_CNT>& mart_getEnums_impl( NAME* ) { return NAME##_Array; }
