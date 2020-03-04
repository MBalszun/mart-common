#include <mart-common/ArrayViewAdaptor.h>

#include <catch2/catch.hpp>

#include <iostream>
#include <memory>
#include <string>

namespace {
template<class T>
struct TestArray : mart::ArrayViewAdaptor<T, TestArray<T>> {
	constexpr TestArray() noexcept = default;
	TestArray( std::initializer_list<T> data )
		: data_( std::make_unique<T[]>( data.size() ) )
		, size_( data.size() )
	{
		std::copy( data.begin(), data.end(), data_.get() );
	}

private:
	std::unique_ptr<T[]> data_;
	std::size_t          size_;

	// crtp interface for mart::ArrayViewAdapto
	friend mart::ArrayViewAdaptor<T, TestArray<T>>;

	T*                 _arrayView_data() { return data_.get(); }
	constexpr const T* _arrayView_data() const { return data_.get(); }

	constexpr std::size_t _arrayView_size() const { return size_; }
};

template<class T, class FI>
void test_array( TestArray<T> data, T start_val, T end_val, FI inc_func )
{
	TestArray<T> array = std::move( data );

	CHECK( array.size() );

	CHECK( array.front() == start_val );
	CHECK( array.back() == end_val );
	CHECK( *array.cbegin() == start_val );
	CHECK( array.begin() == array.cbegin() );

	// index based iteration
	T v = start_val;
	for( std::size_t i = 0; i < array.size(); ++i ) {
		CHECK( v == array[i] );
		//CHECK( v == array.at( i ) );
		inc_func( v );
	}

	// For-loop iteration
	v = start_val;
	for( auto e : array ) {
		CHECK( v == e );
		inc_func( v );
	}

	v = start_val;
	for( auto& e : array ) {
		CHECK( v == e );
		inc_func( v );
	}

	v = start_val;
	for( const auto& e : array ) {
		CHECK( v == e );
		inc_func( v );
	}

	v = start_val;
	for( const auto& e : std::as_const( array ) ) {
		CHECK( v == e );
		inc_func( v );
	}
}

TEST_CASE( "ArrayViewAdaptor_can_iterate", "[ArrayViewAdaptor]" )
{

	using namespace std::string_literals;

	TestArray<int> int_array{1, 2, 3, 4, 5, 6};

	test_array(
		std::move( int_array ), 1, 6, []( int& v ) { v++; } );

	TestArray<std::string> string_array{"a", "aa", "aaa", "aaaa", "aaaaa", "aaaaaa"};

	test_array(
		std::move( string_array ),
		"a"s,
		"aaaaaa"s,
		[]( std::string& v ) { v += "a"; }
		 );
}

} // namespace