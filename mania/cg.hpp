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
        //auto aOld = a;
        //auto bOld = b;
        //auto cOld = c;
        
        a -= b;
        c -= b;
        auto det = a[0] * c[1] - a[1] * c[0];
        assert(det);
        det = 0.5 / det;
        auto asq = a[0]*a[0]+a[1]*a[1];
        auto csq = c[0]*c[0]+c[1]*c[1];
        //auto ctr = det * vec<double, 2>(asq * c[1] - csq * a[1],
         //                               csq * a[0] - asq * c[0]);
        auto ctr0 = det*(asq*c[1]-csq*a[1]);
        auto ctr1 = det*(csq*a[0]-asq*c[0]);
        // fixme: sqrt
        auto ctr = vec<double, 2>(ctr0, ctr1);
        
        auto trueCtr = ctr + b;
        //std::cout << "circumpoints " << length(aOld - trueCtr) << ", " << length(bOld - trueCtr) << ", " << length(cOld - trueCtr) << std::endl;
        
        return circle(ctr + b, length(ctr));
    }
    
    
    
    using namespace gl;
    
    template<std::size_t N>
    using Point = gl::vec<double, N>;
    
    struct Triel {
        Point<2> *pts; // pointer to array of points
        int p[3]; // index of three vertices in ccw order
        int d[3]; // index to daughter triangles
        int stat; // liveness flag
        void setme(int a, int b, int c, Point<2> *ptss) {
            pts = ptss;
            p[0] = a;
            p[1] = b;
            p[2] = c;
            d[0] = d[1] = d[2] = -1;
            stat = 1;
        }
        int contains(Point<2> point) {
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
                if (d == 0.0)
                    ztest = 1;
            }
            return ztest ? 0 : 1;
        }
    };
    
    inline double incircle(Point<2> d, Point<2> a, Point<2> b, Point<2> c) {
        circle cc = circumcircle(a, b, c);
        double radd = (d[0] - cc.center[0]) * (d[0] - cc.center[0]) + (d[1] - cc.center[1]) * (d[1] - cc.center[1]);
        // todo: unncessary square root in circumcircle for this purpose
        return cc.radius * cc.radius - radd;
    }
    
    using std::vector;
    using std::unordered_map;
    
    struct Delaunay {
        int npts, ntri, ntree, ntreemax, opt;
        double delx, dely;
        vector<Point<2>> pts;
        vector<Triel> thelist;
        unordered_map<uint64_t, int> *linehash;
        unordered_map<uint64_t, int> *trihash;
        int* perm;
        
        Delaunay(vector<Point<2>>& pvec, int options = 0);
        
        // interpolate goes here
        
        void insertapoint(int r);
        int whichcontainspoint(const Point<2>& p, int strict = 0);
        int storetriangle(int a, int b, int c);
        void erasetriangle(int a, int b, int c, int d0, int d1, int d2);
        static unsigned int jran;
        static const double fuzz, bigscale;
        
    };

    
}

#endif /* cg_hpp */
