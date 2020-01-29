#include <mart-common/experimental/mt/Channel.h>

#include <catch2/catch.hpp>

#include <future>
#include <iostream>
#include <mart-common/ranges.h>
#include <string>
#include <thread>

TEST_CASE( "mt_channel_clear_empties_buffer", "[channel]" )
{
	mart::experimental::mt::Channel<int> ch;

	ch.send( 5 );
	ch.send( 10 );
	ch.send( 15 );
	ch.clear();
	int r {};
	REQUIRE( !ch.try_receive( r ) );
}

TEST_CASE( "mt_channel_try_receive_gets_send_strings_ST", "[channel]" )
{
	using namespace mart::experimental::mt;

	Channel<std::string> ch;

	for( auto k : mart::irange( 0, 5 ) ) {
		for( auto i : mart::irange( 0, k * 200 ) ) {
			ch.send( std::to_string( i ) );
		}
		for( auto i : mart::irange( 0, k * 200 ) ) {
			std::string str;
			if( ch.try_receive( str ) ) { REQUIRE( std::stoll( str ) == i ); }
		}
	}
}

TEST_CASE( "mt_channel_send_and_receive_via_stream_operator", "[channel]" )
{
	using namespace mart::experimental::mt;

	Channel<std::string> ch;

	for( auto k : mart::irange( 0, 5 ) ) {
		for( auto i : mart::irange( 0, k * 200 ) ) {
			ch << std::to_string( i );
		}
		for( auto i : mart::irange( 0, k * 200 ) ) {
			std::string str;
			if( ch.try_receive( str ) ) { REQUIRE( std::stoll( str ) == i ); }
		}
	}
	for( auto k : mart::irange( 0, 5 ) ) {
		for( auto i : mart::irange( 0, k * 200 ) ) {
			ch << std::to_string( i );
		}
		for( auto i : mart::irange( 0, k * 200 ) ) {
			std::string str;
			// TODO: Test if read is blocking
			ch >> str;
			REQUIRE( std::stoll( str ) == i );
		}
	}
}

TEST_CASE( "mt_channel_dual_thread", "[channel]" )
{
	using namespace mart::experimental::mt;

	Channel<std::string> ch;

	for( auto k : mart::irange( 0, 5 ) ) {
		auto prod = std::async( [&ch, k] {
			for( auto i : mart::irange( 0, k * 200 ) ) {
				ch.send( std::to_string( i ) );
			}
		} );
		auto cons = std::async( [&ch, k] {
			for( auto i : mart::irange( 0, k * 200 ) ) {
				std::string str;
				ch.receive( str );
				REQUIRE( std::stoll( str ) == i );
			}
		} );
		prod.wait();
		cons.wait();
	}
}

TEST_CASE( "mt_channel_multi_thread", "[channel]" )
{
	using namespace mart::experimental::mt;

	Channel<std::string> ch;

	for( auto k : mart::irange( 0, 5 ) ) {
		auto prod1 = std::async( [&ch, k] {
			for( auto i : mart::irange( 0, k * 200 ) ) {
				ch.send( std::to_string( i ) + "_1" );
			}
		} );
		auto prod2 = std::async( [&ch, k] {
			for( auto i : mart::irange( 0, k * 200 ) ) {
				ch.send( std::to_string( i ) + "_2" );
			}
		} );
		auto prod3 = std::async( [&ch, k] {
			for( auto i : mart::irange( 0, k * 200 ) ) {
				ch.send( std::to_string( i ) + "_3" );
			}
		} );
		auto cons  = std::async( [&ch, k] {
            int i1 = 0;
            int i2 = 0;
            int i3 = 0;
            for( auto i : mart::irange( 0, k * 200 * 3 ) ) {
                (void)i;
                std::string str;
                ch.receive( str );
                if( str.back() == '1' ) {
                    REQUIRE( std::stoll( str ) == i1 );
                    i1++;
                } else if( str.back() == '2' ) {
                    REQUIRE( std::stoll( str ) == i2 );
                    i2++;
                } else if( str.back() == '3' ) {
                    REQUIRE( std::stoll( str ) == i3 );
                    i3++;
                } else {
                    REQUIRE( false );
                }
            }
        } );
		prod1.wait();
		prod2.wait();
		prod3.wait();
		cons.wait();
	}
}

TEST_CASE( "mt_channel_unblock", "[channel]" )
{
	using namespace mart::experimental::mt;

	for( auto k : mart::irange( 1, 5 ) ) {
		Channel<std::string> ch;
		auto                 cons  = std::async( [&ch, k] {
            int  i1                 = 0;
            int  i2                 = 0;
            int  i3                 = 0;
            bool exception_happened = false;
            for( auto i : mart::irange( 0, k * 20 * 5 ) ) {
                (void)i;
                std::string str;
                try {
                    ch.receive( str );
                } catch( const Canceled& ) {
                    exception_happened = true;
                    break;
                }
                if( str.back() == '1' ) {
                    REQUIRE( std::stoll( str ) == i1 );
                    i1++;
                } else if( str.back() == '2' ) {
                    REQUIRE( std::stoll( str ) == i2 );
                    i2++;
                } else if( str.back() == '3' ) {
                    REQUIRE( std::stoll( str ) == i3 );
                    i3++;
                } else {
                    REQUIRE( false );
                }
            }
            REQUIRE( exception_happened );
        } );
		auto                 prod1 = std::async( [&ch, k] {
            for( auto i : mart::irange( 0, k * 20 ) ) {
                ch.send( std::to_string( i ) + "_1" );
            }
        } );
		auto                 prod2 = std::async( [&ch, k] {
            for( auto i : mart::irange( 0, k * 20 ) ) {
                ch.send( std::to_string( i ) + "_2" );
            }
        } );
		auto                 prod3 = std::async( [&ch, k] {
            for( auto i : mart::irange( 0, k * 20 ) ) {
                ch.send( std::to_string( i ) + "_3" );
            }
        } );
		prod1.get();
		prod2.get();
		prod3.get();
		ch.cancel_read();
		cons.get();
	}
}