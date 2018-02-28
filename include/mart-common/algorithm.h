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
#include "./cpp_std/type_traits.h"

 /* Project Includes */
#include "ranges.h"
 /* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {

/*############## Wrapper around standard algorithms ################ */

template <class C>
void sort( C& c )
{
	std::sort( c.begin(), c.end() );
}

template <class C, class Comp>
void sort(C& c, Comp comp)
{
	std::sort(c.begin(), c.end(),comp);
}

template <class C, class Pred>
auto partition(C& c, Pred p) -> decltype(c.begin())
{
	return std::partition(c.begin(), c.end(),p);
}


template <class C, class It, class Pred>
void nth_element(C& c, It nth_pos, Pred p)
{
	return std::nth_element(c.begin(),nth_pos,  c.end(), p);
}

template <class C, class It>
void nth_element(C& c, It nth_pos)
{
	return std::nth_element(c.begin(), nth_pos, c.end());
}

template <class C>
auto unique(C& c) -> decltype(c.begin())
{
	return std::unique(c.begin(), c.end());
}

template <class C, class Pred>
auto unique(C& c, Pred p) -> decltype(c.begin())
{
	return std::unique(c.begin(), c.end(), p);
}

template <class C, class T>
T accumulate(const C& c, T init)
{
	return std::accumulate(c.begin(), c.end(), init);
}

template <class C, class T, class BinaryOperation>
T accumulate(const C& c, T init, BinaryOperation op)
{
	return std::accumulate(c.begin(), c.end(), init, op);
}

//find
template <class C, class V>
auto find(const C& c, const V& value ) -> decltype( c.begin() )
{
	return std::find( c.begin(), c.end(), value );
}

template <class C, class UnaryPredicate>
auto find_if(const C& c, UnaryPredicate p ) -> decltype( c.begin() )
{
	return std::find_if( c.begin(), c.end(), p );
}

template <class C, class V>
auto find_ex(const C& c, const V& value) -> mart::EndAwareIterator<decltype(c.begin())>
{
	return{ std::find(c.begin(), c.end(), value), c };
}

template <class C, class UnaryPredicate>
auto find_if_ex(C& c, UnaryPredicate p) -> mart::EndAwareIterator<decltype(c.begin())>
{
	return{ std::find_if(c.begin(), c.end(), p), c };
}

template <class C, class UnaryPredicate>
auto wrapped_find_if(const C& c, decltype(c.cbegin()) start, UnaryPredicate p) -> decltype(c.begin())
{
	auto it = std::find_if(start, c.end(), p);
	if (it != c.end()) {
		return it;
	}
	it = std::find_if(c.begin(), start, p);
	if (it != start) {
		return it
	}
	return c.end();
}

template <class C, class UnaryPredicate>
auto wrapped_find_if_ex(const C& c, decltype(c.cbegin()) start, UnaryPredicate p) -> mart::EndAwareIterator<decltype(c.begin())>
{
	return{ wrapped_find_if(c,start,p), c };
}

template< class C1, class C2 >
auto find_first_of(const C1& in1, const C2& in2) -> decltype(std::begin(in1))
{
	return std::find_first_of(in1.begin(), in1.end(), in2.begin(), in2.end());
}

template< class C1, class C2, class BinaryPredicate >
auto find_first_of(const C1& in1, const C2& in2, BinaryPredicate p)-> decltype(std::begin(in1))
{
	return std::find_first_of(in1.begin(), in1.end(), in2.begin(), in2.end(), p);
}

template< class C1, class C2 >
auto find_first_of_ex(const C1& in1, const C2& in2) -> mart::EndAwareIterator<decltype(std::begin(in1))>
{
	return{ std::find_first_of(in1.begin(), in1.end(), in2.begin(), in2.end()), in1 };
}

template< class C1, class C2, class BinaryPredicate >
auto find_first_of_ex(const C1& in1, const C2& in2, BinaryPredicate p)-> mart::EndAwareIterator<decltype(std::begin(in1))>
{
	return{ std::find_first_of(in1.begin(), in1.end(), in2.begin(), in2.end(), p), in1 };
}

template< class C, class T>
auto lower_bound(const C& c, const T& value) -> decltype(std::begin(c))
{
	return std::lower_bound(std::begin(c), std::end(c), value);
}

template< class C, class T, class Compare>
auto lower_bound(const C& c, const T& value, Compare cmp) -> decltype(std::begin(c))
{
	return std::lower_bound(std::begin(c), std::end(c), value, cmp);
}

template< class C, class Pred>
bool all_of(const C& c, Pred p) {
	return std::all_of(c.begin(), c.end(), p);
}

template< class C, class Pred>
bool any_of(const C& c, Pred p) {
	return std::any_of(c.begin(), c.end(), p);
}

template< class C>
bool any_of(const C& c) {
	return std::any_of(c.begin(), c.end(), [](bool e) {return e; });
}

//min/max

template< class R>
auto min_element(R&& range) -> decltype(std::begin(range))
{
	using std::begin;
	using std::end;
	return std::min_element(begin(range), end(range));
}

template< class R, class Compare >
auto min_element(R&& range, Compare comp) -> decltype(comp(*std::begin(range), *std::begin(range)), std::begin(range))
{
	using std::begin;
	using std::end;
	return std::min_element(begin(range), end(range), comp);
}

template< class R, class Projection >
auto min_element(R&& range, const Projection pr) -> decltype(pr(*std::begin(range)), std::begin(range))
{
	using std::begin;
	using std::end;
	if (begin(range) == end(range)) {
		return end(range);
	}
	auto it_min = begin(range);
	const auto it_end = end(range);
	for (auto it = it_min; it != it_end; ++it) {
		if (pr(*it) < pr(*it_min)) {
			it_min = it;
		}
	}
	return it_min;
}

template< class R>
auto max_element(R&& range) -> decltype(std::begin(range))
{
	using std::begin;
	using std::end;
	return std::max_element(begin(range), end(range));
}

template< class R, class Compare >
auto max_element(R&& range, Compare comp) -> decltype(comp(*std::begin(range), *std::begin(range)),std::begin(range))
{
	using std::begin;
	using std::end;
	return std::max_element(begin(range), end(range), comp);
}

template< class R, class Projection >
auto max_element(R&& range, Projection pr) -> decltype(pr(*std::begin(range)), std::begin(range))
{
	using std::begin;
	using std::end;
	if (begin(range) == end(range)) {
		return end(range);
	}
	auto it_max = begin(range);
	const auto it_end = end(range);
	for (auto it = it_max; it != it_end; ++it) {
		if (pr(*it) > pr(*it_max)) {
			it_max = it;
		}
	}
	return it_max;
}


template< class R>
auto minmax_element(R&& range) -> std::pair<decltype(std::begin(range)), decltype(std::begin(range))>
{
	return std::minmax_element(range.begin(), range.end());
}

template< class R, class Compare >
auto minmax_element(R&& range, Compare comp) -> std::pair<decltype(std::begin(range)), decltype(std::begin(range))>
{
	return std::minmax_element(range.begin(), range.end(), comp);
}

//transform
template< class Input, class Output, class UnaryOperation >
void transform(const Input& in, Output& out, UnaryOperation unary_op) {
	std::transform(in.begin(), in.end(), out.begin(), unary_op);
}

template< class R, class T >
void fill(R& range, const T& value) {
	std::fill(range.begin(), range.end(), value);
}

// set algorithms

template< class InputC1, class InputC2, class OutputIt >
OutputIt set_difference(const InputC1& first1, const InputC2& first2, OutputIt d_first) {
	return set_difference(first1.begin(), first1.end(), first2.begin(), first2.end(), d_first);
}

template< class InputC1, class InputC2, class OutputIt >
OutputIt set_union(const InputC1& first1, const InputC2& first2, OutputIt d_first) {
	return set_union(first1.begin(), first1.end(), first2.begin(), first2.end(), d_first);
}


template <class C1, class DestIt>
auto copy(const C1& src,DestIt dest_it) -> DestIt
{
	return std::copy(src.begin(), src.end(), dest_it);
}

template <class C1, class DestIt>
auto move(C1&& src, DestIt dest_it) -> DestIt
{
	return std::move(src.begin(), src.end(), dest_it);
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

template <class MTYPE, class VAL>
struct EqualByMemberObjectHelper {
	static_assert( std::is_member_object_pointer<MTYPE>::value,
				   "First Template argument is not a member object pointer" );
	MTYPE	  _mem;
	const VAL* _value;

	template <class T>
	bool operator()( const T& l ) const
	{
		return l.*_mem == *_value;
	}
};

template <class MTYPE>
struct LessByMemberObjectHelper {
	static_assert(std::is_member_object_pointer<MTYPE>::value,
				  "First Template argument is not a member object pointer");
	MTYPE	  _mem;

	template <class T>
	bool operator()(const T& l, const T& r) const
	{
		return l.*_mem < r.*_mem;
	}
};

template <class MTYPE, class VAL>
struct EqualByMemberFunctionHelper {
	static_assert( std::is_member_function_pointer<MTYPE>::value,
				   "First Template argument is not a member function pointer" );
	MTYPE	  _mem;
	const VAL* _value;

	template <class T>
	bool operator()( const T& l ) const
	{
		return ( l.*_mem )() == *_value;
	}
};

template <class MTYPE>
struct LessByMemberFunctionHelper {
	static_assert(std::is_member_function_pointer<MTYPE>::value,
				  "First Template argument is not a member function pointer");
	MTYPE	  _mem;

	template <class T>
	bool operator()(const T& l, const T& r) const
	{
		return (l.*_mem)() < (r.*_mem)();
	}
};

} // namespace _impl_algo

template <class MTYPE, class VAL>
mart::enable_if_t<std::is_member_object_pointer<MTYPE>::value, _impl_algo::EqualByMemberObjectHelper<MTYPE, VAL>>
byMember( MTYPE member, const VAL& value )
{
	return {member, &value};
}

template <class MTYPE, class VAL>
mart::enable_if_t<std::is_member_function_pointer<MTYPE>::value, _impl_algo::EqualByMemberFunctionHelper<MTYPE, VAL>>
byMember( MTYPE member, const VAL& value )
{
	return {member, &value};
}

template <class MTYPE>
mart::enable_if_t<std::is_member_object_pointer<MTYPE>::value, _impl_algo::LessByMemberObjectHelper<MTYPE>>
byMember(MTYPE member)
{
	return{ member };
}

template <class MTYPE>
mart::enable_if_t<std::is_member_function_pointer<MTYPE>::value, _impl_algo::LessByMemberFunctionHelper<MTYPE>>
byMember(MTYPE member)
{
	return{ member };
}
}

#endif // !LIB_MART_COMMON_GUARD_ALGORITHM_H
