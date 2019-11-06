//
//  const_matrix_view.hpp
//  mania
//
//  Created by Antony Searle on 22/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef const_matrix_view_hpp
#define const_matrix_view_hpp

#include <cmath>

#include "const_matrix_iterator.hpp"
#include "const_column_vector_view.hpp"

namespace manic {
    
    template<typename T>
    struct const_matrix_view {
        
        using value_type = const_vector_view<T>;
        using size_type = ptrdiff_t;
        using difference_type = ptrdiff_t;
        using reference = value_type;
        using const_reference = value_type;
        using iterator = const_matrix_iterator<T>;
        using const_iterator = const_matrix_iterator<T>;
        
        
        T* _begin;
        ptrdiff_t _columns;
        ptrdiff_t _stride;
        ptrdiff_t _rows;
        
        const_matrix_view() = delete;
        const_matrix_view(const const_matrix_view&) = default;
        
        const_matrix_view(const T* ptr,
                          ptrdiff_t columns,
                          ptrdiff_t stride,
                          ptrdiff_t rows)
        : _begin(const_cast<T*>(ptr))
        , _columns(columns)
        , _stride(stride)
        , _rows(rows) {
        }
        
        ~const_matrix_view() = default;
        
        const_matrix_view& operator=(const const_matrix_view&) = delete;
        const_matrix_view& operator=(const_matrix_view&&) = delete;
        
        const T* data() const { return _begin; }
        ptrdiff_t columns() const { return _columns; }
        ptrdiff_t width() const { return _columns; }
        ptrdiff_t size() const { return _rows; }
        ptrdiff_t rows() const { return _rows; }
        ptrdiff_t height() const { return _rows; }
        ptrdiff_t stride() const { return _stride; }
        
        const_iterator begin() const { return const_iterator(_begin, _columns, _stride); }
        const_iterator end() const { return begin() + _rows; }
        const_iterator cbegin() const { return const_iterator(_begin, _columns, _stride); }
        const_iterator cend() const { return begin() + _rows; }
        
        
        const_vector_view<T> operator[](ptrdiff_t i) const {
            return const_vector_view<T>(_begin + i * _stride, _columns);
        }
        
        const T& operator()(ptrdiff_t i, ptrdiff_t j) const {
            assert(i >= 0);
            assert(j >= 0);
            assert(i < _rows);
            assert(j < _columns);
            return *(_begin + i * _stride + j);
        }
        
        const_vector_view<T> front() const { return *begin(); }
        const_vector_view<T> back() const { return begin()[_rows - 1]; }
        
        const_matrix_view<T> sub(ptrdiff_t i,
                                 ptrdiff_t j,
                                 ptrdiff_t r,
                                 ptrdiff_t c) const {
            assert(0 <= i);
            assert(0 <= j);
            assert(0 < r);
            assert(i + r <= _rows);
            assert(0 < c);
            assert(j + c <= _columns);
            return const_matrix_view(_begin + i * _stride + j, c, _stride, r);
        }
        
        void print() const {
            for (auto&& row : *this) {
                for (auto&& value : row)
                    std::cout << value << ' ';
                std::cout << '\n';
            }
        }
        
        
    };
    
    template<typename T>
    bool operator==(const_matrix_view<T> a, const_matrix_view<T> b) {
        assert(a.rows() == b.rows());
        assert(a.columns() == b.columns());
        return std::equal(a.begin(), a.end(), b.begin());
    }
    
    template<typename T>
    bool operator!=(const_matrix_view<T> a, const_matrix_view<T> b) {
        return !(a == b);
    }
    
    template<typename T>
    T rms(const_matrix_view<T> a) {
        T p = 0.0;
                   T q = 0.0;
        for (auto row : a)
            for (auto x : row) {
                q += x;
                p += x * x;
            }
        
        p /= a.rows() * a.columns();
        return sqrt(p);
    }
                   template<typename T>
                   T variance(const_matrix_view<T> a) {
                       T p = 0.0;
                       for (auto row : a)
                           for (auto x : row) {
                               p += x * x;
                           }
                       
                       p /= a.rows() * a.columns();
                       return p;
                   }

                   template<typename T>
                   T mean(const_matrix_view<T> a) {
                       T p = 0.0;
                       for (auto row : a)
                           for (auto x : row)
                               p += x;
                       p /= a.rows() * a.columns();
                       return p;
                   }

                   
    template<typename T>
    T min(const_matrix_view<T> a) {
        T p = a(0, 0);
        for (auto row : a)
            for (auto x : row)
                p = std::min(p, x);
        return p;
    }
            
    template<typename T>
    T max(const_matrix_view<T> a) {
        T p = a(0, 0);
        for (auto row : a)
            for (auto x : row)
                p = std::max(p, x);
        return p;
    }

            
            template<typename T>
            T stddev(const_matrix_view<T> a) {
                T p = 0.0;
                           T q = 0.0;
                for (auto row : a)
                    for (auto x : row) {
                        q += x;
                        p += x * x;
                    }
                   auto N = a.rows() * a.columns();
                   p /= N;
                   q /= N;
                return sqrt(p - q * q);
            }

    
}

#endif /* const_matrix_view_hpp */
