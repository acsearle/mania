//
//  deque.hpp
//  mania
//
//  Created by Antony Searle on 15/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef deque_hpp
#define deque_hpp

#include <malloc/malloc.h>

#include <algorithm>
#include <utility>
#include <cstring>

namespace manic {
    
    // All types in MANIC are Relocatable: move construction followed by
    // destruction of the moved-from object is equivalent to a bitwise copy.
    // All basic types, STL containers and smart pointers are Relocatable.
    
    template<typename T>
    T* relocate(T* first, T* last, T* d_first) {
        std::memmove(d_first, first, (last - first) * sizeof(T));
        return d_first + (last - first);
    }
    
    template<typename T>
    T* relocate_backward(T* first, T* last, T* d_last) {
        std::memmove(d_last - (last - first), first, (last - first) * sizeof(T));
        return d_last - (last - first);
    }
    
    template<typename T>
    class deque {
        
        T* _begin;
        T* _end;
        T* _allocation_begin;
        T* _allocation_end;
        
        bool _invariant() const {
            return ((_allocation_begin <= _begin)
                    && (_begin <= _end)
                    && (_end <= _allocation_end));
        }
        
    public:
        
        deque()
        : _begin(nullptr)
        , _end(nullptr)
        , _allocation_begin(nullptr)
        , _allocation_end(nullptr) {
            assert(_invariant());
        }
        
        deque(const deque& r)
        : deque(r.begin(), r.end()) {
        }
        
        deque(deque&& r)
        : deque() {
            r.swap(*this);
        }
        
        deque(T* begin_,
              T* end_,
              T* allocation_begin_,
              T* allocation_end_)
        : _begin(begin_)
        , _end(end_)
        , _allocation_begin(allocation_begin_)
        , _allocation_end(allocation_end_) {
            assert(_invariant());
        }
        
        deque with_capacity(ptrdiff_t n) {
            deque d;
            d.require_back(n);
            return d;
        }
        
        deque(const T* first, const T* last)
        : deque() {
            insert(begin(), first, last);
        }
        
        ~deque() {
            clear();
            free(_allocation_begin);
        }
        
        deque& operator=(deque&& r) {
            deque(std::move(r)).swap(*this);
            return *this;
        }
        
        deque& operator=(const deque& r) {
            deque(r).swap(*this);
            return *this;
        }
        
        void assign(const T* first, const T* last) {
            clear();
            insert(begin(), first, last);
        }
        
        void swap(deque& r) {
            using std::swap;
            swap(_begin, r._begin);
            swap(_end, r._end);
            swap(_allocation_begin, r._allocation_begin);
            swap(_allocation_end, r._allocation_end);
            assert(_invariant());
            assert(r._invariant());
        }
        
        void clear() {
            for (; _begin != _end; ++_begin)
                _begin->~T();
            assert(_invariant());
            assert(empty());
        }
        
        bool empty() const {
            return _begin == _end;
        }
        
        ptrdiff_t size() const {
            return _end - _begin;
        }
        
        ptrdiff_t capacity() const {
            return _allocation_end - _allocation_begin;
        }
        
        void reserve_back(ptrdiff_t n) {
            assert(0 <= n);
            auto capacity_back = _allocation_end - _end;
            if (capacity_back < n) {
                auto old_size = size();
                auto old_capacity = capacity();
                if (old_capacity < 3 * old_size + n) {
                    auto new_capacity = std::max(3 * old_size + n, 2 * old_capacity);
                    new_capacity = malloc_good_size(new_capacity * sizeof(T)) / sizeof(T);
                    auto new_allocation_begin = static_cast<T*>(malloc(new_capacity * sizeof(T)));
                    auto new_begin = new_allocation_begin + (new_capacity - old_size - n) / 2;
                    memcpy(new_begin, _begin, old_size * sizeof(T));
                    free(_allocation_begin);
                    _begin = new_begin;
                    _end = new_begin + old_size;
                    _allocation_begin = new_allocation_begin;
                    _allocation_end = new_allocation_begin + new_capacity;
                } else {
                    auto new_begin = _allocation_begin + (old_capacity - old_size - n) / 2;
                    assert(_allocation_begin + old_size <= new_begin);
                    assert(new_begin + old_size + n <= _allocation_end);
                    assert(new_begin + old_size <= _begin);
                    memcpy(new_begin, _begin, old_size * sizeof(T));
                    _begin = new_begin;
                    _end = new_begin + old_size;
                }
            }
            assert((_allocation_end - _end) >= n);
            assert(_invariant());
        }
        
        void reserve_front(ptrdiff_t n) {
            assert(0 <= n);
            auto capacity_front = _begin - _allocation_begin;
            if (capacity_front < n) {
                auto old_size = size();
                auto old_capacity = capacity();
                if (old_capacity < 3 * old_size + n) {
                    auto new_capacity = std::max(3 * old_size + n, 2 * old_capacity);
                    new_capacity = malloc_good_size(new_capacity * sizeof(T)) / sizeof(T);
                    auto new_allocation_begin = static_cast<T*>(malloc(new_capacity * sizeof(T)));
                    auto new_begin = new_allocation_begin + (new_capacity - old_size + n) / 2;
                    memcpy(new_begin, _begin, old_size * sizeof(T));
                    free(_allocation_begin);
                    _begin = new_begin;
                    _end = new_begin + old_size;
                    _allocation_begin = new_allocation_begin;
                    _allocation_end = new_allocation_begin + new_capacity;
                } else {
                    auto new_begin = _allocation_begin + (old_capacity - old_size + n) / 2;
                    assert(_end <= new_begin);
                    memcpy(new_begin, _begin, old_size * sizeof(T));
                    _begin = new_begin;
                    _end = new_begin + old_size;
                }
                assert(_allocation_begin + old_size + n <= _begin);
                assert(_end + old_size <= _allocation_end);
            }
            assert((_begin  - _allocation_begin) >= n);
            assert(_invariant());
        }

        
        
        void push_back(T&& x) {
            reserve_back(1);
            assert(_end < _allocation_end);
            new (_end++) T(std::move(x));
            assert(_invariant());
        }
        
        void push_back(const T& x) {
            reserve_back(1);
            assert(_end < _allocation_end);
            new (_end++) T(x);
            assert(_invariant());
        }
        
        void push_front(T&& x) {
            assert(_invariant());
            reserve_front(1);
            assert(_allocation_begin < _begin);
            new (--_begin) T(std::move(x));
            assert(_invariant());
        }

        void push_front(const T& x) {
            assert(_invariant());
            reserve_front(1);
            assert(_allocation_begin < _begin);
            new (--_begin) T(x);
            assert(_invariant());
        }
        
        T pop_back() {
            assert(_invariant());
            assert(_begin < _end);
            T x;
            memcpy(&x, --_end, sizeof(T));
            assert(_invariant());
            return x;
        }
        
        T pop_front() {
            assert(_invariant());
            assert(_begin < _end);
            T x;
            memcpy(&x, _begin++, sizeof(T));
            assert(_invariant());
            return x;
        }
        
        T* erase(const T* pos) {
            return erase(pos, pos + 1);
        }
        
        T* erase(const T* first, const T* last) {
            assert(_begin <= first);
            assert(first <= last);
            assert(last <= _end);
            auto a = first - _begin;
            auto b = last - first;
            auto c = _end - last;
            for (auto p = _begin + a; p != last; ++p)
                p->~T();
            if (a < c) {
                std::memmove(_begin + b, _begin, a * sizeof(T));
                _begin += b;
            } else {
                std::memmove(_begin + a, _end - c, c * sizeof(T));
                _end -= b;
            }
        }
        
        T* _insert(const T* pos, ptrdiff_t n) {
            
            // We want insert to have the same complexity as push_front and
            // push_back when it is equivalent to (multiple) invocations of
            // them, so we must grow the allocation equivalently.
            
            assert(_begin <= pos);
            assert(pos <= _end);
            assert(n >= 0);
            auto a = _begin - _allocation_begin;
            auto b = pos - _begin;
            auto c = n;
            auto d = _end - pos;
            auto e = _allocation_end - _end;
            auto old_size = size();
            auto old_capacity = capacity();

            if ((b < d) && (a >= c)) {
                // The insertion is nearer the front and there is enough spare
                // capacity to move the front forward
                relocate(_begin, _begin + b, _begin - c);
                _begin -= c;
            } else if ((b >= d) && (e >= c)) {
                // The insertion is nearer the back and there is enough spare
                // capacity to move the back backward
                relocate_backward(_begin + b, _end, _end + c);
                _end += c;
            } else if (old_capacity >= old_size * 3 + c) {
                // Shift within the existing buffer
                auto new_begin = _allocation_begin + (old_capacity - old_size - c) / 2;
                // Unlike push_front and push_back, the new and old ranges can
                // overlap, so we need to copy them carefully.
                if (new_begin < _begin) {
                    relocate(_begin, _begin + b, new_begin);
                    relocate(_begin + b, _end, new_begin + b + c);
                } else {
                    relocate(_begin + b, _end, new_begin + b + c);
                    relocate(_begin, _begin + b, new_begin);
                }
                _begin = new_begin;
                _end = new_begin + b + c + d;
            } else {
                // To maintain good amortized behaviour we have to expand the
                // allocation
                auto new_capacity = std::max(old_size * 3 + c, 2 * old_capacity);
                new_capacity = malloc_good_size(new_capacity * sizeof(T)) / sizeof(T);
                auto new_allocation_begin = static_cast<T*>(malloc(new_capacity * sizeof(T)));
                auto new_begin = new_allocation_begin + (new_capacity - old_size - c) / 2;
                std::memcpy(new_begin, _begin, b * sizeof(T));
                std::memcpy(new_begin + b + c, _begin + b, d * sizeof(T));
                free(_allocation_begin);
                _allocation_begin = new_allocation_begin;
                _allocation_end = new_allocation_begin + new_capacity;
                _begin = new_begin;
                _end = new_begin + b + c + d;
            }
            assert(_invariant());
            assert(size() == old_size + n);
            return _begin + b;
        }
        
        T* insert(const T* pos, const T* first, const T* last) {
            return std::uninitialized_copy(first, last, _insert(pos, last - first));
        }
        
        T* relocate(const T* pos, T* first, T* last) {
            return relocate(first, last, _insert(pos, last - first));
        }
        
        T* insert(const T* pos, T&& x) {
            auto p = _insert(pos, 1);
            new (p) T(std::move(x));
            return ++p;
        }
        
        T* insert(const T* pos, const T& x) {
            auto p = _insert(pos, 1);
            new (p) T(x);
            return ++p;
        }
        
        template<typename... Args>
        T* emplace(const T* pos, Args&&... args) {
            auto p = _insert(pos, 1);
            new (p) T(std::forward<Args>(args)...);
            return ++p;
        }
        
        void shrink_to_fit() {
            auto n = size();
            deque a = with_capacity(n);
            a._begin = a._allocation_begin + (a.capacity() - n) / 2;
            a._end = a._begin + n;
            relocate(_begin, _end, a._begin);
            _begin = _end;
            a.swap(*this);
        }
        
        T* begin() { return _begin; }
        T& front() { return *_begin; }
        T* end() { return _end; }
        T& back() { return *(_end - 1); }
        T& operator[](ptrdiff_t i) {
            assert(i >= 0);
            assert(i < size());
            return _begin[i];
        }
        
    };
    
}

#endif /* deque_hpp */
