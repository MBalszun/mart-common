#include <mart-common/experimental/lin_alg/matrix.h>

#include <catch2/catch.hpp>

TEST_CASE( "square_norm", "[lin_alg]" )
{
	constexpr mart::Matrix<double, 3> m{
		{ 0, 1, 2 }, //
		{ 3, 4, 5 }, //
		{ 6, 7, 8 }  //
	};
	constexpr double ref =      //
		0 * 0 + 1 * 1 + 2 * 2   //
		+ 3 * 3 + 4 * 4 + 5 * 5 //
		+ 6 * 6 + 7 * 7 + 8 * 8;

	constexpr auto norm = mart::linalg::detail::square_norm( m );
	CHECK( norm == ref );
	static_assert( norm == ref );
}

TEST_CASE( "matrix_exponential", "[lin_alg]" )
{
	constexpr mart::Matrix<double, 3> a{
		{ 0, 1, 0 },                 //
		{ 0, 0, 1 },                 //
		{ -6.0478, -5.2856, -0.238 } //
	};

	// calculated in matlab
	constexpr mart::Matrix<double, 3> ref{
		{ 0.3059, 0.1966, 0.2586 },   //
		{ -1.5640, -1.0610, 0.1351 }, //
		{ -0.8169, -2.2779, -1.0931 } //
	};

	constexpr auto acc   = 0.0001;
	const auto     ex1   = mart::linalg::calc_matrix_exp( a, acc );
	const auto     diff1 = mart::linalg::detail::square_norm( ex1 - ref );
	CHECK( diff1 < acc * acc );

	constexpr auto ex2   = mart::linalg::calc_matrix_exp( a, acc );
	constexpr auto diff2 = mart::linalg::detail::square_norm( ex2 - ref );
	static_assert( diff2 < acc * acc );
}

TEST_CASE( "matrix_inverse", "[lin_alg]" )
{
	// https: // www.tutorialspoint.com/cplusplus-program-to-find-inverse-of-a-graph-matrix
	constexpr auto M = mart::Matrix<double, 5>{ {
		//
		{ 1, 2, 3, 4, -2 },          //
		{ { -5, 6, 7, 8, 4 } },      //
		{ { 9, 10, -11, 12, 1 } },   //
		{ { 13, -14, -15, 0, 9 } },  //
		{ { 20, -26, 16, -17, 25 } } //
	} };

	constexpr auto expected_inv
		= mart::Matrix<double, 5>{ { { { 0.0811847, -0.0643008, 0.0493814, -0.0247026, 0.0237006 } },
									 { { -0.126819, -0.0161738, 0.0745377, -0.0713976, 0.0151639 } },
									 { { 0.0933664, 0.0028245, -0.0111876, -0.0220437, 0.0154006 } },
									 { { 0.143624, 0.0582573, -0.0282371, 0.0579023, -0.0175466 } },
									 { { -0.15893, 0.0724272, 0.0259728, -0.00100988, 0.0150219 } } } };


	constexpr auto inv = mart::linalg::calc_inv( M ).value();

	constexpr auto rel_error = mart::linalg::detail::square_norm( ( expected_inv - inv ) / expected_inv );

	CHECK( rel_error < 0.000001 );
	static_assert( rel_error < 0.000001 );
}