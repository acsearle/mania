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

namespace manic {

template<typename T, typename N = u64>
struct rleq {
    
    // Run-length-encoded queue and stack
    
    struct entry {
        T value;
        N count;
    };
    
    vector<entry> _array;
    usize _size = 0;
    
    void push_back(T x) {
        push_back(x, 1);
    }
    
    void push_back(T x, N n) {
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
    
    
};

} // namespace manic

#endif /* rleq_hpp */
