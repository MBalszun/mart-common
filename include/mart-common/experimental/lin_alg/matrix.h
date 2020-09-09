#ifndef LIB_MART_COMMON_GUARD_LIN_ALG_MATRIX_H
#define LIB_MART_COMMON_GUARD_LIN_ALG_MATRIX_H
/**
 * matrix.h (mart-common)
 *
 * Copyright (C) 2015-2018: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *

 *
 */

#include "../../MartVec.h"

#include <algorithm>
#include <numeric>
#include <optional>
#include <vector>
namespace mart {
namespace linalg {
namespace detail {

/* TODO check name
 * calculates sum over all matrix elements squared
 */
template<class T, int N>
constexpr auto square_norm( const mart::Matrix<T, N>& M )
{
	const auto t = mart::inner_product( M, M );
	return mart::reduce( t );
}
} // namespace detail

template<class T, int N>
constexpr mart::Matrix<T, N - 1> getCfactor( const mart::Matrix<T, N>& M, int p, int q )
{
	mart::Matrix<T, N - 1> t{};

	for( int r = 0; r < p; r++ ) {
		int j = 0;
		for( int c = 0; c < N; c++ ) {
			if( c != q ) { t[r][j++] = M[r][c]; }
		}
	}

	for( int r = p + 1; r < N; r++ ) {
		int j = 0;
		for( int c = 0; c < N; c++ ) {
			if( c != q ) { t[r - 1][j++] = M[r][c]; }
		}
	}
	return t;
}

template<class T, int N>
constexpr T calc_det( const mart::Matrix<T, N>& M ) // to find determinant
{
	if constexpr( N == 1 ) {
		return M[0][0];
	} else {

		T D = 0;

		int s = 1; // store sign multiplier

		// To Iterate each element of first row
		for( int f = 0; f < N; f++ ) {
			// For Getting Cofactor of M[0][f] do
			const mart::Matrix<T, N - 1> t = getCfactor( M, 0, f );
			D += s * M[0][f] * calc_det( t );
			s = -s;
		}
		return D;
	}
}

template<class T, int N>
constexpr mart::Matrix<T, N> calc_adj( const mart::Matrix<T, N>& M )
{
	if constexpr( N == 1 ) {
		return mart::Matrix<T, N>{ 1 };
	} else {
		mart::Matrix<T, N> adj{};
		for( int i = 0; i < N; i++ ) {
			for( int j = 0; j < N; j++ ) {
				// sign of adj[j][i] positive if sum of row and column indexes is even.
				const int s = 1 + -2 * ( ( i + j ) % 2 );
				// Interchange rows and columns to get the transpose of the cofactor matrix
				adj[j][i] = s * calc_det( getCfactor( M, i, j ) );
			}
		}
		return adj;
	}
}

template<class T1, int N, class T2 = T1>
constexpr std::optional<mart::Matrix<T2, N>> calc_inv( const mart::Matrix<T1, N>& M )
{
	T2 d = calc_det( M );
	if( d == 0 ) { return {}; }
	return mart::Matrix<T2, N>( calc_adj( M ) / d );
}

template<class T1, int N>
constexpr mart::Matrix<T1, N> calc_matrix_exp( const mart::Matrix<T1, N>& M, const double acc = 0.00001 )
{
	constexpr int batch_size = 10;

	mart::Matrix<T1, N> mtk = eye<N>();
	mart::Matrix<T1, N> ret = eye<N>();

	auto fac      = double{ 1.0 }; // faculty
	auto i        = int{ 0 };
	auto rel_diff = double{ 0.0 };
	do {
		auto local_accum = mart::Matrix<T1, N>{};
		for( int k = 1; k <= batch_size; ++k ) {
			fac = fac * ( k + i * batch_size );
			mtk = mart::mx_multiply( mtk, M );
			local_accum += mtk / fac;
		}
		++i;
		ret += local_accum;

		// Caclulate relative change in this iteration
		// TODO: properly guard against division by zero
		const auto rel_error = ( local_accum * ret ) / ( ret * ret + 0.00001 );
		rel_diff             = detail::square_norm( rel_error );

	} while( rel_diff > acc * acc && fac < std::numeric_limits<double>::infinity() );

	return ret;
}

} // namespace linalg

} // namespace mart

#endif