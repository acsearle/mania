//
//  main.cpp
//  prius
//
//  Created by Antony Searle on 15/12/18.
//  Copyright © 2018 Antony Searle. All rights reserved.
//

#include <iostream>

#include <random>
#include <array>
#include <algorithm>
#include <numeric>
#include <vector>

#include "matrix.hpp"

using namespace manic;

void expand(matrix<int>& v) {
    v.expand(1, 1, v.size() + 2, v.front().size() + 2, -1);
}

int match(matrix_view<int> g, matrix_view<int> s) {
    int quality = 1;
    for (int k = 0; k != 2; ++k)
        for (int l = 0; l != 2; ++l)
            if (g[k][l] != -1) {
                if (g[k][l] == s[k][l])
                    quality += 1;
                else
                    return 0;
            }
    return quality;
}

static int counter = 0;
static int finder = 0;

//void apply(vector<vector<int>>& g, array<array<int, 2>, 2>& s ) {
void apply(matrix<int>& g, matrix_view<int> s ) {
    int best_i = 0, best_j = 0, best_q = 0;
    for (int i = 0; i != g.size() - 1; ++i)
        for (int j = 0; j != g.size() - 1; ++j) {
            int q = match(g.sub(i, j, 2, 2), s);
            if (q == 5) {
                ++finder;
                return;
            }
            if (q > best_q) {
                best_i = i;
                best_j = j;
                best_q = q;
            }
        }
    if (best_q == 0) {
        expand(g);
        apply(g, s); // can't recurse forever because we will eventually have
        // a border 2 thick
        return;
    }
    for (int k = 0; k != 2; ++k)
        for (int l = 0; l != 2; ++l)
            g[best_i+k][best_j+l] = s[k][l];
    ++counter;
}

int main(int argc, const char * argv[]) {
    const auto N = 5;
    std::array<int, N> v;
    std::iota(v.begin(), v.end(), 0);
    // std::array<std::array<int, 2>, 2> s = {};
    matrix<int> s(2,2);
    // vector<vector<int>> current;
    matrix<int> current;
    expand(current);
    for (auto a : v)
        for (auto b: v)
            for (auto c : v)
                for (auto d : v) {
                    //printf("%d%d\n%d%d\n\n", a, b, c, d);
                    s[0][0] = a;
                    s[0][1] = b;
                    s[1][0] = c;
                    s[1][1] = d;
                    apply(current, s);
                    
                }
    
    printf("created %d found %d\n", counter, finder);
    
    for (auto x : current) {
        for (auto&& y : x)
            printf("%d", y+1);
        printf("\n");
    }
    
    finder= 0;
    counter = 0;
    for (auto a : v)
        for (auto b: v)
            for (auto c : v)
                for (auto d : v) {
                    //printf("%d%d\n%d%d\n\n", a, b, c, d);
                    s[0][0] = a;
                    s[0][1] = b;
                    s[1][0] = c;
                    s[1][1] = d;
                    apply(current, s);
                    
                }
    printf("%d\n", counter);
    printf("%ld\n", current.size()*32);
    
}

/*
 // Projections
 
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
 
 if (e*e+f*f == 0)
 return true;
 
 
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
 printf("(%g, %g, %g) -> %g\n"
 "(%g, %g, %g) -> %g\n"
 "(%g, %g, %g)\n"
 "\n",
 _[0],
 _[1],
 _[2],
 sqrt(_[0]*_[0]+_[1]*_[1]+_[2]*_[2]),
 _[3],
 _[4],
 _[5],
 merit(*this), //isometry(*this) * 57,
 _[6],
 _[7],
 _[8]
 );
 }
 
 int main(int argc, const char * argv[]) {
 
 std::vector<mat3> results;
 
 int N = 70; //ceil(sqrt(3) * 64.0);
 
 for (int a = 0; a <= 0; ++a) {
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
 
 }
 */



/*
 // Birth month
 
 int main(int argc, const char * argv[]) {
 std::random_device rd;
 std::mt19937 gen(rd());
 std::uniform_int_distribution<> dis(0, 11);
 
 std::array<int, 12> months;
 std::array<int, 13> histogram;
 histogram.fill(0);
 
 const auto N = 1000000;
 int n = 0;
 for (int i = 0; i != N; ++i) {
 months.fill(0);
 for (int j = 0; j != 35; ++j) {
 ++months[dis(gen)];
 }
 int loaners = 0;
 for (int j = 0; j != 12; ++j) {
 if (months[j] == 1) {
 ++loaners;
 }
 }
 ++histogram[loaners];
 if (loaners)
 ++n;
 }
 
 double p = n / (double) N;
 printf("p = %g\n", p);
 printf("n(loners):\n");
 
 for (int i = 0; i != histogram.size(); ++i) {
 printf("%d : %g\n", i, histogram[i] / (double) N);
 }
 
 
 
 return 0;
 }*/

/* Prius
 template<typename F>
 bool observe(F& f) {
 int triples = 0;
 int prii = 0;
 for (;;) {
 if (f()) {
 ++prii;
 if (prii == 3) {
 ++triples;
 } else if (prii == 4) {
 // We observed a quadruplet
 return false;
 }
 } else {
 prii = 0;
 if (triples == 10) {
 return true;
 }
 }
 }
 }
 
 template<typename F>
 double trial(F&& f) {
 const auto N = 1000;
 int j = 0;
 for (int i = 0; i != N; ++i) {
 if (observe(f))
 ++j;
 }
 return ((double) j) / N;
 }
 
 int main(int argc, const char * argv[]) {
 std::random_device rd;
 std::mt19937 gen(rd());
 std::uniform_real_distribution<> dis(0.0, 1.0);
 
 const auto N = 100;
 for (int i = N; i != 0; i--) {
 double p = ((double) i) / N;
 double q = trial([&]() {
 return dis(gen) <= p;
 });
 
 double r = pow(1 - p, 10);
 
 std::cout << p << ", " << q << ", " << r << std::endl;
 }
 
 
 return 0;
 }
 */
