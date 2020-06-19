#ifndef IM_STR_DETAIL_STRING_VIEW_MIXIN_HPP
#define IM_STR_DETAIL_STRING_VIEW_MIXIN_HPP

#include <iosfwd>
#include <string_view>

namespace mba::_detail {

#define IM_STR_DETAIL_STRING_VIEW_MIXIN_DEFINE_BINARY_OP( OP )                                                         \
	friend constexpr bool operator OP( const str_view_mixin<T>& l, const str_view_mixin<T>& r )                        \
	{                                                                                                                  \
		return l.to_string_view() OP r.to_string_view();                                                               \
	}                                                                                                                  \
	friend constexpr bool operator OP( std::string_view l, const str_view_mixin<T>& r )                                \
	{                                                                                                                  \
		return l == r.to_string_view();                                                                                \
	}                                                                                                                  \
	friend constexpr bool operator OP( const str_view_mixin<T>& l, std::string_view r )                                \
	{                                                                                                                  \
		return l.to_string_view() OP r;                                                                                \
	}

/**
 * @brief A mixin that provides a simple interface similar to std::string_view
 *
 * Types that want to use it need to provide the following two functions:
 *
 *     const char* _data_for_mixin() const noexcept;
 *     std::size_t _size_for_mixin() const noexcept;
 *
 * @tparam T
 */
template<class T>
struct str_view_mixin {
	// type aliases
	using value_type      = const char;
	using pointer         = const char*;
	using const_pointer   = const char*;
	using reference       = const char&;
	using const_reference = const char&;
	using iterator        = const char*;
	using const_iterator  = const char*;

	using reverse_iterator       = std::string_view::reverse_iterator;
	using const_reverse_iterator = std::string_view::const_reverse_iterator;

	using size_type       = std::size_t;
	using difference_type = std::ptrdiff_t;

	static constexpr size_type npos = std::string_view::npos;

	constexpr const_pointer data() const noexcept { return this->_as_derived()._data_for_mixin(); }

	constexpr size_type size() const noexcept { return this->_as_derived()._size_for_mixin(); }
	constexpr size_type length() const noexcept { return this->_as_derived()._size_for_mixin(); }
	constexpr bool      empty() const noexcept { return size() == 0; }

	constexpr char operator[]( size_type i ) const noexcept { return *( data() + i ); }

	// Iterators
	constexpr iterator         begin() const noexcept { return data(); }
	constexpr const_iterator   cbegin() const noexcept { return data(); }
	constexpr reverse_iterator rbegin() const noexcept { return to_string_view().rbegin(); }
	constexpr iterator         end() const noexcept { return data() + size(); }
	constexpr const_iterator   cend() const noexcept { return data() + size(); }
	constexpr reverse_iterator rend() const noexcept { return to_string_view().rend(); }

	constexpr const_reverse_iterator crbegin() const noexcept { return to_string_view().crbegin(); }
	constexpr const_reverse_iterator crend() const noexcept { return to_string_view().crend(); }

	// conversion to string view
	constexpr std::string_view to_string_view() const noexcept
	{
		return std::string_view( this->data(), this->size() );
	}
	constexpr operator std::string_view() const noexcept { return this->to_string_view(); }

	// Relational operators
	IM_STR_DETAIL_STRING_VIEW_MIXIN_DEFINE_BINARY_OP( == )
	IM_STR_DETAIL_STRING_VIEW_MIXIN_DEFINE_BINARY_OP( != )
	IM_STR_DETAIL_STRING_VIEW_MIXIN_DEFINE_BINARY_OP( < )
	IM_STR_DETAIL_STRING_VIEW_MIXIN_DEFINE_BINARY_OP( <= )
	IM_STR_DETAIL_STRING_VIEW_MIXIN_DEFINE_BINARY_OP( > )
	IM_STR_DETAIL_STRING_VIEW_MIXIN_DEFINE_BINARY_OP( >= )

	// print support
	friend std::ostream& operator<<( std::ostream& out, const str_view_mixin<T>& str )
	{
		out << str.to_string_view();
		return out;
	}

protected:
	constexpr T&       _as_derived() noexcept { return static_cast<T&>( *this ); }
	constexpr const T& _as_derived() const noexcept { return static_cast<T const&>( *this ); }

	// Special member functions
	constexpr str_view_mixin() noexcept                           = default;
	constexpr str_view_mixin( const str_view_mixin& ) noexcept = default;
	constexpr str_view_mixin( str_view_mixin&& ) noexcept      = default;
	constexpr str_view_mixin& operator=( const str_view_mixin& ) noexcept = default;
	constexpr str_view_mixin& operator=( str_view_mixin&& ) noexcept = default;
};

#undef IM_STR_DETAIL_STRING_VIEW_MIXIN_DEFINE_BINARY_OP

} // namespace mba::_detail

#endif