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
#include "serialize.hpp"

namespace manic {

constexpr isize CHUNK_SIZE = 16;
constexpr isize CHUNK_MASK = CHUNK_SIZE - 1;

// F(vec<i64, 2>) must yield a function object that yields an N x N
// subscriptable object

template<typename T>
struct _space2_default {
    auto operator()(vec<i64, 2>) const {
        return []() {
            return matrix<T>(16, 16);
        };
    }
};

template<typename T>
struct _dumb_matrix {
    
    T* _ptr;
    
    _dumb_matrix()
    : _ptr((T*) calloc(CHUNK_SIZE * CHUNK_SIZE, sizeof(T))) {
    }
  
    _dumb_matrix(_dumb_matrix&& x)
    : _ptr(std::exchange(x._ptr, nullptr)) {
    }
    
    _dumb_matrix(_dumb_matrix const& x)
    : _dumb_matrix() {
        std::memcpy(_ptr, x._ptr, CHUNK_SIZE * CHUNK_SIZE * sizeof(T));
    }
    
    ~_dumb_matrix() { free(_ptr); }
    
    _dumb_matrix& operator=(_dumb_matrix&& x) {
        _dumb_matrix tmp(std::move(x));
        using std::swap;
        swap(_ptr, tmp._ptr);
        return *this;
    }
    
    _dumb_matrix& operator=(_dumb_matrix const& x) {
        _dumb_matrix tmp{x};
        using std::swap;
        swap(_ptr, tmp._ptr);
        return *this;
    }
    
    T& operator()(vec<i64, 2> xy) { return *(_ptr + xy.x * 16 + xy.y); }
    
};
    
    template<typename T, typename Serializer>
    void serialize(_dumb_matrix<T> const& x, Serializer& s) {
        for (isize i = 0; i != CHUNK_SIZE * CHUNK_SIZE; ++i)
            serialize(x._ptr[i], s);
    }

    template<typename T, typename Deserializer>
    void deserialize(placeholder<_dumb_matrix<T>>, Deserializer& d) {
        _dumb_matrix<T> x;
        for (isize i = 0; i != CHUNK_SIZE * CHUNK_SIZE; ++i)
            x._ptr[i] = deserialize<T>(d);
        return x;
    }


template<typename T>
struct _space2_inline {
    auto operator()(vec<i64, 2>) const {
        return []() { return _dumb_matrix<T>{}; };
    }
};
    
    template<typename T, typename Serializer>
    void serialize(_space2_inline<T> const&, Serializer&) {
        // monostate
    }

    template<typename T, typename Deserializer>
    void deserialize(placeholder<_space2_default<T>>, Deserializer&) {
        return _space2_inline<T>{};
    }

    
template<typename F>
struct space2 {
    
    using M = std::decay_t<decltype(std::declval<F>()(std::declval<vec<i64, 2>>())())>;
    using T = std::decay_t<decltype(std::declval<M>()(std::declval<vec<i64, 2>>()))>;
    
    F _generator;
    table3<vec<i64, 2>, M> _table;
        
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
        
    M& get_chunk(vec<i64, 2> xy) {
        auto uv = _high(xy);
        return _table.entry(uv).or_insert_with(_generator(uv));
    }

    M* try_get_chunk(vec<i64, 2> xy) {
        return _table.try_get(_high(xy));
    }
    
    T* try_get(vec<i64, 2> xy) {
        M* p = _table.try_get(_high(xy));
        return p ? (*p)(_low(xy)) : nullptr;
    }
    
    T& get(vec<i64, 2> xy) {
        return get_chunk(xy)(_low(xy));
    }

    T& operator()(vec<i64, 2> xy) {
        return get(xy);
    }
    
}; // struct space2
    
    template<typename F, typename Serializer>
    void serialize(space2<F>& x, Serializer& s) {
        serialize(x._generator, s);
        serialize(x._table, s);
    }
    
    template<typename F, typename Deserializer>
    void deserialize(placeholder<space2<F>>, Deserializer& d) {
        space2<F> x{deserialize<F>(d)};
        x._table = deserialize<decltype(x._table)>(d);
    }

} // namespace manic

#endif /* space2_hpp */
