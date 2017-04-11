#ifndef LIB_MART_COMMON_GUARD_EXPERIMENTAL_STORAGE_POOL_H
#define LIB_MART_COMMON_GUARD_EXPERIMENTAL_STORAGE_POOL_H
/**
 * StoragePool.h (mart-common/experimetnal)
 *
 * Copyright (C) 2015-2017: Michael Balszun <michael.balszun@mytum.de>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See either the LICENSE file in the library's root
 * directory or http://opensource.org/licenses/MIT for details.
 *
 * @author: Michael Balszun <michael.balszun@mytum.de>
 * @brief:  Provides a storage pool class that allows refcounted management of objects either on the heap or stack
 *
 */

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <array>
#include <atomic>
#include <mutex>
#include <type_traits>

/* Proprietary Library Includes */
#include "../ArrayView.h"
#include "../algorithm.h"
#include "../utils.h"
#include "Optional.h"

/* Project Includes */
#include "RefCntPtr.h"
/* ~~~~~~~~ INCLUDES ~~~~~~~~~ */

namespace mart {
namespace experimental {

namespace detail {
namespace object_store {

template <class T, mart::Synchonized IsSync = Synchonized::True>
struct StorageSlot {
	using Mem_t = typename std::aligned_storage<sizeof( T ), alignof( T )>::type;
	using Cnt_t = std::atomic<int>;

	Cnt_t cnt{-1}; // -1 -> free (and not initialized), 0-> reserved but not initialized
	Mem_t mem;

	T*   rcContent() { return reinterpret_cast<T*>( &mem ); }
	void rcInc() { cnt.fetch_add( 1, std::memory_order_relaxed ); }
	void rcDec()
	{
		if( cnt.fetch_sub( 1, std::memory_order_acq_rel ) == 1 ) {
			rcContent()->~T();
			cnt.store( -1, std::memory_order_release );
		}
	}
};

template <class T>
struct StorageSlot<T, mart::Synchonized::False> {
	using Mem_t = typename std::aligned_storage<sizeof( T ), alignof( T )>::type;
	using Cnt_t = int;

	Cnt_t cnt{-1};
	Mem_t mem;

	T*   rcContent() { return reinterpret_cast<T*>( &mem ); }
	void rcInc() { ++cnt; }
	void rcDec()
	{
		if( --cnt == 0 ) {
			rcContent()->~T();
			cnt = -1;
		}
	}
};
}
}

template <class T, mart::Synchonized IsSync = Synchonized::True>
class Ref;

template <class T, mart::Synchonized IsSync = Synchonized::True>
class ConstRef;

// This is the base class that doesn't care, where the storage for the elements comes from (could be vector, std::array,
// plain array ...)
template <class T, mart::Synchonized IsSynced = Synchonized::True>
class ObjectStore_Base {
public:
	using Slot_t = detail::object_store::StorageSlot<T, IsSynced>;

	template <class... ARGS>
	mart::Optional<ConstRef<T>> construct( ARGS&&... args )
	{
		auto* entry = this->alloc();
		if( entry ) {
			new( entry->rcContent() ) T{std::forward<ARGS>( args )...};
			return ConstRef<T>{entry};
		} else {
			return {};
		}
	}

	template <class... ARGS>
	mart::Optional<Ref<T>> constructMutable( ARGS&&... args )
	{
		auto* entry = this->alloc();
		if( entry ) {
			new( entry->rcContent() ) T{std::forward<ARGS>( args )...};
			return Ref<T>{entry};
		} else {
			return {};
		}
	}

protected:
	ObjectStore_Base( mart::ArrayView<Slot_t> store )
		: _storageArea{store}
	{
	}



	Slot_t* alloc()
	{
		const auto isFree = []( const Slot_t& e ) { return e.cnt.load( std::memory_order_relaxed ) == -1; };

		// TODO: Find better algorithm e.g. with wrapp around and using thread id as starting point
		auto it = mart::find_if(_storageArea, isFree);

		while (it != _storageArea.end() && !my_conpare_exchange_strong(it->cnt,-1, 0)) {
			it = mart::find_if(_storageArea, isFree);
		}

		if (it == _storageArea.end()) {
			return nullptr;
		} else {
			return &*it;
		}
	}

	// Danger: This does not perform deallocation on the old storage area
	void					storageArea( mart::ArrayView<Slot_t> newArea ) { _storageArea = newArea; }
	mart::ArrayView<Slot_t> storageArea() { return _storageArea; }

private:
	mart::ArrayView<Slot_t> _storageArea;

	template<class U>
	static bool my_conpare_exchange_strong(std::atomic<U>& a, U expected, U newValue) {
		return a.compare_exchange_strong(expected, newValue);
	}
};


template <class T, size_t N>
class ObjectStore : public ObjectStore_Base<T> {

public:
	ObjectStore()
		: ObjectStore_Base<T>( _data )
	{
	}

private:
	std::array<typename ObjectStore_Base<T>::Slot_t, N> _data{};
};

template <class T, mart::Synchonized IsSynced>
class Ref {
	using Target_t = typename ObjectStore_Base<T, IsSynced>::Slot_t;

public:
	RcPtr<Target_t> ptr;
	Ref() = default;
	Ref( Target_t* e )
		: ptr{e}
	{
	}
	T& operator*() const { return *ptr.get(); }
	T* operator->() const { return ptr.get(); }
	explicit operator bool() const { return static_cast<bool>( ptr ); }

	ConstRef<T> makeImmutable() && { return ConstRef<T>( std::move( *this ) ); }
};

template <class T, mart::Synchonized IsSynced>
class ConstRef {
	using Target_t = typename ObjectStore_Base<T, IsSynced>::Slot_t;

public:
	RcPtr<Target_t> ptr;
	constexpr ConstRef() = default;
	constexpr ConstRef( Target_t* e )
		: ptr{e}
	{
	}
	ConstRef( Ref<T>&& other )
		: ptr{std::move( other.ptr )}
	{
	}
	const T& operator*() const { return *ptr.get(); }
	const T* operator->() const { return ptr.get(); }
	explicit operator bool() const { return static_cast<bool>( ptr ); }
};
}
}

#endif
