#include <mart-common/MartTime.h>
#include <mart-common/utils.h>
#include <mart-netlib/udp.hpp>

#include <Windows.h>

#include <iostream>

#include <cstddef>

std::byte b;

using namespace std::chrono_literals;

namespace udp = mart::nw::ip::udp;

const auto destination = udp::try_parse_v4_endpoint( "127.0.0.1:3435" ).value();

const char vt_cmd_cursor_reset[] = {0x1B, '[', '0', ';', '0', 'H'};
const char vt_cmd_clear_line[]   = {0x1B, '[', '1', 'M'};
const char vt_cmd_hide_cursor[]  = {0x1B, '[', '?', '2', '5', 'l'};

template<std::size_t N>
void insert_vt_cmd( const char ( &cmd )[N] )
{
	std::cout.write( cmd, sizeof( cmd ) );
}

void clear_top_console_line()
{
	insert_vt_cmd( vt_cmd_cursor_reset );
	insert_vt_cmd( vt_cmd_clear_line );
}

bool is_key_pressed( char key )
{
	return GetKeyState( key ) & 0x8000;
}

int main()
{
	mart::ExecuteOnExit_t cleanup(
		[] { udp::Socket{}.sendto( mart::view_bytes( "EXIT" ).subview( 0, 4 ), destination ); } );
	std::atomic_int heart_rate = 0;
	std::atomic_int blood_ox   = 0;
	bool            changed    = true;

	insert_vt_cmd( vt_cmd_hide_cursor );

	for( mart::PeriodicScheduler sched( 50ms );; sched.sleep() ) {

		if( is_key_pressed( 'W' ) ) {
			heart_rate++;
			changed = true;
		}
		if( is_key_pressed( 'S' ) ) {
			heart_rate--;
			changed = true;
		}
		if( is_key_pressed( 'E' ) ) {
			blood_ox++;
			changed = true;
		}
		if( is_key_pressed( 'D' ) ) {
			blood_ox--;
			changed = true;
		}

		if( is_key_pressed( 'K' ) ) {
			// udp::Socket {}.sendto( mart::view_bytes( "EXIT" ), destination );
			return 0;
		}

		blood_ox   = mart::clamp( blood_ox.load(), 0, 100 );
		heart_rate = mart::clamp( heart_rate.load(), 0, 200 );

		if( changed ) {
			auto msg = std::to_string( heart_rate ) + ":" + std::to_string( blood_ox );
			/*std::cout.write( vt_cmd_cursor_reset, sizeof( vt_cmd_cursor_reset ) );
			std::cout.write( vt_cmd_clear_line, sizeof( vt_cmd_clear_line ) );*/
			clear_top_console_line();
			std::cout << msg;
			udp::Socket{}.sendto( mart::view_elements( msg ).asBytes(), destination );
		}
		changed = false;
	}
}