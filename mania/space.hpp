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
    mutable table3<gl::vec<i64, 2>, matrix<T>> _table;
    
    static const i64 N = 16;
    static const i64 MASK = N - 1;
    
    gl::vec<i64, 2> _make_key(gl::vec<i64, 2> ij) {
        return gl::vec<i64, 2>{ij.x & ~MASK, ij.y & ~MASK};
    }
    
    T& _get_unsafe(gl::vec<i64, 2> ij) const {
        auto key = _make_key(ij);
        // perfect location for entry API
        auto p = _table.try_get(key);
        if (!p) {
            p = &_table.insert(key, matrix<T>(16, 16));
        }
        assert(p);
        return (*p)(ij.x & MASK, ij.y & MASK);
    }
    
    T& operator()(gl::vec<i64, 2> ij) {
        return _get_unsafe(ij);
    }

    T const& operator()(gl::vec<i64, 2> ij) const {
        return _get_unsafe(ij);
    }
    
    bool contains(gl::vec<i64, 2> ij) const {
        return _table.contains(_make_key(ij));
    }

    auto begin() { return _table.begin(); }
    auto end() { return _table.end(); }
    
    auto begin() const { return _table.begin(); }
    auto end() const { return _table.end(); }
    
    
};



} // namespace manic

#endif /* space_hpp */
