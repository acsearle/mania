//
//  matrix.hpp
//  mania
//
//  Created by Antony Searle on 16/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef matrix_hpp
#define matrix_hpp

#include <numeric>
#include <vector>
#include <iostream>

#include "matrix_view.hpp"

namespace manic {
    
    
    //
    // Views into row-major memory.
    //
    // We use a hierarchy matrix<T> : matrix_view<T> : const_matrix_view<T>,
    // corresponding to mutable metadata, mutable data, and immutable.
    //
    // matrix_view can only be copy-constructed from a mutable matrix_view&, so
    // that a const matrix cannot be used to produce a non-const matrix_view.
    //
    // The views have reference sematics.  They can be copy constructed, but
    // assignment (if mutable) copies the viewed sequences.  There is no way
    // to change the view, so it behaves like a non-const reference or a
    // T* const.  Views may be passed by value.
    //


    
    
    
    
    
    
    
    
    
    
   
    
    template<typename T>
    struct matrix : matrix_view<T> {
        
        std::vector<T> _store;
        
        bool _invariant() {
            assert(this->_columns >= 0);
            assert(this->_stride >= 0);
            assert(this->_rows >= 0);
            assert(this->_begin >= _store.data());
            assert(this->_stride >= this->_columns);
            assert(this->_begin + (this->_rows - 1) * this->_stride + this->_columns <= _store.data() + _store.size());
            return true;
        }
        
        
        
        
        matrix() : matrix_view<T>(nullptr, 0, 0, 0), _store() {}
        
        matrix(const matrix& r) : matrix() {
            *this = r;
        }
        
        matrix(matrix&& r) : matrix() {
            r.swap(*this);
        }
        
        matrix(const_matrix_view<T> v)
        : matrix() {
            operator=(v);
        }
        
        matrix(ptrdiff_t rows, ptrdiff_t columns)
        : matrix() {
            discard_and_resize(rows, columns);
        }
        
        matrix(ptrdiff_t rows, ptrdiff_t columns, T x)
        : matrix(rows, columns) {
            *this = x;
        }
        
        ~matrix() = default;
        
        matrix& operator=(const matrix& r) {
            return operator=(static_cast<const_matrix_view<T>>(r));
        }
        
        matrix& operator=(matrix&& r) {
            matrix(std::move(r)).swap(*this);
            return *this;
        }
        
        matrix& operator=(const_matrix_view<T> r) {
            discard_and_resize(r.rows(), r.columns());
            std::copy(r.begin(), r.end(), this->begin());
            return *this;
        }
        
        matrix& operator=(const T& x) {
            std::fill(this->begin(), this->end(), x);
            return *this;
        }
        
        
        using matrix_view<T>::swap;
        void swap(matrix& r) {
            using std::swap;
            swap(this->_begin, r._begin);
            swap(this->_columns, r._columns);
            swap(this->_stride, r._stride);
            swap(this->_rows, r._rows);
            swap(this->_store, r._store);
        }
        
        // Mutators
        
        void crop(ptrdiff_t i, ptrdiff_t j, ptrdiff_t r, ptrdiff_t c) {
            assert(i >= 0);
            assert(j >= 0);
            assert(r >= 0);
            assert(c >= 0);
            assert(i + r <= this->_rows);
            assert(j + c <= this->_columns);
            this->_begin += i * this->_stride + j;
            this->_columns = c;
            this->_rows = r;
        }
        
        void discard_and_resize(ptrdiff_t rows, ptrdiff_t columns) {
            _store.resize(rows * columns, 0);
            this->_begin = _store.data();
            this->_columns = columns;
            this->_stride = columns;
            this->_rows = rows;
        }
        
        void expand(ptrdiff_t i, ptrdiff_t j, ptrdiff_t r, ptrdiff_t c, T x) {
            matrix<T> a(r, c, x);
            a.sub(i, j, this->_rows, this->_columns) = *this;
            a.swap(*this);
        }
        
        void resize(ptrdiff_t r, ptrdiff_t c, T x = T()) {
            matrix<T> a(r, c, x);
            r = std::min(r, this->_rows);
            c = std::min(c, this->_columns);
            a.sub(0, 0, r, c) = this->sub(0, 0, r ,c);
            a.swap(*this);
        }
        
    };
    
    template<typename T>
    void swap(matrix<T>& a, matrix<T>& b) {
        a.swap(b);
    }
    
    template<typename T> matrix<T> operator+(matrix_view<T> a, matrix_view<T> b) {
        assert(a.rows() == b.rows());
        assert(b.columns() == b.columns());
        matrix<T> c(a.rows(), a.columns());
        for (ptrdiff_t i = 0; i != a.rows(); ++i)
            for (ptrdiff_t j = 0; j != a.columns(); ++j)
                c(i, j) = a(i, j) + b(i, j);
        return c;
    }
    

    template<typename T> matrix<T> operator+(matrix_view<T> a, T b) {
        matrix<T> c(a.rows(), a.columns());
        for (ptrdiff_t i = 0; i != a.rows(); ++i)
            for (ptrdiff_t j = 0; j != a.columns(); ++j)
                c(i, j) = a(i, j) + b;
        return c;
    }
 
    template<typename T> matrix<T> transpose(const_matrix_view<T> a) {
        matrix<T> b(a.columns(), a.rows());
        for (ptrdiff_t i = 0; i != a.rows(); ++i)
            for (ptrdiff_t j = 0; j != a.columns(); ++j)
                b(j, i) = a(i, j);
        return b;
    }
    
    template<typename T> matrix<T> outer_product(const_vector_view<T> a,
                                                 const_vector_view<T> b) {
        matrix<T> c(a.size(), b.size());
        for (ptrdiff_t i = 0; i != c.rows(); ++i)
            for (ptrdiff_t j = 0; j != c.columns(); ++j)
                c(i, j) = a[i] * b[j];
        return c;
    }
    
    template<typename T>
    matrix<T> operator-(const_matrix_view<T> a, const_matrix_view<T> b) {
        matrix<T> c(a.rows(), a.columns());
        for (ptrdiff_t i = 0; i != a.rows(); ++i)
            for (ptrdiff_t j = 0; j != a.columns(); ++j)
                c(i, j) = a(i, j) - b(i, j);
        return c;
    }
    
    
    template<typename A, typename B, typename C>
    void filter_rows(matrix_view<C> c, const_matrix_view<A> a, const_vector_view<B> b) {
        assert(c.rows() == a.rows());
        assert(c.columns() + b.columns() == a.columns());
        for (ptrdiff_t i = 0; i != c.rows(); ++i)
            for (ptrdiff_t j = 0; j != c.columns(); ++j) {
                for (ptrdiff_t k = 0; k != b.size(); ++k)
                    c(i, j) += a(i, j + k) * b(k);
            }
    }
    
    template<typename A, typename B, typename C>
    void filter_columns(matrix_view<C> c, const_matrix_view<A> a, const_vector_view<B> b) {
        assert(c.columns() == a.columns());
        assert(c.rows() + b.size() == a.rows());
        for (ptrdiff_t i = 0; i != c.rows(); ++i)
            for (ptrdiff_t j = 0; j != c.columns(); ++j) {
                for (ptrdiff_t k = 0; k != b.size(); ++k)
                    c(i, j) += a(i + k, j) * b(k);
            }
    }
    
    template<typename A, typename B>
    void explode(matrix_view<B> b, const_matrix_view<A> a) {
        assert(b.rows() == 2 * a.rows());
        assert(b.columns() == 2 * a.columns());
        for (ptrdiff_t i = 0; i != a.rows(); ++i)
            for (ptrdiff_t j = 0; j != a.columns(); ++j)
                b(2 * i, 2 * j) = a(i, j);
    }
    
    

    
}

#endif /* matrix_hpp */
