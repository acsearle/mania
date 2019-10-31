//
//  space.hpp
//  mania
//
//  Created by Antony Searle on 31/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef space_hpp
#define space_hpp

#include "table3.hpp"
#include "vec.hpp"
#include "matrix.hpp"

namespace manic {

template<typename T>
struct space {
    
    // to speed lookup, consider
    // * reducing depth of indices
    // * storing T* instead and constructing view from implicit 16x16 knowledge
    mutable table3<gl::vec<u64, 2>, matrix<T>> _table;
    
    static const u64 N = 16;
    static const u64 MASK = N - 1;
    
    T& _get_unsafe(u64 i, u64 j) const {
        gl::vec<u64, 2> key{i & ~MASK, j & ~MASK};
        // perfect location for entry API
        auto p = _table.try_get(key);
        if (!p) {
            p = &_table.insert(key, matrix<T>(16, 16));
        }
        assert(p);
        return (*p)(i & MASK, j & MASK);
    }
    
    T& operator()(u64 i, u64 j) {
        return _get_unsafe(i, j);
    }

    T const& operator()(u64 i, u64 j) const {
        return _get_unsafe(i, j);
    }

    auto begin() { return _table.begin(); }
    auto end() { return _table.end(); }
    
    auto begin() const { return _table.begin(); }
    auto end() const { return _table.end(); }
    
    
};



} // namespace manic

#endif /* space_hpp */
