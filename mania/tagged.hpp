//
//  tagged.hpp
//  aarc
//
//  Created by Antony Searle on 12/9/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#ifndef tagged_hpp
#define tagged_hpp

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "atomic.hpp"
#include "common.hpp"

namespace manic {

template<typename T>
union TaggedPtr {
    
    static constexpr usize TAG = alignof(T) - 1;
    static constexpr usize PTR = ~TAG;
    
    usize _raw;
    
    struct _ptr_t {
        
        usize _raw;
        
        T& as_mut() const {
            return *reinterpret_cast<T*>(_raw & PTR);
        }
        
        T const* operator->() const {
            return reinterpret_cast<T const*>(_raw & PTR);
        }
        
        T const& operator*() const {
            assert(_raw & PTR);
            return *reinterpret_cast<T const*>(_raw & PTR);
        }
        
        operator T const*() const {
            return reinterpret_cast<T const*>(_raw & PTR);
        }
        
        explicit operator bool() const {
            return _raw & PTR;
        }
        
        _ptr_t& operator=(T const* other) {
            assert(!(reinterpret_cast<usize>(other) & TAG));
            _raw = reinterpret_cast<usize>(other) | (_raw & TAG);
        }
        
        _ptr_t& operator=(std::nullptr_t) {
            _raw &= TAG;
        }
        
    } ptr;
    
    struct _tag_t {
        
        usize _raw;
        
        usize operator++(int) {
            usize old = _raw & TAG;
            _raw = (_raw & PTR) | ((_raw + 1) & TAG);
            return old;
        }
        
        usize operator--(int) {
            usize old = _raw & TAG;
            _raw = (_raw & PTR) | ((_raw - 1) & TAG);
            return old;
        }
        
        _tag_t& operator++() {
            _raw = (_raw & PTR) | ((_raw + 1) & TAG);
            return *this;
        }
        
        _tag_t& operator--() {
            _raw = (_raw & PTR) | ((_raw - 1) & TAG);
            return *this;
        }
        
        usize operator+() const {
            return +(_raw & TAG);
        }
        
        usize operator-() const {
            return -(_raw & TAG);
        }
        
        bool operator!() const {
            return !(_raw & TAG);
        }
        
        usize operator~() const {
            return ~(_raw & TAG);
        }
        
        operator usize() const {
            return _raw & TAG;
        }
        
        explicit operator bool() const {
            return static_cast<bool>(_raw & TAG);
        }
        
        _tag_t& operator=(usize other) {
            _raw = (_raw & PTR) | (other & TAG);
            return *this;
        }
        
        _tag_t& operator+=(usize other) {
            _raw = (_raw & PTR) | ((_raw + other) & TAG);
            return *this;
        }
        
        _tag_t& operator-=(usize other) {
            _raw = (_raw & PTR) | ((_raw - other) & TAG);
            return *this;
        }
        
        _tag_t& operator*=(usize other) {
            _raw = (_raw & PTR) | ((_raw * other) & TAG);
            return *this;
        }
        
        _tag_t& operator/=(usize other) {
            assert(other);
            _raw = (_raw & PTR) | ((_raw & TAG) / other);
            return *this;
        }
        
        _tag_t& operator%=(usize other) {
            assert(other);
            _raw = (_raw & PTR) | ((_raw & TAG) % other);
            return *this;
        }
        
        _tag_t& operator<<=(int other) {
            assert(other >= 0);
            _raw = (_raw & PTR) | ((_raw << other) & TAG);
            return *this;
        }
        
        _tag_t& operator>>=(int other) {
            assert(other >= 0);
            _raw = (_raw & PTR) | ((_raw & TAG) >> other);
            return *this;
        }
        
        _tag_t& operator&=(usize other) {
            _raw &= other | PTR;
            return *this;
        }
        
        _tag_t& operator^=(usize other) {
            _raw ^= other & TAG;
            return *this;
        }
        
        _tag_t& operator|=(usize other) {
            _raw |= other & TAG;
        }
        
    } tag;
    
    TaggedPtr() = default;
    explicit TaggedPtr(usize x) : _raw(x) {}
    explicit TaggedPtr(T const* p) : _raw(reinterpret_cast<usize>(p)) {}
    
    TaggedPtr operator++(int) {
        TaggedPtr tmp = *this;
        ++*this;
        return tmp;
    }
    
    TaggedPtr operator--(int) {
        TaggedPtr tmp = *this;
        ++*this;
        return tmp;
    }
    
    T const* operator->() const {
        return ptr.operator->();
    }
    
    TaggedPtr operator++() {
        ++tag;
        return *this;
    }
    
    TaggedPtr operator--() {
        --tag;
        return *this;
    }
    
    bool operator!() const {
        return !_raw;
    }
    
    TaggedPtr operator~() const {
        return TaggedPtr { _raw ^ TAG };
    }
    
    T const& operator*() const {
        return ptr.operator*();
    }
    
    
    TaggedPtr operator&(usize t) const {
        return TaggedPtr { _raw & (t | PTR) };
    }

    TaggedPtr operator^(usize t) const {
        return TaggedPtr { _raw ^ (t & TAG) };
    }

    TaggedPtr operator|(usize t) const {
        return TaggedPtr { _raw | (t & TAG) };
    }

};


template<typename T>
union Atomic<TaggedPtr<T>> {
    
    TaggedPtr<T> _value;
    mutable std::atomic<usize> _atomic;
    
public:
    
    Atomic() : _value() {}
    
    Atomic(TaggedPtr<T> x) : _value(x) {}
    
    Atomic(Atomic const&) = delete;
    Atomic(Atomic& other) : _value(other._value) {}
    Atomic(Atomic&& other) : _value(std::move(other._value)) {}
    
    ~Atomic() = default;
    
    Atomic& operator=(TaggedPtr<T> x) & { _value = x; return *this; }
    
    Atomic& operator=(Atomic const&) & = delete;
    Atomic& operator=(Atomic& other) & { _value = other._value; return *this; }
    Atomic& operator=(Atomic&& other) & { _value = std::move(other._value); return *this; }
    
    void swap(Atomic& other) & { using std::swap; swap(_value, other._value); }
    void swap(TaggedPtr<T>& x) & { using std::swap; swap(_value, x); }
    
    operator TaggedPtr<T>&() { return _value; }
    explicit operator bool() { return static_cast<bool>(_value); }
    
    // const-qualified
    
    TaggedPtr<T> load(std::memory_order) = delete; // <-- provide deleted mutable overloads to trap misuse of const methods
    TaggedPtr<T> load(std::memory_order order) const {
        return TaggedPtr<T> { _atomic.load(order) };
    }
    
    void store(TaggedPtr<T>, std::memory_order) = delete;
    void store(TaggedPtr<T> desired, std::memory_order order) const {
        _atomic.store(desired, order);
    }
    
    TaggedPtr<T> exchange(TaggedPtr<T>, std::memory_order) = delete;
    TaggedPtr<T> exchange(TaggedPtr<T> desired, std::memory_order order) const {
        return TaggedPtr<T> { _atomic.exchange(desired, order) };
    }
    
    bool compare_exchange_weak(TaggedPtr<T>&, TaggedPtr<T>, std::memory_order, std::memory_order) = delete;
    bool compare_exchange_weak(TaggedPtr<T>& expected,
                               TaggedPtr<T> desired,
                               std::memory_order success,
                               std::memory_order failure) const {
        return _atomic.compare_exchange_weak(expected._raw, desired._raw, success, failure);
    }
    
    bool compare_exchange_strong(TaggedPtr<T>&, TaggedPtr<T>, std::memory_order, std::memory_order) = delete;
    bool compare_exchange_strong(TaggedPtr<T>& expected,
                                 TaggedPtr<T> desired,
                                 std::memory_order success,
                                 std::memory_order failure) const {
        return _atomic.compare_exchange_strong(expected._raw, desired._raw, success, failure);
    }
    
    template<typename F>
    TaggedPtr<T> fetch_update(F&& f, std::memory_order order) {
        std::memory_order failure;
        switch (order) {
            case std::memory_order_release:
                failure = std::memory_order_relaxed;
                break;
            case std::memory_order_acq_rel:
                failure = std::memory_order_acquire;
                break;
            default:
                failure = order;
                break;
        }
        TaggedPtr<T> expected = load(failure);
        while (!compare_exchange_weak(expected,
                                      f(std::as_const(expected)),
                                      order,
                                      failure))
            ;
        return expected;
    }
    
    TaggedPtr<T> fetch_add(usize, std::memory_order) = delete;
    TaggedPtr<T> fetch_add(usize n, std::memory_order order) const {
        return fetch_update([=](TaggedPtr<T> expected) {
            return expected + n;
        }, order);
    }
    
    TaggedPtr<T> fetch_sub(usize, std::memory_order) = delete;
    TaggedPtr<T> fetch_sub(usize n, std::memory_order order) const {
        return fetch_update([=](TaggedPtr<T> expected) {
            return expected - n;
        }, order);
    }
    
    TaggedPtr<T> fetch_and(usize, std::memory_order) = delete;
    TaggedPtr<T> fetch_and(usize x, std::memory_order order) const {
        return TaggedPtr<T> { _atomic.fetch_and(x | TaggedPtr<T>::PTR, order) };
    }
    
    TaggedPtr<T> fetch_or(usize, std::memory_order) = delete;
    TaggedPtr<T> fetch_or(usize x, std::memory_order order) const {
        return TaggedPtr<T> { _atomic.fetch_or(x & TaggedPtr<T>::TAG, order) };
    }
    
    TaggedPtr<T> fetch_xor(usize, std::memory_order) = delete;
    TaggedPtr<T> fetch_xor(usize x, std::memory_order order) const {
        return TaggedPtr<T> { _atomic.fetch_xor(x & TaggedPtr<T>::TAG, order) };
    }
    
    void wait(TaggedPtr<T>, std::memory_order) noexcept = delete;
    void wait(TaggedPtr<T> old, std::memory_order order) const noexcept {
        std::atomic_wait_explicit(&_atomic, old._raw, order);
    }
    
    void notify_one() noexcept = delete;
    void notify_one() const noexcept {
        std::atomic_notify_one(&_atomic);
    }
    
    void notify_all() noexcept = delete;
    void notify_all() const noexcept {
        std::atomic_notify_all(&_atomic);
    }
    
};

} // namespace manic


#endif /* tagged_hpp */
