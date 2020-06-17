#ifndef IM_STR_DEBUG_HOOKS
#define IM_STR_DEBUG_HOOKS
#endif // !1

#include <im_str/detail/ref_cnt_buf.hpp>

#include "include_catch.hpp"

#include <iostream>

using namespace ::mba;

namespace {
void check_stats_zero()
{
	CHECK( _detail_im_str::stats().get_total_cnt_accesses() == 0 );
	CHECK( _detail_im_str::stats().get_total_allocs() == 0 );
	CHECK( _detail_im_str::stats().get_current_allocs() == 0 );
	CHECK( _detail_im_str::stats().get_inc_ref_cnt() == 0 );
	CHECK( _detail_im_str::stats().get_dec_ref_cnt() == 0 );
}

} // namespace

TEST_CASE( "ref_cnt_buf_default_construction_does_nothing", "[im_str]" )
{
	_detail_im_str::stats().reset();

	check_stats_zero();

	_detail_im_str::atomic_ref_cnt_buffer b1;

	auto b2 = _detail_im_str::atomic_ref_cnt_buffer{};
	auto b3( b2 );

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-assign-overloaded"
#endif

	// yes, we want to check self assignment here
	// TODO: also check with non-default constructed buffers
	b1 = b1;

#ifdef __clang__
#pragma GCC diagnostic pop
#endif

	b1 = b3;
	b3 = std ::move( b1 );

	check_stats_zero();
}

TEST_CASE( "ref_cnt_buf_various", "[im_str]" )
{
	using namespace ::mba::_detail_im_str;
	_detail_im_str::stats().reset();

	check_stats_zero();

	[[maybe_unused]] auto [data, handle]
		= _detail_im_str::atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer( 14 );
	(void)data;
	{
		auto b3( handle );
		CHECK( stats().get_total_allocs() == 1 );
		CHECK( stats().get_current_allocs() == 1 );
		CHECK( stats().get_inc_ref_cnt() == 1 );
	}

	CHECK( stats().get_total_allocs() == 1 );
	CHECK( stats().get_current_allocs() == 1 );

	{
		auto b4( std::move( handle ) );
	}
	CHECK( _detail_im_str::stats().get_total_allocs() == 1 );
	CHECK( _detail_im_str::stats().get_current_allocs() == 0 );
	CHECK( _detail_im_str::stats().get_inc_ref_cnt() == 1 );
	CHECK( _detail_im_str::stats().get_dec_ref_cnt() == 2 );
}

namespace {
struct Foo {
	_detail_im_str::atomic_ref_cnt_buffer handle;
	std::size_t                           size;
	char*                                 data;
};

Foo foo()
{
	auto [data, handle] = _detail_im_str::atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer( 14 );
	return { std::move( handle ), 14, data };
}
} // namespace

TEST_CASE( "ref_cnt_buf_return_from_Function_1", "[im_str]" )
{
	_detail_im_str::stats().reset();
	_detail_im_str::atomic_ref_cnt_buffer b1 = [] {
		auto [data, handle] = _detail_im_str::atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer( 14 );
		(void)data;
		return std::move( handle );
	}();

	CHECK( _detail_im_str::stats().get_dec_ref_cnt() == 0 );
	CHECK( _detail_im_str::stats().get_total_allocs() == 1 );
}

TEST_CASE( "ref_cnt_buf_return_from_Function_2", "[im_str]" )
{
	_detail_im_str::stats().reset();
	[[maybe_unused]] auto res = foo();

	CHECK( _detail_im_str::stats().get_dec_ref_cnt() == 0 );
	CHECK( _detail_im_str::stats().get_total_allocs() == 1 );
}
