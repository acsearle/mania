//
//  ordset.hpp
//  mania
//
//  Created by Antony Searle on 7/3/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef ordset_hpp
#define ordset_hpp

#include <algorithm>
#include <vector>

#include "common.hpp"


namespace manic {

    template<typename T>
    class ordered_multiset {
        
        std::vector<T> _v;
        
    public:
        
        using const_iterator = typename std::vector<T>::const_iterator;
        
        void insert(const T& x) {
            _v.insert(std::upper_bound(_v.begin(), _v.end(), x), x);
        }
        
        template<typename I>
        void insert(I a, I b) {
            _v.insert(a, b);
            std::stable_sort(_v.begin(), _v.end());
        }
        
        const_iterator erase(const_iterator i) {
            return _v.erase(i);
        }
        
        const_iterator erase(const_iterator a, const_iterator b) {
            return _v.erase(a, b);
        }
        
        auto erase(const T& x) {
            auto p = std::equal_range(_v.begin(), _v.end(), x);
            auto n = p.second - p.first;
            _v.erase(p.first, p.second);
            return n;
        }
        
        isize count(const T& x) const {
            auto p = std::equal_range(_v.begin(), _v.end(), x);
            return p.second - p.first;
        }
        
        
        
    };
    
}

#endif /* ordset_hpp */
