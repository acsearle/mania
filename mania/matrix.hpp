//
//  matrix.hpp
//  mania
//
//  Created by Antony Searle on 16/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef matrix_hpp
#define matrix_hpp

#include <vector>

namespace manic {
    
    template<typename T, typename ContiguousContainer = std::vector<T>>
    struct matrix {
        
        using value_type = typename ContiguousContainer::value_type;
        using size_type = typename ContiguousContainer::size_type;
        using difference_type = typename ContiguousContainer::difference_type;
        using pointer = typename ContiguousContainer::pointer;
        
        ContiguousContainer _data;
        size_type  _m, _n;
        
        matrix() : _m(0), _n(0) {}
        matrix(const matrix&) = default;
        matrix(matrix&&) = default;
        ~matrix() = default;
        matrix& operator=(const matrix&) = default;
        matrix& operator=(matrix&&) = default;
        
        matrix(size_t rows, size_t columns)
        : _m(rows)
        , _n(columns)
        , _data(rows * columns) {}
        
        void resize(size_t rows, size_t columns) {
            _data.resize(rows * columns);
        }
        
        size_type rows() const { return _m; }
        size_type columns() const {return _m; }
        
        value_type* data() { return _data.data(0); }
        
        value_type& operator()(size_type i, size_type j) {
            assert(i < _m);
            assert(j < _n);
            return _data[i * _n + j];
        }

        value_type* operator[](size_type i) {
            assert(i < _m);
            return _data.data() + i * _n;
        }

        const value_type* operator[](size_type i) const {
            assert(i < _m);
            return _data.data() + i * _n;
        }

        
    };
    
}

#endif /* matrix_hpp */
