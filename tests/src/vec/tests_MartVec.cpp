#include <mart-common/MartVec.h>

#include <catch2/catch.hpp>
#include <iostream>
#include <type_traits>

template<class T, int N>
std::ostream& operator<<( std::ostream& out, mart::Vec<T, N> vec )
{
	out << "[";
	for( int i = 0; i < N; ++i ) {
		out << vec[i] << ", ";
	}
	out << "]";
	return out;
}

namespace {
struct Wrapper {
	int			   v;
	friend Wrapper operator*( Wrapper l, int r ) { return {l.v * r}; }
	friend Wrapper operator*( int l, Wrapper r ) { return {l * r.v}; }
	friend Wrapper operator*( Wrapper l, Wrapper r ) { return {l.v * r.v}; }
	friend bool	operator==( Wrapper l, Wrapper r ) { return l.v == r.v; }
};
}

TEST_CASE( "MartVec_some_random_vector_math_code", "[vec]" )
{

	const mart::Vec3D<Wrapper> expected{{1}, {2}, {-2}};

	const mart::Vec3D<Wrapper> base{{1}, {1}, {2}};
	const auto				   r = base * mart::Vec3D<int>{1, 2, -1};
	static_assert( std::is_same_v<decltype( expected ), decltype( r )> );
	CHECK( expected == r );
}

namespace {

template<class T, int N>
mart::Vec<T, N> generateOnes()
{
	mart::Vec<T, N> v{};
	for( int i = 0; i < N; ++i ) {
		v[i] = 1;
	}
	return v;
}

template<class T, int N>
mart::Vec<T, N> generate1()
{
	mart::Vec<T, N> vec{};
	T				v{-N / 2};
	for( int i = 0; i < N; ++i ) {
		vec[i] = v;
		v	  = v + T{1};
	}
	return vec;
}

template<class T, int N>
mart::Vec<T, N> generate2()
{
	mart::Vec<T, N> vec{};
	T				v{0};
	for( int i = 0; i < N; ++i ) {
		vec[i] = v;
		v	  = v + T{1};
	}
	return vec;
}

template<class T, int N>
mart::Vec<T, N> generate3()
{
	mart::Vec<T, N> vec{};
	T				v{0};
	for( int i = 0; i < N; ++i ) {
		vec[i] = v;
		v	  = v * 2;
	}
	return vec;
}

template<class T, int N>
void check_addition_subtraction()
{
	auto v1 = generate1<T, N>();
	auto v2 = generate2<T, N>();
	auto v3 = v1 + v2;

	CHECK( v1 == v3 - v2 );
}

template<class T, int N>
void check_multiplication_division()
{
	auto v1 = generate1<T, N>();
	auto v2 = generate1<T, N>() * generate1<T, N>() + generateOnes<T, N>();
	auto v3 = v1 * v2;

	CHECK( v1 == v3 / v2 );
}
}

TEST_CASE( "MartVec_same_type_2d_operations", "[vec]" )
{
	check_addition_subtraction<int, 1>();
	check_addition_subtraction<int, 2>();
	check_addition_subtraction<int, 3>();
	check_addition_subtraction<int, 4>();
	check_addition_subtraction<int, 5>();
	check_addition_subtraction<int, 6>();
	check_addition_subtraction<int, 7>();
}

TEST_CASE( "MartVec_same_type_3d_operations", "[vec]" )
{
	check_multiplication_division<int, 1>();
	check_multiplication_division<int, 2>();
	check_multiplication_division<int, 3>();
	check_multiplication_division<int, 4>();
	check_multiplication_division<int, 5>();
	check_multiplication_division<int, 6>();
	check_multiplication_division<int, 7>();
}

TEST_CASE( "MartVec_element_comparison", "[vec]" )
{
	using Vec_t = mart::Vec<int, 5>;
	Vec_t vec1{1, 2, 3, 4, 5};
	Vec_t vec2{2, 2, 2, 2, 2};

	using BVec_t = mart::Vec<bool, 5>;

	CHECK( mart::elementLess( vec1, vec2 ) == BVec_t{true, false, false, false, false} );
	CHECK( mart::elementLessEqual( vec1, vec2 ) == BVec_t{true, true, false, false, false} );
	CHECK( mart::elementGreater( vec1, vec2 ) == BVec_t{false, false, true, true, true} );
	CHECK( mart::elementGreaterEqual( vec1, vec2 ) == BVec_t{false, true, true, true, true} );
	CHECK( mart::elementNE( vec1, vec2 ) == BVec_t{true, false, true, true, true} );
	CHECK( mart::elementEquals( vec1, vec2 ) == BVec_t{false, true, false, false, false} );
}

TEST_CASE( "MartVec_element_logic", "[vec]" )
{
	using BVec_t = mart::Vec<bool, 5>;
	BVec_t vec1{true, false, true, false, true};
	BVec_t vec2{true, true, false, false, false};

	CHECK( mart::elementAnd( vec1, vec2 ) == BVec_t{true, false, false, false, false} );
	CHECK( mart::elementOr( vec1, vec2 ) == BVec_t{true, true, true, false, true} );
}

TEST_CASE( "MartVec_element_min_max", "[vec]" )
{
	using Vec_t = mart::Vec<int, 5>;
	Vec_t vec1{1, 4, -2, 3, -10};
	Vec_t vec2{2, -4, -2, 3, 5};

	CHECK( mart::max( vec1, vec2 ) == Vec_t{2, 4, -2, 3, 5} );
	CHECK( mart::min( vec1, vec2 ) == Vec_t{1, -4, -2, 3, -10} );
}

TEST_CASE( "MartVec_decimal_to_integral", "[vec]" )
{
	using DVec_t = mart::Vec<double, 5>;
	using IVec_t = mart::Vec<int, 5>;
	DVec_t vec1{2.3, 2.7, 0.0, -2.3, -2.7};
	// BVec_t vec2{ true, true, false, false, false };

	CHECK( mart::ceil( vec1 ) == DVec_t{3, 3, 0, -2, -2} );
	CHECK( mart::floor( vec1 ) == DVec_t{2, 2, 0, -3, -3} );
	CHECK( mart::round( vec1 ) == DVec_t{2, 3, 0, -2, -3} );
	CHECK( mart::iround( vec1 ) == IVec_t{2, 3, 0, -2, -3} );
	CHECK( mart::lround( vec1 ) == mart::Vec<long, 5>{2, 3, 0, -2, -3} );

	// CHECK(mart::lround(vec1) == IVec_t{ 2, 3, 0, -2, -3 });
}
