#ifndef LIB_MART_COMMON_GUARD_ALGORITHM_H
#define LIB_MART_COMMON_GUARD_ALGORITHM_H
/**
 * algorithm.h (mart-common)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	provides wrappers around standard algorithms
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <algorithm>
#include <numeric>

/* Proprietary Library Includes */
#include "./cpp_std/execution.h"
#include "./cpp_std/type_traits.h"

/* Project Includes */
#include "./algorithms/find.h"
#include "./algorithms/mod-sequence-ops.h"
#include "./algorithms/non-mod-sequence-ops.h"
#include "./algorithms/numeric.h"
#include "./algorithms/set_ops.h"
#include "./algorithms/sorting.h"
#include "ranges.h"

/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

/*############## Wrapper around standard algorithms ################ */

template<class ExecutionPolicy, class RNG, class F>
void for_each( ExecutionPolicy&& p, RNG&& rng, F f )
{
#if MART_COMMON_STDLIB_HAS_PARALLEL_ALGORITHMS // defined in cpp_std/execution.h
	std::for_each( std::forward<ExecutionPolicy>( p ), rng.begin(), rng.end(), std::move( f ) );
#else
	std::for_each( rng.begin(), rng.end(), std::move( f ) );
	(void)p;
#endif
}

template<class C, class Comp, class = std::enable_if_t<!mart::is_execution_policy_v<std::decay_t<C>>>>
void sort( C& c, Comp comp )
{
	std::sort( c.begin(), c.end(), comp );
}

template<class ExecutionPolicy,
		 class C,
		 class = std::enable_if_t<mart::is_execution_policy_v<std::decay_t<ExecutionPolicy>>>>
void sort( ExecutionPolicy&& policy, C& c )
{
#if MART_COMMON_STDLIB_HAS_PARALLEL_ALGORITHMS
	std::sort( std::forward<ExecutionPolicy>( policy ), c.begin(), c.end() );
#else
	std::sort( c.begin(), c.end() );
	(void)policy;
#endif
}

template<class ExecutionPolicy, class C, class Comp>
void sort( ExecutionPolicy&& policy, C& c, Comp comp )
{
#if MART_COMMON_STDLIB_HAS_PARALLEL_ALGORITHMS
	std::sort( std::forward<ExecutionPolicy>( policy ), c.begin(), c.end(), comp );
#else
	std::sort( c.begin(), c.end(), comp );
	(void)policy;
#endif
}

template<class C, class Pred>
auto partition( C& c, Pred p ) -> decltype( c.begin() )
{
	return std::partition( c.begin(), c.end(), p );
}

template<class C>
auto unique( C& c ) -> decltype( c.begin() )
{
	return std::unique( c.begin(), c.end() );
}

template<class C, class Pred>
auto unique( C& c, Pred p ) -> decltype( c.begin() )
{
	return std::unique( c.begin(), c.end(), p );
}

template<class R1, class R2>
bool equal( const R1& r1, const R2& r2 )
{
	return std::equal( r1.begin(), r1.end(), r2.begin(), r2.end() );
}

template<class C, class UnaryPredicate>
auto find_last_if( C&& c, UnaryPredicate p ) -> decltype( c.begin() )
{
	auto last = c.end();
	for( auto it = mart::find_if( c, p ); it != c.end(); it = std::find_if( std::next( it ), c.end(), p ) ) {
		last = it;
	}
	return last;
}

template<class C, class V>
auto find_ex( C&& c, const V& value ) -> mart::EndAwareIterator<decltype( c.begin() )>
{
	return {std::find( c.begin(), c.end(), value ), c};
}

template<class C, class UnaryPredicate>
auto find_if_ex( C&& c, UnaryPredicate p ) -> mart::EndAwareIterator<decltype( c.begin() )>
{
	return {std::find_if( c.begin(), c.end(), p ), c};
}

template<class C, class UnaryPredicate>
auto wrapped_find_if( C&& c, decltype( c.cbegin() ) start, UnaryPredicate p ) -> decltype( c.begin() )
{
	auto it = std::find_if( start, c.end(), p );
	if( it != c.end() ) { return it; }
	it = std::find_if( c.begin(), start, p );
	if( it != start ) { return it; }
	return c.end();
}

template<class C, class UnaryPredicate>
auto wrapped_find_if_ex( C&& c, decltype( c.cbegin() ) start, UnaryPredicate p )
	-> mart::EndAwareIterator<decltype( c.begin() )>
{
	return {wrapped_find_if( c, start, p ), c};
}

template<class C1, class C2>
auto find_first_of_ex( C1&& in1, C2&& in2 ) -> mart::EndAwareIterator<decltype( std::begin( in1 ) )>
{
	return {std::find_first_of( in1.begin(), in1.end(), in2.begin(), in2.end() ), in1};
}

template<class C1, class C2, class BinaryPredicate>
auto find_first_of_ex( C1&& in1, C2&& in2, BinaryPredicate p ) -> mart::EndAwareIterator<decltype( std::begin( in1 ) )>
{
	return {std::find_first_of( in1.begin(), in1.end(), in2.begin(), in2.end(), p ), in1};
}

template<class C>
bool any_of( const C& c )
{
	return std::any_of( c.begin(), c.end(), []( bool e ) { return e; } );
}

// min/max

template<class R>
auto min_element( R&& range ) -> decltype( std::begin( range ) )
{
	using std::begin;
	using std::end;
	return std::min_element( begin( range ), end( range ) );
}

template<class R, class Compare>
auto min_element( R&& range, Compare comp )
	-> decltype( comp( *std::begin( range ), *std::begin( range ) ), std::begin( range ) )
{
	using std::begin;
	using std::end;
	return std::min_element( begin( range ), end( range ), comp );
}

template<class R, class Projection>
auto min_element( R&& range, const Projection pr ) -> decltype( pr( *std::begin( range ) ), std::begin( range ) )
{
	using std::begin;
	using std::end;
	if( begin( range ) == end( range ) ) { return end( range ); }
	auto       it_min = begin( range );
	const auto it_end = end( range );
	for( auto it = it_min; it != it_end; ++it ) {
		if( pr( *it ) < pr( *it_min ) ) { it_min = it; }
	}
	return it_min;
}

template<class R>
auto max_element( R&& range ) -> decltype( std::begin( range ) )
{
	using std::begin;
	using std::end;
	return std::max_element( begin( range ), end( range ) );
}

template<class R, class Compare>
auto max_element( R&& range, Compare comp )
	-> decltype( comp( *std::begin( range ), *std::begin( range ) ), std::begin( range ) )
{
	using std::begin;
	using std::end;
	return std::max_element( begin( range ), end( range ), comp );
}

template<class R, class Projection>
auto max_element( R&& range, Projection pr ) -> decltype( pr( *std::begin( range ) ), std::begin( range ) )
{
	using std::begin;
	using std::end;
	if( begin( range ) == end( range ) ) { return end( range ); }
	auto       it_max = begin( range );
	const auto it_end = end( range );
	for( auto it = it_max; it != it_end; ++it ) {
		if( pr( *it ) > pr( *it_max ) ) { it_max = it; }
	}
	return it_max;
}

template<class R>
auto minmax_element( R&& range ) -> std::pair<decltype( std::begin( range ) ), decltype( std::begin( range ) )>
{
	return std::minmax_element( range.begin(), range.end() );
}

template<class R, class Compare>
auto minmax_element( R&& range, Compare comp )
	-> std::pair<decltype( std::begin( range ) ), decltype( std::begin( range ) )>
{
	return std::minmax_element( range.begin(), range.end(), comp );
}

// transform
template<class Input, class Output, class UnaryOperation>
void transform( const Input& in, Output& out, UnaryOperation unary_op )
{
	std::transform( in.begin(), in.end(), out.begin(), unary_op );
}

/*### algorithm related ###*/

/*
* Creates function object that can be used by algorithms,
* when value should be compared to a member of the elements instead of the elements themselves
*
* Example :

struct Foo {
   int ID;
};

std::vector<Foo> ids{Foo{1},Foo{3},Foo{10}};

auto result = std::find_if(ids.begin(),ids.end(),byMember(&Foo::ID,3));

//result will now be iterator to second element
//equivalent long form:
auto result = std::find_if(ids.begin(),ids.end(),[](const Foo& foo)->bool{ return foo.*ID == 3; });

*
*/
namespace _impl_algo {

template<class MTYPE, class VAL>
struct EqualByMemberObjectHelper {
	static_assert( std::is_member_object_pointer<MTYPE>::value,
				   "First Template argument is not a member object pointer" );
	MTYPE      _mem;
	const VAL* _value;

	template<class T>
	bool operator()( const T& l ) const
	{
		return l.*_mem == *_value;
	}
};

template<class MTYPE>
struct LessByMemberObjectHelper {
	static_assert( std::is_member_object_pointer<MTYPE>::value,
				   "First Template argument is not a member object pointer" );
	MTYPE _mem;

	template<class T>
	bool operator()( const T& l, const T& r ) const
	{
		return l.*_mem < r.*_mem;
	}
};

template<class MTYPE, class VAL>
struct EqualByMemberFunctionHelper {
	static_assert( std::is_member_function_pointer<MTYPE>::value,
				   "First Template argument is not a member function pointer" );
	MTYPE      _mem;
	const VAL* _value;

	template<class T>
	bool operator()( const T& l ) const
	{
		return ( l.*_mem )() == *_value;
	}
};

template<class MTYPE>
struct LessByMemberFunctionHelper {
	static_assert( std::is_member_function_pointer<MTYPE>::value,
				   "First Template argument is not a member function pointer" );
	MTYPE _mem;

	template<class T>
	bool operator()( const T& l, const T& r ) const
	{
		return ( l.*_mem )() < ( r.*_mem )();
	}
};

} // namespace _impl_algo

template<class MTYPE, class VAL>
mart::enable_if_t<std::is_member_object_pointer<MTYPE>::value, _impl_algo::EqualByMemberObjectHelper<MTYPE, VAL>>
byMember( MTYPE member, const VAL& value )
{
	return {member, &value};
}

template<class MTYPE, class VAL>
mart::enable_if_t<std::is_member_function_pointer<MTYPE>::value, _impl_algo::EqualByMemberFunctionHelper<MTYPE, VAL>>
byMember( MTYPE member, const VAL& value )
{
	return {member, &value};
}

template<class MTYPE>
mart::enable_if_t<std::is_member_object_pointer<MTYPE>::value, _impl_algo::LessByMemberObjectHelper<MTYPE>>
byMember( MTYPE member )
{
	return {member};
}

template<class MTYPE>
mart::enable_if_t<std::is_member_function_pointer<MTYPE>::value, _impl_algo::LessByMemberFunctionHelper<MTYPE>>
byMember( MTYPE member )
{
	return {member};
}
} // namespace mart

#endif // !LIB_MART_COMMON_GUARD_ALGORITHM_H
