//
//  terrain2.hpp
//  mania
//
//  Created by Antony Searle on 6/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef terrain2_hpp
#define terrain2_hpp

#include "terrain.hpp"
#include "table3.hpp"
#include "vec.hpp"
#include "matrix.hpp"

namespace manic {

struct terrain2 {
    
    using T = u8;
    
    // to speed lookup, consider
    // * reducing depth of indices
    // * storing T* instead and constructing view from implicit 16x16 knowledge
    mutable table3<vec<i64, 2>, matrix<T>> _table;
    
    static const i64 N = 16;
    static const i64 MASK = N - 1;
    
    static vec<i64, 2> _make_key(vec<i64, 2> ij) {
        return vec<i64, 2>{ij.x & ~MASK, ij.y & ~MASK};
    }
    
    double read(i64 i, i64 j) const {
        auto key = _make_key(vec<i64, 2>(i, j));
        matrix<T>* p = _table.try_get(key);
        T v = p ? (*p)(i & MASK, j & MASK) : 0;
        return v;
    }
    
    matrix<T> _make_for(i64 i, i64 j) {
        auto a = terrain(i & ~MASK, j & ~MASK, N, N, 0);
        matrix<u8> b(N, N);
        for (i = 0; i != N; ++i)
            for (j = 0; j != N; ++j)
                b(i, j) = (a(i, j) > 0);
        return b;
    }
    
    void write(i64 i, i64 j, double v) {
        auto key = _make_key(vec<i64, 2>(i, j));
        matrix<T>* p = _table.try_get(key);
        if (!p)
            p = &_table.insert(key, _make_for(i, j)).value;
        assert(p);
        (*p)(i & MASK, j & MASK) = v;
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
    
    T& _get_unsafe(vec<i64, 2> ij) {
        auto key = _make_key(ij);
        // perfect location for entry API
        auto p = _table.try_get(key);
        if (!p) {
            p = &_table.insert(key, _make_for(ij.x, ij.y)).value;
        }
        assert(p);
        T* q = &(*p)(ij.x & MASK, ij.y & MASK);
        return *q;
    }
    
    
    
};



} // namespace manic

#endif /* terrain2_hpp */
