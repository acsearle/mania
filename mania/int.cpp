//
//  int.cpp
//  mania
//
//  Created by Antony Searle on 8/4/18.
//  Copyright © 2018 Antony Searle. All rights reserved.
//

#include "int.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

/*
 
 8  1   2.2.2    2 a 0.5
 12 1.5 2.2.3    3 c 0.75
 16 2   2.2.2.2  4 b 1.0
 20 2.5 2.2.5    5 c 1.25
 24 3   2.2.2.3  6 a 1.5
 28
 32
 36 4.5 2.2.3.3  9 c 2.25
 40 5   2.2.2.5 10 a 2.5
 44
 48
 52
 56 7   2.2.2.7 14 a
 
 168 hailfire inside, 24*4 = 96 flanges outside
 
 
 60 = 2.30 = 2.2.15 = 2.2.3.5
 
 1:2 = 2:4 3:6 5:10
 1:3 = 2:6 3:9
 1:5 = 2:10
 1:7 = 2:14
 2:3 = 2:3 4:6
 2:5 = 2:5 4:10
 2:7 = 2:14
 3:5 = 3:5 6:10
 
 Linear whole
 2:6 2:10 4:4 6:10 3:5 3:9
 1:3 1:5  1:1 3:5  3:5 1:3
 Offset whole (hypot(1,2) = sqrt(5) = 2.23 ~= 2.25 studs (0.11 mm difference)
 3:6 4:5
 1:2 4:5
 Linear half
 2:4 4:6 4:10 5:9
 1:2 2:3 2:5  5:9
 
 
 
 
 2.2.3.5
 1.1.1.1
 
 3-4-5
 

 
 */

namespace manic {
    
    class bigint {
        std::vector<uint64_t> _limbs;
        
        // __builtin_addc(x, y, carryin, &carryout) is correct only when
        // carryin is 0 or 1; it cannot be an arbitrary number.  More precisely,
        // x + y + carryin must be less than 2^(n+2)-1 for n bit integers, since
        // if carries occur twice in the two additions, only one carry is
        // reported
        
        uint64_t add(uint64_t* first, uint64_t* last, uint64_t x) {
            uint64_t carry = x;
            for (; carry && (first != last); ++first)
                *first = __builtin_addcll(*first, 0, carry, &carry);
            return carry;
        }
        
        uint64_t add(uint64_t* first, uint64_t* last, uint64_t* right) {
            uint64_t carry = 0;
            for (; first != last; ++first, ++right) {
                *first = __builtin_addcll(*first, *right, carry, &carry);
            }
            return carry;
        }

        uint64_t mul(uint64_t* first, uint64_t* last, uint64_t x) {
            uint64_t carry = 0;
            for (; (first != last); ++first) {
                unsigned __int128 y = (unsigned __int128) *first * (unsigned __int128) x;
                *first = __builtin_addcll((uint64_t) y, carry, 0, &carry);
                carry += (uint64_t) (y >> 64); // cannot overflow
            }
            return carry;
        }
        
        uint64_t add(uint64_t* z, const uint64_t* x, const uint64_t* y, const uint64_t* y_end) {
            uint64_t carry = 0;
            for (; y != y_end; ++z, ++x, ++y) {
                *z = __builtin_addcll(*x, *y, carry, &carry);
            }
            return carry;
        }
        
        // z += x * y
        void mul(uint64_t* z, const uint64_t* x, const uint64_t* x_end, uint64_t y) {
            uint64_t carrylo = 0;
            uint64_t carryhi = 0;
            for (; x != x_end; ++z, ++x) {
                unsigned __int128 w = (unsigned __int128) *x * (unsigned __int128) y;
                uint64_t carryout1;
                uint64_t carryout2;
                *z = __builtin_addcll(*z, carrylo, 0, &carryout1);
                *z = __builtin_addcll(*z, (uint64_t) w, 0, &carryout2);
                carrylo = __builtin_addcll((uint64_t) (w >> 64), (carryout1 + carryout2 + carryhi), 0, &carryhi);
            }
            // now is just addition
            *z = __builtin_addcll(*z, carrylo, 0, &carrylo);
            carrylo += carryhi;
            while (carrylo) {
                ++z;
                *z = __builtin_addcll(*z, carrylo, 0, &carrylo);
            }
        }
        
        // z += x * y
        void mul(uint64_t* z, const uint64_t* x, const uint64_t* x_end, const uint64_t* y, const uint64_t* y_end) {
            for (; y != y_end; ++y, ++z) {
                mul(z, x, x_end, *y);
            }
        }
        
        
    public:
        bigint& operator+=(uint64_t x) {
            auto c = add(_limbs.data(), _limbs.data() + _limbs.size(), x);
            if (c)
                _limbs.push_back(c);
            return *this;
        }
        bigint& operator*=(uint64_t x) {
            auto c = mul(_limbs.data(), _limbs.data() + _limbs.size(), x);
            if (c)
                _limbs.push_back(c);
            return *this;
        }
    };
    
 
    
}
