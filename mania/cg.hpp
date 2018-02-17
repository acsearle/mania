//
//  cg.hpp
//  mania
//
//  Created by Antony Searle on 20/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef cg_hpp
#define cg_hpp

#include <unordered_map>

#include "vec.hpp"

namespace manic {
    
    using gl::vec;
    
    struct circle {
        
        vec<double, 2> center;
        double r2;
        
        circle(vec<double, 2> x, double radius_squared)
        : center(x), r2(radius_squared) {
        }
        
    };
    
    inline std::ostream& operator<<(std::ostream& a, const circle& b) {
        return a << "circle(center=" << b.center << ",radius^2=" << b.r2 << ")";
    }
    
    inline circle circumcircle(vec<double, 2> a,
                               vec<double, 2> b,
                               vec<double, 2> c) {
        a -= b;
        c -= b;
        auto det = cross(a, c);
        // precondition: points are not colinear
        assert(det);
        det = 0.5 / det;
        auto ctr = det * (sqr(c) * perp(a) - sqr(a) * perp(c));
        // postcondition:
        // length(a - ctr) \approxeq length(ctr);
        // length(ctr) \approxeq (c - ctr);
        return circle(ctr + b, sqr(ctr));
    }
    
    
    
    using namespace gl;
    
    struct Triel {
        vec<double, 2> *pts; // pointer to array of points
        int p[3]; // index of three vertices in ccw order
        int d[3]; // index to daughter triangles
        int stat; // liveness flag
        void setme(int a, int b, int c, vec<double, 2> *ptss) {
            pts = ptss;
            p[0] = a;
            p[1] = b;
            p[2] = c;
            d[0] = d[1] = d[2] = -1;
            stat = 1;
        }
        int contains(vec<double, 2> point) {
            double d;
            int i, j, ztest = 0;
            for (i = 0; i != 3; ++i) {
                j = (i + 1) % 3;
                // which side of edge ij is point on?
                //d = cross(pts[p[j]] - pts[p[i]], point - pts[p[i]]);
                
                d = (pts[p[j]][0]-pts[p[i]][0])*(point[1]-pts[p[i]][1])
                - (pts[p[j]][1]-pts[p[i]][1])*(point[0]-pts[p[i]][0]);
                
                // outside
                if (d < 0.0)
                    return -1;
                // on the edge
                if (d == 0.0) {
                    //std::cout << "Triel::contains hit an edge" << std::endl;
                    ztest = 1;
                }
            }
            return ztest ? 0 : 1;
        }
    };
    
    inline double incircle(vec<double, 2> d, vec<double, 2> a, vec<double, 2> b, vec<double, 2> c) {
        circle cc = circumcircle(a, b, c);
        return cc.r2 - sqr(d - cc.center);
    }
    
    using std::vector;
    using std::unordered_map;
    
    struct Delaunay {
        int npts, ntri, ntree, ntreemax, opt;
        double delx, dely;
        vector<vec<double, 2>> pts;
        vector<Triel> thelist;
        unordered_map<uint64_t, int> *linehash;
        unordered_map<uint64_t, int> *trihash;
        //int* perm;
        
        Delaunay(vector<vec<double, 2>>& pvec, int options = 0);
        
        // interpolate goes here
        
        void insertapoint(int r);
        int whichcontainspoint(const vec<double, 2>& p, int strict = 0);
        int storetriangle(int a, int b, int c);
        void erasetriangle(int a, int b, int c, int d0, int d1, int d2);
        
        vector<vec<int, 3>> triangles() const;
        
        //static unsigned int jran;
        static const double fuzz, bigscale;
        
    };

    
}

#endif /* cg_hpp */
