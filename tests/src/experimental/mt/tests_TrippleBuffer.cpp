#include <mart-common/experimental/mt/TrippleBuffer.h>

#include <catch2/catch.hpp>

#include <string>
#include <thread>

TEST_CASE( "TrippleBuffer_dummy", "[mt][TrippleBuffer]" )
{
	mart::experimental::mt::TrippleBuffer<std::string> buffer("0");
	static constexpr int ItCnt = 2000;
	auto producer = [&buffer]()
	{
		for(int i=0 ; i< ItCnt; ++i  ) {
			std::string& t = buffer.get_write_buffer();

			t = std::to_string(i);

			buffer.commit();
		}
	};
	auto consumer = [&buffer]()
	{
		int last=0;
		for (; last < ItCnt-1; ) {
			buffer.fetch_update();
			std::string& t = buffer.get_read_buffer();
			auto r = std::stoi(t);
			REQUIRE(r < ItCnt);
			REQUIRE(last <= r);
			last = r;
		}
	};

	std::thread ct(consumer);
	std::thread pt(producer);

	pt.join();
	ct.join();
}
