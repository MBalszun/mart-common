#include <im_str/im_str.hpp>

#include "include_catch.hpp"

#include <memory>
#include <memory_resource>
#include <string>
#include <vector>

namespace {

// Test allocator for debugging purposes - DO NOT USE IN PRODUCTION
struct TestAlloc : std::pmr::memory_resource {
	static constexpr std::size_t chunk_size = 1024;

	struct Alloc {
		void*       data;
		std::size_t start;
		std::size_t size;
	};

	std::vector<Alloc>                   allocs;
	std::vector<Alloc>                   all_allocs;
	std::vector<std::unique_ptr<char[]>> data;

	std::size_t next = 0;

	static std::size_t roundUp( std::size_t numToRound, std::size_t multiple )
	{
		if( multiple == 0 ) return numToRound;

		std::size_t remainder = numToRound % multiple;
		if( remainder == 0 ) return numToRound;

		return numToRound + multiple - remainder;
	}

	void* do_allocate( std::size_t bytes, std::size_t alignment ) override
	{
		if( bytes > chunk_size ) { throw std::bad_alloc{}; }

		const auto start = roundUp( next, alignment );

		auto chunk  = start / chunk_size;
		auto offset = start % chunk_size;

		if( ( offset + bytes ) > chunk_size ) {
			chunk++;
			offset = 0;
		}
		while( chunk >= data.size() ) {
			data.push_back( std::make_unique<char[]>( chunk_size ) );
		}

		void* ptr = data[chunk].get() + offset;
		Alloc alloc{ ptr, chunk * chunk_size + offset, bytes };
		allocs.push_back( alloc );
		all_allocs.push_back( alloc );

		next = chunk * chunk_size + offset + bytes;

		return ptr;
	}
	void do_deallocate( void* p, std::size_t bytes, std::size_t /*alignment*/ ) override
	{
		auto pos = std::find_if( allocs.begin(), allocs.end(), [p]( const Alloc& a ) { return a.data == p; } );
		assert( pos != allocs.end() );
		assert( pos->size == bytes );

		// NOTE: for this test allocator, the memory isn't actually reclaimed
		allocs.erase( pos );
	}
	bool do_is_equal( const std::pmr::memory_resource& other ) const noexcept override { return this == &other; }
};

TestAlloc alloc;
} // namespace


TEST_CASE( "custom_alloc", "[im_str]" )
{

	{ 	// construction from string causes single allocation
		std::string s{ "Hello World" };
		CHECK( alloc.allocs.size() == 0 );
		mba::im_zstr str( s, &alloc );
		CHECK( alloc.allocs.size() == 1 );

		// move construction keeps allocation the same
		mba::im_zstr str2 = std::move( str );
		CHECK( alloc.allocs.size() == 1 );
		CHECK( str2[1] == 'e' );

		// second construction causes second allocation
		mba::im_zstr str3( std::string_view("Hello World3"), &alloc );
		CHECK( alloc.allocs.size() == 2 );

		// move assignment causes deallocation of original memory
		str3 = str2;
		CHECK( alloc.allocs.size() == 1 );

		// explicit construction without allocation allocation
		mba::im_zstr str4( std::string_view( "Hello World3" ), mba::im_str::trust_me_this_is_from_a_string_litteral );
		CHECK( alloc.allocs.size() == 1 );
	}

	// after all strings have been destructed, all memory is deallocated
	CHECK( alloc.allocs.size() == 0 );

	// in total there where two allocations happening
	CHECK( alloc.all_allocs.size() == 2 );
}
