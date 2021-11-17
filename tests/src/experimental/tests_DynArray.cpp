#include <mart-common/experimental/DynArray.h>

#include <catch2/catch.hpp>

#include <span>

TEST_CASE( "experimental_DynArrayTrivial_has_correct_size", "[experimental][dynarray]" )
{
	mart::DynArrayTriv<int> arr1;
	CHECK( arr1.size() == 0 );

	mart::DynArrayTriv<int> arr2( 5 );
	CHECK( arr2.size() == 5 );

	mart::DynArrayTriv<int> arr3{ 5 };
	CHECK( arr3.size() == 1 );

	int carr[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

#if __cpp_lib_span
	mart::DynArrayTriv<int> arr4{ std::span<int>( carr ) };
	CHECK( arr4.size() == 10 );
	for( std::size_t i = 0; i < 10; ++i ) {
		CHECK( arr4[i] == i );
	}
#endif

	mart::DynArrayTriv<int> arr5( std::begin( carr ), std::end( carr ) );
	CHECK( arr5.size() == 10 );
	for( std::size_t i = 0; i < 10; ++i ) {
		CHECK( arr5[i] == i );
	}
}

TEST_CASE( "experimental_DynArrayTrivial_copy", "[experimental][dynarray]" )
{
	mart::DynArrayTriv<int> arr1( 5 );
	CHECK( arr1.size() == 5 );

	std::iota( arr1.begin(), arr1.end(), 0 );

	for( std::size_t i = 0; i < 5; ++i ) {
		CHECK( arr1[i] == i );
	}

	mart::DynArrayTriv<int> arr2 = arr1;

	for( std::size_t i = 0; i < 5; ++i ) {
		CHECK( arr2[i] == i );
	}
}

TEST_CASE( "experimental_DynArrayTrivial_append", "[experimental][dynarray]" )
{
	mart::DynArrayTriv<int> arr1{ 1, 2, 3, 4, 5, 6 };

	auto arr2 = arr1.append( 7 );
	CHECK( arr2.back() == 7 );
}

 namespace {

template<class T, std::size_t S = 10>
struct AllocT {
	using value_type =T;
	static T* allocate( std::size_t n )
	{
		auto cnt = S > n ? S : n;
		return new T[cnt];
	}

	static void deallocate( T* t, std::size_t ) { delete t; }

	static T* reallocate( T* t, std::size_t n )
	{
		if( n <= S ) {
			return t;
		} else {
			return nullptr;
		}
	}
};

using Alloc =  AllocT<int,100>;

#if __cpp_concepts
static_assert( mart::detail::AllocWithRealloc<Alloc> );
#endif

} // namespace

TEST_CASE( "experimental_DynArrayTrivial_append_move", "[experimental][dynarray]" )
{


	mart::DynArrayTriv<int, Alloc> arr2;
	{

		mart::DynArrayTriv<int, Alloc> arr1{ 1, 2, 3, 4, 5, 6 };

		arr2 = std::move( arr1 ).append( 7 );
		CHECK( arr1.size() == 0 );
	}
	CHECK( arr2.back() == 7 );
}
