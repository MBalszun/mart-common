#pragma once

/**
 *  MartVec.h
 *
 *  @date: 2016.04.22
 *  @author: Michael Balszun <michael.balszun@mytum.de>
 *
 *  @brief Provides mathematical vector  with some default operations
 *
 *  The main goal of this file is to provide simple structures that
 *  can describe a 2D or 3D position, but it was generalized to
 *  N-Dimensional vectors.
 *  It also provides the common operator overloads, but doesn't aim
 *  at providing a full fledged linear algebra library with matrices
 *  and so on (if you need something like this look for one of the
 *  millions of libraries out there.
 *
 *
 *  TODO:
 *  	- Add functions for coordinate transformation
 *
 *
 */

#ifndef MARTVEC_H_
#define MARTVEC_H_

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <array>
#include <cassert>
#include <type_traits>
#include <functional>
#include <cmath>
#include <algorithm>

/* Proprietary Library Includes */

/* Project Includes */

namespace mart {
//### forward declarations ###

template <class T, int N>
struct Vec;

template <class T>
using Vec2D = Vec<T, 2>;

template <class T>
using Vec3D = Vec<T, 3>;

template <class T>
using Vec4D = Vec<T, 4>;

template <class T, int N>
using Matrix = Vec<Vec<T,N>,N>;


namespace mp{
	/*### index sequence (placeholder for c++14 index sequence) #### */

	template< int... I> struct index_sequence
	{
		typedef int value_type;
		static constexpr size_t size() noexcept { return sizeof...(I) ; }
	};

	namespace impl_helper {

		/*##### Helper index sequence (deprecated in c++14 ####*/
		template< int N, int... I >
		struct make_is
		{
		   using type = typename make_is< N-1, N-1,I...>::type;
		};


		template< int... I >
		struct make_is<0,I...>
		{
		   using type = index_sequence<I...>;
		};
	}

	template<int N >
	using make_index_sequence = typename impl_helper::make_is<N>::type;

}

template<class T, int N>
struct Vec {
	static_assert(N>0, "mart::Vector must at least have a size of 1");
	using value_type = T;
	std::array<T, N> data;

	//### ctor ###
	constexpr Vec() = default;

//c++14:
//	constexpr Vec(std::initializer_list<T> init){
//		std::copy_n(init.begin(),std::min(init.size(),data.size()),data.begin());
//	}

	//### Data access ###
	T &operator[](int idx)
	{
		assert(0 <= idx && idx < (int)N);
		return data[idx];
	}

	const T &operator[](int idx) const
	{
		assert(0 <= idx && idx < (int)N);
		return data[idx];
	}
	static constexpr int size() { return N; }


	T squareNorm() const {
		T sum{};
		for (int i=0;i<N;++i) {
			sum+=(*this)[i]*(*this)[i];
		}
		return sum;
	}
	T norm() const {
		return std::sqrt(squareNorm());
	}

	Vec<T,N> unityVec() const {
		Vec<T,N> res(*this);
		T abs = norm();
		for (size_t i=0;i<N;++i) {
			res[i]/= abs;
		}
		return res;
	}

	template<int K>
	Vec<T,K> toKDim() const {
		return toKDim_helper<K>(mp::make_index_sequence<(K < N? K: N)>{});
	}

private:
	template<int K, int ...I>
	Vec<T,K> toKDim_helper(mp::index_sequence<I...>) const {
		return Vec<T,K>{(*this)[I]...};
	}
};

/**
 * Left-Multiplies a (colum) vector with a square matrix.
 *
 * Main purpose is coordinate transformation. For that,
 * the colums of the matrix shall correspond to the
 * axis of the source coordinate system in the target
 * system.
 * @param vec
 * @param mx matrix to multiply vector with
 * @return
 */
template<class T,int N>
Vec<T,N> mx_multiply(const Matrix<T,N>& mx,const Vec<T,N>& vec) {
	Vec<T,N> ret{};

	for (int i=0;i<N;++i) {
		ret = ret+ mx[i]*vec[i];
	}
	return ret;
}

template<class T,int N>
T inner_product(const Vec<T,N>& l, const Vec<T,N>& r){
	return std::inner_product(l.data.begin(),l.data.end(),r.data.begin(),T{});
}

namespace _impl_vec {

	/**
	 * used in function template parameter lists.
	 *
	 * E.g. when two parameter shall be of same type, but only one gets deduced:
	 *
	 * template<class T>
	 * void foo(T l, Type<T> r){...}
	 *
	 * foo(0.0, 10u); -> T gets deduced to double
	 */
	template<class T>
	using Type=T;

	template<class T, class F, int ...I> constexpr auto apply_helper(const Vec<T,sizeof...(I)>& l, F func, mp::index_sequence<I...>) -> mart::Vec<decltype(func(l[0])	),sizeof...(I)>	{ return {func(l[I])...};	}

	template<class T, class F, int ...I> constexpr auto apply_helper(const Vec<T,sizeof...(I)>& l,	const Vec<T,sizeof...(I)>& r, 	F func, mp::index_sequence<I...>) -> mart::Vec<decltype(func(l[0],r[0])	),sizeof...(I)>	{ return {func(l[I],r[I])...};	}
	template<class T, class F, int ...I> constexpr auto apply_helper(Type<T> l, 					const Vec<T,sizeof...(I)>& r, 	F func, mp::index_sequence<I...>) -> mart::Vec<decltype(func(l,r[0])	),sizeof...(I)> { return {func(l,r[I])...};		}
	template<class T, class F, int ...I> constexpr auto apply_helper(const Vec<T,sizeof...(I)>& l,	Type<T> r, 						F func, mp::index_sequence<I...>) -> mart::Vec<decltype(func(l[0],r)	),sizeof...(I)>	{ return {func(l[I],r)...};		}

	//general dispatcher for applying functor that forwards the call to the respective apply_helper- functions
	//c++14: remove -> decltype(...)
	template<size_t N, class F, class ... ARGS>
	constexpr auto apply(F func, ARGS&& ... args) -> decltype(_impl_vec::apply_helper(std::forward<ARGS>(args)...,func,mp::make_index_sequence<N>{}))	{
		return _impl_vec::apply_helper(std::forward<ARGS>(args)...,func,mp::make_index_sequence<N>{});
	}

	template<class T>
	struct maximum {
		T operator()(const T& l, const T& r){
			return std::max(l,r);
		}
	};

	template<class T>
	struct minimum {
		T operator()(const T& l, const T& r){
			return std::min(l,r);
		}
	};

}

/**
 * Macro for defining all permutations for element wise vector operations:
 * OP(Vec, Vec)
 * OP(Vec, Scalar)
 * OP(Scalar, Vec)
 *
 * Macro parameters:
 * OP: name of the resulting function
 * FUNC: name of functor template that will be used to perform individual operations e.g. std::plus
 */
//c++14: remove "-> decltype(...)"
#define DEFINE_ND_VECTOR_OP(OP,FUNC) \
template<class T, int N> constexpr auto OP(const Vec<T,N>& l,	 const Vec<T,N>& r	 ) -> decltype(_impl_vec::apply<N>(FUNC<T>{},l,r)) { return _impl_vec::apply<N>(FUNC<T>{},l,r); } \
template<class T, int N> constexpr auto OP(_impl_vec::Type<T> l, const Vec<T,N>& r	 ) -> decltype(_impl_vec::apply<N>(FUNC<T>{},l,r)) { return _impl_vec::apply<N>(FUNC<T>{},l,r); }\
template<class T, int N> constexpr auto OP(const Vec<T,N>& l,	 _impl_vec::Type<T> r) -> decltype(_impl_vec::apply<N>(FUNC<T>{},l,r)) { return _impl_vec::apply<N>(FUNC<T>{},l,r); }

/**
 * same as DEFINE_ND_VECTOR_OP for unary operations on vec
 */
#define DEFINE_UNARY_ND_VECTOR_OP(OP,FUNC) \
template<class T, int N> constexpr auto OP(const Vec<T,N>& l) -> decltype(_impl_vec::apply<N>(FUNC<T>{},l)) { return _impl_vec::apply<N>(FUNC<T>{},l); }


//#### Define actual vector functions ######

//overloaded operators
//math operators
DEFINE_UNARY_ND_VECTOR_OP(operator-,std::negate)
DEFINE_ND_VECTOR_OP(operator+,std::plus)
DEFINE_ND_VECTOR_OP(operator*,std::multiplies)
DEFINE_ND_VECTOR_OP(operator-,std::minus)
DEFINE_ND_VECTOR_OP(operator/,std::divides)

//min max
DEFINE_ND_VECTOR_OP(max,_impl_vec::maximum)
DEFINE_ND_VECTOR_OP(min,_impl_vec::minimum)

//element wise logical ops
DEFINE_UNARY_ND_VECTOR_OP(operator!,std::logical_not)
DEFINE_ND_VECTOR_OP(elementAnd,std::logical_and)
DEFINE_ND_VECTOR_OP(elementOr,std::logical_or)

DEFINE_ND_VECTOR_OP(elementEquals,std::equal_to)
DEFINE_ND_VECTOR_OP(elementNE,std::not_equal_to)

DEFINE_ND_VECTOR_OP(elementLess,std::less)
DEFINE_ND_VECTOR_OP(elementLessEqual,std::less_equal)
DEFINE_ND_VECTOR_OP(elementGreater,std::greater)
DEFINE_ND_VECTOR_OP(elementGreaterEqual,std::greater_equal)

//comparison operators TODO: should those operator overloads be implemented or is this confusing?
//DEFINE_ND_VECTOR_OP(operator<,std::less)
//DEFINE_ND_VECTOR_OP(operator<=,std::less_equal)
//DEFINE_ND_VECTOR_OP(operator>,std::greater)
//DEFINE_ND_VECTOR_OP(operator>=,std::greater_equal)


#undef DEFINE_UNARY_ND_VECTOR_OP
#undef DEFINE_ND_VECTOR_OP

namespace _impl_vec {
	template<class T, size_t N, class F, size_t I=0>
	struct Fold {
		constexpr auto operator()(const Vec<T,N>& l, F op, T init =true) const ->decltype(op(init,init))  {
			return Fold<T,N,F,I+1>{}(l,op,init & l[I]);
		}
	};

	template<class T, size_t N, class F>
	struct Fold<T,N,F,N> {
		constexpr auto operator()(const Vec<T,N>& , F , T init) const -> T  {
			return init;
		}
	};
}

template<class T, int N>
constexpr bool operator==(const Vec<T,N> l, const Vec<T,N> r){
	return _impl_vec::Fold<T,N,std::logical_and<T>,0>{}(elementEquals(l,r),std::logical_and<T>{}) ;
}

template<class T, int N>
constexpr bool operator!=(const Vec<T,N> l, const Vec<T,N> r){
	return !(l==r) ;
}


//clang-format on

}

#endif /* MARTVEC_H_ */
