#pragma once

#include <iterator>
#include "cpp_std/type_traits.h"


namespace mart {
	template<class T>
	class IIterator {
		static_assert(std::is_integral<T>::value, "IIterator can only be used for integral values");
	public:
		using difference_type = mart::make_signed_t<T>;
		using value_type = T;
		using pointer = T*;
		using reference = const T&;
		using iterator_category = std::random_access_iterator_tag;

		IIterator(T value = T()) :
			i{ value }
		{};

		reference operator*() const { return i; }
		pointer operator->() const { return &i; }

		IIterator& operator++() { ++i; return *this; }
		IIterator operator++(int) { return IIterator{ i++ }; }

		IIterator& operator--() { --i; return *this; }
		IIterator operator--(int) { return IIterator{ i-- }; }

		IIterator& operator+=(difference_type diff) { i += diff; return *this; }
		IIterator& operator-=(difference_type diff) { i -= diff; return *this; }

		value_type operator[](difference_type diff) const { return i + diff; }

		friend bool operator!=(IIterator l, IIterator r) { return l.i != r.i; }
		friend bool operator==(IIterator l, IIterator r) { return l.i == r.i; }
		friend bool operator<(IIterator l, IIterator r) { return l.i < r.i; }
		friend bool operator<=(IIterator l, IIterator r) { return l.i <= r.i; }
		friend bool operator>(IIterator l, IIterator r) { return l.i > r.i; }
		friend bool operator>=(IIterator l, IIterator r) { return l.i >= r.i; }

		friend IIterator operator+(IIterator l, difference_type n) { return l.i + n; }
		friend IIterator operator+(difference_type n, IIterator l) { return n + l.i; }

		friend difference_type operator-(IIterator l, IIterator r) { return l.i - r.i; }
		friend IIterator operator-(IIterator l, difference_type n) { return l.i - n; }

	private:
		T i;
	};

namespace _impl_irange {

template<class T>
struct irange_t {
	static_assert(std::is_integral<T>::value, "irange can only be used for integral values");
	using iterator = IIterator<T>;

	iterator begin() const { return _start; };
	iterator end() const { return _past_end; };

	iterator _start;
	iterator _past_end;
};

template<class T>
struct non_deduced {
	using type = T;
};

template<class T>
using non_deduced_t = typename non_deduced<T>::type;

template<class IT>
struct is_random_it {
	static constexpr bool value = std::is_same< typename std::iterator_traits<IT>::iterator_category, std::random_access_iterator_tag	>::value;
};

template<class U, class = typename U::iterator>
struct has_random_it {
	static constexpr bool value = is_random_it<typename U::iterator>::value;
};


}//_impl_irange

template<class T>
_impl_irange::irange_t<T> irange(_impl_irange::non_deduced_t<T> start, T end) {
	return _impl_irange::irange_t<T>{start, end};
}

template<class T, class = mart::enable_if_t<std::is_integral<T>::value> >
_impl_irange::irange_t<T> irange(T end) {
	return _impl_irange::irange_t<T>{0, end};
}

template<class C, class IT = typename C::iterator, class = mart::enable_if_t<std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<IT>::iterator_category>::value> >
_impl_irange::irange_t<typename C::size_type> irange(const C& container) {
	return _impl_irange::irange_t<typename C::size_type>{0, container.size()};
}



}