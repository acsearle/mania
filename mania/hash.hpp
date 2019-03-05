//
//  hash.hpp
//  mania
//
//  Created by Antony Searle on 20/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef hash_hpp
#define hash_hpp

#include <cstdint>
#include <cmath>

#include "common.hpp"

namespace manic {
    
    template<typename T>
    auto sqr(const T& t) {
        return t * t;
    }
    
    class rand {
        
        uint64_t _x;
        
    public:
        
        explicit rand(uint64_t seed = 0)
        : _x(4101842887655102017ull) {
            _x ^= seed;
        }
        
        uint64_t operator()() {
            _x ^= _x >> 21; _x ^= _x << 35; _x ^= _x >> 4;
            return _x * 2685821657736338717ull;
        }
        
        using result_type = uint64_t;
        static constexpr uint64_t min() { return 1; }
        static constexpr uint64_t max() { return -1ull; }
        
    };
    
    template<typename T = rand>
    struct uniform {
        T _x;
        explicit uniform(uint64_t seed = 0)
        : _x(seed) {
        }
        double operator()() {
            return _x() * 5.42101086242752217e-20;
        }
    };
    
    template<typename T = uniform<>>
    class normal {
        
        T _x;
        
    public:
        
        explicit normal(uint64_t seed = 0) : _x(seed) {}
        
        double operator()() {
            // normal deviate by ratio of uniforms
            double u, v, q;
            do {
                u = _x();
                v = 1.7156 * (_x() - 0.5);
                auto x = u - 0.449871;
                using std::abs;
                auto y = abs(v) + 0.386595;
                q = sqr(x) + y * (0.19600 * y - 0.25472 * x);
            } while ((q > 0.27597)
                     && ((q > 0.27846)
                         || (sqr(v) > -4.0 * log(u) * sqr(u))));
            return v / u;
        }
        
    };

    // Numerical Recipes 7.1.4
    //
    // "a random hash of the integers, one that passes serious tests for
    // randomness, even for very ordered sequences of [input]", i.e. hash(i++)
    // is a high-quality random number generator
    //
    // hash(x) is also an injective function
    //
    // better than libc++'s trivial std::hash implementation
    //
    inline uint64_t hash(uint64_t x) {
        x = x * 3935559000370003845ull + 2691343689449507681ull;
        x ^= x >> 21; x ^= x << 37; x ^= x >> 4;
        x *= 4768777513237032717ull;
        x ^= x << 20; x ^= x >> 41; x ^= x << 5;
        return x;
    }
    
    
    
    
}

#endif /* hash_hpp */
