#ifndef IM_STR_DETAIL_REF_CNT_BUF_H
#define IM_STR_DETAIL_REF_CNT_BUF_H

#include <atomic>
#include <cassert>
#include <cstdint>
#include <new>     // placement new
#include <utility> // std::move

namespace mba::detail {

#ifdef IM_STR_DEBUG_HOOKS
inline namespace debug_version {
struct Stats {
	std::atomic_uint64_t total_cnt_accesses {0};
	std::atomic_uint64_t total_allocs {0};
	std::atomic_uint64_t current_allocs {0};
	std::atomic_uint64_t inc_ref_cnt {0};
	std::atomic_uint64_t dec_ref_cnt {0};

	void inc_ref()
	{
		total_cnt_accesses.fetch_add( 1, std::memory_order_relaxed );
		inc_ref_cnt.fetch_add( 1, std::memory_order_relaxed );
	}

	void dec_ref()
	{
		total_cnt_accesses.fetch_add( 1, std::memory_order_relaxed );
		dec_ref_cnt.fetch_add( 1, std::memory_order_relaxed );
	}

	void alloc()
	{
		total_allocs.fetch_add( 1, std::memory_order_relaxed );
		current_allocs.fetch_add( 1, std::memory_order_relaxed );
	}

	void dealloc() { current_allocs.fetch_sub( 1, std::memory_order_relaxed ); }

	std::uint64_t get_total_cnt_accesses() const { return total_cnt_accesses.load( std::memory_order_relaxed ); };
	std::uint64_t get_total_allocs() const { return total_allocs.load( std::memory_order_relaxed ); };
	std::uint64_t get_current_allocs() const { return current_allocs.load( std::memory_order_relaxed ); };
	std::uint64_t get_inc_ref_cnt() const { return inc_ref_cnt.load( std::memory_order_relaxed ); };
	std::uint64_t get_dec_ref_cnt() const { return dec_ref_cnt.load( std::memory_order_relaxed ); };

	constexpr Stats() noexcept = default;
	Stats( const Stats& other )
		: total_cnt_accesses( other.total_cnt_accesses.load( std::memory_order_relaxed ) )
		, total_allocs( other.total_allocs.load( std::memory_order_relaxed ) )
		, current_allocs( other.current_allocs.load( std::memory_order_relaxed ) )
		, inc_ref_cnt( other.inc_ref_cnt.load( std::memory_order_relaxed ) )
		, dec_ref_cnt( other.dec_ref_cnt.load( std::memory_order_relaxed ) )
	{
	}

	void reset()
	{
		total_cnt_accesses = 0;
		total_allocs       = 0;
		current_allocs     = 0;
		inc_ref_cnt        = 0;
		dec_ref_cnt        = 0;
	}
};
#else
struct Stats {
	constexpr Stats() noexcept = default;

	constexpr void inc_ref() noexcept {}
	constexpr void dec_ref() noexcept {}
	constexpr void alloc() noexcept {}
	constexpr void dealloc() noexcept {}
	constexpr void reset() {};
};
#endif

static Stats& stats()
{
	static Stats stats {};
	return stats;
}

constexpr struct defer_ref_cnt_tag_t {
} defer_ref_cnt_tag;

// Note: std::exchange is not constexpr in c++17
template<class T, class U = T>
constexpr T c_expr_exchange( T& obj, U&& new_value )
{
	T old_value = std::move( obj );
	obj         = std::forward<U>( new_value );
	return old_value;
}

struct AllocResult;

/**
 * Note: Almost all of the member functions are labled constexpr.
 * However, they can only be used in a constexpr context of the
 * handle is default constructed (i.e. _cnt == nullptr)
 */
class atomic_ref_cnt_buffer {
	using Cnt_t = std::atomic_int;

public:
	/*#### Constructors and special member functions ######*/
	static AllocResult allocate_null_terminated_char_buffer( int size );

	constexpr atomic_ref_cnt_buffer() noexcept = default;
	constexpr atomic_ref_cnt_buffer( const atomic_ref_cnt_buffer& other, defer_ref_cnt_tag_t ) noexcept
		: _cnt {other._cnt}
	{
	}

	constexpr atomic_ref_cnt_buffer( const atomic_ref_cnt_buffer& other ) noexcept
		: _cnt {other._cnt}
	{
		_incref();
	}
	constexpr atomic_ref_cnt_buffer( atomic_ref_cnt_buffer&& other ) noexcept
		: _cnt {c_expr_exchange( other._cnt, nullptr )}
	{
	}

	constexpr atomic_ref_cnt_buffer& operator=( const atomic_ref_cnt_buffer& other ) noexcept
	{
		// inc before dec to protect against dropping in self assignment
		other._incref();
		_decref();
		_cnt = other._cnt;
		return *this;
	}
	constexpr atomic_ref_cnt_buffer& operator=( atomic_ref_cnt_buffer&& other ) noexcept
	{
		assert( ( ( _cnt == nullptr ) || ( this != &other ) ) && "Move assignment to self is not allowed, if cnt!=0" );
		_decref();
		_cnt = c_expr_exchange( other._cnt, nullptr );
		return *this;
	}

	~atomic_ref_cnt_buffer() { _decref(); }

	friend constexpr void swap( atomic_ref_cnt_buffer& l, atomic_ref_cnt_buffer& r ) noexcept
	{
		// TODO: C++20 		std::swap( l._cnt, r._cnt );  (not yet constexpr)
		auto tmp = l._cnt;
		l._cnt   = r._cnt;
		r._cnt   = tmp;
	}
	/*#### API ######*/

	constexpr int add_ref_cnt( int cnt ) const
	{
		if( !_cnt ) {
			return 0;
		} else {
			stats().inc_ref();
			return _cnt->fetch_add( cnt, std::memory_order_relaxed ) + cnt;
		}
	}

private:
	// This is used in allocate_null_terminated_char_buffer
	constexpr explicit atomic_ref_cnt_buffer( Cnt_t* buffer ) noexcept
		: _cnt( buffer )
	{
	}

	constexpr void _decref() const noexcept
	{
		if( _cnt ) {
			stats().dec_ref();
			if( _cnt->fetch_sub( 1 ) == 1 ) {
				stats().dealloc();
				_cnt->~Cnt_t();
				delete[]( reinterpret_cast<char*>( _cnt ) );
			}
		}
	}

	constexpr void _incref() const noexcept
	{
		if( _cnt ) {
			stats().inc_ref();
			_cnt->fetch_add( 1, std::memory_order_relaxed );
		}
	}

	Cnt_t* _cnt = nullptr;
};

struct AllocResult {
	char*                 data;
	atomic_ref_cnt_buffer handle;
};

inline AllocResult atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer( int size )
{
	assert( size >= 0 );
	stats().alloc();
	const auto total_size = size + 1 + (int)sizeof( Cnt_t );
	auto*      raw        = new char[total_size];

	raw[total_size - 1] = '\0'; // zero terminate
	auto* cnt_ptr       = new( raw ) Cnt_t {1};

	// TODO: Is this guaranteed by the standard?
	assert( reinterpret_cast<char*>( cnt_ptr ) == raw );

	return {raw + sizeof( Cnt_t ), atomic_ref_cnt_buffer {cnt_ptr}};
}

#ifdef IM_STR_DEBUG_HOOKS
} // inline namespace debug_version
#endif

} // namespace mba::detail

#endif
