#include <im_str/im_str.hpp>

#include <chrono>

#include <algorithm>
#include <numeric>

#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace mba;

std::vector<std::string> generate_random_strings( int cnt )
{
	std::vector<std::string> ret;

	std::string base(
		":::::::::::::::::::::::: "
		"                                                                ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,     "
		"sfladskjflasdjfslfslafdjlsadfjsafdsaljfdlsafdkjsaldfsadfsafowieuroquwreoqweurowqureowqieuroiwqeurouqwnfsalfvnn"
		"sa"
		"::::::::::::::::::::::::::"
		"safsaHHQWWEEERRTTZUIOPPPOLksdfkasdfasfdsafdljdflsafdoweroiequrwoieurqoureowquerwqorewqureiwKKJHGFDSASDFGHFDsaf"
		"dsaf"
		"d                                                                "
		"safdsafdsadfsadfsadfsadfsfasdfsadfsadfsadfsadfdsfafdafsfdsfwfwafs"
		"adfasfwafSAYXCVBVCXCVBNMNBVCXSDFGH          :::::::::,,,,,,,,,,,,,;;;;;;;;;;;////////////"
		"1231231981239123129387129387129381239817239172398127398217398172398123918273981739127391828312938721"
		"91273198273192739182739812739821731287319823712938172398712931298371982317293127381928319273921319237129" );

	base = base + base + base + base + base + base + base;

	for( int i = 0; i < cnt; ++i ) {
		std::shuffle( base.begin(), base.end(), std::random_device{} );
		ret.push_back( base );
	}
	return ret;
}

im_str::DynArray_t flatten( std::vector<im_str::DynArray_t>& collections )
{
	const std::size_t total_size
		= std::accumulate( collections.begin(), collections.end(), size_t( 0 ), []( size_t size, const auto& e ) {
			  return size + e.size();
		  } );
	im_str::DynArray_t ret( total_size );

	auto out_it = ret.begin();

	for( auto&& c : collections ) {
		out_it = std::move( c.begin(), c.end(), out_it );
	}

	return ret;
}

template<int Algo>
im_str run( const im_str::DynArray_t& strings, const std::vector<char>& split_chars )
{
	auto cstrings = strings;
	for( char split_char : split_chars ) {
		std::vector<im_str::DynArray_t> tmp( cstrings.size() );

		size_t i = 0;

		for( auto&& s : cstrings ) {
			static_assert( 0 <= Algo && Algo < 1, "No algorithm with that number available at the moment" );
			if constexpr( Algo == 0 ) {
				tmp[i++] = s.split_full( split_char );
			} /*else {
				// put other algorithm here
			} */
		}
		cstrings = flatten( tmp );
	}

	return concat( cstrings );
}

template<int Algo, int Rep = 10, int It = 20>
// __declspec(noinline)
void test_algo( const im_str::DynArray_t& s, const std::vector<char>& split_chars )
{
	using namespace std::chrono;
	std::vector<int> res( Rep );
	assert( res.size() == Rep );

	for( int z = -1; z < Rep; ++z ) {

		const auto start = steady_clock::now();
		for( int i = 0; i < It; ++i ) {
			run<Algo>( s, split_chars );
		}
		const auto end = steady_clock::now();

		// discard first repetition
		if( z >= 0 ) {
			const auto total = ( end - start ) / std::chrono::milliseconds{ 1 } / It;
			std::cout << total << "ms per iteration" << std::endl;
			res[z] = (int)total;
		}
	}

	auto avg  = std::accumulate( res.begin(), res.end(), 0 ) / (double)res.size();
	auto avgs = std::accumulate( res.begin(), res.end(), 0, []( auto acc, auto v ) { return acc + v * v; } )
				/ (double)res.size();
	std::cout << "Avg: " << avg << "ms | std-dev:"<< std::sqrt(avgs - avg * avg) << "ms" << std::endl;

}

int main()
{
	const auto my_strings = generate_random_strings( 200 );

	const std::vector<char> split_chars{ ' ', ':', '/', ';', ',' };
	im_str::DynArray_t      cstrings( my_strings.size() );

	auto it = cstrings.begin();
	for( const auto& s : my_strings ) {
		*it = ( im_str( s ) );
		++it;
	}

	test_algo<0>( cstrings, split_chars );
	std::cout << "========================================================" << std::endl;
	// test_algo<2>( cstrings, split_chars );
	// std::cout << "========================================================" << std::endl;
	// test_algo<3>( cstrings, split_chars );
	// std::cout << "========================================================" << std::endl;
	// test_algo<3>( cstrings, split_chars );
	// std::cout << "========================================================" << std::endl;
	// test_algo<2>( cstrings, split_chars );
	// std::cout << "========================================================" << std::endl;
	// test_algo<1>( cstrings, split_chars );
	// std::cout << mba::detail::stats().get_total_allocs() << std::endl;
	std::cout << "========================================================" << std::endl;
}
