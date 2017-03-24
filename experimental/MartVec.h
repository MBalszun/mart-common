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
#include <numeric>

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
using Matrix = Vec<Vec<T,N>,N>; //each vector is a colum

//some metaprogramming stuff
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

/*################# Vec class implementation ######################*/
template<class T, int N>
struct Vec {
	static_assert(N>0, "mart::Vector must at least have a size of 1");
	static constexpr size_t Dim = N;
	using value_type = T;
	using square_type = decltype(std::declval<T>()*std::declval<T>());
	std::array<T, N> data;
//c++14:
//	constexpr Vec(std::initializer_list<T> init){
//		std::copy_n(init.begin(),std::min(init.size(),data.size()),data.begin());
//	}

	//### Data access ###
	T &operator[](int idx)
	{
		assert(0 <= idx && idx < N);
		return data[idx];
	}

	const T &operator[](int idx) const
	{
		assert(0 <= idx && idx < N);
		return data[idx];
	}
	static constexpr int size() { return N; }



	T squareNorm() const {
		return std::inner_product(data.begin(), data.end(), data.begin(), T{});
	}

	T norm() const {
		return std::sqrt(squareNorm());
	}

	Vec<T, N>& operator+=(const Vec<T, N>& other);
	Vec<T, N>& operator-=(const Vec<T, N>& other);
	Vec<T, N>& operator*=(const Vec<T, N>& other);
	Vec<T, N>& operator/=(const Vec<T, N>& other);


	//Creates a vector of length 1 that points in the same direction as the original one
	Vec<T,N> unityVec() const {
		Vec<T,N> res(*this);
		auto abs = norm();
		for (size_t i=0;i<N;++i) {
			res[i]/= abs;
		}
		return res;
	}

	//returns a K dimensional vector
	//if K<=N, the first K values are copied
	//if K>N, all values are copied and the remaining values are zero-initialized
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


template<class T>
struct Vec<T,2> {
	static constexpr size_t N = 2;
	static constexpr size_t Dim = 2;
	using value_type = T;
	using square_type = decltype(std::declval<T>()*std::declval<T>());
	T x;
	T y;
	//c++14:
	//	constexpr Vec(std::initializer_list<T> init){
	//		std::copy_n(init.begin(),std::min(init.size(),data.size()),data.begin());
	//	}

	//### Data access ###
	T &operator[](int idx)
	{
		assert(0 <= idx && idx < N);
		return idx == 0 ? x : y ;
	}

	const T &operator[](int idx) const
	{
		assert(0 <= idx && idx < N);
		return idx == 0 ? x : y;
	}
	static constexpr int size() { return N; }

	T squareNorm() const {
		return x*x+y*y;
	}

	T norm() const {
		return std::sqrt(squareNorm());
	}

	Vec<T, 2>& operator+=(const Vec<T, 2>& other);
	Vec<T, 2>& operator-=(const Vec<T, 2>& other);
	Vec<T, 2>& operator*=(const Vec<T, 2>& other);
	Vec<T, 2>& operator/=(const Vec<T, 2>& other);


	//Creates a vector of length 1 that points in the same direction as the original one
	Vec<T, 2> unityVec() const {
		auto abs = norm();
		return { x / abs, y / abs };
	}

	//returns a K dimensional vector
	//if K<=N, the first K values are copied
	//if K>N, all values are copied and the remaining values are zero-initialized
	template<int K>
	Vec<T, K> toKDim() const {
		return toKDim_helper<K>(mp::make_index_sequence<(K < N ? K : N)>{});
	}

private:
	template<int K, int ...I>
	Vec<T, K> toKDim_helper(mp::index_sequence<I...>) const {
		return Vec<T, K>{(*this)[I]...};
	}
};

template<class T>
struct Vec<T, 3> {
	static constexpr size_t N = 3;
	static constexpr size_t Dim = 3;
	using value_type = T;
	using square_type = decltype(std::declval<T>()*std::declval<T>());
	T x;
	T y;
	T z;
	//c++14:
	//	constexpr Vec(std::initializer_list<T> init){
	//		std::copy_n(init.begin(),std::min(init.size(),data.size()),data.begin());
	//	}

	//### Data access ###
	T &operator[](int idx)
	{
		assert(0 <= idx && idx < N);
		return idx == 0 ? x : idx == 1? y : z;
	}

	const T &operator[](int idx) const
	{
		assert(0 <= idx && idx < N);
		return idx == 0 ? x : idx == 1 ? y : z;
	}
	static constexpr int size() { return N; }

	T squareNorm() const {
		return x*x + y*y + z*z;
	}

	T norm() const {
		return std::sqrt(squareNorm());
	}

	Vec& operator+=(const Vec& other);
	Vec& operator-=(const Vec& other);
	Vec& operator*=(const Vec& other);
	Vec& operator/=(const Vec& other);


	//Creates a vector of length 1 that points in the same direction as the original one
	Vec unityVec() const {
		auto abs = norm();
		return { x / abs, y / abs, z/abs };
	}

	//returns a K dimensional vector
	//if K<=N, the first K values are copied
	//if K>N, all values are copied and the remaining values are zero-initialized
	template<int K>
	Vec<T, K> toKDim() const {
		return toKDim_helper<K>(mp::make_index_sequence<(K < N ? K : N)>{});
	}

private:
	template<int K, int ...I>
	Vec<T, K> toKDim_helper(mp::index_sequence<I...>) const {
		return Vec<T, K>{(*this)[I]...};
	}
};

//there should be a check for any instantiated vector type, but this has to do for now
static_assert(std::is_trivial < Vec<int, 5>>::value, "mart::Vec is not a trivial type");

namespace _impl_mart_vec {
	template<class T, int ...I1, int ...I2>
	Vec<T, sizeof...(I1)+sizeof...(I2)> concat_impl(const Vec<T,sizeof...(I1)>& v1, const Vec<T, sizeof...(I2)>& v2, mp::index_sequence<I1...>, mp::index_sequence<I2...>)
	{
		return { v1[I1]...,v2[I2]... };
	}
}


template<class T, int N1, int N2>
Vec<T, N1+N2> concat(const Vec<T, N1>& v1, const Vec<T,N2>& v2)
{
	return _impl_mart_vec::concat_impl(v1, v2, mp::make_index_sequence<N1>{}, mp::make_index_sequence<N2>{});
}

/**
 * Left-Multiplies a (colum) vector with a square matrix.
 * mx * vec
 *
 * Main purpose is coordinate transformation. For that,
 * the colums of the matrix shall correspond to the
 * axis of the source coordinate system represented in the target
 * system.
 * @param vec
 * @param mx matrix to multiply vector with
 * @return
 */
//template<class T,int N>
//Vec<T,N> mx_multiply(const Matrix<T,N>& mx,const Vec<T,N>& vec) {
//	Vec<T,N> ret{};
//
//	for (int i=0;i<N;++i) {
//		ret = ret+ mx[i]*vec[i];
//	}
//	return ret;
//}


template<class T,int N>
T inner_product(const Vec<T,N>& l, const Vec<T,N>& r){
	T ret{};
	for (int i = 0; i < N; ++i) {
		ret += l[i] * r[i];
	}
	return ret;
}


template<class T, int N>
Vec<T, N> mx_multiply(const Matrix<T, N>& mx, const Vec<T, N>& vec) {
	Vec<T, N> ret{};

	for (int i = 0; i<N; ++i) {
		ret[i] = inner_product(mx[i], vec);
	}
	return ret;
}

namespace _impl_vec {

	/**
	 * This type alias is used in function template parameter lists.
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

	//applies unary function F to each element of l and stores the results in a new vector
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

	//std::plus,td::multiplies,... - like function objects for maximum and minimum
	template<class T>
	struct maximum {
		T operator()(const T& l, const T& r){
			using std::max;
			return max(l,r);
		}
	};

	template<class T>
	struct minimum {
		T operator()(const T& l, const T& r){
			using std::min;
			return min(l,r);
		}
	};

	template<class T>
	struct ceil {
		T operator()(const T& l) {
			using std::ceil;
			return ceil(l);
		}
	};

	template<class T>
	struct floor {
		T operator()(const T& l) {
			using std::floor;
			return floor(l);
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

DEFINE_UNARY_ND_VECTOR_OP(ceil, _impl_vec::ceil)
DEFINE_UNARY_ND_VECTOR_OP(floor, _impl_vec::floor)

//min max
DEFINE_ND_VECTOR_OP(max,_impl_vec::maximum)
DEFINE_ND_VECTOR_OP(min,_impl_vec::minimum)

//element wise logical ops
DEFINE_UNARY_ND_VECTOR_OP(operator!,std::logical_not)
DEFINE_ND_VECTOR_OP(elementAnd,std::logical_and)
DEFINE_ND_VECTOR_OP(elementOr,std::logical_or)

//element wise comparison operations
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
		constexpr auto operator()(const Vec<T,N>& l, F op, T init) const ->decltype(op(init,init))  {
			return Fold<T,N,F,I+1>{}(l,op,op(init,  l[I]));
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
constexpr bool operator==(const Vec<T,N>& l, const Vec<T,N>& r){
	//first compare the vectors element wise and then fold the results voer &&
	return _impl_vec::Fold<bool,N,std::logical_and<T>,0>{}(elementEquals(l,r),std::logical_and<T>{},true) ;
}

template<class T, int N>
constexpr bool operator!=(const Vec<T,N> l, const Vec<T,N> r){
	return !(l==r) ;
}

template<class T, int N>
Vec<T, N>& Vec<T, N>::operator+=(const Vec<T, N>& other) { *this = *this + other; return *this; }

template<class T, int N>
Vec<T, N>& Vec<T, N>::operator-=(const Vec<T, N>& other) { *this = *this - other; return *this; };

template<class T, int N>
Vec<T, N>& Vec<T, N>::operator*=(const Vec<T, N>& other) { *this = *this * other; return *this; };

template<class T, int N>
Vec<T, N>& Vec<T, N>::operator/=(const Vec<T, N>& other) { *this = *this / other; return *this; };

}

#endif /* MARTVEC_H_ */
