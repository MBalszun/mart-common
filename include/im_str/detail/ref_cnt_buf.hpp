#ifndef IM_STR_DETAIL_REF_CNT_BUF_H
#define IM_STR_DETAIL_REF_CNT_BUF_H

#include <atomic>
#include <cassert>
#include <cstdint>
#include <new>     // placement new
#include <utility> // std::move
#include <cstdlib>

#ifndef IM_STR_USE_ALLOC
	#if __has_include( <memory_resource>)
		#include <memory_resource>
		#define IM_STR_USE_ALLOC 1
	#endif
#else
	#define IM_STR_USE_ALLOC 0
#endif



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
	using Cnt_t     = std::atomic_int;
	using size_type = int;

public:
#if IM_STR_USE_ALLOC
	using alloc_t     = std::pmr::memory_resource;
	using alloc_ptr_t = alloc_t*;
#else
	using alloc_ptr_t = std::nullptr_t;
#endif
	/*#### Constructors and special member functions ######*/
	static AllocResult allocate_null_terminated_char_buffer( int size, alloc_ptr_t = nullptr );

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
	struct Header {
		Cnt_t       ref_cnt;
		size_type   size;
		alloc_ptr_t alloc;
	};

	// This is used in allocate_null_terminated_char_buffer
	constexpr explicit atomic_ref_cnt_buffer( Header& buffer ) noexcept
		: _cnt( &( buffer.ref_cnt ) )
	{
	}

	static void dealloc_buffer( Header* ptr );

	constexpr void _decref() const noexcept
	{
		if( _cnt ) {
			stats().dec_ref();
			if( _cnt->fetch_sub( 1 ) == 1 ) {
				Header* header = static_cast<Header*>( static_cast<void*>( _cnt ) );
				dealloc_buffer( header );
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

	static constexpr auto alignment = alignof( Header );
};

struct AllocResult {
	char*                 data;
	atomic_ref_cnt_buffer handle;
};

inline AllocResult atomic_ref_cnt_buffer::allocate_null_terminated_char_buffer( size_type size, alloc_ptr_t resource )
{
	assert( size >= 0 );
	stats().alloc();

	const auto total_size       = sizeof( Header ) + size + 1;

#if IM_STR_USE_ALLOC
	const bool bool_use_default = resource == nullptr;
	char* const start = (char*)( bool_use_default                //
									 ? std::malloc( total_size ) //
									 : resource->allocate( total_size, alignment ) );
#else
	char* const start = (char*)std::malloc( total_size );
#endif

	auto* const header_ptr = new( start ) Header {Cnt_t {1}, size_type {size}, resource};

	auto* const data_ptr = start + sizeof( Header ); // Start of string
	data_ptr[size]       = '\0';                     // zero terminate

	return {data_ptr, atomic_ref_cnt_buffer {*header_ptr}};
}

inline void atomic_ref_cnt_buffer::dealloc_buffer( Header* header )
{
	stats().dealloc();

#if IM_STR_USE_ALLOC
	alloc_ptr_t alloc = header->alloc;
	if( alloc == nullptr ) {
		std::free( header );
	} else {
		size_type size = header->size;
		alloc->deallocate( header, size, alignment );
	}
#else
	std::free( header );
#endif
}

#ifdef IM_STR_DEBUG_HOOKS
} // inline namespace debug_version
#endif

} // namespace mba::detail

#endif
