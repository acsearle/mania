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
    
    // Interleave bits to achieve a 1D indexing of 2D space with decent
    // locality properties.  Good for spatial hashing
    //
    // https://en.wikipedia.org/wiki/Z-order_curve
    
    constexpr u64 _morton_expand(u64 x) noexcept {
        assert(x == (x & 0x00000000FFFFFFFF));
        x = (x | (x << 16)) & 0x0000FFFF0000FFFF;
        x = (x | (x <<  8)) & 0x00FF00FF00FF00FF;
        x = (x | (x <<  4)) & 0x0F0F0F0F0F0F0F0F;
        x = (x | (x <<  2)) & 0x3333333333333333;
        x = (x | (x <<  1)) & 0x5555555555555555;
        return x;
    }
    
    constexpr u64 _morton_contract(u64 x) noexcept {
        assert(x == (x & 0x5555555555555555));
        x = (x | (x >>  1)) & 0x3333333333333333;
        x = (x | (x >>  2)) & 0x0F0F0F0F0F0F0F0F;
        x = (x | (x >>  4)) & 0x00FF00FF00FF00FF;
        x = (x | (x >>  8)) & 0x0000FFFF0000FFFF;
        x = (x | (x >> 16)) & 0x00000000FFFFFFFF;
        return x;
    }
    
    constexpr u64 morton(u64 x, u64 y) noexcept {
        return _morton_expand(x) | (_morton_expand(y) << 1);
    }
    
    constexpr u64 morton2(u64 x) noexcept {
        // We use the XOR-trick to swap bit ranges.  We achive interleaving
        // by swapping the middle quarters of the bit range, and then recursing
        // down.
        u64 b = (x ^ (x >> 16)) & 0x00000000FFFF0000;
        x ^= b | (b << 16);
        b = (x ^ (x >> 8)) & 0x0000FF000000FF00;
        x ^= b | (b << 8);
        b = (x ^ (x >> 4)) & 0x00F000F000F000F0;
        x ^= b | (b << 4);
        b = (x ^ (x >> 2)) & 0x0C0C0C0C0C0C0C0C;
        x ^= b | (b << 2);
        b = (x ^ (x >> 1)) & 0x2222222222222222;
        x ^= b | (b << 1);
        return x;
    }
    
    constexpr u64 morton2_reverse(u64 x) noexcept {
        // Reverse the operation
        u64 b = (x ^ (x >> 1)) & 0x2222222222222222;
        x ^= b | (b << 1);
        b = (x ^ (x >> 2)) & 0x0C0C0C0C0C0C0C0C;
        x ^= b | (b << 2);
        b = (x ^ (x >> 4)) & 0x00F000F000F000F0;
        x ^= b | (b << 4);
        b = (x ^ (x >> 8)) & 0x0000FF000000FF00;
        x ^= b | (b << 8);
        b = (x ^ (x >> 16)) & 0x00000000FFFF0000;
        x ^= b | (b << 16);
        return x;
    }
    
}

#endif /* hash_hpp */
