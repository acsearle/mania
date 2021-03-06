//
//  vector.hpp
//  mania
//
//  Created by Antony Searle on 22/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef vector_hpp
#define vector_hpp

#include "raw_vector.hpp"
#include "vector_view.hpp"

namespace manic {

// General-purpose dynamic array.  Contiguous elements.  Supports amortized O(1)
// push_back, pop_back, and pop_front, but O(N) pop_front, making it a
// performant stack and queue but not a performant deque.
    
    template<typename T>
    struct vector : vector_view<T>, raw_vector<T> {
        
        vector() : vector_view<T>(nullptr, 0), raw_vector<T>() {};
        
        vector(const vector& v) : vector(static_cast<const_vector_view<T>>(v)) {}
        vector(vector&& v) = default;
        
        explicit vector(ptrdiff_t n)
        : vector_view<T>(nullptr, 0)
        , raw_vector<T>(n) {
            this->_begin = this->_allocation;
            this->_size = n;
            std::uninitialized_default_construct_n(this->_begin, n);
        }
        
        explicit vector(const_vector_view<T> v)
        : vector_view<T>(nullptr, 0)
        , raw_vector<T>(v._size) {
            this->_begin = this->_allocation;
            this->_size = v._size;
            std::uninitialized_copy_n(v._begin, v._size, this->_begin);
        }
        
        ~vector() {
            std::destroy_n(this->_begin, this->_size);
        }
        
        vector& operator=(const vector& v) {
            return *this = static_cast<const_vector_view<T>>(v);
        }
        
        vector& operator=(vector&& v) {
            vector(std::move(v)).swap(*this);
            return *this;
        }
        
        vector& operator=(const_vector_view<T> v) {
            std::destroy_n(this->_begin, this->_size);
            if (this->capacity() < v.size()) {
                raw_vector<T>(std::max(this->_capacity * 2, v.size())).swap(*this);
            }
            this->_begin = this->_allocation;
            this->_size = v.size();
            std::uninitialized_copy_n(v.begin(), v.size(), this->_begin);
            return *this;
        }
        
        void swap(vector& v) {
            using std::swap;
            swap(this->_begin, v._begin);
            swap(this->_size, v._size);
            raw_vector<T>::swap(v);
        }
        
        void clear() {
            std::destroy_n(this->_begin, this->_size);
            this->_size = 0;
        }
        
        ptrdiff_t _capacity_front() const {
            return this->_begin - this->_allocation;
        }
        
        ptrdiff_t _capacity_back() const {
            return (this->_allocation + this->_capacity) - (this->_begin + this->_size);
        }
        
        using vector_view<T>::size;
        using vector_view<T>::begin;
        using vector_view<T>::end;
        using vector_view<T>::operator[];
        
        void resize(ptrdiff_t n) {
            if (n <= this->_size) {
                std::destroy_n(this->_begin + n, this->_size - n);
            } else if ((n - this->_size) <= _capacity_back()) {
                std::uninitialized_default_construct_n(this->_begin + this->_size, n - this->_size);
                this->_size = n;
            } else if ((n <= this->_capacity) && (this->_size <= _capacity_front())) {
                // relocate
                std::memcpy(this->_allocation, this->_begin, this->_size * sizeof(T));
                this->_begin = this->_allocation;
                std::uninitialized_default_construct_n(this->_begin + this->_size, n - this->_size);
                this->_size = n;
            } else {
                raw_vector<T> v(std::max(n, 2 * this->_capacity));
                std::memcpy(v._allocation, this->_begin, this->_size * sizeof(T));
                std::uninitialized_default_construct_n(v._allocation + this->_size, n - this->_size);
                this->_begin = v._allocation;
                this->_size = n;
                v.swap(*this);
            }
        }
        
        void resize(ptrdiff_t n, const T& x) {
            if (n <= this->_size) {
                std::destroy_n(this->_begin + n, this->_size - n);
            } else if ((n - this->_size) <= _capacity_back()) {
                std::uninitialized_fill_n(this->_begin + this->_size, n - this->_size, x);
                this->_size = n;
            } else if ((n <= this->_capacity) && (this->_size <= _capacity_front())) {
                // relocate
                std::memcpy(this->_allocation, this->_begin, this->_size * sizeof(T));
                this->_begin = this->_allocation;
                std::uninitialized_fill_n(this->_begin + this->_size, n - this->_size, x);
                this->_size = n;
            } else {
                raw_vector<T> v(std::max(n, 2 * this->_capacity));
                std::memcpy(v._allocation, this->_begin, this->_size * sizeof(T));
                std::uninitialized_fill_n(v._allocation + this->_size, n - this->_size, x);
                this->_begin = v._allocation;
                this->_size = n;
                v.swap(*this);
            }
        }
        
        void reserve(ptrdiff_t new_cap) {
            if (new_cap > this->_capacity) {
                raw_vector<T> v(std::max(new_cap, this->_capacity * 2));
                std::memcpy(v._allocation, this->_begin, this->_size * sizeof(T));
                this->_begin = v._allocation;
                v.swap(*this);
            }
        }
        
        void shrink_to_fit() {
            raw_vector<T> v(this->_size);
            std::memcpy(v._allocation, this->_begin, this->_size * sizeof(T));
            this->_begin = v._allocation;
            v.swap(*this);
        }
        
        T pop_front() {
            T t(std::move(*this->_begin));
            this->_begin->~T();
            ++this->_begin;
            --this->_size;
            return t;
        }
        
        T pop_back() {
            --this->_size;
            T t(std::move(*(this->_begin + this->_size)));
            (this->_begin + this->_size)->~T();
            return t;
        }
        
        void _reserve_back() {
            if (_capacity_back() == 0) {
                if ((_capacity_front() >= this->_size) && (this->_capacity > this->_size)) {
                    std::memcpy(this->_allocation, this->_begin, this->_size * sizeof(T));
                    this->_begin = this->_allocation;
                } else {
                    raw_vector<T> v(std::max(this->_size + 1, this->_capacity * 2));
                    std::memcpy(v._allocation, this->_begin, this->_size * sizeof(T));
                    this->_begin = v._allocation;
                    v.swap(*this);
                }
            }
            assert(_capacity_back() > 0);
        }
        
        void push_back(const T& x) {
            _reserve_back();
            new (this->_begin + this->_size++) T(x);
        }
        
        void push_back(T&& x) {
            _reserve_back();
            new (this->_begin + this->_size++) T(std::move(x));
        }
        
        template<typename... Args>
        void emplace_back(Args&&... args) {
            _reserve_back();
            new (this->_begin + this->_size++) T(std::forward<Args>(args)...);
        }
        
        T swap_remove(ptrdiff_t i) {
            using std::swap;
            swap((*this)[i], this->back());
            return pop_back();
        }
    
        void erase(ptrdiff_t i) {
            assert(0 <= i);
            assert(i < this->_size);
            (this->_begin + i)->~T();
            std::memmove(this->_begin + i, this->_begin + i + 1, (this->_size - i - 1) * sizeof(T));
            --this->_size;
        }
        
        void _reserve_back(isize n) {
            if (_capacity_back() < n) {
                // this->_begin
                // this->_size
                // this->_allocation
                // this->_capacity
                if ((_capacity_front() >= this->_size) && (this->_capacity >= (this->_size + n))) {
                    // copy forward
                    std::memcpy(this->_allocation, this->_begin, this->_size * sizeof(T));
                    this->_begin = this->_allocation;
                } else {
                    // reallocate
                    raw_vector<T> v(std::max(this->_size + n, this->_capacity * 2));
                    std::memcpy(v._allocation, this->_begin, this->_size * sizeof(T));
                    v.swap(*this);
                    this->_begin = this->_allocation;
                }
                assert(_capacity_back() >= n);
            }
        }
        
        void append(T const* a, T const* b) {
            auto n = b - a;
            _reserve_back(n);
            std::memcpy(this->_begin + this->_size, a, n * sizeof(T));
            this->_size += n;
        }
        
        template<typename It>
        void assign(It a, It b) {
            clear();
            while (a != b) {
                push_back(*a);
                ++a;
            }
        }
        
    };
    
    template<typename T>
    void swap(vector<T>& a, vector<T>& b) {
        a.swap(b);
    }
    
template<typename T, typename Deserializer>
inline auto deserialize(placeholder<vector<T>>, Deserializer& d) {
    auto n = deserialize<isize>(d);
    vector<T> x;
    x.reserve(n);
    while (n--)
        x.push_back(deserialize<T>(d));
    return x;
}
    
} // namespace manic

#endif /* vector_hpp */
