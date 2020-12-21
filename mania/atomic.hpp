//
//  atomic.hpp
//  mania
//
//  Created by Antony Searle on 22/9/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#ifndef atomic_hpp
#define atomic_hpp

#include <atomic>
#include <cassert>

#include "atomic_wait.hpp"
#include "common.hpp"

namespace manic {

// remove any release ordering from a memory order to make it suitable for a
// pure load operation
constexpr std::memory_order load_memory_order(std::memory_order order) {
    switch (order) {
    case std::memory_order::acq_rel:
        return std::memory_order::acquire;
    case std::memory_order::release:
        return std::memory_order::relaxed;
    default:
        return order;
    }
}

// remove any acquire (or consume) ordering from a memory order to make it
// suitable for a pure store operation
constexpr std::memory_order store_memory_order(std::memory_order order) {
    switch (order) {
    case std::memory_order::consume:
        [[fallthrough]];
    case std::memory_order::acquire:
        return std::memory_order::relaxed;
    case std::memory_order::acq_rel:
        return std::memory_order::release;
    default:
        return order;
    }
}

template<typename T>
constexpr auto nand(T a, T b) {
    return ~(a & b);
}

// Rust-style atomic that uses constness as a guide to switch between atomic
// and non-atomic operations.  Mutable overloads of atomic const member
// functions are deleted to enforce correct usage

template<typename T>
union Atomic {
    
    T _value;
    mutable std::atomic<T> _atomic;
    
public:
    
    Atomic() : _value() {}
    
    Atomic(T x) : _value(x) {}
    
    Atomic(Atomic const&) = delete;
    Atomic(Atomic& other) : _value(other._value) {}
    Atomic(Atomic&& other) : _value(std::move(other._value)) {}
    
    ~Atomic() = default;
    
    Atomic& operator=(T x) & { _value = x; return *this; }
    
    Atomic& operator=(Atomic const&) & = delete;
    Atomic& operator=(Atomic& other) & { _value = other._value; return *this; }
    Atomic& operator=(Atomic&& other) & { _value = std::move(other._value); return *this; }
    
    void swap(Atomic& other) & { using std::swap; swap(_value, other._value); }
    void swap(T& x) & { using std::swap; swap(_value, x); }
    
    operator T&() { return _value; }
    explicit operator bool() { return static_cast<bool>(_value); }
    
    // const-qualified
    
    T load(std::memory_order) = delete; // <-- provide deleted mutable overloads to trap misuse of const methods
    T load(std::memory_order order) const {
        return _atomic.load(order);
    }
    
    void store(T, std::memory_order) = delete;
    void store(T x, std::memory_order order) const {
        _atomic.store(x, order);
    }
    
    T exchange(T, std::memory_order) = delete;
    T exchange(T x, std::memory_order order) const {
        return _atomic.exchange(x, order);
    }
    
    bool compare_exchange_weak(T&, T, std::memory_order, std::memory_order) = delete;
    bool compare_exchange_weak(T& expected, T desired, std::memory_order success, std::memory_order failure) const {
        return _atomic.compare_exchange_weak(expected, desired, success, failure);
    }
    
    bool compare_exchange_strong(T&, T, std::memory_order, std::memory_order) = delete;
    bool compare_exchange_strong(T& expected, T desired, std::memory_order success, std::memory_order failure) const {
        return _atomic.compare_exchange_strong(expected, desired, success, failure);
    }
    
    T fetch_add(T, std::memory_order) = delete;
    T fetch_add(T x, std::memory_order order) const {
        return _atomic.fetch_add(x, order);
    }
    
    T fetch_sub(T, std::memory_order) = delete;
    T fetch_sub(T x, std::memory_order order) const {
        return _atomic.fetch_sub(x, order);
    }
    
    T fetch_and(T, std::memory_order) = delete;
    T fetch_and(T x, std::memory_order order) const {
        return _atomic.fetch_and(x, order);
    }
    
    T fetch_or(T, std::memory_order) = delete;
    T fetch_or(T x, std::memory_order order) const {
        return _atomic.fetch_or(x, order);
    }
    
    T fetch_xor(T, std::memory_order) = delete;
    T fetch_xor(T x, std::memory_order order) const {
        return _atomic.fetch_xor(x, order);
    }
    
    void wait(T, std::memory_order) noexcept = delete;
    void wait(T old, std::memory_order order) const noexcept {
        if constexpr(std::is_integral_v<T> || std::is_pointer_v<T>) {
            std::atomic_wait_explicit(&_atomic, old, order);
        } else {
            using U = typename _uint_t<sizeof(T) * 8>::type;
            std::atomic_wait_explicit((std::atomic<U>*) &_atomic,
                                      (U&) old,
                                      order);
        }
    }
    
    void notify_one() noexcept = delete;
    void notify_one() const noexcept {
        if constexpr(std::is_integral_v<T> || std::is_pointer_v<T>) {
            std::atomic_notify_one(&_atomic);
            
        } else {
            using U = typename _uint_t<sizeof(T) * 8>::type;
            std::atomic_notify_one((std::atomic<U>*) &_atomic);
        }
    }
    
    void notify_all() noexcept = delete;
    void notify_all() const noexcept {
        if constexpr(std::is_integral_v<T> || std::is_pointer_v<T>) {
            std::atomic_notify_all(&_atomic);
        } else {
            using U = typename _uint_t<sizeof(T) * 8>::type;
            std::atomic_notify_all((std::atomic<U>*) &_atomic);
        }
    }
    
};

template<typename T>
void swap(Atomic<T>& a, Atomic<T>& b) {
    a.swap(b);
}

template<typename T>
void swap(Atomic<T>& a, T& b) {
    a.swap(b);
}

template<typename T>
void swap(T& a, Atomic<T>& b) {
    b.swap(a);
}

} // namespace manic

#endif /* atomic_hpp */
