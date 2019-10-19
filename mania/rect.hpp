//
//  rect.hpp
//  mania
//
//  Created by Antony Searle on 9/3/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef rect_hpp
#define rect_hpp

#include "vec.hpp"

namespace manic {
    
    // There are many defensible choices for rect storage
    //     top-left, bottom-right
    //     interval-x, interval-y
    //     top-left, width-height
    //
    // Storing corner vertices means that two of the quad vertices are copies
    
    template<typename T>
    class rect {
        
    public:
        
        gl::vec<T, 2> a, b;
        
        bool invariant() const {
            return (a.x <= b.x) && (a.y <= b.y);
        }
        
        rect(const rect&) = default;
        
        rect(const gl::vec<T, 2>& x,
             const gl::vec<T, 2>& y)
        : a(x)
        , b(y) {
        }
        
        rect(T ax, T ay, T bx, T by)
        : a(ax, ay)
        , b(bx, by) {
        }
        
        void canonicalize() {
            using std::swap;
            if (a.x > b.x)
                swap(a.x, b.x);
            if (a.y > b.y)
                swap(a.y, b.y);
        }
        
        gl::vec<T, 2> size() const { return b - a; }
        
        T width() const { return b.x - a.x; }
        T height() const { return b.y - a.y; }
        
        friend T area(const rect<T>& x) { return product(x.b - x.a); }
        
    };
    
    struct area_cmp {
        template<typename A, typename B>
        bool operator()(A&& a, B&& b) const {
            return area(std::forward<A>(a)) < area(std::forward<B>(b));
        }
    };
    
    template<typename T>
    rect<T> operator+(const rect<T>& x) {
        return x;
    }
    
    template<typename T>
    rect<T> operator-(const rect<T>& x) {
        return rect(-x.b, -x.a);
    }
    
    // Minkowski sum
    
    template<typename T>
    rect<T> operator+(const rect<T>& a, const rect<T>& b) {
        return rect(a.a + b.a, a.b + b.b);
    }
    
    // Minkowski difference is defined so that
    //
    //     (A - B) + B == A
    //
    // but
    //
    //     A - B != A + (-B)
    
    template<typename T>
    rect<T> operator-(const rect<T>& a, const rect<T>& b) {
        return rect(a.a - b.a, a.b - b.b);
    }
    
    // Scaling
    
    template<typename T>
    rect<T> operator*(const rect<T>& a, T b) {
        return rect<T>(a.a * b, a.b * b);
    }
    
    template<typename T>
    rect<T> operator*(T a, const rect<T>& b) {
        return rect<T>(a * b.a, a * b.b);
    }
    
    template<typename T>
    rect<T> operator/(const rect<T>& a, T b) {
        return rect<T>(a.a / b, a.b / b);
    }
    
    // Enclosing rect
    
    template<typename T>
    rect<T> hull(const rect<T>& a, const rect<T>& b) {
        return rect<T>(std::min(a.a.x, b.a.x),
                       std::min(a.a.y, b.a.y),
                       std::max(a.b.x, b.b.x),
                       std::max(a.b.y, b.b.y));
    }
    
    template<typename T>
    bool overlap(const rect<T>& a, const rect<T>& b) {
        return (((a.a.x < b.b.x) && (b.a.x < a.b.x)) &&
                ((a.a.x < b.b.x) && (b.a.x < a.b.x)));
    }
    
    template<typename T>
    rect<T> intersection(const rect<T>& a, const rect<T>& b) {
        return rect<T>(std::max(a.a.x, b.a.x),
                       std::max(a.a.y, b.a.y),
                       std::min(a.b.x, b.b.x),
                       std::min(a.b.y, b.b.y));
    }
    
} // namespace manic

#endif /* rect_hpp */