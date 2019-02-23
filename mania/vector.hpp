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
    
    template<typename T>
    struct vector : vector_view<T>, raw_vector<T> {
        
        vector() = default;
        
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
            // do better
            vector(v).swap(*this);
            return *this;
        }
        
        void swap(vector& v) {
            vector_view<T>::swap(v);
            raw_vector<T>::swap(v);
        }
        
        void resize(ptrdiff_t n);
        void resize(ptrdiff_t n, const T& x);
        
        T pop_front() {
            T t(std::move(*this->_begin));
            this->_begin->~T();
            ++this->_begin;
            return t;
        }
        
        T pop_back();
        T push_back(const T&);
        void push_back(T&&);
        
        template<typename... Args>
        void emplace_back(Args&&... args);
        
        T swap_remove(ptrdiff_t i) {
            using std::swap;
            swap((*this)[i], this->back());
            return pop_back();
        }
        
    };
    
    
} // namespace manic

#endif /* vector_hpp */
