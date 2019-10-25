//
//  bag.hpp
//  mania
//
//  Created by Antony Searle on 26/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef bag_hpp
#define bag_hpp

#include "table3.hpp"

namespace manic {

template<typename T,
         typename N = usize>
struct bag {
    
    table3<T, N> _table;
    usize _size;
    
    void push(T x) {
        if (_table.contains(x)) {
            ++_table.get(x);
        } else {
            _table.insert(x, 1);
        }
        ++_size;
    }
    
    T pop() {
        assert(_table.size());
        auto& r = _table.front();
        T x(r.key);
        if (--r.value) {
            _table.erase(r.key);
        }
        --_size;
        return x;
    }
    
    bool empty() const {
        return !_size;
    }
    
    bool size() const {
        return _size;
    }
    
    T const& top() const {
        return _table.front().key;
    }
    
    auto begin() const {
        return _table.begin();
    }
    
    auto end() const {
        return _table.end();
    }
    
};

} // namespace manic

#endif /* bag_hpp */
