#ifndef LIB_MART_COMMON_GUARD_LINK_H
#define LIB_MART_COMMON_GUARD_LINK_H
/**
 * Link.h (mart-common)
 *
 * Copyright (C) 2022: Michael Balszun <michael.balszun@tum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:	Provides Link - a pointer that tracks target object through moves and deletion
 *
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>

namespace mart {

namespace detail {

template<class T>
struct Span {
	Span() noexcept  = default;
	T*          data = nullptr;
	std::size_t size = 0;

	T* begin() const { return data; };
	T* end() const { return data + size; };
};

} // namespace detail

struct LinkTargetBase;

struct LinkBase {
	LinkTargetBase* _target = nullptr;

	LinkBase() noexcept = default;

	LinkBase( LinkBase&& other ) noexcept;
	LinkBase( const LinkBase& other ) noexcept;

	LinkBase& operator=( LinkBase&& other ) noexcept;
	LinkBase& operator=( const LinkBase& other ) noexcept;
};

struct LinkTargetBase {
	friend LinkBase;

protected:
	detail::Span<LinkBase*> _back_ptrs;

	explicit LinkTargetBase( detail::Span<LinkBase*> back_ptrs ) noexcept
		: _back_ptrs( back_ptrs )
	{
	}

	void update_refs_after_move()
	{
		for( auto* back_ref : _back_ptrs ) {
			retarget_to_this( back_ref );
		}
	}

	~LinkTargetBase()
	{
		for( auto* link : _back_ptrs ) {
			if( link ) {
				link->_target = nullptr;
			}
		}
	}
	void retarget_to_this( LinkBase* link )
	{
		if( link ) {
			link->_target = this;
		}
	}

	bool update_back_ref( LinkBase* old_addr, LinkBase* new_addr )
	{
		auto it = std::find( _back_ptrs.begin(), _back_ptrs.end(), old_addr );
		if( it != _back_ptrs.end() ) {
			*it = new_addr;
			return true;
		} else {
			assert( false && "trying to update non-existing backref" );
			return false;
		}
	}

public:
	void sever_all_links() noexcept
	{
		for( auto*& back_ref : _back_ptrs ) {
			back_ref = nullptr;
		}
	}

	bool add_back_ref( LinkBase* new_addr )
	{
		auto it = std::find( _back_ptrs.begin(), _back_ptrs.end(), nullptr );
		if( it != _back_ptrs.end() ) {
			*it = new_addr;
			return true;
		} else {
			assert( false && "Backref buffer capacity exhausted" );
			return false;
		}
	}

	bool remove_back_ref( LinkBase* old_addr )
	{
		auto it = std::find( _back_ptrs.begin(), _back_ptrs.end(), old_addr );
		if( it != _back_ptrs.end() ) {
			*it = nullptr;
			return true;
		} else {
			assert( false && "trying to remove non-existing backref" );
			return false;
		}
	}
};

LinkBase::LinkBase( LinkBase&& other ) noexcept
	: _target( std::exchange(other._target,nullptr ))
{
	if( _target ) {
		_target->update_back_ref( &other, this );
	}
}

LinkBase::LinkBase( const LinkBase& other ) noexcept
{
	if( other._target ) {
		if( other._target->add_back_ref( this ) ) {
			_target = other._target;
		}
	}
}

LinkBase& LinkBase::operator=( LinkBase&& other ) noexcept
{
	if( _target ) {
		_target->remove_back_ref( this );
	}
	_target = std::exchange( other._target, nullptr );
	if( _target ) {
		_target->update_back_ref( &other, this );
	}
	return *this;
}
LinkBase& LinkBase::operator=( const LinkBase& other ) noexcept
{
	if( _target ) {
		_target->remove_back_ref( this );
	}
	_target = other._target;
	if( _target ) {
		_target->add_back_ref( this );
	}
	return *this;
}

template<std::size_t MaxLink>
struct LinkTarget : LinkTargetBase {
	std::array<LinkBase*, MaxLink> _back_ptr_store{};
	LinkTarget() noexcept
		: LinkTargetBase( {_back_ptr_store.data(), _back_ptr_store.size()} )
	{
	}

	// Do not retarget on copy construction and assignment
	LinkTarget( const LinkTarget& ) noexcept
		: LinkTarget()
	{
	}
	LinkTarget& operator=( const LinkTarget& ) { return *this; }

	// Retarget on moves
	LinkTarget( LinkTarget&& other ) noexcept
		: LinkTargetBase( {_back_ptr_store.data(), _back_ptr_store.size()} )
		, _back_ptr_store( other._back_ptr_store )
	{
		other._back_ptr_store.fill( nullptr );
		update_refs_after_move();
	}

	LinkTarget& operator=( LinkTarget&& other )
	{
		_back_ptr_store = other._back_ptr_store;
		other._back_ptr_store.fill( nullptr );

		update_refs_after_move();
		return *this;
	}
};

template<class T>
struct Link : LinkBase {
	Link() noexcept = default;
	explicit Link( T& target )
		: Link()
	{
		connect( target );
	}

	bool connect( T& target )
	{
		if( static_cast<LinkTargetBase&>( target ).add_back_ref( this ) ) {
			disconnect();
			_target = &target;
			return true;
		} else {
			return false;
		}
	}

	bool disconnect() noexcept
	{
		if( _target ) {
			_target->remove_back_ref( this );
			_target = nullptr;
			return true;
		} else {
			return false;
		}
	}

	T&       operator*() const { return *static_cast<T*>( _target ); }
	T*       operator->() const { return static_cast<T*>( _target ); }
	explicit operator bool() const { return _target != nullptr; }
};

} // namespace mdev

#endif
