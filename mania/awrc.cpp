//
//  awrc.cpp
//  mania
//
//  Created by Antony Searle on 2/12/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include <atomic>
#include <utility>
#include <thread>
#include "common.hpp"

#include <unistd.h>

namespace manic {

template<typename T>
struct weighted_ptr {
    
    constexpr static u64 MASK = 0x0000FFFFFFFFFFFF;
    
    struct manager {
        std::atomic<u64> _count;
        T _payload;
        template<typename... Args>
        explicit manager(Args&&... args)
        : _count(0x10000)
        , _payload(std::forward<Args>(args)...) {
        }
    };
    
    u64 _value;
    
    weighted_ptr()
    : _value(0) {
    }
    
    weighted_ptr(weighted_ptr const& x)
    : _value(x._value & MASK) {
        if (_value) {
            manager* p = reinterpret_cast<manager*>(_value & MASK);
            p->_count.fetch_add(0x10000, std::memory_order_relaxed);
            _value &= 0xFFFF;
        }
    }
    
    weighted_ptr(weighted_ptr&& x)
    : _value(std::exchange(x._value, 0)) {
    }
    
    weighted_ptr(weighted_ptr& x) {
        if (x._value & ~MASK) {
            x._value -= (MASK + 1);
            _value = (x._value & MASK) | (MASK + 1);
        } else {
            manager* p = reinterpret_cast<manager*>(_value & MASK);
            p->_count.fetch_add(0x1FFFF, std::memory_order_relaxed);
            x._value &= 0xFFFF;
            _value = x._value;
        }
    }
    ~weighted_ptr() {
        if (_value & MASK) {
            manager* p = reinterpret_cast<manager*>(_value & MASK);
            u64 n = (_value >> 48) + 1;
            if (p->_count.fetch_sub(n, std::memory_order_release) == n) {
                std::atomic_thread_fence(std::memory_order_acquire);
                delete p;
            }
        }
    }
    
    void swap(weighted_ptr& x) {
        std::swap(_value, x._value);
    }
    
    weighted_ptr& operator=(weighted_ptr const& x);
    weighted_ptr& operator=(weighted_ptr&& x);
    weighted_ptr& operator=(weighted_ptr& x);
    
    template<typename... Args>
    static weighted_ptr from(Args&&... args) {
        weighted_ptr x;
        manager* y = new manager(std::forward<Args>(args)...);
        x._value = reinterpret_cast<manager*>(y);
        assert(!(x._value & ~MASK));
        x._value |= 0xFFFF000000000000;
    }
    
    T const* operator->() const {
        return &reinterpret_cast<manager*>(_value & MASK)->_payload;
    }
    
    T const& operator*() const {
        return reinterpret_cast<manager*>(_value & MASK)->_payload;
    }

};

template<typename T>
weighted_ptr<T>& weighted_ptr<T>::operator=(weighted_ptr<T> const& x) {
    weighted_ptr{x}.swap(*this);
    return *this;
}

template<typename T>
weighted_ptr<T>& weighted_ptr<T>::operator=(weighted_ptr<T>&& x) {
    weighted_ptr<T>{std::move(x)}.swap(*this);
    return *this;
}

template<typename T>
weighted_ptr<T>& weighted_ptr<T>::operator=(weighted_ptr<T>& x) {
    weighted_ptr<T>{x}.swap(*this);
    return *this;
}

template<typename T>
struct atomic_weighted_ptr {
    
    std::atomic<u64> _value;
    
    atomic_weighted_ptr() : _value(0) {}
    
    atomic_weighted_ptr(atomic_weighted_ptr const&) = delete;
    
    ~atomic_weighted_ptr() {
        weighted_ptr<T> p;
        p._value = _value.load(std::memory_order_acquire);
    }
    
    explicit atomic_weighted_ptr(weighted_ptr<T>&& x)
    : _value(std::exchange(x._value, 0)) {
    }
    
    atomic_weighted_ptr& operator=(atomic_weighted_ptr const&) = delete;
    
    atomic_weighted_ptr& operator=(weighted_ptr<T>&& x) {
        store(std::move(x));
        return *this;
    }
    
    void store(weighted_ptr<T>&& x) {
        exchange(std::move(x));
        return *this;
    }
    
    weighted_ptr<T> exchange(weighted_ptr<T>&& x) {
        weighted_ptr<T> y{std::move(x)};
        y._value = _value.exchange(y._value, std::memory_order_acq_rel);
        return y;
    }
    
    weighted_ptr<T> load() {

        using weighted_ptr<T>::MASK;

        usize x = _value.load(std::memory_order_relaxed);
        usize y = 0;

        for(;;) {
            if (x & ~MASK) {
                y = x - 0x0001'0000'0000'0000;
                if (_value.compare_exchange_weak(x, y, std::memory_order_acquire, std::memory_order_relaxed))
                    break;
            } else {
                std::this_thread::yield();
                x = _value.load(std::memory_order_relaxed);
            }
        }
        if (!(y & weighted_ptr<T>::MASK)) {
            // No more loads can be done.
            auto p = reinterpret_cast<typename weighted_ptr<T>::manager*>(y);
            // We now have two leases on the variable with 1 count each.
            // We need to add more weight to the atomic variable, but another
            // thread might have already changed it.
            // So we only acquire as much extra weight as we can stash into
            // just the result
            // We can add a maxium of 0xFFFF to the value we return.
            
            p->_count.fetch_add(0xFFFF);
            x = y | 0xFFFF'0000'0000'0000;
            weighted_ptr<T> q;
            if (_value.compare_exchange_strong(y, x, std::memory_order_release, std::memory_order_relaxed)) {
                // success, we installed more weight into the same pointer
                q._value = y;
                return q;
            } else {
                // failure, somebody else changed the pointer first and it is
                // no longer our problem
                q._value = x;
                return q;
            }
        }
        weighted_ptr<T> q;
        q._value = x & MASK;
    }
    
};

} // namespace manic
