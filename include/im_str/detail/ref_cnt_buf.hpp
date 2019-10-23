#ifndef IM_STR_DETAIL_REF_CNT_BUF_H
#define IM_STR_DETAIL_REF_CNT_BUF_H

#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory_resource>
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

inline constexpr std::size_t c_expr_max( std::size_t l, std::size_t r )
{
	return l > r ? l : r;
}
inline constexpr std::size_t c_expr_aligned_offset( std::size_t alignment, std::size_t min )
{
	if( alignment >= min ) {
		return alignment;
	} else {
		return min + ( alignment - min % alignment );
	}
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
	static AllocResult allocate_null_terminated_char_buffer( int size, std::pmr::memory_resource* = nullptr );

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

	friend constexpr bool operator==( const atomic_ref_cnt_buffer& l, std::nullptr_t ) { return l._cnt == nullptr; }
	friend constexpr bool operator==( std::nullptr_t, const atomic_ref_cnt_buffer& r ) { return r._cnt == nullptr; }
	friend constexpr bool operator!=( const atomic_ref_cnt_buffer& l, std::nullptr_t ) { return l._cnt != nullptr; }
	friend constexpr bool operator!=( std::nullptr_t, const atomic_ref_cnt_buffer& r ) { return r._cnt != nullptr; }

private:
	// This is used in allocate_null_terminated_char_buffer
	constexpr explicit atomic_ref_cnt_buffer( Cnt_t* buffer ) noexcept
		: _cnt( buffer )
	{
	}

	static void dealloc_buffer( Cnt_t* ptr );

	constexpr void _decref() const noexcept
	{
		if( _cnt ) {
			stats().dec_ref();
			if( _cnt->fetch_sub( 1 ) == 1 ) { dealloc_buffer( _cnt ); }
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

	using alloc_ptr_t = std::pmr::memory_resource*;
	using size_type   = int;

	static constexpr auto alignment   = c_expr_max( alignof( Cnt_t ), alignof( alloc_ptr_t ) );
	static constexpr auto offset_cnt  = 0;
	static constexpr auto offset_size = c_expr_aligned_offset( alignof( size_type ), offset_cnt + sizeof( Cnt_t ) );
	static constexpr auto offset_alloc
		= c_expr_aligned_offset( alignof( alloc_ptr_t ), offset_size + sizeof( size_type ) );
	static constexpr auto offset_data = c_expr_aligned_offset( 1, offset_alloc + sizeof( alloc_ptr_t ) );
};

struct AllocResult {
	char*                 data;
	atomic_ref_cnt_buffer handle;
};

inline AllocResult atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer( size_type                  size,
																				std::pmr::memory_resource* resource )
{
	assert( size >= 0 );
	stats().alloc();

	const auto total_size       = offset_data + size + 1;
	const bool bool_use_default = resource == nullptr;

	static_assert( alignment <= alignof( std::max_align_t ) );
	char* const start = (char*)( bool_use_default                //
									 ? std::malloc( total_size ) //
									 : resource->allocate( total_size, alignment ) );

	[[maybe_unused]] auto* cnt_ptr   = new( start + offset_cnt ) Cnt_t {1}; // construct ref count
	[[maybe_unused]] auto* size_ptr  = new( start + offset_size ) size_type {size};
	[[maybe_unused]] auto* alloc_ptr = new( start + offset_alloc )
		std::pmr::memory_resource* {resource}; // construct pointer to allocator TODO: save memory when ptr is null
	[[maybe_unused]] auto* data_ptr = start + offset_data; // Start of string
	data_ptr[size]                  = '\0';                // zero terminate

	// TODO: Is this guaranteed by the standard?
	assert( reinterpret_cast<char*>( cnt_ptr ) == start );

	return {data_ptr, atomic_ref_cnt_buffer {cnt_ptr}};
}

inline void atomic_ref_cnt_buffer::dealloc_buffer( Cnt_t* ptr )
{
	stats().dealloc();
	auto*       start = reinterpret_cast<char*>( ptr ) - offset_cnt;
	alloc_ptr_t alloc = *reinterpret_cast<alloc_ptr_t*>( start + offset_alloc );
	if( alloc == nullptr ) {
		std::free( start );
	} else {
		size_type   size  = *reinterpret_cast<size_type*>( start + offset_size );
		alloc->deallocate( start, size, alignment );
	}
}

#ifdef IM_STR_DEBUG_HOOKS
} // inline namespace debug_version
#endif

} // namespace mba::detail

#endif
