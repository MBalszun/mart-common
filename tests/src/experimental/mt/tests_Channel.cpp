#include <mart-common/experimental/mt/Channel.h>

#include <catch2/catch.hpp>

TEST_CASE("mt_channel_clear_empties_buffer")
{
	mart::experimental::mt::Channel<int> ch;

	ch.send(5);
	ch.send(10);
	ch.send(15);
	ch.clear();
	int r{};
	REQUIRE(!ch.try_receive(r));
}