//
//  stride_ptr.hpp
//  mania
//
//  Created by Antony Searle on 22/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef stride_ptr_hpp
#define stride_ptr_hpp

#include <stddef.h>

namespace manic {
    
    // Pointer that strides over an array, as when traversing a column of a
    // row-major matrix.
    //
    // It could potentially stride by multiples of alignof(T) rather than
    // sizeof(T).
    
    template<typename T>
    struct stride_ptr {
        
        using difference_type = ptrdiff_t;
        using value_type = std::remove_const_t<T>;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;
        
        T* _ptr;
        ptrdiff_t _stride;
        
        stride_ptr(T* p, ptrdiff_t n) : _ptr(p), _stride(n) {}
        
        T* operator->() const { return _ptr; }
        T& operator*() const { return *_ptr; }
        T& operator[](ptrdiff_t i) const { return _ptr[_stride * i]; }
        
        stride_ptr& operator++() { _ptr += _stride; return *this; }
        stride_ptr operator++(int) { stride_ptr a(*this); ++*this; return a; }
        stride_ptr& operator+=(ptrdiff_t i) { _ptr += _stride * i; return *this; }
        
        stride_ptr& operator--() { _ptr -= _stride; return *this; }
        stride_ptr operator--(int) { stride_ptr a(*this); --*this; return a; }
        stride_ptr& operator-=(ptrdiff_t i) { _ptr -= _stride * i; return *this; }
        
    };

    template<typename T>
    stride_ptr<T> operator+(stride_ptr<T> a, ptrdiff_t b) {
        return stride_ptr(a._ptr + a._stride * b, a._stride);
    }

    template<typename T>
    stride_ptr<T> operator+(ptrdiff_t a, stride_ptr<T> b) {
        return stride_ptr(b._ptr + a * b._stride, b._stride);
    }

    template<typename T>
    stride_ptr<T> operator-(stride_ptr<T> a, ptrdiff_t b) {
        return stride_ptr(a._ptr - a._stride * b, a._stride);
    }
    
    template<typename T>
    stride_ptr<T> operator-(ptrdiff_t a, stride_ptr<T> b) {
        return stride_ptr(b._ptr - a * b._stride, b._stride);
    }
    
    template<typename T, typename U>
    ptrdiff_t operator-(stride_ptr<T> a, stride_ptr<U> b) {
        assert(a._stride == b._stride);
        return (a._ptr - b._ptr) / a._stride;
    }

#define F(X)\
    template<typename T, typename U>\
    bool operator X (stride_ptr<T> a, stride_ptr<U> b) {\
        assert(a._stride == b._stride);\
        return a._ptr X b._ptr;\
    }
    
    F(==)
    F(!=)
    F(<)
    F(>)
    F(<=)
    F(>=)
    
#undef F
    
}

#endif /* stride_ptr_hpp */
