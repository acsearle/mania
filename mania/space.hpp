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
    mutable table3<vec<i64, 2>, matrix<T>> _table;
    
    static const i64 N = 16;
    static const i64 MASK = N - 1;
    
    static vec<i64, 2> _make_key(vec<i64, 2> ij) {
        return vec<i64, 2>{ij.x & ~MASK, ij.y & ~MASK};
    }
    
    T read(i64 i, i64 j) const {
        auto key = _make_key(vec<i64, 2>(i, j));
        T* p = _table.try_get(key);
        T v = p ? p->operator()(i & MASK, j & MASK) : 0;
        return v;
    }
    
    void write(i64 i, i64 j, u64 v) const {
        auto key = _make_key(vec<i64, 2>(i, j));
        T* p = _table.try_get(key);
        if (!p)
            p = &_table.insert(key, matrix<T>(16, 16)).value;
        assert(p);
        [[maybe_unused]] T v_old = *p;
        *p = v;
        // notify observer?
    }
    
    T& operator()(vec<i64, 2> ij) {
        return _get_unsafe(ij);
    }

    T operator()(vec<i64, 2> ij) const {
        return read(ij.x, ij.y);
    }
    
    T& operator()(i64 i, i64 j) {
        return _get_unsafe(vec<i64, 2>{i, j});
    }

    T operator()(i64 i, i64 j) const {
        return read(i, j);
    }

    
    bool contains(vec<i64, 2> ij) const {
        return _table.contains(_make_key(ij));
    }

    auto begin() { return _table.begin(); }
    auto end() { return _table.end(); }
    
    auto begin() const { return _table.begin(); }
    auto end() const { return _table.end(); }

    T* _try_get(vec<i64, 2> ij) const {
        auto key = _make_key(ij);
        auto p = _table.try_get(key);
        return p ? &(*p)(ij.x & MASK, ij.y & MASK) : nullptr;
    }
    
    T& _get_unsafe(vec<i64, 2> ij) const {
        auto key = _make_key(ij);
        // perfect location for entry API
        auto p = _table.try_get(key);
        if (!p) {
            p = &_table.insert(key, matrix<T>(16, 16)).value;
        }
        assert(p);
        T* q = &(*p)(ij.x & MASK, ij.y & MASK);
        return *q;
    }
    
    
    
};



} // namespace manic

#endif /* space_hpp */
