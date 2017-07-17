//
//  matrix.hpp
//  mania
//
//  Created by Antony Searle on 16/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef matrix_hpp
#define matrix_hpp

#include <cstddef>

namespace manic {
    
    template<typename ContiguousContainer>
    struct matrix {
        
        using value_type = typename ContiguousContainer::value_type;
        using size_type = typename ContiguousContainer::size_type;
        using difference_type = typename ContiguousContainer::difference_type;
        using pointer = typename ContiguousContainer::pointer;
        
        ContiguousContainer _data;
        size_type  _m, _n;
        size_type _stride;
        
        matrix() = default;
        matrix(const matrix&) = default;
        matrix(matrix&&) = default;
        ~matrix() = default;
        matrix& operator=(const matrix&) = default;
        matrix& operator=(matrix&&) = default;
        
        value_type* data() { return _data.data(0); }
        size_type stride() { return _stride(); }
        
        value_type& operator()(size_type i, size_type j) {
            assert(i < _m);
            assert(j < _n);
            return _data[i * _stride + j];
        }

        value_type* operator[](size_type i) {
            return _data.data() + i * _stride;
        }
        
    };
    
}

#endif /* matrix_hpp */
