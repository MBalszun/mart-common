#include <mart-netlib/udp.hpp>
#include <mart-common/MartTime.h>

#include <atomic>
#include <iostream>
#include <string_view>

namespace udp = mart::nw::ip::udp;
using namespace std::chrono_literals;


std::string_view to_stringview(mart::ConstMemoryView data) {
	return std::string_view {data.asConstCharPtr(), data.size()};
}

void serv_task( std::atomic<bool>& stop_requested, udp::endpoint local_ep )
{
	udp::Socket sock;
	sock.set_rx_timeout( 100ms );

	sock.bind( local_ep );
	std::string buffer( 10, '\0' );
	while (!stop_requested) {
		auto res = sock.try_recv( mart::view_elements_mutable( buffer ).asBytes() );
		if (res.isValid()) {
			std::cout << to_stringview( res ) << std::endl;
		}
	}
	std::cout << "Server shutting down\n" << std::flush;
}

int main() {
	constexpr udp::endpoint local_ep = udp::try_parse_v4_endpoint( "127.0.0.1:3435" ).value();

	std::atomic<bool> stop_flag = false;
	std::thread       th( [&stop_flag, local_ep] { serv_task( stop_flag,local_ep ); } );

	std::cout << "Start sending data to the server\n" << std::flush;
	for (mart::PeriodicScheduler sched(1000ms); sched.invocationCnt() < 10; sched.sleep()) {
		auto msg = "PkgNr. " + std::to_string(sched.invocationCnt());
		udp::Socket{}.sendto( mart::view_elements( msg ).asBytes(), local_ep );
	}

	std::cout << "Requesting server shutdown\n" << std::flush;
	stop_flag = true;
	th.join();
}