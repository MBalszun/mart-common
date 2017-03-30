#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_STRINGVIEW_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_STRINGVIEW_H

#include "../StringView.h"

///* ######## INCLUDES ######### */
///* Standard Library Includes */
//#include <algorithm>
//#include <cstring>
//#include <memory>
//#include <ostream>
//#include <string>
//#include <type_traits>
//
///* Proprietary Library Includes */
//
///* Project Includes */
//#include "ArrayViewAdaptor.h"
//
//namespace mart {
//
//class StringView : public ArrayViewAdaptor<const char, StringView> {
//public:
//	// type defs
//	using CharT		  = char;
//	using traits_type = std::char_traits<CharT>;
//
//	static constexpr size_type npos = size_type( -1 );
//
//public:
//	/* #### CTORS #### */
//	constexpr StringView() noexcept = default;
//
//	StringView( const std::string& other ) noexcept
//		: _start( other.data() )
//		, _size( other.size() )
//	{
//	}
//
//	constexpr StringView( const char* other, size_type size ) noexcept
//		: _start( other )
//		, _size( size )
//	{
//	}
//
//	static StringView fromZString( const char* other ) { return {other, std::strlen( other )}; }
//
//	// NOTE: Use only for string literals!!!
//	template <size_t N>
//	constexpr StringView( const char ( &other )[N] ) noexcept
//		: _start( other )
//		, _size( N - 1 )
//	{
//	}
//
//	constexpr StringView(const char &other) noexcept
//		: _start(&other)
//		, _size(1)
//	{
//	}
//
//	//prevent construction from integral type except char
//	template<class T, class = typename std::enable_if<std::is_integral<T>::value && (!std::is_same<T, char>::value)>::type >
//	constexpr StringView(const T &other) = delete;
//
//	template <class T>
//	StringView( const T* const& other ) = delete;
//
//	/* #### Special member functions #### */
//	constexpr StringView( const StringView& other ) noexcept = default;
//	StringView& operator=( const StringView& other ) noexcept = default;
//	constexpr StringView(StringView&& other) noexcept = default;
//	StringView& operator=(StringView&& other) noexcept = default;
//
//	/*#### string functions ####*/
//	std::string to_string() const { return std::string( this->cbegin(), this->cend() ); }
//
//	constexpr StringView substr( size_t offset, size_t count = npos) const
//	{
//		return offset + count <= this->_size
//								? StringView{ this->_start + offset, count }
//								: count == npos
//										 ? substr(offset , size() - offset)
//										 : throw std::out_of_range(
//											   "Tried to create a substring that would exceed the original string. "
//											   "Original string:\n\n" + this->to_string() + "\n" );
//	}
//
//	/**
//	 * Splits the string at given position and returns a pair holding both substrings
//	 *  - if 0 <= pos < size():
//	 *		return substrings [0...pos) [pos ... size())
//	 *	- if pos == size() or npos:
//	 *		returns a copy of the current stringview and a default constructed one
//	 *	- if pos > size()
//	 *		throws std::out_of_range exception
//	 */
//	constexpr std::pair<StringView,StringView> split(size_t pos) const
//	{
//		return pos < _size
//					? std::make_pair( StringView{ this->_start, pos }, StringView{ this->_start + pos + 1, _size-pos-1 } )
//					: (pos == _size || pos == npos)
//							? std::pair<StringView, StringView>{ *this, StringView{} }
//							: throw std::out_of_range(	"\nTried to create a substring that would exceed the original string. "
//														"Original string:\n\n"
//														+ this->to_string()
//														+ "\n");
//	}
//
//	/*#### algorithms ####*/
//
//	size_type find( char c, size_type start_pos = 0 ) const
//	{
//		if (start_pos >= size()) {
//			return npos;
//		}
//
//		const size_t pos = std::find( this->cbegin() + start_pos, this->cend(), c ) - this->cbegin();
//		return pos < this->size() ? pos : npos;
//	}
//
//	template <class P>
//	size_type find_if( P p, size_type start_pos = 0 ) const
//	{
//		if (start_pos >= size()) {
//			return npos;
//		}
//
//		const size_t pos = std::find_if( this->cbegin() + start_pos, this->cend(), p ) - this->cbegin();
//		return pos < this->size() ? pos : npos;
//	}
//
//	StringView substr_sentinel(size_t offset, char sentinel ) const
//	{
//		return substr(offset, this->find(sentinel,offset));
//	}
//
//	template <class P>
//	StringView substr_predicate(size_t offset, P p) const
//	{
//		return substr(offset, this->find_if(p, offset));
//	}
//
//	friend int compare( StringView l, StringView r );
//	friend std::ostream& operator<<( std::ostream& out, const StringView string )
//	{
//		out.write( string.data(), string.size() );
//		return out;
//	}
//
//	bool isValid() const { return _start != nullptr; }
//
//protected:
//	friend class ArrayViewAdaptor<const char, StringView>;
//	constexpr size_type		_arrayView_size() const { return _size; }
//	constexpr const_pointer _arrayView_data() const { return _start; }
//
//	const char* _start = nullptr;
//	size_type   _size  = 0;
//};
//
//inline int compare( StringView l, StringView r )
//{
//	if( ( l._start == r._start ) && ( l.size() == l.size() ) ) {
//		return 0;
//	}
//	int ret = StringView::traits_type::compare( l.cbegin(), r.cbegin(), std::min( l.size(), r.size() ) );
//
//	if( ret == 0 ) {
//		// couldn't find a difference yet -> compare sizes
//		if( l.size() < r.size() ) {
//			ret = -1;
//		} else if( l.size() > r.size() ) {
//			ret = 1;
//		}
//	}
//	return ret;
//}
//
///* operator overloads */
//// clang-format off
//inline bool operator==(const StringView& l, const StringView& r) { return compare(l,r) == 0; }
//inline bool operator!=(const StringView& l, const StringView& r) { return !(l == r); }
//inline bool operator< (const StringView& l, const StringView& r) { return compare(l,r) < 0; }
//inline bool operator> (const StringView& l, const StringView& r) { return r<l; }
//inline bool operator<=(const StringView& l, const StringView& r) { return !(l>r); }
//inline bool operator>=(const StringView& l, const StringView& r) { return !(l < r); }
//// clang-format on
//constexpr StringView EmptyStringView{""};
//}
//
//namespace std {
//template <>
//struct hash<mart::StringView> {
//	// form http://stackoverflow.com/questions/24923289/is-there-a-standard-mechanism-to-retrieve-the-hash-of-a-c-string
//	std::size_t operator()( mart::StringView str ) const noexcept
//	{
//		std::size_t h = 0;
//
//		for( auto c : str ) {
//			h += h * 65599 + c;
//		}
//
//		return h ^ ( h >> 16 );
//	}
//};
//}

#endif /* LIBS_MART_COMMON_EXPERIMENTAL_STRINGVIEW_H_ */
