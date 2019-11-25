//
//  dense_table.hpp
//  mania
//
//  Created by Antony Searle on 27/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef dense_table_hpp
#define dense_table_hpp

#include "common.hpp"
#include "raw_vector.hpp"

namespace manic {

template<typename K, typename V>
struct dense_table {
    
    static_assert(std::is_integral_v<K>);
    static_assert(std::is_trivial_v<V>); // is_relocatable?
    
    raw_vector<V> _array;
    
    usize size() const {
        return _array.size();
    }
    
    void resize(usize n) {
        if (n > size()) {
            raw_vector<V> v(std::max(n, size() * 2));
            std::memcpy(v.begin(), _array.begin(), size() * sizeof(V));
            swap(_array, v);
        }
    }
    
    void insert(K k, V v) {
        resize(k);
        _array[k] = v;
    }
    
    V& get(K key) {
        return _array[key];
    }

    V const& get(K key) const {
        return _array[key];
    }

};

} // namespace manic

#endif /* dense_table_hpp */
