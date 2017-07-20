//
//  cg.cpp
//  mania
//
//  Created by Antony Searle on 20/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include <unordered_map>
#include <vector>

#include "cg.hpp"
#include "hash.hpp"



namespace manic {
    
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
                d = cross(pts[p[j]] - pts[p[i]], point - pts[p[i]]);
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
    
    double incircle(Point<2> d, Point<2> a, Point<2> b, Point<2> c) {
        circle cc = circumcircle(a, b, c);
        double radd = dot(d - cc.center);
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
    
    const double Delaunay::fuzz = 1e-6;
    const double Delaunay::bigscale = 1000.0;
    unsigned int Delaunay::jran = 14921620; // why not use a real rng?
    
    Delaunay::Delaunay(vector<Point<2>>& pvec, int options)
    : npts(pvec.size())
    , ntri(0)
    , ntree(0)
    , ntreemax(10*npts+1000)
    , opt(options)
    , pts(npts + 3) // extra for bounding triangle
    , thelist(ntreemax) {
        
        int j;
        double xl, xh, yl, yh;
        linehash = new unordered_map<uint64_t, int>(6*npts+12);
        trihash = new unordered_map<uint64_t, int>(2*npts+6);
        perm = new int[npts]; // permutation to randomize order
        xl = xh = pvec[0].x;
        yl = yh = pvec[0].y;
        for (j = 0; j < npts; ++j) { // for incoming points...
            pts[j] = pvec[j]; // copy
            perm[j] = j;      // init
            if (pvec[j].x < xl) xl = pvec[j].x; // bound
            if (pvec[j].x > xh) xh = pvec[j].x;
            if (pvec[j].y < yl) yl = pvec[j].y;
            if (pvec[j].y > yh) yh = pvec[j].y;
        }
        // make bounding triangle from points-past-the-end
        delx = xh - xl;
        dely = yh - yl;
        pts[npts] = Point<2>(0.5 * (xl + xh), yh + bigscale * dely);
        pts[npts+1] = Point<2>(xl - 0.5*bigscale * delx, yl - 0.5 * bigscale * dely);
        pts[npts+2] = Point<2>(xl + 0.5*bigscale * delx, yl - 0.5 * bigscale * dely);
        // create a permutation
        using std::swap;
        for (j = npts; j > 0; j--)
            swap(perm[j-1], perm[hash(jran++) % j]);
        // insert points in permuted order
        for (j = 0; j < npts; j++)
            insertapoint(perm[j]);
        for (j = 0; j < ntree; j++) { // delete root triangle and connectng edges (but it persists, for fast lookup)
            if (thelist[j].stat > 0) { // triangle is live
                if (thelist[j].p[0] >= npts ||
                    thelist[j].p[1] >= npts ||
                    thelist[j].p[2] >= npts) {
                    // the triangle is connected to the phantom vertices
                    thelist[j].stat = -1;
                    ntri--;
                }
            }
        }
        if (!(opt & 1)) {
            delete[] perm;
            delete trihash;
            delete linehash;
        }
    }
    
    
    
    
    
    
    
    
    struct gjy {
        
        gjy() {
            
            auto c = circumcircle(vec<double, 2>(1,0),
                                  vec<double, 2>(1,1),
                                  vec<double, 2>(0,1));
            
            std::cout << c << std::endl;
            
            
            
            std::unordered_map<uint64_t, uint64_t> h(1000);
            
            for (int i = 0; i != 1000; ++i) {
                std::cout << i << "->" << h.bucket(i) << "\n";
            }
            
            
            
            exit(0);
            
        };
        
    } gha;
    
    
    
}
