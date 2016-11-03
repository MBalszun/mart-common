/*
 * CopyableAtomic.h
 *
 *  Created on: Dec 16, 2015
 *      Author: balszun
 */

#ifndef SRC_UTILS_COPYABLEATOMIC_H_
#define SRC_UTILS_COPYABLEATOMIC_H_
#pragma once

/* ######## INCLUDES ######### */
/* Standard Library Includes */
#include <atomic>

/* Proprietary Library Includes */

/* Project Includes */

namespace mart {

/**
 * Drop in replacement for std::atomic that provides a copy constructor and copy assignment operator.
 *
 * Contrary to normal atomics, these atomics don't prevent the generation of
 * default constructor and copy operators for classes they are members of.
 *
 * Copying those atomics is thread safe, but be aware that
 * it doesn't provide any form of synchronization.
 * Their main purpose is to easily enable classes that have those atomics as members to
 * be copied around during setup, while for actually synchronization the normal member functions are used.
 */
template<class T>
class CopyableAtomic : public std::atomic<T>
{
public:
    //defaultinitializes value
    constexpr CopyableAtomic() :
        std::atomic<T>(T{})
    {}

    constexpr CopyableAtomic(T desired) :
        std::atomic<T>(desired)
    {}

    constexpr CopyableAtomic(const CopyableAtomic<T>& other) :
        CopyableAtomic(other.load(std::memory_order_relaxed))
    {}

    CopyableAtomic& operator=(const CopyableAtomic<T>& other) {
        this->store(other.load(std::memory_order_relaxed), std::memory_order_relaxed);
        return *this;
    }
};

}

#endif /* SRC_UTILS_COPYABLEATOMIC_H_ */
