//
//  vector_view.hpp
//  mania
//
//  Created by Antony Searle on 22/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef vector_view_hpp
#define vector_view_hpp

namespace manic {
    
    template<typename T>
    struct vector_view
    : const_vector_view<T> {
        
        using reference = T&;
        using iterator = T*;
        
        
        vector_view() = delete;
        
        // Unusal copy semantics ensure that we cannot discard the const
        // qualification of a view by copying it
        
        vector_view(const vector_view&) = delete;
        vector_view(const vector_view&&) = delete;
        vector_view(vector_view&) = default;
        vector_view(vector_view&&) = default;
        
        vector_view(std::nullptr_t, ptrdiff_t n)
        : const_vector_view<T>(nullptr, n) {
        }
        
        vector_view(T* ptr, ptrdiff_t n)
        : const_vector_view<T>(ptr, n) {
        }
        
        vector_view(T* first, T* last)
        : const_vector_view<T>(first, last) {
        }
        
        vector_view(std::vector<T>& v)
        : const_vector_view<T>(v.data(), v.size()) {
        }
        
        ~vector_view() = default;
        
        vector_view operator=(const vector_view& v) {
            return *this = static_cast<const_vector_view<T>>(v);
        }
        
        vector_view operator=(vector_view&& v) {
            return *this = static_cast<const_vector_view<T>>(v);
        }
        
        vector_view operator=(const_vector_view<T> v) {
            assert(this->size() == v.size());
            std::copy(v.begin(), v.end(), begin());
            return *this;
        }
        
        vector_view operator=(T x) {
            std::fill(begin(), end(), x);
            return *this;
        }
        
        template<typename It>
        void assign(It first, It last) {
            assert(std::distance(first, last) == this->_size);
            std::copy(first, last, begin());
        }
        
        void swap(vector_view<T> v) {
            assert(this->size() == v.size());
            std::swap_ranges(begin(), end(), v.begin());
        }
        
        using const_vector_view<T>::begin;
        using const_vector_view<T>::end;
        using const_vector_view<T>::operator[];
        using const_vector_view<T>::operator();
        using const_vector_view<T>::front;
        using const_vector_view<T>::back;
        using const_vector_view<T>::sub;
        
        T* begin() { return this->_begin; }
        T* end() { return this->_begin + this->_size; }
        T& operator[](ptrdiff_t i) { return this->_begin[i]; }
        T& operator()(ptrdiff_t i) { return this->_begin[i]; }
        T& front() { return *this->_begin; }
        T& back() { return this->_begin[this->_size - 1]; }
        
        vector_view sub(ptrdiff_t i, ptrdiff_t n) {
            return vector_view(this->_begin + i, n);
        }
        
    }; // struct vector_view<T>
    
    template<typename T>
    vector_view<T> operator/=(vector_view<T> a, T b) {
        for (ptrdiff_t i = 0; i != a.size(); ++i)
            a(i) /= b;
        return a;
    }
    
    template<typename T>
    vector_view<T> operator*=(vector_view<T> a, T b) {
        for (ptrdiff_t i = 0; i != a.size(); ++i)
            a(i) *= b;
        return a;
    }

    
}

#endif /* vector_view_hpp */
