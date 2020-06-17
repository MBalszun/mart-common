#include <im_str/im_str.hpp>
#include <cassert>

int main()
{
	using namespace mba;

	im_str name = "John";
	assert( name == "John" );
	assert( name.size() == 4 );

	im_str cpy = name;
	name       = im_str( "Jane Doe" ); // No allocation here
	assert( cpy == "John" );           // cpy hasn't changed when reassigning the original name variable

	auto [first1, second1] = name.split_on_first( ' ' );
	assert( first1 == "Jane" );
	assert( second1 == "Doe" );

	auto [first2, second2] = im_str( "Hello;World" ).split_on_first( ';', mba::im_str::Split::After );
	assert( first2 == "Hello;" );
	assert( second2 == "World" );
}