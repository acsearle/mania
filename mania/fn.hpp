//
//  fn.hpp
//  aarc
//
//  Created by Antony Searle on 12/8/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#ifndef fn_hpp
#define fn_hpp

#include <cassert>
#include <chrono>

#include "atomic.hpp"
#include "maybe.hpp"
#include "finally.hpp"

namespace manic {

// fn is a polymorphic function wrapper like std::function
//
// move-only (explicit maybe-clone)
// reference-counted
// ready to participate in intrusive lockfree data structures

using u64 = std::uint64_t;

namespace detail {

// packed_ptr layout:

static constexpr u64 CNT = 0xFFFF'0000'0000'0000;
static constexpr u64 PTR = 0x0000'FFFF'FFFF'FFF0;
static constexpr u64 TAG = 0x0000'0000'0000'000F;
static constexpr u64 INC = 0x0001'0000'0000'0000;

inline u64 cnt(u64 v) {
    return (v >> 48) + 1;
}

template<typename T>
struct node;

template<typename R, typename... Args>
struct alignas(16) node<R(Args...)> {
    
    inline static const Atomic<u64> _extant{0};
    
    //
    // layout:
    //
    //  0: __vtbl
    //  8: _raw_next + _atomic_next
    // 16: _count
    // 24: { _fd, _flags } + _t + _promise
        
    Atomic<u64> _next;
    Atomic<u64> _count;
        
    union {
        struct {
            int _fd;
            int _flags;
        };
        std::chrono::time_point<std::chrono::steady_clock> _t;
        Atomic<u64> _promise;
    };
    
    node()
    : _next{0}
    , _count{0}
    , _promise{0} {
        _extant.fetch_add(1, std::memory_order_relaxed);
    }
    
    node(node const&) = delete;
            
    virtual ~node() noexcept {
        auto n = _extant.fetch_sub(1, std::memory_order_relaxed);
        assert(n > 0);
    }
    
    node& operator=(node const&) = delete;

    virtual u64 try_clone() const {
        auto v = reinterpret_cast<u64>(new node);
        assert(!(v & ~PTR));
        return v;
    }

    void release(u64 n) const {
        auto m = _count.fetch_sub(n, std::memory_order_release);
        assert(m >= n);
        // printf("released %p to 0x%0.5llx (-0x%0.5llx)%s\n", this, m - n, n, (m == n) ? " <--" : "");
        if (m == n) {
            [[maybe_unused]] auto o = _count.load(std::memory_order_acquire); // <-- read to synchronize with release on other threads
            assert(o == 0);
            delete this;
        }
    }

    virtual R mut_call(Args...) { abort(); }
    virtual void erase() const noexcept {}

    virtual void erase_and_delete() const noexcept { delete this; }
    virtual void erase_and_release(u64 n) const noexcept { release(n); }
    
    virtual R mut_call_and_erase(Args...) { abort(); }
    virtual R mut_call_and_erase_and_delete(Args...) { abort(); }
    virtual R mut_call_and_erase_and_release(u64 n, Args...) { abort(); }

}; // node

template<typename F, typename T>
struct wrapper;

template<typename R, typename... Args, typename T>
struct wrapper<R(Args...), T> final : node<R(Args...)> {
        
    maybe<T> _payload;
                
    virtual ~wrapper() noexcept override final = default;
    
    virtual R mut_call(Args... args) override final {
        return _payload.value(std::forward<Args>(args)...);
    }
    
    virtual void erase() const noexcept override final {
        _payload.erase();
    }
    
    virtual R mut_call_and_erase(Args... args) override final {
        auto guard = finally([=]{ erase(); });
        return mut_call(std::forward<Args>(args)...);
    }
    
    virtual void erase_and_delete() const noexcept override final {
        _payload.erase();
        delete this;
    };

    virtual void erase_and_release(u64 n) const noexcept override final {
        _payload.erase();
        this->release(n); // <-- encourage inlined destructor as part of same call
    };

    virtual R mut_call_and_erase_and_delete(Args... args) override final {
        auto guard = finally([=]{ erase_and_delete(); });
        return mut_call(std::forward<Args>(args)...);
    }

    virtual R mut_call_and_erase_and_release(u64 n, Args... args) override final {
        auto guard = finally([=]{ erase_and_release(n); });
        return mut_call(std::forward<Args>(args)...);
    }

    virtual u64 try_clone() const override final {
        if constexpr (std::is_copy_constructible_v<T>) {
            auto p = new wrapper;
            p->_payload.emplace(_payload.value);
            auto v = reinterpret_cast<u64>(p);
            assert(!(v & ~PTR));
            return (u64) p;
        } else {
            return 0;
        }
        
    }

};

} // namespace detail

template<typename> struct fn;

template<typename R, typename... Args>
struct fn<R(Args...)> {

    static constexpr auto PTR = detail::PTR;
    static constexpr auto TAG = detail::TAG;

    u64 _value;

    static detail::node<R(Args...)>* ptr(u64 v) {
        return reinterpret_cast<detail::node<R(Args...)>*>(v & PTR);
    }
    
    fn() : _value{0} {}

    fn(fn const&) = delete;
    fn(fn&) = delete;
    fn(fn&& other) : _value(std::exchange(other._value, 0)) {}
    fn(fn const&&) = delete;

    template<typename T, typename = std::void_t<decltype(std::declval<T>()())>>
    fn(T f) {
        auto p = new detail::wrapper<R(Args...), T>;
        p->_payload.emplace(std::forward<T>(f));
        _value = reinterpret_cast<u64>(p);
        assert(!(_value & ~PTR));
    }
    
    explicit fn(std::uint64_t value)
    : _value(value) {
    }
    
    fn try_clone() const {
        auto p = ptr(_value);
        u64 v = 0;
        if (p)
            v = p->try_clone();
        return fn(v);
    }

    detail::node<R>* get() {
        return ptr(_value);
    }

    detail::node<R> const* get() const {
        return ptr(_value);
    }

    ~fn() {
        if (auto p = ptr(_value))
            p->erase_and_delete();
    }
    
    void swap(fn& other) {
        using std::swap;
        swap(_value, other._value);
    }

    fn& operator=(fn const&) = delete;
    
    fn& operator=(fn&& other) {
        fn(std::move(other)).swap(*this);
        return *this;
    }
    
    R operator()(Args... args) {
        auto p = ptr(_value);
        assert(p);
        _value = 0;
        if constexpr (std::is_same_v<R, void>) {
            p->mut_call_and_erase_and_delete(std::forward<Args>(args)...);
            return;
        } else {
            R r{p->mut_call_and_erase_and_delete(std::forward<Args>(args)...)};
            return r;
        }
    }
    
    operator bool() const {
        return ptr(_value);
    }
    
    std::uint64_t tag() const {
        return _value & detail::TAG;
    }
        
    detail::node<R(Args...)>* operator->() {
        return ptr(_value);
    }

    detail::node<R(Args...)> const* operator->() const {
        return ptr(_value);
    }

};

}

#endif /* fn_hpp */
