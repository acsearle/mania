//
//  projections.cpp
//  mania
//
//  Created by Antony Searle on 6/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include <iostream>
#include <algorithm>
#include <vector>
#include <cassert>
#include <cmath>

int magnitude2(int a, int b, int c) {
    return a*a + b*b + c*c;
}

double dot(double a, double b, double c, double d, double e, double f) {
    return a*d+b*e+c*f;
}

bool validate(int a, int b, int c, int d, int e, int f) {
    assert(magnitude2(a, b, c) == magnitude2(d, e, f));
    assert(dot(a,b,c,d,e,f) == 0);
    return true;
}

int gcd(int a, int b) {
    while (b != 0)  {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}
bool boring(int a, int b, int c, int d, int e, int f) {
    int r1 = magnitude2(a, b, c);
    
    
    if (r1 == a*a)
        return true;
    if (r1 == b*b)
        return true;
    if (r1 == c*c)
        return true;
    
    
    if ((a*a + d*d) == 0)
        return true;
    /*
    if (e*e+f*f == 0)
        return true;
    */
    
    if (gcd(gcd(gcd(gcd(gcd(a, b), c), d), e), f) > 1)
        return true;
    
    return false;
}


struct mat3 {
    double _[9];
    
    
    void print();
};


double isometry(mat3 m) {
    double g = std::abs(m._[6]);
    double h = std::abs(m._[7]);
    double i = std::abs(m._[8]);
    
    double r = sqrt(g*g+h*h+i*i);
    double d = (g+h+i)/(r*sqrt(3));
    double angle = acos(d);
    //printf("%g %g %g -> %g\n", g, h, i, angle * 57);
    return angle;
};

double merit(mat3 m) {
    
    double g = std::abs(m._[6]);
    double h = std::abs(m._[7]);
    double i = std::abs(m._[8]);
    double r = sqrt(g*g+h*h+i*i);
    g /= r;
    h /= r;
    i /= r;
    
    return r;
    
    double merit = 1;
    using std::min;
    // Penalize if on a coordinate plane
    merit = min(merit, g);
    merit = min(merit, h);
    merit = min(merit, i);
    // Penalize if on a diagonal
    merit = min(merit, fabs(g-h));
    merit = min(merit, fabs(g-i));
    merit = min(merit, fabs(h-i));
    
    return merit;
}



void mat3::print() {
    printf("(%g, %g, %g) -> sqr %g\n"
           "(%g, %g, %g) -> merit %g\n"
           "(%g, %g, %g) -> degrees %g\n"
           "\n",
           _[0],
           _[1],
           _[2],
           sqrt(_[0]*_[0]+_[1]*_[1]+_[2]*_[2]),
           _[3],
           _[4],
           _[5],
           merit(*this),
           _[6],
           _[7],
           _[8],
           isometry(*this) * 57
           );
}


int main_projections(int argc, char ** argv) {
    
    // These are R3 to R2 projections that map integer coordinates in R3 to
    // integer coordinates in R2, and are representable as an orthonormal
    // rotation matrix and a uniform scaling.  Their advantage is for 2d
    // game representing 3d objects with pixel art, any integer offset becomes
    // an integer pixel offset.  For example
    //     [  0,  3,  3 ]
    //     [  4, -1,  1 ]
    // is close to the isometric projection
    
    // For games that only care about 2d offsets, this is less relevant
    
    std::vector<mat3> results;
    
    int N = 70; //ceil(sqrt(3) * 64.0);
    
    for (int a = 0; a <= N; ++a) {
        int aa = a*a;
        int b_lim = floor(sqrt(N*N-aa)+0.1);
        for (int b = a; b <= b_lim; ++b) {
            int aabb = b*b+aa;
            int c_lim = floor(sqrt(N*N-aabb)+0.1);
            for (int c = std::max(b, 1); c <= c_lim; ++c) {
                int r2 = aabb + c*c;
                if (r2 == 0)
                    continue;
                double r = sqrt(r2);
                int d_bound = floor(r + 0.001);
                for (int d = 0; d <= d_bound; ++d) {
                    int e_bound = floor(sqrt(r2 - d*d) + 0.001);
                    for (int e = -e_bound; e <= e_bound; ++e) {
                        int f2 = r2 - d*d - e*e;
                        int f = f2;
                        if (f * f != f2)
                            continue;
                        int fc = -(a*d + b*e);
                        if (fc != f * c)
                            continue;
                        if (boring(a,b,c,d,e,f))
                            continue;
                        double g = (b*f-c*e)/r;
                        double h = (c*d-a*f)/r;
                        double i = (a*e-b*d)/r;
                        /*
                        printf("(%d, %d, %d) -> %g\n"
                               "(%d, %d, %d)\n"
                               "(%g, %g, %g)\n"
                               "\n",
                               a,b,c,r,//acos(d / r) * 57,
                               d,e,f,
                               g,
                               h,
                               i
                               );
                         */
                        validate(a,b,c,d,e,f);
                        
                        mat3 m = {{
                            (double) a,(double) b,(double) c,
                            (double) d,(double) e,(double) f,
                            g,h,i}};
                        results.push_back(m);
                    }
                }
            }
        }
    }
    
    
    std::sort(results.begin(), results.end(), [=](mat3& a, mat3& b) {
        return merit(a) < merit(b); });
    
    std::cout << "best:" << std::endl;
    for (auto& x : results)
        x.print();
    
    return 0;
    
}
