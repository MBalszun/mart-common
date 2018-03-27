#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_MART_VEC_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_MART_VEC_H
/**
 * MartVec.h (mart-common)
 *
 * Copyright (C) 2015-2018: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author:	Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Provides mathematical vector with some default operations
 *
 * The main goal of this file is to provide simple structures that
 * can describe a 2D or 3D position, but it was generalized to
 * N-Dimensional vectors.
 * It also provides the common operator overloads, but doesn't aim
 * at providing a full fledged linear algebra library with matrices
 * and so on (if you need something like this look for one of the
 * millions of libraries out there.
 *
 * TODO:
 * 	- Add functions for coordinate transformation
 *
 */

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
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

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
		/*                                           v--- we might need a cast here... */
		static constexpr int size() noexcept { return sizeof...(I) ; }
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
	static constexpr int Dim = N;
	using value_type = T;
	using square_type = decltype(std::declval<T>()*std::declval<T>());
	std::array<T, N> data;
//c++14:
//	constexpr Vec(std::initializer_list<T> init){
//		std::copy_n(init.begin(),std::min(init.size(),data.size()),data.begin());
//	}

	//### Data access ###
	constexpr T& operator[](int idx)
	{
		//assert(0 <= idx && idx < N);
		return data[idx];
	}

	constexpr const T& operator[](int idx) const
	{
		//assert(0 <= idx && idx < N);
		return data[idx];
	}
	static constexpr int size() { return N; }



	square_type squareNorm() const {
		return std::inner_product(data.begin(), data.end(), data.begin(), T{});
	}

	T norm() const {
		using std::sqrt;
		return sqrt(squareNorm());
	}

	Vec<T, N>& operator+=(const Vec<T, N>& other);
	Vec<T, N>& operator-=(const Vec<T, N>& other);
	Vec<T, N>& operator*=(const Vec<T, N>& other);
	Vec<T, N>& operator/=(const Vec<T, N>& other);


	//Creates a vector of length 1 that points in the same direction as the original one
	Vec<T,N> unityVec() const {
		Vec<T,N> res(*this);
		auto abs = norm();
		for (int i=0;i<N;++i) {
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
	static constexpr int N = 2;
	static constexpr int Dim = 2;
	using value_type = T;
	using square_type = decltype(std::declval<T>()*std::declval<T>());
	T x;
	T y;
	//c++14:
	//	constexpr Vec(std::initializer_list<T> init){
	//		std::copy_n(init.begin(),std::min(init.size(),data.size()),data.begin());
	//	}

	//### Data access ###
	constexpr T &operator[](int idx)
	{
///		assert(0 <= idx && idx < N);
		return idx == 0 ? x : y ;
	}

	constexpr const T &operator[](int idx) const
	{
//		assert(0 <= idx && idx < N);
		return idx == 0 ? x : y;
	}
	static constexpr int size() { return N; }

	constexpr square_type squareNorm() const {
		return x*x+y*y;
	}

	T norm() const {
		using std::sqrt;
		return sqrt(squareNorm());
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
	static constexpr int N = 3;
	static constexpr int Dim = 3;
	using value_type = T;
	//TODO: should not require std::declval<T>()*std::declval<T>() to be valid expression if not used
	using square_type = decltype(std::declval<T>()*std::declval<T>());
	T x;
	T y;
	T z;
	//c++14:
	//	constexpr Vec(std::initializer_list<T> init){
	//		std::copy_n(init.begin(),std::min(init.size(),data.size()),data.begin());
	//	}

	//### Data access ###
	constexpr T &operator[](int idx)
	{
		//assert(0 <= idx && idx < N);
		return idx == 0 ? x : idx == 1? y : z;
	}

	constexpr const T &operator[](int idx) const
	{
		//assert(0 <= idx && idx < N);
		return idx == 0 ? x : idx == 1 ? y : z;
	}
	static constexpr int size() { return N; }

	constexpr square_type squareNorm() const {
		return x*x + y*y + z*z;
	}

	T norm() const {
		using std::sqrt;
		return sqrt(squareNorm());
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

template<int K, class T>
Vec<T, K> expand_to_dim(T v) {
	Vec<T, K> vec;
	for (int i = 0; i < K; ++i) {
		vec[i] = v;
	}
	return vec;
}

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


template<class T, class U,int N>
auto inner_product(const Vec<T,N>& l, const Vec<U,N>& r) -> decltype(l[0]*r[0]) {
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
	template<class T,		class F, int ...I> constexpr auto apply_helper(const Vec<T,sizeof...(I)>& l, F func, mp::index_sequence<I...>) -> mart::Vec<decltype(func(l[0])	),sizeof...(I)>	{ return {func(l[I])...};	}

	template<class T, class U, class F, int ...I> constexpr auto apply_helper(const Vec<T,sizeof...(I)>& l,	const Vec<U,sizeof...(I)>& r, 	F func, mp::index_sequence<I...>) -> mart::Vec<decltype(func(l[0],r[0])	),sizeof...(I)>	{ return {func(l[I],r[I])...};	}
	template<class T, class U, class F, int ...I> constexpr auto apply_helper(const U& l, 					const Vec<T,sizeof...(I)>& r, 	F func, mp::index_sequence<I...>) -> mart::Vec<decltype(func(l,r[0])	),sizeof...(I)> { return {func(l,r[I])...};		}
	template<class T, class U, class F, int ...I> constexpr auto apply_helper(const Vec<T,sizeof...(I)>& l,	const U& r,						F func, mp::index_sequence<I...>) -> mart::Vec<decltype(func(l[0],r)	),sizeof...(I)>	{ return {func(l[I],r)...};		}

	//general dispatcher for applying functor that forwards the call to the respective apply_helper- functions
	//c++14: remove -> decltype(...)
	template<int N, class F, class ... ARGS>
	constexpr auto apply(F func, ARGS&& ... args) -> decltype(_impl_vec::apply_helper(std::forward<ARGS>(args)...,func,mp::make_index_sequence<N>{}))	{
		return _impl_vec::apply_helper(std::forward<ARGS>(args)...,func,mp::make_index_sequence<N>{});
	}

	//std::plus,td::multiplies,... - like function objects for maximum and minimum
	struct maximum {
		template<class T>
		constexpr T operator()(const T& l, const T& r) const {
			using std::max;
			return max(l,r);
		}
	};

	struct minimum {
		template<class T>
		constexpr T operator()(const T& l, const T& r) const {
			using std::min;
			return min(l,r);
		}
	};

	struct ceil {
		template<class T>
		T operator()(const T& l) const {
			using std::ceil;
			return ceil(l);
		}
	};

	struct floor {
		template<class T>
		T operator()(const T& l) const {
			using std::floor;
			return floor(l);
		}
	};

	struct round {
		template<class T>
		T operator()(const T& l) const {
			using std::round;
			return round(l);
		}
	};

	struct lround {
		template<class T>
		long operator()(const T& l) const {
			using std::lround;
			return lround(l);
		}
	};

	struct iround {
		template<class T>
		int operator()(const T& l) const {
			using std::lround;
			return lround(l);
		}
	};

	struct multiplies {
		template<class T, class U>
		constexpr auto operator()(const T& l, const U& r) const -> decltype(l*r) {
			return l*r;
		}
	};

	struct divides {
		template<class T, class U>
		constexpr auto operator()(const T& l, const U& r) const -> decltype(l/r) {
			return l/r;
		}
	};
	struct plus {
		template<class T, class U>
		constexpr auto operator()(const T& l, const U& r) const -> decltype(l+r) {
			return l + r;
		}
	};
	struct minus {
		template<class T, class U>
		constexpr auto operator()(const T& l, const U& r) const -> decltype(l-r) {
			return l - r;
		}
	};
	struct negate {
		template<class T>
		constexpr auto operator()(const T& l) const -> decltype(-l) {
			return -l;
		}
	};
	struct logical_not {
		template<class T>
		constexpr auto operator()(const T& l) const -> decltype(!l) {
			return !l;
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
 * FUNC: name of functor that will be used to perform individual operations e.g. std::plus
 *
 * This macro is for operations where the scalar type on the left and right side can be different (e.g. multiplying meters and seconds is fine)
 * Also FUNC needs to be a full class name (not just a template)
 */
//c++14: remove "-> decltype(...)"
#define DEFINE_HETEROGEN_ND_VECTOR_OP(OP,FUNC) \
template<class T, class U,int N> constexpr auto OP(const Vec<T,N>& l,	const Vec<U,N>& r ) -> decltype(_impl_vec::apply<N>(FUNC{},l,r)) { return _impl_vec::apply<N>(FUNC{},l,r); } \
template<class T, class U,int N> constexpr auto OP(const U& l,			const Vec<T,N>& r ) -> decltype(_impl_vec::apply<N>(FUNC{},l,r)) { return _impl_vec::apply<N>(FUNC{},l,r); }\
template<class T, class U,int N> constexpr auto OP(const Vec<T,N>& l,	const U& r		  ) -> decltype(_impl_vec::apply<N>(FUNC{},l,r)) { return _impl_vec::apply<N>(FUNC{},l,r); }


/** Same as DEFINE_HETEROGEN_ND_VECTOR_OP, but scalar types of left and right side have to  be identical */
//c++14: remove "-> decltype(...)"
#define DEFINE_HOMOGEN_ND_VECTOR_OP(OP,FUNC) \
template<class T, int N> constexpr auto OP(const Vec<T,N>& l,	 const Vec<T,N>& r	 ) -> decltype(_impl_vec::apply<N>(FUNC{},l,r)) { return _impl_vec::apply<N>(FUNC{},l,r); } \
template<class T, int N> constexpr auto OP(_impl_vec::Type<T>& l, const Vec<T,N>& r	 ) -> decltype(_impl_vec::apply<N>(FUNC{},l,r)) { return _impl_vec::apply<N>(FUNC{},l,r); }\
template<class T, int N> constexpr auto OP(const Vec<T,N>& l,	 _impl_vec::Type<T>& r) -> decltype(_impl_vec::apply<N>(FUNC{},l,r)) { return _impl_vec::apply<N>(FUNC{},l,r); }

/**
 * same as DEFINE_ND_VECTOR_OP for unary operations on vec
 */
#define DEFINE_UNARY_ND_VECTOR_OP(OP,FUNC) \
template<class T, int N> constexpr auto OP(const Vec<T,N>& l) -> decltype(_impl_vec::apply<N>(FUNC{},l)) { return _impl_vec::apply<N>(FUNC{},l); }

//#### Define actual vector functions ######

//overloaded operators
//math operators
DEFINE_UNARY_ND_VECTOR_OP(operator-, _impl_vec::negate)
DEFINE_HETEROGEN_ND_VECTOR_OP(operator+, _impl_vec::plus)
DEFINE_HETEROGEN_ND_VECTOR_OP(operator*, _impl_vec::multiplies)
DEFINE_HETEROGEN_ND_VECTOR_OP(operator-, _impl_vec::minus)
DEFINE_HETEROGEN_ND_VECTOR_OP(operator/, _impl_vec::divides)

DEFINE_UNARY_ND_VECTOR_OP(ceil, _impl_vec::ceil)
DEFINE_UNARY_ND_VECTOR_OP(floor, _impl_vec::floor)
DEFINE_UNARY_ND_VECTOR_OP(round, _impl_vec::round)
DEFINE_UNARY_ND_VECTOR_OP(lround, _impl_vec::lround)
DEFINE_UNARY_ND_VECTOR_OP(iround, _impl_vec::iround)

//min max
DEFINE_HOMOGEN_ND_VECTOR_OP(max,_impl_vec::maximum)
DEFINE_HOMOGEN_ND_VECTOR_OP(min,_impl_vec::minimum)

//element wise logical ops
DEFINE_UNARY_ND_VECTOR_OP(operator!, std::logical_not<void>)
DEFINE_HETEROGEN_ND_VECTOR_OP(elementAnd,std::logical_and<void>)
DEFINE_HETEROGEN_ND_VECTOR_OP(elementOr,std::logical_or<void>)

//element wise comparison operations
DEFINE_HETEROGEN_ND_VECTOR_OP(elementEquals,std::equal_to<void>)
DEFINE_HETEROGEN_ND_VECTOR_OP(elementNE,std::not_equal_to<void>)

DEFINE_HETEROGEN_ND_VECTOR_OP(elementLess,std::less<void>)
DEFINE_HETEROGEN_ND_VECTOR_OP(elementLessEqual,std::less_equal<void>)
DEFINE_HETEROGEN_ND_VECTOR_OP(elementGreater,std::greater<void>)
DEFINE_HETEROGEN_ND_VECTOR_OP(elementGreaterEqual,std::greater_equal<void>)

#undef DEFINE_UNARY_ND_VECTOR_OP
#undef DEFINE_HETEROGEN_ND_VECTOR_OP
#undef DEFINE_HOMOGEN_ND_VECTOR_OP


template<class T, int N, class F, class Init_t>
constexpr auto reduce(const Vec<T, N>& v, F f, Init_t init)
{
	for (int i = 0; i < N; ++i) {
		init = f(v[i], init);
	}
	return init;
}

template<class T, int N>
constexpr bool operator==(const Vec<T, N>& l, const Vec<T, N>& r) {
	//first compare the vectors element wise and then fold the results voer &&
	return reduce(elementEquals(l, r), std::logical_and<bool>{}, true);
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

#endif
