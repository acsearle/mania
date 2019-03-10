//
//  bit.hpp
//  mania
//
//  Created by Antony Searle on 24/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef bit_hpp
#define bit_hpp

// From C++20

#include <cstring>

namespace std {
    
    template<typename To, typename From>
    constexpr To bit_cast(const From& from) noexcept;
    
    // integral powers of 2
    template <class T>
    constexpr bool ispow2(T x) noexcept;
    template <class T>
    constexpr T ceil2(T x) noexcept;
    template <class T>
    constexpr T floor2(T x) noexcept;
    template <class T>
    constexpr T log2p1(T x) noexcept;
    
    template<typename T>
    constexpr bool ispow2(T x) noexcept {
        return !(x & (x - 1));
    }
    
    template<typename T>
    constexpr T ceil2(T x) noexcept {
        --x;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        x |= x >> 32;
        return ++x;
    }
    
    template<typename T>
    constexpr T floor2(T x) noexcept {
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        x |= x >> 32;
        return x - (x >> 1);
    }
    
    template<typename T>
    constexpr int log2p1(T x) noexcept {
        return x ? (63 - __builtin_clzll(x)) : 0;
    }
    
}

#endif /* bit_hpp */
