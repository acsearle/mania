//
//  raw_vector.hpp
//  mania
//
//  Created by Antony Searle on 22/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef raw_vector_hpp
#define raw_vector_hpp

#include <cstdlib>
#include <utility>

#include "common.hpp"
#include "vector_view.hpp"

namespace manic {
    
    // raw_vector manages a slab of raw memory.  It can be combined with some
    // external managment to control lifetimes.
    
    template<typename T>
    struct raw_vector {
        
        T* _allocation;
        isize _capacity;
        
        raw_vector() : _allocation(nullptr), _capacity(0) {}
        
        raw_vector(const raw_vector&) = delete;
        raw_vector(raw_vector&& v) : raw_vector() { swap(v); }
        explicit raw_vector(isize capacity) {
            _allocation = (T*) std::malloc(capacity * sizeof(T));
            _capacity = capacity;
        }
        raw_vector(T* ptr, isize n) : _allocation(ptr), _capacity(n) {}
        
        ~raw_vector() { free(_allocation); }
        
        raw_vector& operator=(const raw_vector&) = delete;
        raw_vector& operator=(raw_vector&& v) {
            raw_vector(std::move(v)).swap(*this);
            return *this;
        }
        
        void swap(raw_vector& v) {
            using std::swap;
            swap(_allocation, v._allocation);
            swap(_capacity, v._capacity);
        }
        
        isize capacity() const { return _capacity; }

        // Dicey methods
        
        isize size() const { return _capacity; }
        T* begin() const { return _allocation; }
        T* end() const { return _allocation + _capacity; }
        T& operator[](isize i) const { return _allocation[i]; }
        
    };
    
    template<typename T>
    struct raw_vector<const T>;
    
    template<typename T>
    void swap(raw_vector<T>& a, raw_vector<T>& b) {
        a.swap(b);
    }
    
    
    // A basic vector that does not support
    
    template<typename T>
    struct basic_vector
    : vector_view<T> {
        
        basic_vector() = default;
        
        basic_vector(const basic_vector& r)
        : basic_vector(static_cast<const_vector_view<T>>(r)) {
        }
        
        basic_vector(basic_vector&& r)
        : basic_vector() {
            r.swap(*this);
        }
        
        basic_vector(const_vector_view<T> r)
        : basic_vector() {
            this->_begin = std::malloc(r.size() * sizeof(T));
            this->_size = r.size();
            std::uninitialized_copy(r.begin(), r.end(), this->_begin);
        }
        
        ~basic_vector() {
            std::destroy_n(this->_begin, this->_size);
        }
        
        void swap(basic_vector& r) {
            using std::swap;
            swap(this->_begin, r._begin);
            swap(this->_size, r._size);
        }
        
        void clear() {
            basic_vector().swap(*this);
        }
        
        void resize(isize n) {
            basic_vector r;
            r.swap(*this);
            this->_begin = std::malloc(n * sizeof(T));
            this->_size = n;
            std::uninitialized_move_n(r.begin(), std::min(this->_size, r._size), this->_begin);
            if (this->size() > r._size)
                std::uninitialized_default_construct_n(this->_begin + r._size, this->_size);
        }
        
    };
    
    template<typename T>
    void swap(basic_vector<T>& a, basic_vector<T>& b) {
        a.swap(b);
    }
    
    
}

#endif /* raw_vector_hpp */
