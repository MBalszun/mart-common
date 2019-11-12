#ifndef IM_STR_DYNAMIC_ARRAY_HPP
#define IM_STR_DYNAMIC_ARRAY_HPP

#include <cstddef>
#include <cstdint>
#include <memory>

namespace mba::detail {

/**
 * A minimal implementation of an array with fixed runtim-size.
 * It can be used as a return type by im_str::split_full,
 * if std::vector is not available.
 */
template<class T>
class dynamic_array {
	std::size_t          _size = 0;
	std::unique_ptr<T[]> _data;

public:
	using value_type = T;

	constexpr std::size_t size() const noexcept { return _size; }

	T*       begin() noexcept { return _data.get(); }
	const T* begin() const noexcept { return _data.get(); }

	T*       end() noexcept { return _data.get() + _size; };
	const T* end() const noexcept { return _data.get() + _size; };

	constexpr dynamic_array() noexcept = default;

	dynamic_array( std::size_t size )
		: _size( size )
		, _data( std::make_unique<T[]>( size ) )
	{
	}

	dynamic_array( const dynamic_array& other )
		: dynamic_array( other._size )
	{
		std::copy( other.begin(), other.end(), begin() );
	}

	dynamic_array( dynamic_array&& other ) noexcept
		: _size( other._size )
		, _data( std::move( other._data ) )
	{
		other._size = 0;
	}

	dynamic_array& operator=( const dynamic_array& other )
	{
		if( other._size > _size ) { _data = std::make_unique<T[]>( other._size ); }

		_size = other._size;
		std::copy( other.begin(), other.end(), begin() );
		return *this;
	}

	dynamic_array& operator=( dynamic_array&& other ) noexcept
	{
		_data       = std::move( other._data );
		_size       = other._size;
		other._size = 0;
		return *this;
	}
};

} // namespace mba::detail
#endif // !IM_STR_DYNAMIC_ARRAY_HPP
