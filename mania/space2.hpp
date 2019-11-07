//
//  space2.hpp
//  mania
//
//  Created by Antony Searle on 7/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef space2_hpp
#define space2_hpp

#include "table3.hpp"
#include "vec.hpp"
#include "matrix.hpp"

namespace manic {

// F(vec<i64, 2>) must yield a function object that yields an N x N matrix<T>
//

template<typename T>
struct _space2_default {
    auto operator()(vec<i64, 2>) const {
        return []() {
            return matrix<T>(16, 16);
        };
    }
};

template<typename T, typename F = _space2_default<T>>
struct space2 {
    
    table3<vec<i64, 2>, matrix<T>> _table;
    F _generator;
    
    static constexpr i64 N = 16;
    static constexpr i64 MASK = N - 1;
    
    space2() = default;
    
    space2(F const& f) : _generator(f) {}
    space2(F&& f) : _generator(std::move(f)) {}
    
    static vec<i64, 2> _high(vec<i64, 2> xy) {
        return vec<i64, 2>{xy.x & ~MASK, xy.y & ~MASK};
    }

    static vec<i64, 2> _low(vec<i64, 2> xy) {
        return vec<i64, 2>{xy.x & MASK, xy.y & MASK};
    }
    
    bool contains(vec<i64, 2> xy) {
        return _table.contains(_high(xy));
    }
        
    matrix<T>& get_chunk(vec<i64, 2> xy) {
        auto uv = _high(xy);
        return _table.get_or_insert_with(uv, _generator(uv));
    }

    matrix<T>* try_get_chunk(vec<i64, 2> xy) {
        return _table.try_get(_high(xy));
    }
    
    T* try_get(vec<i64, 2> xy) {
        matrix<T>* p = _table.try_get(_high(xy));
        return p ? (*p)(_low(xy)) : nullptr;
    }
    
    T& get(vec<i64, 2> xy) {
        return get_chunk(xy)(_low(xy));
    }

    T& operator()(vec<i64, 2> xy) {
        return get(xy);
    }
    
};

} // namespace manic

#endif /* space2_hpp */
