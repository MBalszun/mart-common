#include <mart-common/MartVec.h>

#include <catch2/catch.hpp>
#include <iostream>
#include <type_traits>

namespace {
// Useful for debugging
template<class T, int N>
[[maybe_unused]] std::ostream& operator<<( std::ostream& out, mart::Vec<T, N> vec )
{
	out << "[";
	for( int i = 0; i < N; ++i ) {
		out << vec[i] << ", ";
	}
	out << "]";
	return out;
}

struct Wrapper {
	int                                       v;
	[[maybe_unused]] friend constexpr Wrapper operator*( Wrapper l, int r ) { return { l.v * r }; }
	[[maybe_unused]] friend constexpr Wrapper operator*( int l, Wrapper r ) { return { l * r.v }; }
	[[maybe_unused]] friend constexpr Wrapper operator*( Wrapper l, Wrapper r ) { return { l.v * r.v }; }
	[[maybe_unused]] friend constexpr Wrapper operator/( Wrapper l, int r ) { return { l.v / r }; }
	[[maybe_unused]] friend constexpr Wrapper operator/( int l, Wrapper r ) { return { l / r.v }; }
	[[maybe_unused]] friend constexpr Wrapper operator/( Wrapper l, Wrapper r ) { return { l.v / r.v }; }
	friend constexpr bool                     operator==( Wrapper l, Wrapper r ) { return l.v == r.v; }
};

template<class T, int N>
constexpr mart::Vec<T, N> generate_sequence( T start = 0, T inc = 1 )
{
	mart::Vec<T, N> ret{};

	for( int i = 0; i < N; ++i ) {
		ret[i] = start;
		start += inc;
	}

	return ret;
}

template<class T, int N>
constexpr mart::Vec<T, N> generateOnes()
{
	mart::Vec<T, N> v{};
	for( int i = 0; i < N; ++i ) {
		v[i] = 1;
	}
	return v;
}

template<class T, int N>
constexpr mart::Vec<T, N> generate1()
{
	mart::Vec<T, N> vec{};

	T v{ -N / 2 };
	for( int i = 0; i < N; ++i ) {
		vec[i] = v;
		v      = v + T{ 1 };
	}
	return vec;
}

template<class T, int N>
constexpr mart::Vec<T, N> generate2()
{
	mart::Vec<T, N> vec{};
	T               v{ 0 };
	for( int i = 0; i < N; ++i ) {
		vec[i] = v;
		v      = v + T{ 1 };
	}
	return vec;
}

template<class T, int N>
constexpr mart::Vec<T, N> generate3()
{
	mart::Vec<T, N> vec{};
	T               v{ 1 };
	for( int i = 0; i < N; ++i ) {
		vec[i] = v;
		v      = v * 2;
	}
	return vec;
}

template<class T, int N>
constexpr bool check_addition_subtraction()
{
	constexpr auto v1 = generate1<T, N>();
	constexpr auto v2 = generate2<T, N>();
	constexpr auto v3 = v1 + v2;

	const bool check1 = v1 == v3 - v2;

	constexpr auto v4 = generate1<T, N>();
	constexpr auto v5 = generate1<T, N>();
	auto           vt = v4;
	vt += v5;

	const bool check2 = vt == v4 + v5;

	vt -= v5;
	const bool check3 = vt == v4;

	return check1 && check2 && check3;
}

template<class T, int N>
constexpr bool check_multiplication_division()
{
	constexpr auto v1     = generate1<T, N>();
	constexpr auto v2     = generate1<T, N>() * generate1<T, N>() + generateOnes<T, N>();
	constexpr auto v3     = v1 * v2;
	constexpr bool check1 = v1 == v3 / v2;

	constexpr auto v4 = generate1<T, N>();
	constexpr auto v5 = generate3<T, N>(); // all entries must be non-zero
	auto           vt = v4;
	vt *= v5;

	const bool check2 = vt == v4 * v5;

	vt /= v5;
	const bool check3 = vt == v4;

	return check1 && check2 && check3;
}

namespace same_type_math_ops {
static_assert( check_addition_subtraction<int, 1>() );
static_assert( check_addition_subtraction<int, 2>() );
static_assert( check_addition_subtraction<int, 3>() );
static_assert( check_addition_subtraction<int, 4>() );
static_assert( check_addition_subtraction<int, 5>() );
static_assert( check_addition_subtraction<int, 6>() );
static_assert( check_addition_subtraction<int, 7>() );

static_assert( check_multiplication_division<int, 1>() );
static_assert( check_multiplication_division<int, 2>() );
static_assert( check_multiplication_division<int, 3>() );
static_assert( check_multiplication_division<int, 4>() );
static_assert( check_multiplication_division<int, 5>() );
static_assert( check_multiplication_division<int, 6>() );
static_assert( check_multiplication_division<int, 7>() );
} // namespace same_type_math_ops

namespace element_wise_comparison_ops {

using Vec_t = mart::Vec<int, 5>;
constexpr Vec_t vec1{ 1, 2, 3, 4, 5 };
constexpr Vec_t vec2{ 2, 2, 2, 2, 2 };

using BVec_t = mart::Vec<bool, 5>;

static_assert( mart::elementLess( vec1, vec2 ) == BVec_t{ true, false, false, false, false } );
static_assert( mart::elementLessEqual( vec1, vec2 ) == BVec_t{ true, true, false, false, false } );
static_assert( mart::elementGreater( vec1, vec2 ) == BVec_t{ false, false, true, true, true } );
static_assert( mart::elementGreaterEqual( vec1, vec2 ) == BVec_t{ false, true, true, true, true } );
static_assert( mart::elementNE( vec1, vec2 ) == BVec_t{ true, false, true, true, true } );
static_assert( mart::elementEquals( vec1, vec2 ) == BVec_t{ false, true, false, false, false } );

} // namespace element_wise_comparison_ops

namespace element_Wise_logic_ops {

using BVec_t = mart::Vec<bool, 5>;
constexpr BVec_t vec1{ true, false, true, false, true };
constexpr BVec_t vec2{ true, true, false, false, false };

static_assert( mart::elementAnd( vec1, vec2 ) == BVec_t{ true, false, false, false, false } );
static_assert( mart::elementOr( vec1, vec2 ) == BVec_t{ true, true, true, false, true } );

} // namespace element_Wise_logic_ops

namespace element_wise_min_max {

using Vec_t = mart::Vec<int, 5>;
constexpr Vec_t vec1{ 1, 4, -2, 3, -10 };
constexpr Vec_t vec2{ 2, -4, -2, 3, 5 };

static_assert( mart::max( vec1, vec2 ) == Vec_t{ 2, 4, -2, 3, 5 } );
static_assert( mart::min( vec1, vec2 ) == Vec_t{ 1, -4, -2, 3, -10 } );

} // namespace element_wise_min_max

namespace equal_unequal {
using Vec_t = mart::Vec<int, 5>;

constexpr Vec_t v1{ 1, 2, 3, 4, -5 };
constexpr Vec_t v2{ 1, 2, 3, 4, -5 };
constexpr Vec_t v3{ 1, 2, 3, 4, 5 };

static_assert( v1 == v2 );
static_assert( v1 != v3 );

static_assert( v2 == v1 );
static_assert( v2 != v3 );

static_assert( v3 != v1 );
static_assert( v3 != v2 );
} // namespace equal_unequal

namespace random_math {

constexpr mart::Vec3D<Wrapper> base{ { 1 }, { 1 }, { 2 } };

constexpr auto r1 = base * mart::Vec3D<int>{ 1, 2, -1 };
static_assert( std::is_same_v<std::remove_cv_t<decltype( r1 )>, mart::Vec3D<decltype( Wrapper{} * int{} )>> );
static_assert( r1 == mart::Vec3D<Wrapper>{ { 1 }, { 2 }, { -2 } } );

constexpr auto r2 = base * base;
static_assert( std::is_same_v<mart::Vec3D<decltype( Wrapper{} * Wrapper{} )>, std::remove_cv_t<decltype( r2 )>> );
static_assert( r2 == mart::Vec3D<Wrapper>{ { 1 }, { 1 }, { 4 } } );

static_assert( 2 * base * 2 == mart::Vec3D<Wrapper>{ { 4 }, { 4 }, { 8 } } );
} // namespace random_math

namespace matrix_multiplication {
constexpr auto mx1 = mart::Matrix<int, 3>{              //
										   { 0, 1, 2 }, //
										   { 3, 4, 5 }, //
										   { 6, 7, 8 } };

constexpr auto mx2 = mart::Matrix<int, 3>{                 //
										   { 12, 23, 34 }, //
										   { 45, 56, 67 }, //
										   { 78, 89, 90 } };

constexpr auto res = mart::Matrix<int, 3>{                    //
										   { 201, 234, 247 }, //
										   { 606, 738, 820 }, //
										   { 1011, 1242, 1393 } };

static_assert( res == mx_multiply( mx1, mx2 ) );

static_assert( mart::Matrix<bool, 3>{ { true, true, true },  //
									  { true, true, true },  //
									  { true, true, true } } //
			   == mart::elementEquals( mx1, mx1 ) );

static_assert( mart::Matrix<bool, 3>{ { true, true, true },  //
									  { true, false, true }, //
									  { true, true, true } } //
			   != mart::elementEquals( mx1, mx1 ) );

static_assert( mart::Matrix<bool, 3>{ { false, false, false },  //
									  { false, false, false },  //
									  { false, false, false } } //
			   == mart::elementNE( mx1, mx1 ) );

} // namespace matrix_multiplication

template<class T, int N>
constexpr bool check_constexpr_operator_availability()
{
	static_assert( std::is_trivial_v<mart::Vec<T, N>> || !std::is_trivial_v<T>, "mart::Vec is not a trivial type" );

	constexpr auto                  v1  = generate1<T, N>();
	[[maybe_unused]] constexpr auto t1  = v1.squareNorm();
	[[maybe_unused]] constexpr auto t2  = v1.template toKDim<1>();
	[[maybe_unused]] constexpr auto t3  = v1.template toKDim<2>();
	[[maybe_unused]] constexpr auto t4  = v1.template toKDim<3>();
	[[maybe_unused]] constexpr auto t5  = v1.template toKDim<4>();
	[[maybe_unused]] constexpr auto t6  = v1.template toKDim<5>();
	[[maybe_unused]] constexpr auto t7  = v1.template toKDim<6>();
	[[maybe_unused]] constexpr auto t8  = v1.template toKDim<7>();
	[[maybe_unused]] constexpr auto t9  = v1.template toKDim<8>();
	[[maybe_unused]] constexpr auto t10 = v1.template toKDim<9>();
	[[maybe_unused]] constexpr auto t11 = v1.template toKDim<10>();
	[[maybe_unused]] constexpr auto t12 = v1.template toKDim<11>();
	[[maybe_unused]] constexpr auto t13 = v1.template toKDim<12>();
	return true;
}

static_assert( check_constexpr_operator_availability<int, 1>() );
static_assert( check_constexpr_operator_availability<int, 2>() );
static_assert( check_constexpr_operator_availability<int, 3>() );
static_assert( check_constexpr_operator_availability<int, 4>() );
static_assert( check_constexpr_operator_availability<int, 5>() );
static_assert( check_constexpr_operator_availability<int, 6>() );
static_assert( check_constexpr_operator_availability<int, 7>() );
static_assert( check_constexpr_operator_availability<int, 8>() );
static_assert( check_constexpr_operator_availability<int, 9>() );
static_assert( check_constexpr_operator_availability<int, 10>() );

static_assert( check_constexpr_operator_availability<int, 11>() );
static_assert( check_constexpr_operator_availability<int, 12>() );
static_assert( check_constexpr_operator_availability<int, 13>() );
static_assert( check_constexpr_operator_availability<int, 14>() );
static_assert( check_constexpr_operator_availability<int, 15>() );
static_assert( check_constexpr_operator_availability<int, 16>() );
static_assert( check_constexpr_operator_availability<int, 17>() );
static_assert( check_constexpr_operator_availability<int, 18>() );
static_assert( check_constexpr_operator_availability<int, 19>() );

static_assert( check_constexpr_operator_availability<double, 1>() );
static_assert( check_constexpr_operator_availability<double, 2>() );
static_assert( check_constexpr_operator_availability<double, 3>() );
static_assert( check_constexpr_operator_availability<double, 4>() );
static_assert( check_constexpr_operator_availability<double, 5>() );
static_assert( check_constexpr_operator_availability<double, 6>() );
static_assert( check_constexpr_operator_availability<double, 7>() );
static_assert( check_constexpr_operator_availability<double, 8>() );
static_assert( check_constexpr_operator_availability<double, 9>() );
static_assert( check_constexpr_operator_availability<double, 10>() );

static_assert( check_constexpr_operator_availability<double, 11>() );
static_assert( check_constexpr_operator_availability<double, 12>() );
static_assert( check_constexpr_operator_availability<double, 13>() );
static_assert( check_constexpr_operator_availability<double, 14>() );
static_assert( check_constexpr_operator_availability<double, 15>() );
static_assert( check_constexpr_operator_availability<double, 16>() );
static_assert( check_constexpr_operator_availability<double, 17>() );
static_assert( check_constexpr_operator_availability<double, 18>() );
static_assert( check_constexpr_operator_availability<double, 19>() );

constexpr bool check_diag()
{
	constexpr auto vec = mart::Vec<double, 5>{ 1, 2, 3, 4, 5 };
	constexpr auto mx  = mart::mx::diag( vec );
	static_assert( std::is_same_v<decltype( mx ), const mart::Matrix<double, 5, 5>>,
				   "mx::diag produces wrong matrix type" );

	for( int r = 0; r < 5; ++r ) {
		for( int c = 0; c < 5; ++c ) {
			if( c != r ) {
				if( mx[r][c] != 0.0 ) { return false; };
			} else {
				if( mx[r][c] != r + 1 ) { return false; };
			}
		}
	}
	return true;
}

static_assert( check_diag() );

template<int M, int N, class Tl, class Tr>
constexpr bool check_outer_multiplication_and_division()
{
	constexpr auto left  = generate_sequence<Tl, M>( 1, 1 );
	constexpr auto right = generate_sequence<Tr, N>( 10, 1 );
	constexpr auto mxp   = mart::outer_product( left, right );
	constexpr auto mxd = mart::outer_division( left, right );

	for( int r = 0; r < M; ++r ) {
		for( int c = 0; c < N; ++c ) {
			if( mxp[r][c] != ( r + 1 ) * ( c + 10 ) ) { return false; };
			if( mxd[r][c] != ( r + 1 ) / ( c + 10 ) ) { return false; };
		}
	}
	return true;
}

template<int M, int N, class Tl, class Tr>
constexpr bool check_outer_multiplication()
{
	constexpr auto left  = generate_sequence<Tl, M>( 1, 1 );
	constexpr auto right = generate_sequence<Tr, N>( 10, 1 );
	constexpr auto mx    = mart::outer_product( left, right );

	for( int r = 0; r < M; ++r ) {
		for( int c = 0; c < N; ++c ) {
			if( mx[r][c] != ( r + 1 ) * ( c + 10 ) ) { return false; };
		}
	}

	return true;
}

static_assert( check_outer_multiplication<1, 1, int, double>() );
static_assert( check_outer_multiplication<2, 1, int, double>() );
static_assert( check_outer_multiplication<3, 1, int, double>() );
static_assert( check_outer_multiplication<4, 1, int, double>() );
static_assert( check_outer_multiplication<5, 1, int, double>() );
static_assert( check_outer_multiplication<10, 1, int, double>() );
static_assert( check_outer_multiplication<20, 1, int, double>() );

static_assert( check_outer_multiplication<1, 1, double, double>() );
static_assert( check_outer_multiplication<1, 2, double, double>() );
static_assert( check_outer_multiplication<1, 3, double, double>() );
static_assert( check_outer_multiplication<1, 4, double, double>() );
static_assert( check_outer_multiplication<1, 5, double, double>() );
static_assert( check_outer_multiplication<1, 10, double, double>() );
static_assert( check_outer_multiplication<1, 20, double, double>() );

static_assert( check_outer_multiplication<1, 10, double, double>() );
static_assert( check_outer_multiplication<2, 10, double, double>() );
static_assert( check_outer_multiplication<3, 10, double, double>() );
static_assert( check_outer_multiplication<4, 10, double, double>() );
static_assert( check_outer_multiplication<5, 10, double, double>() );
static_assert( check_outer_multiplication<10, 10, double, double>() );
static_assert( check_outer_multiplication<20, 10, double, double>() );

static_assert( check_outer_multiplication<1, 20, double, double>() );
static_assert( check_outer_multiplication<2, 20, double, double>() );
static_assert( check_outer_multiplication<3, 20, double, double>() );
static_assert( check_outer_multiplication<4, 20, double, double>() );
static_assert( check_outer_multiplication<5, 20, double, double>() );
static_assert( check_outer_multiplication<10, 20, double, double>() );
static_assert( check_outer_multiplication<20, 20, double, double>() );

static_assert( check_outer_multiplication<10, 1, double, double>() );
static_assert( check_outer_multiplication<10, 2, double, double>() );
static_assert( check_outer_multiplication<10, 3, double, double>() );
static_assert( check_outer_multiplication<10, 4, double, double>() );
static_assert( check_outer_multiplication<10, 5, double, double>() );
static_assert( check_outer_multiplication<10, 10, double, double>() );
static_assert( check_outer_multiplication<10, 20, double, double>() );

static_assert( check_outer_multiplication<20, 1, double, double>() );
static_assert( check_outer_multiplication<20, 2, double, double>() );
static_assert( check_outer_multiplication<20, 3, double, double>() );
static_assert( check_outer_multiplication<20, 4, double, double>() );
static_assert( check_outer_multiplication<20, 5, double, double>() );
static_assert( check_outer_multiplication<20, 10, double, double>() );
static_assert( check_outer_multiplication<20, 20, double, double>() );

template<class T, int N>
void check_runtime_operator_availability()
{
	if constexpr( !std::is_same_v<T, int> ) {
		constexpr auto v1 = generate1<T, N>();
		(void)v1.norm();
		(void)v1.unityVec();
	}
}

} // namespace

TEST_CASE( "MartVec_operator_availability_int", "[vec]" )
{
	check_runtime_operator_availability<int, 1>();
	check_runtime_operator_availability<int, 2>();
	check_runtime_operator_availability<int, 3>();
	check_runtime_operator_availability<int, 4>();
	check_runtime_operator_availability<int, 5>();
	check_runtime_operator_availability<int, 6>();
	check_runtime_operator_availability<int, 7>();
	check_runtime_operator_availability<int, 8>();
	check_runtime_operator_availability<int, 9>();
	check_runtime_operator_availability<int, 10>();

	check_runtime_operator_availability<int, 11>();
	check_runtime_operator_availability<int, 12>();
	check_runtime_operator_availability<int, 13>();
	check_runtime_operator_availability<int, 14>();
	check_runtime_operator_availability<int, 15>();
	check_runtime_operator_availability<int, 16>();
	check_runtime_operator_availability<int, 17>();
	check_runtime_operator_availability<int, 18>();
	check_runtime_operator_availability<int, 19>();
}

TEST_CASE( "MartVec_operator_availability_double", "[vec]" )
{
	check_runtime_operator_availability<double, 1>();
	check_runtime_operator_availability<double, 2>();
	check_runtime_operator_availability<double, 3>();
	check_runtime_operator_availability<double, 4>();
	check_runtime_operator_availability<double, 5>();
	check_runtime_operator_availability<double, 6>();
	check_runtime_operator_availability<double, 7>();
	check_runtime_operator_availability<double, 8>();
	check_runtime_operator_availability<double, 9>();
	check_runtime_operator_availability<double, 10>();

	check_runtime_operator_availability<double, 11>();
	check_runtime_operator_availability<double, 12>();
	check_runtime_operator_availability<double, 13>();
	check_runtime_operator_availability<double, 14>();
	check_runtime_operator_availability<double, 15>();
	check_runtime_operator_availability<double, 16>();
	check_runtime_operator_availability<double, 17>();
	check_runtime_operator_availability<double, 18>();
	check_runtime_operator_availability<double, 19>();
}

TEST_CASE( "MartVec_decimal_to_integral", "[vec]" )
{
	using DVec_t = mart::Vec<double, 5>;
	using IVec_t = mart::Vec<int, 5>;
	DVec_t vec1{ 2.3, 2.7, 0.0, -2.3, -2.7 };
	// BVec_t vec2{ true, true, false, false, false };

	CHECK( mart::ceil( vec1 ) == DVec_t{ 3, 3, 0, -2, -2 } );
	CHECK( mart::floor( vec1 ) == DVec_t{ 2, 2, 0, -3, -3 } );
	CHECK( mart::round( vec1 ) == DVec_t{ 2, 3, 0, -2, -3 } );
	CHECK( mart::iround( vec1 ) == IVec_t{ 2, 3, 0, -2, -3 } );
	CHECK( mart::lround( vec1 ) == mart::Vec<long, 5>{ 2, 3, 0, -2, -3 } );

	// CHECK(mart::lround(vec1) == IVec_t{ 2, 3, 0, -2, -3 });
}
