//
//  f8.hpp
//  mania
//
//  Created by Antony Searle on 24/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef f8_h
#define f8_h

#include <cmath>
#include <iostream>

#include "common.hpp"

namespace manic {
    
    // 8-bit fixed-point format representing the closed interval [0.0, 1.0],
    // suitable for texture components.  With extreme restrictions on range
    // and accuracy, the class is best used as the beginning or end of a
    // chain of operations.
    //
    // As for u8, (mixed) arithmetic will promote to a wider type.
    //
    // We define conversions from i32 to improve interaction with simple
    // literals.
    
    struct f8 {
        
        u8 _u8;
        
        f8() = default;
        
        explicit f8(i32 x) : _u8(x * 255) {}
        explicit f8(f32 x) : _u8(std::round(x * 255.0f)) {}
        explicit f8(f64 x) : _u8(std::round(x * 255.0)) {}
        
        f8& operator=(i32 x) { _u8 = x * 255; return *this; }
        f8& operator=(f32 x) { _u8 = static_cast<u8>(std::round(x * 255.0f)); return *this; }
        f8& operator=(f64 x) { _u8 = static_cast<u8>(std::round(x * 255.0)); return *this; }
        
        explicit operator bool() const { return static_cast<bool>(_u8); }
        explicit operator f32() const { return _u8 / 255.0f; }
        explicit operator f64() const { return _u8 / 255.0; }
        
        f8 operator+() const { return *this; }
        double operator-() const { return _u8 / -255.0; }
        
        bool operator!() const { return !_u8; }
        
        f8 operator+=(f8 x) { _u8 += x._u8; return *this; }
        f8 operator-=(f8 x) { _u8 -= x._u8; return *this; }
        f8 operator*=(f8 x) { _u8 = _u8 * x._u8 / 255; return *this; }
        f8 operator/=(f8 x) { _u8 /= x._u8; return *this; }
        
        f8 operator*=(i32 x) { _u8 *= x; return *this; }
        f8 operator/=(i32 x) { _u8 /= x; return *this; }
        
        f8 operator+=(f32 x) { _u8 = static_cast<u8>(std::round(_u8 + x * 255.0f)); return *this; }
        f8 operator-=(f32 x) { _u8 = static_cast<u8>(std::round(_u8 - x * 255.0f)); return *this; }
        f8 operator*=(f32 x) { _u8 = static_cast<u8>(std::round(_u8 * x)); return *this; }
        f8 operator/=(f32 x) { _u8 = static_cast<u8>(std::round(_u8 / x)); return *this; }
        
        f8 operator+=(f64 x) { _u8 = static_cast<u8>(std::round(_u8 + x * 255.0)); return *this; }
        f8 operator-=(f64 x) { _u8 = static_cast<u8>(std::round(_u8 - x * 255.0)); return *this; }
        f8 operator*=(f64 x) { _u8 = static_cast<u8>(std::round(_u8 * x)); return *this; }
        f8 operator/=(f64 x) { _u8 = static_cast<u8>(std::round(_u8 / x)); return *this; }
        
    };
    
#define G(X, T)\
bool operator X (f8 a, T b) { return a._u8 X (b * 255); }\
bool operator X (T a, f8 b) { return (a * 255) X b._u8; }\

#define H(X)\
    G(X,u8)\
    G(X,i8)\
    G(X,u16)\
    G(X,i16)\
    G(X,u32)\
    G(X,i32)\
    G(X,u64)\
    G(X,i64)\
    G(X,f32)\
    G(X,f64)\

#define F(X)\
bool operator X (f8 a, f8 b) { return a._u8 X b._u8; }\
H(X)

    F(==)
    F(!=)
    F(<)
    F(>)
    F(<=)
    F(>=)

    
#undef G
#define G(X, T)\
    double operator X (f8 a, T b) { return (a._u8 / 255.0) X b; }\
    double operator X (T a, f8 b) { return a X (b._u8 / 255.0); }\

    H(+)
    H(-)
    H(*)
    H(/)

    f64 operator+(f8 a, f8 b) {
        return (a._u8 + b._u8) / 255.0;
    }
    
    f64 operator-(f8 a, f8 b) {
        return (a._u8 - b._u8) / 255.0;
    }

    f64 operator*(f8 a, f8 b) {
        return a._u8 * b._u8 / (255.0 * 255.0);
    }
    
    f64 operator/(f8 a, f8 b) {
        return static_cast<f64>(a._u8) / b._u8;
    }
    
    std::ostream& operator<<(std::ostream& a, f8 b) {
        return a << static_cast<f32>(b);
    }
    
}


#endif /* f8_h */
