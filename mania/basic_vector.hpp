//
//  basic_vector.hpp
//  mania
//
//  Created by Antony Searle on 25/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef basic_vector_hpp
#define basic_vector_hpp

#include "vector_view.hpp"

namespace manic {

// A basic vector built on raw_vector, that does not support resizing

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

} // namespace manic

#endif /* basic_vector_hpp */
