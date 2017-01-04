#pragma once
#include <algorithm>

#include "./cpp_std/type_traits.h"
#include "ranges.h"

namespace mart {

/*############## Wrapper around standard algorithms ################ */

template <class C>
void sort( C& c )
{
	std::sort( c.begin(), c.end() );
}

template <class C, class T>
void accumulate(C& c, T init)
{
	std::accumulate(c.begin(), c.end(), init);
}

template <class C, class T, class BinaryOperation>
T accumulate(C& c, T init, BinaryOperation op)
{
	return std::accumulate(c.begin(), c.end(), init, op);
}

template <class C, class V>
auto find( C& c, const V& value ) -> decltype( c.begin() )
{
	return std::find( c.begin(), c.end(), value );
}

template <class C, class UnaryPredicate>
auto find_if( C& c, UnaryPredicate p ) -> decltype( c.begin() )
{
	return std::find_if( c.begin(), c.end(), p );
}

template <class C, class V>
auto find_ex(C& c, const V& value) -> mart::EndAwareIterator<decltype(c.begin())>
{
	return{ std::find(c.begin(), c.end(), value), c };
}

template <class C, class UnaryPredicate>
auto find_if_ex(C& c, UnaryPredicate p) -> mart::EndAwareIterator<decltype(c.begin())>
{
	return{ std::find_if(c.begin(), c.end(), p), c };
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
struct ByMemberObjectHelper {
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

template <class MTYPE, class VAL>
struct ByMemberFunctionHelper {
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

} // namespace _impl_algo

template <class MTYPE, class VAL>
mart::enable_if_t<std::is_member_object_pointer<MTYPE>::value, _impl_algo::ByMemberObjectHelper<MTYPE, VAL>>
byMember( MTYPE member, const VAL& value )
{
	return {member, &value};
}

template <class MTYPE, class VAL>
mart::enable_if_t<std::is_member_function_pointer<MTYPE>::value, _impl_algo::ByMemberFunctionHelper<MTYPE, VAL>>
byMember( MTYPE member, const VAL& value )
{
	return {member, &value};
}
}
