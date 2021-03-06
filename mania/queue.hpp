//
//  queue.hpp
//  aarc
//
//  Created by Antony Searle on 12/8/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#ifndef queue_hpp
#define queue_hpp

#include <deque>

#include "atomic.hpp"
#include "common.hpp"
#include "maybe.hpp"

namespace manic {

template<typename T>
struct queue {
    
    std::deque<T> _deque;
    
    bool empty() const {
        return _deque.empty();
    }
    
    template<typename... Args>
    void push(Args... args) {
        _deque.emplace_back(std::forward<Args>(args)...);
    }
    
    T pop() {
        assert(!_deque.empty());
        T tmp{std::move(_deque.front())};
        _deque.pop_front();
        return tmp;
    }
    
};

template<typename T>
union Atomic<queue<T>> {
    
    static constexpr u64 PTR = 0x0000'FFFF'FFFF'FFF0;
    static constexpr u64 CNT = 0xFFFF'0000'0000'0000;
    static constexpr u64 INC = 0x0001'0000'0000'0000;
    static constexpr u64 LOW = 0x0000'0000'0000'FFFF;
    
    struct node {
        
        Atomic<i64> _count;
        Atomic<u64> _next;
        maybe<T> _payload;
        
        void erase() const {
            _payload.erase();
        }
        void release(u64 n) const {
            auto m = _count.fetch_sub(n, std::memory_order_release);
            assert(m >= n);
            if (m == n) {
                m = _count.load(std::memory_order_acquire); // synch with releases
                assert(m == 0);
                delete this;
            }
        }
        void erase_and_release(u64 n) const {
            erase();
            release(n);
        }
    };
    
    alignas(64) Atomic<u64> _head;
    alignas(64) Atomic<u64> _tail;
    
    Atomic() {
        _head = _tail = CNT | (u64) new node{0x2'0000, 0};
    }
    
    ~Atomic() {
        u64 a, b;
        for (;;) {
            a = _tail;
            b = mptr(a)->_next;
            if (!b)
                break;
            _tail = b;
            ptr(a)->release(cnt(a));
            ptr(b)->release(1); // <-- coalesce this somehow?
        }
        // advance head
        for (;;) {
            a = _head;
            b = mptr(a)->_next;
            if (!b)
                break;
            _head = b;
            ptr(a)->release(cnt(a));
            ptr(b)->erase_and_release(1);
        }
        assert(ptr(_head) == ptr(_tail));
        ptr(_head)->release(cnt(_head) + cnt(_tail));
    }
    
    static node const* ptr(u64 a) { return (node const*) (a & PTR); }
    static node* mptr(u64 a) { return (node*) (a & PTR); }
    static u64 cnt(u64 a) { return (a >> 48) + 1; }

    static std::pair<u64, u64> _acquire(Atomic<u64> const& p, u64 expected) {
        for (;;) {
            assert(expected & PTR); // <-- nonnull pointer bits
            if (__builtin_expect(expected & CNT, true)) { // <-- nonzero counter bits
                u64 desired = expected - INC;
                if (p.compare_exchange_weak(expected, desired, std::memory_order_acquire, std::memory_order_relaxed)) {
                    if (__builtin_expect(expected & desired & CNT, true)) {
                        return {desired, 1}; // <-- fast path completes
                    } else { // <-- counter is a power of two
                        expected = desired;
                        ptr(expected)->_count.fetch_add(LOW, std::memory_order_relaxed);
                        do if (p.compare_exchange_weak(expected, desired = expected | CNT, std::memory_order_release, std::memory_order_relaxed)) {
                            if (__builtin_expect((expected & CNT) == 0, false)) // <-- we fixed an exhausted counter
                                p.notify_all();        // <-- notify potential waiters
                            return{desired, cnt(expected)};
                        } while (!((expected ^ desired) & PTR)); // <-- while the pointer bits are unchanged
                        ptr(desired)->release(1 + LOW); // <-- start over
                    }
                }
            } else { // <-- the counter is zero
                p.wait(expected, std::memory_order_relaxed); // <-- until counter may have changed
                expected = p.load(std::memory_order_relaxed);
            }
        }
    }
                
    
    
    template<typename... Args>
    void push(Args&&... args) const {
        node* ptr_mut = new node{0x0000'0000'0002'0000, 0};
        // nodes are created with
        //     weight MAX-1 to be installed in tail
        //   + weight     1 to be awarded to the tail installing thread
        //   + weight MAX-1 to be installed in head
        //   + weight     1 to be awarded to the head installing thread
        ptr_mut->_payload.emplace(std::forward<Args>(args)...);
        std::uint64_t z = 0xFFFE'0000'0000'0000 | (std::uint64_t) ptr_mut;
        ptr_mut = nullptr;
        node const* ptr = nullptr;
        std::uint64_t a = _tail.load(std::memory_order_relaxed);
        std::uint64_t b = 0;
        std::uint64_t c = 0;
        for (;;) {
            // _tail will always be a valid pointer (points to sentinel when queue is empty)
            assert(a & PTR);
            assert(a & CNT);
            b = a - INC;
            if (_tail.compare_exchange_weak(a, b, std::memory_order_acquire, std::memory_order_relaxed)) {
                // we take partial ownership of _tail and can dereference it
            alpha:
                ptr = (node const*) (b & PTR);
                c = 0;
                do if (ptr->_next.compare_exchange_weak(c, z, std::memory_order_acq_rel, std::memory_order_acquire)) {
                    // we installed a new node

                    // we can try to eagerly swing the head here
                    // not clear if this is an optimization (we do less total work)
                    // or a pessimization (we increase contention on _tail)
                    z |= CNT;
                    do if (_tail.compare_exchange_weak(b, z, std::memory_order_acq_rel, std::memory_order_acquire)) {
                        // release tail's current count plus our one unit
                        ptr->release((b >> 48) + 2);
                        return;
                    } while ((b & PTR) == (a & PTR));
                     
                    // somebody else won the race
                    ptr->release(1); // release our one unit of old tail
                    return;
                    
                } while (!c);
                // we failed to install the node and instead must swing tail to next
                do if (_tail.compare_exchange_weak(b, c, std::memory_order_acq_rel, std::memory_order_acquire)) {
                    // we swung tail and are awarded one unit of ownership
                    ptr->release((b >> 48) + 2); // release old tail
                    a = b = c;
                    // resume attempt to install new node
                    // todo: express as better control flow
                    goto alpha;
                } while ((b & PTR) == (a & PTR)); // allow spurious failures and changed counts
                // another thread advanced the tail
                ptr->release(1);
                a = b;
                // start loop over with new tail we just read
            }
            // failed to take ownership, continue
        }
    }
    
    bool try_pop(T& x) const {
        std::uint64_t a = _head.load(std::memory_order_relaxed);
        std::uint64_t b = 0;
        node const* ptr = nullptr;
        std::uint64_t c = 0;
        for (;;) {
            // _head always points to the sentinel before the (potentially empty) queue
            assert(a & PTR);
            assert(a & CNT); // <-- sentinel was drained, can happen if hammering on empty
            b = a - INC;
            if (_head.compare_exchange_weak(a, b, std::memory_order_acquire, std::memory_order_relaxed)) {
                // we can now read _head->_next
                ptr = (node*) (b & PTR);
                c = ptr->_next.load(std::memory_order_acquire);
                if (c & PTR) {
                    do if (_head.compare_exchange_weak(b, c, std::memory_order_release, std::memory_order_relaxed)) {
                        // we installed _head and have one unit of ownership of the new head node
                        ptr->release((b >> 48) + 2); // release old head node
                        ptr = (node*) (c & PTR);
                        // we have established unique access to the payload
                        x = std::move(const_cast<T&>(ptr->_payload.value));
                        ptr->_payload.erase();
                        ptr->release(1); // release new head node
                        return true;
                    } while ((b & PTR) == (a & PTR));
                    // somebody else swung the head, release the old one
                    ptr->release(1);
                    // start over with new head we just read
                    a = b;
                } else {
                    // queue is empty
                    do if (_head.compare_exchange_weak(b, b + INC, std::memory_order_relaxed, std::memory_order_relaxed)) {
                        // we put back the local weight we took
                        return false;
                    } while ((b & PTR) == (a & PTR));
                    // head was changed before we could return weight, so we put weight back to global count
                    ptr->release(1);
                    return false;
                }
            }
            // failed to acquire head, try again
        }
    }
         
};

}

#endif /* queue_hpp */
