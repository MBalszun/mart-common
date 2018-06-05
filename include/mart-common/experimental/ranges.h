#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_RANGES_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_RANGES_H
/**
 * ranges.h (mart-common/experimetnal)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	extensions to the facilities in the main ranges.h header that need more development
 *
 */

#include "../enum/EnumHelpers.h"
#include "../ranges.h"

namespace mart {
namespace experimental {

template<class It>
struct range {
	It _begin;
	It _end;

	constexpr It begin() const { return _begin; }
	constexpr It end() const { return _end; }
};

template<class R>
auto view_reversed( R&& r ) -> range<std::reverse_iterator<decltype( r.begin() )>>
{
	using Rit = std::reverse_iterator<decltype( r.begin() )>;
	return {Rit{r.end()}, Rit{r.begin()}};
}

template<class Enum, class Ut = mart::underlying_type_t<Enum>>
class DefaultEnumRange {
public:
	constexpr Enum operator[]( int i ) const { return static_cast<Enum>( i ); }

	struct Iterator {
		constexpr Iterator( Enum e )
			: idx{toUType( e )}
		{
		}
		constexpr Enum		   operator*() const { return static_cast<Enum>( idx ); }
		/*constexpr*/ Iterator operator++() { return ++idx; }
		Iterator			   operator++( int )
		{
			auto t = *this;
			idx++;
			return t;
		}
		friend bool operator!=( Iterator l, Iterator r ) { return l.idx != r.idx; }
		friend bool operator!=( Iterator l, Iterator r ) { return l.idx == r.idx; }

	private:
		Ut idx;
	};
	constexpr Iterator begin() const { return Iterator{first}; }
	constexpr Iterator end() const { return Iterator{static_cast<Enum>( static_cast<Ut>( last ) + Ut{1} )}; }
	const Enum		   first;
	const Enum		   last;
};

namespace _impl_frange {
template<class T>
struct frange_t;
}

template<class T>
class FIterator {
	static_assert( std::is_floating_point<T>::value, "FIterator can only be used for floating point values" );

public:
	using difference_type   = std::ptrdiff_t;
	using value_type		= T;
	using pointer			= T*;
	using reference			= const T&;
	using iterator_category = std::random_access_iterator_tag;

	FIterator( T value = T(), T step = T( 1.0 ) )
		: i{0}
		, offset{value}
		, step{step}
	{
	}

	value_type operator*() const { return i * step + offset; }
	// pointer operator->() const { return &i; }

	FIterator& operator++()
	{
		++i;
		return *this;
	}
	FIterator operator++( int )
	{
		auto tmp = *this;
		i++;
		return tmp;
	}

	FIterator& operator--()
	{
		--i;
		return *this;
	}
	FIterator operator--( int )
	{
		auto tmp = *this;
		i--;
		return tmp;
	}

	FIterator& operator+=( difference_type diff )
	{
		i += diff;
		return *this;
	}
	FIterator& operator-=( difference_type diff )
	{
		i -= diff;
		return *this;
	}

	value_type operator[]( difference_type diff ) const { return i + diff; }

	friend bool operator!=( FIterator l, FIterator r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i != r.i;
	}
	friend bool operator==( FIterator l, FIterator r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i == r.i;
	}
	friend bool operator<( FIterator l, FIterator r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i < r.i;
	}
	friend bool operator<=( FIterator l, FIterator r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i <= r.i;
	}
	friend bool operator>( FIterator l, FIterator r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i > r.i;
	}
	friend bool operator>=( FIterator l, FIterator r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i >= r.i;
	}

	friend FIterator operator+( FIterator it, difference_type n )
	{
		it.i += n;
		return it;
	}
	friend FIterator operator+( difference_type n, FIterator it ) { return it + n; }

	friend difference_type operator-( FIterator l, FIterator r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i - r.i;
	}
	friend FIterator operator-( FIterator it, difference_type n )
	{
		it.i -= n;
		return it;
	}
	friend FIterator operator-( difference_type n, FIterator it ) { return it - n; }

private:
	template<class>
	friend struct _impl_frange::frange_t;

	std::ptrdiff_t i;
	T			   offset;
	T			   step;
};

namespace _impl_frange {
template<class T>
struct frange_t {
	static_assert( std::is_floating_point<T>::value, "frange can only be used for floating point values" );
	using iterator = FIterator<T>;

	iterator begin() const { return iterator{_start_v, _step}; };
	iterator end() const
	{
		iterator ret{_start_v, _step};
		ret.i = static_cast<std::ptrdiff_t>( ( _end_v - _start_v ) / _step ) + 1;
		return ret;
	};

	frange_t( T start, T end, T step = T( 1.0 ) )
		: _start_v{start}
		, _end_v{end}
		, _step{step}
	{
	}

	std::ptrdiff_t size() const { return static_cast<std::ptrdiff_t>( ( _end_v - _start_v ) / _step ) + 1; }

	T		 _start_v;
	T		 _end_v;
	T		 _step;
	frange_t step( T value ) const
	{
		frange_t n( *this );
		n._step = value;
		return n;
	}
};
} // namespace _impl_frange

template<class T>
_impl_frange::frange_t<T> frange( _impl_irange::non_deduced_t<T> start, T end )
{
	return _impl_frange::frange_t<T>{start, end};
}

namespace _impl_vrange {

template<class T>
class VIterator_rnd {
public:
	using difference_type   = std::ptrdiff_t;
	using value_type		= T;
	using pointer			= T*;
	using reference			= const T&;
	using iterator_category = std::random_access_iterator_tag;

	VIterator_rnd() = default;

	VIterator_rnd( T rng_start, T rng_step, T value )
		: offset{rng_start}
		, step{rng_step}
		, i{value_to_idx( rng_start, rng_step, value )}
	{
	}

	value_type operator*() const
	{
#ifdef MSC_VER
#pragma warning( push )
#pragma warning( disable : 4244 )
		// we have to suppress the warning about implicit conversion from ptrdiff_t to double here,
		// as it depends on the actual type of step, whether that conversion happens atall
		// (so an explicit cast might result in the wrong type)
#endif
		return step * i + offset;
#ifdef MSC_VER
#pragma warning( pop )
#endif
	}
	// pointer operator->() const { return &i; }

	VIterator_rnd& operator++()
	{
		++i;
		return *this;
	}
	VIterator_rnd operator++( int )
	{
		auto tmp = *this;
		i++;
		return tmp;
	}

	VIterator_rnd& operator--()
	{
		--i;
		return *this;
	}
	VIterator_rnd operator--( int )
	{
		auto tmp = *this;
		i--;
		return tmp;
	}

	VIterator_rnd& operator+=( difference_type diff )
	{
		i += diff;
		return *this;
	}
	VIterator_rnd& operator-=( difference_type diff )
	{
		i -= diff;
		return *this;
	}

	value_type operator[]( difference_type diff ) const { return i + diff; }

	friend bool operator!=( VIterator_rnd l, VIterator_rnd r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i != r.i;
	}
	friend bool operator==( VIterator_rnd l, VIterator_rnd r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i == r.i;
	}
	friend bool operator<( VIterator_rnd l, VIterator_rnd r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i < r.i;
	}
	friend bool operator<=( VIterator_rnd l, VIterator_rnd r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i <= r.i;
	}
	friend bool operator>( VIterator_rnd l, VIterator_rnd r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i > r.i;
	}
	friend bool operator>=( VIterator_rnd l, VIterator_rnd r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i >= r.i;
	}

	friend VIterator_rnd operator+( VIterator_rnd it, difference_type n )
	{
		it.i += n;
		return it;
	}
	friend VIterator_rnd operator+( difference_type n, VIterator_rnd it ) { return it + n; }

	friend difference_type operator-( VIterator_rnd l, VIterator_rnd r )
	{
		assert( l.offset == r.offset && l.step == r.step );
		return l.i - r.i;
	}
	friend VIterator_rnd operator-( VIterator_rnd it, difference_type n )
	{
		it.i -= n;
		return it;
	}
	friend VIterator_rnd operator-( difference_type n, VIterator_rnd it ) { return it - n; }

private:
	static std::ptrdiff_t value_to_idx( T rng_start, T rng_step, T value )
	{
		if( value == rng_start ) {
			return 0;
		} else {
			return static_cast<std::ptrdiff_t>( ( value - rng_start ) / rng_step );
		}
	}
	T offset;
	T step;

	std::ptrdiff_t i;
};

template<class T>
class VIterator_fwd {
public:
	using difference_type   = std::ptrdiff_t;
	using value_type		= T;
	using pointer			= T*;
	using reference			= const T&;
	using iterator_category = std::forward_iterator_tag;

	constexpr VIterator_fwd() = default;
	constexpr VIterator_fwd( T rng_start, T rng_step, T value )
		: value{rng_start}
		, step{rng_step}
	{
		(void)value;
	}

	value_type operator*() const { return value; }
	pointer	operator->() const { return &value; }

	VIterator_fwd& operator++()
	{
		value += step;
		return *this;
	}
	VIterator_fwd operator++( int )
	{
		auto tmp = *this;
		value += step;
		return tmp;
	}

	friend bool operator!=( VIterator_fwd l, VIterator_fwd r ) { return l.value != r.value; }
	friend bool operator==( VIterator_fwd l, VIterator_fwd r ) { return l.value == r.value; }

private:
	T value;
	T step;
};

template<class T>
constexpr bool can_perform_arithmetic( T* )
{
	return false;
}

template<class T, class = decltype( static_cast<std::ptrdiff_t>( ( T{} - T{} ) / T{} ) )>
constexpr bool can_perform_arithmetic( std::nullptr_t )
{
	return true;
}

template<class T>
constexpr bool can_perform_arithmetic()
{
	return can_perform_arithmetic<T>( nullptr );
}

template<class T>
using VIterator = std::conditional_t<can_perform_arithmetic<T>(), VIterator_rnd<T>, VIterator_fwd<T>>;

template<class T>
struct vrange_t {
	using iterator = VIterator<T>;

	iterator begin() const { return iterator{_start_v, _step, _start_v}; };
	iterator end() const { return iterator{_start_v, _step, _end_v + _step}; };

	vrange_t( T start, T end )
		: _start_v{start}
		, _end_v{end}
	{
	}

	vrange_t step( T value ) const
	{
		vrange_t n( *this );
		n._step = value;
		return n;
	}

	T _start_v;
	T _end_v;
	T _step{};
};

} // namespace _impl_vrange

template<class T>
_impl_vrange::vrange_t<T> vrange( _impl_irange::non_deduced_t<T> start, T end )
{
	return _impl_vrange::vrange_t<T>{start, end};
}

} // namespace experimental
} // namespace mart

#endif
