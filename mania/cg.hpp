//
//  cg.hpp
//  mania
//
//  Created by Antony Searle on 20/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef cg_hpp
#define cg_hpp

#include "vec.hpp"

namespace manic {
    
    using gl::vec;
    
    struct circle {
        
        vec<double, 2> center;
        double radius; // would it be better to store r^2?
        
        circle(vec<double, 2> x, double r)
        : center(x), radius(r) {
        }
        
    };
    
    std::ostream& operator<<(std::ostream& a, const circle& b) {
        return a << "circle(" << b.center << "," << b.radius << ")";
    }
    
    inline circle circumcircle(vec<double, 2> a, vec<double, 2> b, vec<double, 2> c) {
        a -= b;
        c -= b;
        auto det = a[0] * c[1] - a[1] * c[0];
        assert(det);
        det = 0.5 / det;
        auto asq = dot(a);
        auto csq = dot(c);
        auto ctr = det * vec<double, 2>(asq * c[1] - csq * a[1],
                                        csq * a[0] - asq * c[0]);
        // fixme: sqrt
        return circle(ctr + b, length(ctr));
    }
    
    
    
}

#endif /* cg_hpp */
