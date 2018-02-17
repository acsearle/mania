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
    
        
    const double Delaunay::fuzz = 1e-6;
    const double Delaunay::bigscale = 1000.0;
    
    Delaunay::Delaunay(vector<vec<double, 2>>& pvec, int options)
    : npts((int) pvec.size())
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
        //perm = new int[npts]; // permutation to randomize order
        std::vector<int> perm(npts);
        
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
        pts[npts] = vec<double, 2>(0.5 * (xl + xh), yh + bigscale * dely);
        pts[npts+1] = vec<double, 2>(xl - 0.5*bigscale * delx, yl - 0.5 * bigscale * dely);
        pts[npts+2] = vec<double, 2>(xl + 0.5*bigscale * delx, yl - 0.5 * bigscale * dely);
        storetriangle(npts, npts+1, npts+2);
        // create a permutation
        // this is a defensive shuffle that is not needed if the input is already shuffled
        std::shuffle(perm.begin(), perm.end(), rand());
        // insert points in permuted order
        for (j = 0; j < npts; j++) {
            insertapoint(perm[j]);
        }
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
            delete trihash;
            delete linehash;
        }
    }
    
    void Delaunay::insertapoint(int r) {
        int i, j, k,l,s,tno=0,ntask,d0,d1,d2;
        uint64_t key;
        int tasks[50], taski[50], taskj[50];
        
        
        static normal<> jran;
        
        for(j=0;j<3;j++) {
            tno = whichcontainspoint(pts[r],1);
            if (tno >= 0)
                break;
            // point is on an edge; perturb it
            pts[r][0] += fuzz * delx * (jran()-0.5);
            pts[r][1] += fuzz * dely * (jran()-0.5);
            // todo: replace fuzz with deletion of triangles on both sides,
            // and ensure result still converges
        }
        if (j == 3) assert(false); // still degenerate!
        ntask = 0;
        i = thelist[tno].p[0]; j = thelist[tno].p[1]; k = thelist[tno].p[2];
        if (opt & 2 && i < npts && j < npts && k < npts) return; // convex hull abort
        d0 = storetriangle(r, i, j);
        tasks[++ntask] = r; taski[ntask] = i; taskj[ntask] = j;
        d1 = storetriangle(r, j, k);
        tasks[++ntask] = r; taski[ntask] = j; taskj[ntask] = k;
        d2 = storetriangle(r, k, i);
        tasks[++ntask] = r; taski[ntask] = k; taskj[ntask] = i;
        erasetriangle(i, j, k, d0, d1, d2);
        while (ntask) {
            //std::cout << "ntask " << ntask << std::endl;
            s = tasks[ntask]; i = taski[ntask]; j = taskj[ntask--];
            
            key = hash(j) - hash(i);
            auto it = linehash->find(key);
            if (it == linehash->end()) continue;
            l = it->second;
            if (incircle(pts[l], pts[j], pts[s], pts[i]) > 0.0) {
                // reverse the quad
                d0 = storetriangle(s, l, j);
                d1 = storetriangle(s, i, l);
                erasetriangle(s, i, j, d0, d1, -1);
                erasetriangle(l, j, i, d0, d1, -1);
                // erase the edge both ways
                key = hash(i) - hash(j);
                linehash->erase(key);
                key = 0 - key;
                linehash->erase(key);
                // add new edges to check
                tasks[++ntask] = s; taski[ntask] = l; taskj[ntask] = j;
                tasks[++ntask] = s; taski[ntask] = i; taskj[ntask] = l;
            }
        }
    }
    
    int Delaunay::whichcontainspoint(const vec<double, 2>& p, int strict) {
        int i,j=-1,k=0;
        while (thelist[k].stat <= 0) {
            for (i = 0; i<3; i++) {
                if ((j = thelist[k].d[i]) < 0) continue;
                if (strict) {
                    if (thelist[j].contains(p) > 0) break;
                } else {
                    if (thelist[j].contains(p) >= 0) break;
                }
            }
            if (i == 3) return -1;
            k = j;
        }
        //std::cout << "found triangle " << k << " with " << thelist[k].stat << std::endl;
        return k;
    }
    
    void Delaunay::erasetriangle(int a, int b, int c, int d0, int d1, int d2) {
        
        
        //std::cout << "erasing " << a << ", " << b << ", " << c << std::endl;
        
        uint64_t key;
        int j;
        key = hash(a) ^ hash(b) ^ hash(c);
        auto it = trihash->find(key);
        if (it == trihash->end())
            assert(false); // triangle not present
        j = it->second;
        trihash->erase(it);
        thelist[j].d[0] = d0; thelist[j].d[1] = d1; thelist[j].d[2] = d2;
        thelist[j].stat = 0;
        ntri--;
    }
    
    int Delaunay::storetriangle(int a, int b, int c) {

        //std::cout << "storing " << a << ", " << b << ", " << c << std::endl;

        
        uint64_t key;
        thelist[ntree].setme(a, b, c, &pts[0] /*pts.data()*/);
        key = hash(a) ^ hash(b) ^ hash(c);
        //trihash->insert(std::make_pair(key, ntree));
        (*trihash)[key] = ntree;
        key = hash(b) - hash(c);
        //linehash->insert(std::make_pair(key, a));
        (*linehash)[key] = a;
        key = hash(c) - hash(a);
        //linehash->insert(std::make_pair(key, b));
        (*linehash)[key] = b;
        key = hash(a) - hash(b);
        //linehash->insert(std::make_pair(key, c));
        (*linehash)[key] = c;
        if (++ntree == ntreemax)
            assert(false); // he boot too big
        ntri++;
        return (ntree - 1);
    }
    
    vector<gl::vec<int,3>> Delaunay::triangles() const {
        vector<gl::vec<int,3>> r;
        auto b = thelist.data();
        auto e = b + ntree;
        for (; b != e; ++b)
            if (b->stat > 0)
                r.push_back(vec<int, 3>(b->p[0],b->p[1],b->p[2]));
        return r;        
    }
    
    
    struct gjy {
        
        gjy() {
            
            std:vector<vec<double, 2>> x;
            /*
            int n = 100000;
            for (int i = 0; i != n; ++i) {
                x.push_back(vec<double, 2>(hash(i) * 5.4210109e-20,
                                     hash(i + n) * 5.4210109e-20));
            }
            
            Delaunay d(x, 0);
            */
            
            
            
            /*
             auto c = circumcircle(vec<double, 2>(1,0),
             vec<double, 2>(1,1),
             vec<double, 2>(0,1));
             
             std::cout << c << std::endl;
             
             
             
             std::unordered_map<uint64_t, uint64_t> h(1000);
             
             for (int i = 0; i != 1000; ++i) {
             std::cout << i << "->" << h.bucket(i) << "\n";
             }
             */
            
            
            
            //exit(0);
            
        };
        
    } gha;
    
    
    
}
