//
//  rleq.hpp
//  mania
//
//  Created by Antony Searle on 26/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef rleq_hpp
#define rleq_hpp

#include "common.hpp"

#include "vector.hpp"
#include "serialize.hpp"

namespace manic {

template<typename T, typename I = u64>
struct rleq {
    
    // Run-length-encoded queue and stack
    
    struct entry {
        T value;
        I count;
    };
    
    vector<entry> _array;
    usize _size = 0;
    
    bool _invariant() {
        I m = 0;
        for (auto&& [_, n] : _array)
            m += n;
        return _size == m;
    }
    
    void push_back(T x) {
        push_back(x, 1);
    }
    
    void push_back(T x, I n) {
        if (!_array.size() || (_array.back().value != x)) {
            _array.push_back(entry{x, n});
        } else {
            assert(_array.back().count);
            _array.back().count += n;
            assert(_array.back().count); // we have overflowed the counter
        }
        _size += n;
    }
    
    T pop_front() {
        assert(_array.size());
        assert(_array.front().count);
        T x = _array.front().value;
        if (!--_array.front().count) {
            _array.pop_front();
        }
        --_size;
        return x;
    }
    
    T pop_back() {
        assert(_array.size());
        assert(_array.back().count);
        T x = _array.back().value;
        if (!--_array.back().count) {
            _array.pop_back();
        }
        --_size;
    }
    
    bool empty() const {
        return _array.empty();
    }
    
    usize size() const {
        return _size;
    }
    
    T const& front() const {
        assert(_array.size());
        return _array.front().value;
    }
    
    T const& back() const {
        assert(_array.size());
        return _array.back().value;
    }
    
}; // rleq<T, I>

template<typename T, typename I, typename Serializer>
void serialize(rleq<T, I> const& x, Serializer& s) {
    assert(x._invariant());
    serialize(x._array.size(), s);
    for (auto&& [y, n] : x._array) {
        serialize(y, s);
        serialize(n, s);
    }
}

template<typename T, typename I, typename Deserializer>
auto deserialize(placeholder<rleq<T, I>>, Deserializer& d) {
    auto n = deserialize<isize>(d);
    vector<typename rleq<T, I>::entry> y;
    y.reserve(n);
    I m = 0;
    while (n--) {
        auto a = deserialize<T>(d);
        auto b = deserialize<I>(d);
        m += b;
        y.push_back(typename rleq<T, I>::entry{std::move(a), std::move(b)});
    }
    return rleq<T, I>{std::move(y), m};
}

} // namespace manic

#endif /* rleq_hpp */
