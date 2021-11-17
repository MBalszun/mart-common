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
#include <cassert>
#include <cmath>
#include <utility>

/* Proprietary Library Includes */
/* Project Includes */
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
//### forward declarations ###

template<class T, int N>
struct Vec;

template<class T>
using Vec2D = Vec<T, 2>;

template<class T>
using Vec3D = Vec<T, 3>;

template<class T>
using Vec4D = Vec<T, 4>;

template<class T, int N1, int N2 = N1>
using Matrix = Vec<Vec<T, N2>, N1>; // each vector is a row

namespace _impl_mart_vec {

template<int K, class T, int N, int... I>
[[nodiscard]] constexpr Vec<T, K> toKDim_helper( const Vec<T, N>& base, std::integer_sequence<int, I...> )
{
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif // ! _MSC_VER

	return Vec<T, K>{ ( base )[I]... };

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // ! _MSC_VER
}

template<class U, class T, int N, int... I>
[[nodiscard]] constexpr Vec<U, N> elements_cast_helper( const Vec<T, N>& base, std::integer_sequence<int, I...> )
{
	return { static_cast<U>( base[I] )... };
}

} // namespace _impl_mart_vec

/*################# Vec class implementation ######################*/
template<class T, int N>
struct Vec {
	static_assert( N > 0, "mart::Vector must at least have a size of 1" );
	static constexpr int Dim = N;
	using value_type         = T;

	T data[N];

	//### Data access ###
	[[nodiscard]] constexpr T& operator[]( int idx )
	{
		// assert(0 <= idx && idx < N);
		return data[idx];
	}

	[[nodiscard]] constexpr const T& operator[]( int idx ) const
	{
		// assert(0 <= idx && idx < N);
		return data[idx];
	}
	[[nodiscard]] static constexpr int size() { return N; }

	[[nodiscard]] constexpr auto squareNorm() const
	{
		decltype( std::declval<T>() * std::declval<T>() ) init{};
		for( const auto& e : data ) {
			init += e * e;
		}
		return init;
	}

	[[nodiscard]] auto norm() const
	{
		using std::sqrt;
		return sqrt( squareNorm() );
	}

	// Implementation NOTE: all defined at the end of the file
	constexpr Vec<T, N>& operator+=( const Vec<T, N>& other );
	constexpr Vec<T, N>& operator-=( const Vec<T, N>& other );
	constexpr Vec<T, N>& operator*=( const Vec<T, N>& other );
	constexpr Vec<T, N>& operator/=( const Vec<T, N>& other );

	// Creates a vector of length 1 that points in the same direction as the original one
	[[nodiscard]] Vec<T, N> unityVec() const
	{
		Vec<T, N> res( *this );
		auto      abs = norm();
		for( int i = 0; i < N; ++i ) {
			res[i] /= abs;
		}
		return res;
	}

	// returns a K dimensional vector
	// if K<=N, the first K values are copied
	// if K>N, all values are copied and the remaining values are zero-initialized
	template<int K>
	[[nodiscard]] constexpr Vec<T, K> toKDim() const
	{
		// C++20:
		// return [this]<int... I>( std::integer_sequence<int, I...> ) { return Vec<T, K>{( *this )[I]...}; }
		//( std::make_integer_sequence<int, ( K < N ? K : N )>{} );
		return _impl_mart_vec::toKDim_helper<K>( *this, std::make_integer_sequence<int, ( K < N ? K : N )>{} );
	}

	template<class U>
	[[nodiscard]] constexpr mart::Vec<U, N> elments_cast_to() const
	{
		return _impl_mart_vec::elements_cast_helper<U>( *this, std::make_integer_sequence<int, N>{} );
	}
};

template<class T>
struct Vec<T, 2> {
	static constexpr int N   = 2;
	static constexpr int Dim = 2;
	using value_type         = T;

	T x;
	T y;

	//### Data access ###
	[[nodiscard]] constexpr T& operator[]( int idx )
	{
		///		assert(0 <= idx && idx < N);
		return idx == 0 ? x : y;
	}

	[[nodiscard]] constexpr const T& operator[]( int idx ) const
	{
		//		assert(0 <= idx && idx < N);
		return idx == 0 ? x : y;
	}
	[[nodiscard]] static constexpr int size() { return N; }

	[[nodiscard]] constexpr auto squareNorm() const { return x * x + y * y; }

	[[nodiscard]] auto norm() const
	{
		using std::sqrt;
		return sqrt( squareNorm() );
	}

	constexpr Vec& operator+=( const Vec& other )
	{
		*this = *this + other;
		return *this;
	}
	constexpr Vec& operator-=( const Vec& other )
	{
		*this = *this - other;
		return *this;
	}
	constexpr Vec& operator*=( const Vec& other )
	{
		*this = *this * other;
		return *this;
	}
	constexpr Vec& operator/=( const Vec& other )
	{
		*this = *this / other;
		return *this;
	}

	// Creates a vector of length 1 that points in the same direction as the original one
	[[nodiscard]] Vec<T, 2> unityVec() const
	{
		auto abs = norm();
		return { T( x / abs ), T( y / abs ) };
	}

	// returns a K dimensional vector
	// if K<=N, the first K values are copied
	// if K>N, all values are copied and the remaining values are zero-initialized
	template<int K>
	[[nodiscard]] constexpr Vec<T, K> toKDim() const
	{
		// C++20:
		// return [this]<int... I>( std::integer_sequence<int, I...> ) { return Vec<T, K>{( *this )[I]...}; }
		//( std::make_integer_sequence<int, ( K < N ? K : N )>{} );

		return _impl_mart_vec::toKDim_helper<K>( *this, std::make_integer_sequence<int, ( K < N ? K : N )>{} );
	}

	template<class U>
	[[nodiscard]] constexpr mart::Vec<U, N> elments_cast_to() const
	{
		return _impl_mart_vec::elements_cast_helper<U>( *this, std::make_integer_sequence<int, N>{} );
	}
};

template<class T>
struct Vec<T, 3> {
	static constexpr int N   = 3;
	static constexpr int Dim = 3;
	using value_type         = T;

	T x;
	T y;
	T z;
	// c++14:
	//	constexpr Vec(std::initializer_list<T> init){
	//		std::copy_n(init.begin(),std::min(init.size(),data.size()),data.begin());
	//	}

	//### Data access ###
	[[nodiscard]] constexpr T& operator[]( int idx )
	{
		// assert(0 <= idx && idx < N);
		return idx == 0 ? x : idx == 1 ? y : z;
	}

	[[nodiscard]] constexpr const T& operator[]( int idx ) const
	{
		// assert(0 <= idx && idx < N);
		return idx == 0 ? x : idx == 1 ? y : z;
	}
	[[nodiscard]] static constexpr int size() { return N; }

	[[nodiscard]] constexpr auto squareNorm() const { return x * x + y * y + z * z; }

	[[nodiscard]] auto norm() const
	{
		using std::sqrt;
		return sqrt( squareNorm() );
	}

	constexpr Vec& operator+=( const Vec& other )
	{
		*this = *this + other;
		return *this;
	}
	constexpr Vec& operator-=( const Vec& other )
	{
		*this = *this - other;
		return *this;
	}
	constexpr Vec& operator*=( const Vec& other )
	{
		*this = *this * other;
		return *this;
	}
	constexpr Vec& operator/=( const Vec& other )
	{
		*this = *this / other;
		return *this;
	}

	// Creates a vector of length 1 that points in the same direction as the original one
	[[nodiscard]] Vec unityVec() const
	{
		auto abs = norm();
		return { T( x / abs ), T( y / abs ), T( z / abs ) };
	}

	// returns a K dimensional vector
	// if K<=N, the first K values are copied
	// if K>N, all values are copied and the remaining values are zero-initialized
	template<int K>
	[[nodiscard]] constexpr Vec<T, K> toKDim() const
	{
		return _impl_mart_vec::toKDim_helper<K>( *this, std::make_integer_sequence<int, ( K < N ? K : N )>{} );
	}

	template<class U>
	[[nodiscard]] constexpr mart::Vec<U, N> elments_cast_to() const
	{
		return _impl_mart_vec::elements_cast_helper<U>( *this, std::make_integer_sequence<int, N>{} );
	}
};

template<int K, class T>
[[nodiscard]] constexpr Vec<T, K> expand_to_dim( T v )
{
	Vec<T, K> vec;
	for( int i = 0; i < K; ++i ) {
		vec[i] = v;
	}
	return vec;
}

namespace _impl_mart_vec {
template<class T, int... I1, int... I2>
constexpr auto concat_impl( const Vec<T, sizeof...( I1 )>& v1,
							const Vec<T, sizeof...( I2 )>& v2,
							std::integer_sequence<int, I1...>,
							std::integer_sequence<int, I2...> ) //
	-> Vec<T, sizeof...( I1 ) + sizeof...( I2 )>
{
	return { v1[I1]..., v2[I2]... };
}
} // namespace _impl_mart_vec

template<class T, int N1, int N2>
[[nodiscard]] constexpr Vec<T, N1 + N2> concat( const Vec<T, N1>& v1, const Vec<T, N2>& v2 )
{
	return _impl_mart_vec::concat_impl(
		v1, v2, std::make_integer_sequence<int, N1>{}, std::make_integer_sequence<int, N2>{} );
}

template<class T, class U, int N>
[[nodiscard]] constexpr auto inner_product( const Vec<T, N>& l, const Vec<U, N>& r )
{
	decltype( l[0] * r[0] ) ret{};
	for( int i = 0; i < N; ++i ) {
		ret += l[i] * r[i];
	}
	return ret;
}

// assumes colum times row vetor
template<class T, class U, int N>
[[nodiscard]] constexpr auto outer_product( const Vec<T, N>& cv, const Vec<U, N>& rv )
{
	mart::Matrix<decltype( cv[0] * rv[0] ), N> ret{};
	for( int c = 0; c < N; ++c ) {
		for( int r = 0; r < N; ++r ) {
			ret[r][c] = cv[r] * rv[c];
		}
	}
	return ret;
}

template<class T, int N, int M>
[[nodiscard]] constexpr Matrix<T, M, N> transpose( const Matrix<T, N, M> m )
{
	Matrix<T, M, N> r{};
	for( int i = 0; i < N; ++i ) {
		for( int j = 0; j < M; ++j ) {
			r[i][j] = m[j][i];
		}
	}
	return r;
}

template<class T, int N>
[[nodiscard]] constexpr Matrix<T, 1, N> transpose( const mart::Vec<T, N> v )
{
	return Matrix<T, 1, N>{
		v //  the vector is now the first row in the matrix
	};
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
 *
 * NOTE:
 * This can either be matrix vector or Matrix Matrix,
 * as a matrix is just a vector of row vectors */
template<class T, class U, int N1, int N2>
[[nodiscard]] constexpr auto mx_multiply( const Matrix<T, N1, N2>& mx, const Vec<U, N2>& vec )
{
	Vec<decltype( mart::inner_product( mx[0], vec ) ), N1> ret{};

	for( int i = 0; i < N1; ++i ) {
		ret[i] = mart::inner_product( mx[i], vec );
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
using Type = T;

// applies unary function F to each element of l and stores the results in a new vector
template<class T, class F, int... I>
constexpr auto apply_unary_helper( const Vec<T, sizeof...( I )>& l, F func, std::integer_sequence<int, I...> s )
{
	using r_type = mart::Vec<decltype( func( l[0] ) ), s.size()>;
	return r_type{ func( l[I] )... };
}

template<class T, class U, class F, int... I>
constexpr auto apply_binary_helper( const Vec<T, sizeof...( I )>&    l,
									const Vec<U, sizeof...( I )>&    r,
									F                                func,
									std::integer_sequence<int, I...> s )
{
	using r_type = mart::Vec<decltype( func( l[0], r[0] ) ), s.size()>;
	return r_type{ func( l[I], r[I] )... };
}
template<class T, class U, class F, int... I>
constexpr auto
apply_binary_helper( const U& l, const Vec<T, sizeof...( I )>& r, F func, std::integer_sequence<int, I...> s )
{
	using r_type = mart::Vec<decltype( func( l, r[0] ) ), s.size()>;
	return r_type{ func( l, r[I] )... };
}
template<class T, class U, class F, int... I>
constexpr auto
apply_binary_helper( const Vec<T, sizeof...( I )>& l, const U& r, F func, std::integer_sequence<int, I...> s )
{
	using r_tye = mart::Vec<decltype( func( l[0], r ) ), s.size()>;
	return r_tye{ func( l[I], r )... };
}

// general dispatcher for applying functor that forwards the call to the respective apply_helper- functions
template<int N, class F, class T>
constexpr auto apply_unary( F func, T&& arg )
{
	return _impl_vec::apply_unary_helper( arg, func, std::make_integer_sequence<int, N>{} );
}

template<int N, class F, class T1, class T2>
constexpr auto apply_binary( F func, const T1& arg1, const T2& arg2 )
{
	return _impl_vec::apply_binary_helper( arg1, arg2, func, std::make_integer_sequence<int, N>{} );
}

// std::plus,td::multiplies,... - like function objects
struct maximum {
	template<class T>
	[[nodiscard]] constexpr T operator()( const T& l, const T& r ) const noexcept
	{
		return l > r ? l : r;
	}
};

struct minimum {
	template<class T>
	[[nodiscard]] constexpr T operator()( const T& l, const T& r ) const noexcept
	{
		return l < r ? l : r;
	}
};

struct ceil {
	template<class T>
	[[nodiscard]] constexpr T operator()( const T& l ) const noexcept
	{
		using std::ceil;
		return ceil( l );
	}
};

struct floor {
	template<class T>
	[[nodiscard]] constexpr T operator()( const T& l ) const noexcept
	{
		using std::floor;
		return floor( l );
	}
};

struct round {
	template<class T>
	[[nodiscard]] constexpr T operator()( const T& l ) const noexcept
	{
		using std::round;
		return round( l );
	}
};

struct lround {
	template<class T>
	[[nodiscard]] constexpr long operator()( const T& l ) const noexcept
	{
		using std::lround;
		return lround( l );
	}
};

struct iround {
	template<class T>
	[[nodiscard]] constexpr int operator()( const T& l ) const noexcept
	{
		using std::lround;
		return lround( l );
	}
};

struct multiplies {
	template<class T, class U>
	[[nodiscard]] constexpr auto operator()( const T& l, const U& r ) const noexcept
	{
		return l * r;
	}
};

struct divides {
	template<class T, class U>
	[[nodiscard]] constexpr auto operator()( const T& l, const U& r ) const noexcept
	{
		return l / r;
	}
};

struct plus {
	template<class T, class U>
	[[nodiscard]] constexpr auto operator()( const T& l, const U& r ) const noexcept
	{
		return l + r;
	}
};

struct minus {
	template<class T, class U>
	[[nodiscard]] constexpr auto operator()( const T& l, const U& r ) const noexcept
	{
		return l - r;
	}
};

struct negate {
	template<class T>
	[[nodiscard]] constexpr auto operator()( const T& l ) const noexcept
	{
		return -l;
	}
};

struct logical_not {
	template<class T>
	[[nodiscard]] constexpr auto operator()( const T& l ) const noexcept
	{
		return !l;
	}
};

struct logical_or {
	template<class T, class U>
	[[nodiscard]] constexpr auto operator()( const T& l, const U& r ) const noexcept
	{
		return l || r;
	}
};

struct abs {
	template<class T>
	[[nodiscard]] constexpr auto operator()( const T& l ) const noexcept
	{
		using std::abs;
		return abs( l );
	}
};

struct logical_and {
	template<class T, class U>
	[[nodiscard]] constexpr auto operator()( const T& l, const U& r ) const noexcept
	{
		return l && r;
	}
};

struct element_equal {
	template<class T, class U>
	constexpr auto impl( const T& l, const U& r, std::nullptr_t ) const noexcept -> decltype( elementEquals( l, r ) )
	{
		return elementEquals( l, r );
	}
	template<class T, class U>
	constexpr auto impl( const T& l, const U& r, const void* ) const noexcept -> decltype( l == r )
	{
		return l == r;
	}

	template<class T, class U>
	[[nodiscard]] constexpr auto operator()( const T& l, const U& r ) const noexcept
	{
		return this->impl( l, r, nullptr );
	}
};

struct element_not_equal {
	template<class T, class U>
	constexpr auto impl( const T& l, const U& r, std::nullptr_t ) const noexcept -> decltype( elementNE( l, r ) )
	{
		return elementNE( l, r );
	}
	template<class T, class U>
	constexpr auto impl( const T& l, const U& r, const void* ) const noexcept -> decltype( l != r )
	{
		return l != r;
	}

	template<class T, class U>
	[[nodiscard]] constexpr auto operator()( const T& l, const U& r ) const noexcept
	{
		return this->impl( l, r, nullptr );
	}
};

struct element_less {
	template<class T, class U>
	constexpr auto impl( const T& l, const U& r, std::nullptr_t ) const noexcept -> decltype( elementLess( l, r ) )
	{
		return elementLess( l, r );
	}
	template<class T, class U>
	constexpr auto impl( const T& l, const U& r, const void* ) const noexcept -> decltype( l < r )
	{
		return l < r;
	}

	template<class T, class U>
	[[nodiscard]] constexpr auto operator()( const T& l, const U& r ) const noexcept
	{
		return this->impl( l, r, nullptr );
	}
};

struct element_less_equal {
	template<class T, class U>
	constexpr auto impl( const T& l, const U& r, std::nullptr_t ) const noexcept -> decltype( elementLessEqual( l, r ) )
	{
		return elementLessEqual( l, r );
	}
	template<class T, class U>
	constexpr auto impl( const T& l, const U& r, const void* ) const noexcept -> decltype( l <= r )
	{
		return l <= r;
	}

	template<class T, class U>
	[[nodiscard]] constexpr auto operator()( const T& l, const U& r ) const noexcept
	{
		return this->impl( l, r, nullptr );
	}
};

struct element_greater {
	template<class T, class U>
	constexpr auto impl( const T& l, const U& r, std::nullptr_t ) const noexcept -> decltype( elementGreater( l, r ) )
	{
		return elementGreater( l, r );
	}
	template<class T, class U>
	constexpr auto impl( const T& l, const U& r, const void* ) const noexcept -> decltype( l > r )
	{
		return l > r;
	}

	template<class T, class U>
	[[nodiscard]] constexpr auto operator()( const T& l, const U& r ) const noexcept
	{
		return this->impl( l, r, nullptr );
	}
};

struct element_greater_equal {
	template<class T, class U>
	constexpr auto impl( const T& l, const U& r, std::nullptr_t ) const noexcept
		-> decltype( elementGreaterEqual( l, r ) )
	{
		return elementGreaterEqual( l, r );
	}
	template<class T, class U>
	constexpr auto impl( const T& l, const U& r, const void* ) const noexcept -> decltype( l >= r )
	{
		return l >= r;
	}

	template<class T, class U>
	[[nodiscard]] constexpr auto operator()( const T& l, const U& r ) const noexcept
	{
		return this->impl( l, r, nullptr );
	}
};

} // namespace _impl_vec

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
 * This macro is for operations where the scalar type on the left and right side can be different (e.g. multiplying
 * meters and seconds is fine) Also FUNC needs to be a full class name (not just a template)
 */
#define DEFINE_HETEROGEN_ND_VECTOR_OP( OP, FUNC )                                                                      \
	template<class T, class U, int N>                                                                                  \
	[[nodiscard]] constexpr auto OP( const Vec<T, N>& l, const Vec<U, N>& r )                                          \
	{                                                                                                                  \
		return _impl_vec::apply_binary<N>( FUNC{}, l, r );                                                             \
	}                                                                                                                  \
	template<class T, class U, int N>                                                                                  \
	[[nodiscard]] constexpr auto OP( const U& l, const Vec<T, N>& r )                                                  \
	{                                                                                                                  \
		return _impl_vec::apply_binary<N>( FUNC{}, l, r );                                                             \
	}                                                                                                                  \
	template<class T, class U, int N>                                                                                  \
	[[nodiscard]] constexpr auto OP( const Vec<T, N>& l, const U& r )                                                  \
	{                                                                                                                  \
		return _impl_vec::apply_binary<N>( FUNC{}, l, r );                                                             \
	}

/** Same as DEFINE_HETEROGEN_ND_VECTOR_OP, but scalar types of left and right side have to  be identical */
#define DEFINE_HOMOGEN_ND_VECTOR_OP( OP, FUNC )                                                                        \
	template<class T, int N>                                                                                           \
	[[nodiscard]] constexpr auto OP( const Vec<T, N>& l, const Vec<T, N>& r )                                          \
	{                                                                                                                  \
		return _impl_vec::apply_binary<N>( FUNC{}, l, r );                                                             \
	}                                                                                                                  \
	template<class T, int N>                                                                                           \
	[[nodiscard]] constexpr auto OP( const _impl_vec::Type<T>& l, const Vec<T, N>& r )                                 \
	{                                                                                                                  \
		return _impl_vec::apply_binary<N>( FUNC{}, l, r );                                                             \
	}                                                                                                                  \
	template<class T, int N>                                                                                           \
	[[nodiscard]] constexpr auto OP( const Vec<T, N>& l, const _impl_vec::Type<T>& r )                                 \
	{                                                                                                                  \
		return _impl_vec::apply_binary<N>( FUNC{}, l, r );                                                             \
	}

/**
 * same as DEFINE_ND_VECTOR_OP for unary operations on vec
 */
#define DEFINE_UNARY_ND_VECTOR_OP( OP, FUNC )                                                                          \
	template<class T, int N>                                                                                           \
	[[nodiscard]] constexpr auto OP( const Vec<T, N>& l )                                                              \
	{                                                                                                                  \
		return _impl_vec::apply_unary<N>( FUNC{}, l );                                                                 \
	}

//#### Define actual vector functions ######

// overloaded operators
// math operators
DEFINE_UNARY_ND_VECTOR_OP( operator-, _impl_vec::negate )
DEFINE_HETEROGEN_ND_VECTOR_OP( operator+, _impl_vec::plus )
DEFINE_HETEROGEN_ND_VECTOR_OP( operator*, _impl_vec::multiplies )
DEFINE_HETEROGEN_ND_VECTOR_OP( operator-, _impl_vec::minus )
DEFINE_HETEROGEN_ND_VECTOR_OP( operator/, _impl_vec::divides )

DEFINE_UNARY_ND_VECTOR_OP( ceil, _impl_vec::ceil )
DEFINE_UNARY_ND_VECTOR_OP( floor, _impl_vec::floor )
DEFINE_UNARY_ND_VECTOR_OP( round, _impl_vec::round )
DEFINE_UNARY_ND_VECTOR_OP( lround, _impl_vec::lround )
DEFINE_UNARY_ND_VECTOR_OP( iround, _impl_vec::iround )
DEFINE_UNARY_ND_VECTOR_OP( abs, _impl_vec::abs )

// min max
DEFINE_HOMOGEN_ND_VECTOR_OP( max, _impl_vec::maximum )
DEFINE_HOMOGEN_ND_VECTOR_OP( min, _impl_vec::minimum )

// element wise logical ops
DEFINE_UNARY_ND_VECTOR_OP( elementNot, _impl_vec::logical_not )
DEFINE_HETEROGEN_ND_VECTOR_OP( elementAnd, _impl_vec::logical_and )
DEFINE_HETEROGEN_ND_VECTOR_OP( elementOr, _impl_vec::logical_or )

// element wise comparison operations
DEFINE_HETEROGEN_ND_VECTOR_OP( elementEquals, _impl_vec::element_equal )
DEFINE_HETEROGEN_ND_VECTOR_OP( elementNE, _impl_vec::element_not_equal )

DEFINE_HETEROGEN_ND_VECTOR_OP( elementLess, _impl_vec::element_less )
DEFINE_HETEROGEN_ND_VECTOR_OP( elementLessEqual, _impl_vec::element_less_equal )
DEFINE_HETEROGEN_ND_VECTOR_OP( elementGreater, _impl_vec::element_greater )
DEFINE_HETEROGEN_ND_VECTOR_OP( elementGreaterEqual, _impl_vec::element_greater_equal )

#undef DEFINE_UNARY_ND_VECTOR_OP
#undef DEFINE_HETEROGEN_ND_VECTOR_OP
#undef DEFINE_HOMOGEN_ND_VECTOR_OP

template<class T, int N, class F, class Init_t>
[[nodiscard]] constexpr auto reduce( const Vec<T, N>& v, F f, Init_t init )
{
	for( int i = 0; i < N; ++i ) {
		init = f( v[i], init );
	}
	return init;
}

template<class T, int N>
[[nodiscard]] constexpr auto reduce( const Vec<T, N>& v )
{
	T init{};
	for( int i = 0; i < N; ++i ) {
		init += v[i];
	}
	return init;
}

template<class T, int N>
[[nodiscard]] constexpr bool operator==( const Vec<T, N>& l, const Vec<T, N>& r )
{
	bool ret = true;
	for( int i = 0; i < N; ++i ) {
		ret = ret & ( l[i] == r[i] );
	}
	return ret;
}

template<class T, int N>
[[nodiscard]] constexpr bool operator!=( const Vec<T, N> l, const Vec<T, N> r )
{
	return !( l == r );
}

template<class T, int N>
constexpr Vec<T, N>& Vec<T, N>::operator+=( const Vec<T, N>& other )
{
	*this = *this + other;
	return *this;
}

template<class T, int N>
constexpr Vec<T, N>& Vec<T, N>::operator-=( const Vec<T, N>& other )
{
	*this = *this - other;
	return *this;
};

template<class T, int N>
constexpr Vec<T, N>& Vec<T, N>::operator*=( const Vec<T, N>& other )
{
	*this = *this * other;
	return *this;
};

template<class T, int N>
constexpr Vec<T, N>& Vec<T, N>::operator/=( const Vec<T, N>& other )
{
	*this = *this / other;
	return *this;
};

// extract a matrix of size M_R x N_R starting at element m,n
template<int M_R, int N_R, class T, int M, int N>
[[nodiscard]] constexpr Matrix<T, M_R, N_R>
submatrix( const mart::Matrix<T, M, N>& mat, const int m = 0, const int n = 0 ) noexcept
{
	assert( M_R + m <= M );
	assert( N_R + n <= N );
	mart::Matrix<T, M_R, N_R> ret{};
	for( int i = 0; i < M_R; ++i ) {
		for( int j = 0; j < N_R; ++j ) {
			ret[i][j] = mat[i + m][j + n];
		}
	}
	return ret;
}

namespace mx {

template<class T, int M, int N1, int N2>
[[nodiscard]] constexpr Matrix<T, M, N1 + N2> horzcat( const Matrix<T, M, N1>& m1, const Matrix<T, M, N2>& m2 ) noexcept
{
	Matrix<T, M, N1 + N2> ret;
	for( int m = 0; m < M; ++m ) {
		for( int n = 0; n < N1; ++n ) {
			ret[m][n] = m1[m][n];
		}
	}
	for( int m = 0; m < M; ++m ) {
		for( int n = 0; n < N2; ++n ) {
			ret[m][n + N1] = m2[m][n];
		}
	}
	return ret;
}

template<class T, int M1, int M2, int N>
[[nodiscard]] constexpr Matrix<T, M1 + M2, N> vertcat( const Matrix<T, M1, N>& m1, const Matrix<T, M2, N>& m2 ) noexcept
{
	return mart::concat( m1, m2 );
}

template<int N, class T = double>
[[nodiscard]] constexpr Matrix<T, N> eye( T v = 1 )
{
	Matrix<T, N> r{};
	for( int i = 0; i < N; ++i ) {
		r[i][i] = v;
	}
	return r;
}

template<int M, int N, class T = double>
[[nodiscard]] constexpr Matrix<T, M, N> zeros()
{
	return {};
}

} // namespace mx

// backwards compat
using mart::mx::eye;

} // namespace mart

#endif
