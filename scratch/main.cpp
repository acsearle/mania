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

#include <Accelerate/Accelerate.h>

#include "matrix.hpp"
#include "image.hpp"

#include "table.hpp"
#include "terrain.hpp"

using namespace manic;

namespace ijk {
    
    template<typename T>
    struct iterable {

        T& _derived() { return reinterpret_cast<T&>(*this); }

        auto begin() { return _derived().begin(); }
        auto end() { return _derived().end(); }

    };
    
    template<typename T>
    iterable(T&&) -> iterable<T>;
    
    template<typename T>
    auto sum(iterable<T>& x) {
        std::decay_t<decltype(*x.begin())> z = 0;
        for (auto&& y : x) {
            z += y;
        }
        return z;
    };
    
    template<typename T>
    struct hybrid : iterable<hybrid<T>> {
        std::vector<T> v;
        auto begin() { return v.begin(); }
        auto end() { return v.end(); }
    };
    
    
    void foo() {
        hybrid<double> v;
        v.v.push_back(7.0);
        v.v.push_back(8.0);
        std::cout << sum(v) << std::endl;
    }
}


int main(int argc, char** argv) {
    ijk::foo();
}



int main_mlcg(int argc, char** argv) {

    // Try to understand MLCG
    //
    //         x *= 4768777513237032717ull;
    
    
    auto a = 4768777513237032717ull;
    std::cout << a << std::endl;
    
    while (a) {
        std::cout << !!(a & (1ull << 63));
        a <<= 1;
    }
    std::cout << std::endl;
    a = 0;
    --a;
    std::cout << a << std::endl;
    
    /*
    for (int i = 64; i--;) {
        auto x = a << i;
        auto b = a;
        auto j = 1;
        while (x * b != x) {
            b *= b;
            ++j;
        }
        std::cout << std::oct << b << std::endl;
        std::cout << j << std::endl;
    }
     */

    
    
    
    // Try to crack the hash function: x = x * 3935559000370003845ull + 2691343689449507681ull;
    // The rest of it maps zero to zero

    /*
    const u64 a = 3935559000370003845ull;
    const u64 b = 2691343689449507681ull;
    
    u64 c = b;
    u64 d = 0;
    for (int i = 0; i != 64; ++i) {
        if (c & (1ull << i)) {
            c += (a << i);
            assert(!(c & 1));
            d |= (1ull << i);
        }
    }
    std::cout << std::hex << d << std::endl;
    
    std::cout << hash(d) << std::endl;
*/
    return 0;
}


int main_table(int argc, char** argv) {
    
    table<uint64_t, uint64_t> t;
    
    for (uint64_t i = 0; i != 20000000; ++i)
        t[i] = i;
    
    //t.print();
    
    return 0;
}


/*
void vDSP_fft2d_zripD(FFTSetupD __Setup, const DSPDoubleSplitComplex *__C, vDSP_Stride __IC0, vDSP_Stride __IC1, vDSP_Length __Log2N0, vDSP_Length __Log2N1, FFTDirection __flag);
 */

matrix<double> make_filter() {
    ptrdiff_t n = 256;
    ptrdiff_t log2n = log2(n);
    matrix<double> a(n, n);
    for (ptrdiff_t i = 0; i != n; ++i)
        for (ptrdiff_t j = 0; j != n; ++j) {
            double r = hypot(i - n/2, j - n/2);
            if (r == 0)
                r = 1;
            double log2r = log2(r);
            //double s = std::clamp(log2r - log2n + 2.0, 0.0, 2.0);
            double s = std::clamp(log2n - 1 - log2r, 0.0, 7.0);
            s = sin(s * M_PI / 7.0);
            //s = (s > 0) && (s < 6);
            
            a((i + 128) % 256, (j + 128) % 256) = (s*s) / (r*r);
        }
    return a;
}

matrix<double> make_filter_lowpass() {
    ptrdiff_t n = 256;
    ptrdiff_t log2n = log2(n);
    matrix<double> a(n, n);
    for (ptrdiff_t i = 0; i != n; ++i)
        for (ptrdiff_t j = 0; j != n; ++j) {
            double r = hypot(i - n/2, j - n/2);
            if (r == 0)
                r = 1;
            double log2r = log2(r);
            //double s = std::clamp(log2r - log2n + 2.0, 0.0, 2.0);
            double s = std::clamp(log2n - 1 - log2r, 0.0, 1.0);
            s = sin(s * M_PI / 2.0);
            
            a((i + 128) % 256, (j + 128) % 256) = (s*s)/(r*r);
        }
    return a;
}



void fft_thing(matrix_view<double> d, matrix_view<double> imaginary, int direction) {
    assert(d.stride() == imaginary.stride());
    auto n = log2(std::max(d.rows(), d.columns()));
    FFTSetupD s = vDSP_create_fftsetupD(ceil(n), 2);
    DSPDoubleSplitComplex a;
    a.realp = d.data();
    a.imagp = imaginary.data();
    vDSP_fft2d_zipD(s, &a, 1, d.stride(), log2(d.rows()), log2(d.columns()), direction);
    d /= n;
    imaginary /= n;
    
    
    //d = e;
    
    //for (auto&& [a0, a1] : zip(d, e))
        //for (auto&& [b, c] : zip(a0, a1))
    /*
    for (int i = 0; i != d.rows(); ++i)
        for (int j = 0; j != d.columns(); ++j)
            d(i, j) = hypot(d(i, j), e(i, j));
    */
}



matrix<double> convolve(const_matrix_view<double> a, const_matrix_view<double> b) {
    ptrdiff_t r = a.rows() - b.rows();
    ptrdiff_t c = a.columns() - b.columns();
    matrix<double> d(r, c);
    for (ptrdiff_t i = 0; i != r; ++i)
        for (ptrdiff_t j = 0; j != c; ++j) {
            double e = 0.0;
            for (ptrdiff_t k = 0; k != b.rows(); ++k)
                for (ptrdiff_t l = 0; l != b.columns(); ++l)
                    e += a(i + k, j + l) * b(k, l);
            d(i, j) = e;
        }
    return d;
}

pixel hue(double s) {
    gl::vec<double, 3> mid(0.5, 0.5, 0.5);
    gl::vec<double, 3> red(1.0, 0.0, 0.0);
    gl::vec<double, 3> green = cross(mid, red);
    red = cross(mid, green);
    red /= length(red);
    red *= std::hypot(0.5, 0.25, 0.25);
    green /= length(green);
    green *= std::hypot(0.5, 0.25, 0.25);
    mid += red*cos(s) + green*sin(s);
    pixel p;
    p.rgb = mid * 255;
    p.a = 255;
    return p;
}

void save(const_matrix_view<double> n) {
    double lo = n[0][0];
    double hi = n[0][0];
    for (auto&& a : n)
        for (auto&& b : a) {
            lo = std::min(lo, b);
            hi = std::max(hi, b);
        }
    
    std::cout << lo << ", " << hi << std::endl;
    image z(n.rows(), n.columns());
    for (ptrdiff_t i = 0; i != n.rows(); ++i)
        for (ptrdiff_t j = 0; j != n.columns(); ++j) {
            z(i, j).rgb = 255.0 * (n(i, j) - lo) / (hi - lo);
            z(i, j).a = 255;
            //z(i, j) = hue((n(i, j) - lo) / (hi - lo) * 2 * M_PI);
        }
    to_png(z, "/Users/acsearle/Downloads/textures/noise.png");
    
}

void posterize(matrix_view<double> v, ptrdiff_t n = 4) {
    double lo = v[0][0];
    double hi = v[0][0];
    for (auto&& a : v)
        for (auto&& b : a) {
            lo = std::min(lo, b);
            hi = std::max(hi, b);
        }
    for (ptrdiff_t i = 0; i != v.rows(); ++i)
        for (ptrdiff_t j = 0; j != v.columns(); ++j)
            v(i, j) = floor((v(i, j) - lo) / (hi - lo) * n);

}


matrix<double> inflate(const_matrix_view<double> a) {
    matrix<double> b(a);
    matrix<double> cplx(a.rows(), a.columns());
    cplx = 0.0;
    fft_thing(b, cplx, kFFTDirection_Forward);
    matrix<double> c(512, 512);
    matrix<double> cplx_c(512, 512);
    c = 0.0;
    cplx_c = 0.0;
    c.sub(0,0,128,128) = b.sub(0,0,128,128);
    c.sub(0,384,128,128) = b.sub(0,128,128,128);
    c.sub(384,0,128,128) = b.sub(128,0,128,128);
    c.sub(384,384,128,128) = b.sub(128,128,128,128);
    cplx_c.sub(0,0,128,128) = cplx.sub(0,0,128,128);
    cplx_c.sub(0,384,128,128) = cplx.sub(0,128,128,128);
    cplx_c.sub(384,0,128,128) = cplx.sub(128,0,128,128);
    cplx_c.sub(384,384,128,128) = cplx.sub(128,128,128,128);
    fft_thing(c, cplx_c, kFFTDirection_Inverse);
    return c;
}

double power(const_matrix_view<double> v) {
    double p = 0;
    for (ptrdiff_t i = 0; i != v.rows(); ++i)
        for (ptrdiff_t j = 0; j != v.columns(); ++j)
            p += v(i, j) * v(i, j);
    return p;
}



template<typename T, typename G>
void perturb(matrix_view<T> a, G& g) {
    for (ptrdiff_t i = 0; i != a.rows(); ++i)
        for (ptrdiff_t j = 0; j != a.columns(); ++j)
            a(i, j) += g();
}

template<typename T>
void threshold(matrix_view<T> a, T b = 0) {
    for (ptrdiff_t i = 0; i != a.rows(); ++i)
        for (ptrdiff_t j = 0; j != a.columns(); ++j)
            a(i, j) = b < a(i, j);
}

bool is_odd(ptrdiff_t a) {
    return a & 1;
}

bool is_even(ptrdiff_t a) {
    return !is_odd(a);
}



int main_terrain(int argc, char** argv) {
    
    
    
    auto n = 1024;
    matrix<double> a(n*2,n*2);
    
    a.sub(0,0,n+1,n+1) = terrain(0,0,n+1,n+1);
    a.sub(0,n+1,n+1,n-1) = terrain(0,n+1,n+1,n-1);
    a.sub(n+1,0,n-1,n+1) = terrain(n+1,0,n-1,n+1);
    a.sub(n+1,n+1,n-1,n-1) = terrain(n+1,n+1,n-1,n-1);
    
    auto b = terrain(0,0,2*n,2*n);
    //a -= b;
    //threshold(a);
    
    //auto n = 1024;
    //auto a = terrain(0,0,n,n);
    //threshold(a);
    //posterize(a, 2);
    save(a);
    
    //matrix<double> a = terrain(128, 128, 1024, 1024);
    //threshold(a);
    //save(a);
    
    
    /*
    {
        
        vector<double> filter(16);
        for (ptrdiff_t i = 0; i != 16; ++i) {
            filter[i] = exp(-sqr(i - 7.5) / 8.0);
        }
        filter *= sqrt(8.0) / sum(filter);
        filter.print();
        
        manic::normal rng(0);
        matrix<double> a(32, 32);
        a = 0.0;
        
        for (ptrdiff_t k = 0; k != 6; ++k) {
            perturb(a, rng);
            matrix<double> b(a.rows() * 2, a.columns() * 2);
            b = 0.0;
            explode(b, a);
            a.discard_and_resize(b.rows(), b.columns() - filter.size());
            a = 0.0;
            filter_rows(a, b, filter);
            b.discard_and_resize(a.rows() - filter.size(), a.columns());
            b = 0.0;
            filter_columns(b, a, filter);
            swap(a, b);
        }
        threshold(a);
        save(a);
        
    }
    */
    
    
    /*
    auto n = make_filter();
    
    manic::normal rg(459);
    
    matrix<double> nz(n.rows(), n.columns());
    matrix<double> nzc(n.rows(), n.columns());
    
    for (auto&& a : nz)
        for (auto&& b : a)
            b = rg();
    nzc = 0;
    fft_thing(nz, nzc, +1);
    nz *= n;
    nzc *= n;
    //nz = n;
    //nzc = 0;
    
    fft_thing(n, nzc, -1);
    */
    /*
    auto n2 = inflate(n);
    
    std::cout << power(n) << std::endl;
    std::cout << power(n2) << std::endl;
    n2 *= 2.0;
    std::cout << power(n2) << std::endl;
    

    n2.crop(128,128,256,256);
    n2 = inflate(n2);
    n2 *= 2.0;
    n2.crop(128,128,256,256);
    n += n2;
    */
    //n = inflate(n);
    
    //fft_thing(n, kFFTDirection_Forward);
    

    
    /*for (auto&& a : n)
        for (auto&& b : a)
            b = b > 0;*/
     
    //n *= 1000;
    
    //auto m = 8;
    //n.sub(m, 0, 257-2*m, 256) = 0;
    //n.sub(0, m, 256, 257-2*m) = 0;
    
    //fft_thing(n,+1);
    // n.print();
    
    //n.sub(0,0,1,128).print();
    
    //n.sub(0,0,256,128).swap(n.sub(0,128,256,128));
    //n.sub(0,0,128,256).swap(n.sub(128,0,128,256));
    
    /*
    auto k = n;
    for (int i = 0; i != 128; ++i)
        for (int j = 0; j != 128; ++j)
            k(i + 64, j + 64) += n(i * 2, j * 2) * 2;
    n = k;
     */
    
    //k.crop(127 - m, 127 - m, 2 * m - 1, 2 * m - 1);
    //save(n);
    
    
    /*
    
    std::vector<double> a(15, 0.0);
    std::vector<double> b(15, 0.0);
    double as = 0, bs = 0;
    for (int i = 0; i != a.size(); ++i) {
        double x = i - 7;
        as += a[i] = exp(-x * x * 0.125);
        bs += b[i] = exp(-x * x * 0.125/4);
    }
    for (int i = 0; i != a.size(); ++i) {
        a[i] /= as;
        b[i] /= bs;
        std::cout << a[i] << ", " << b[i] << std::endl;
    }
    matrix<double> f = outer_product<double>(b, b) - outer_product<double>(a, a);
    
    matrix<double> n(256, 256);
    for (auto&& a : n)
        for (auto&& b : a)
            b = rand() / (double) RAND_MAX;
    
    n = convolve(n, f);
    //n.print();
     */
    
    return 0;
}

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

void applyz(matrix<int>& g, matrix_view<int> s ) {
    int best_i = 0, best_j = 0, best_q = 0;
    for (int i = 0; i != g.size() - 1; ++i)
        for (int j = 0; j != g.size() - 1; ++j) {
            int q = match(g.sub(i, j, 2, 2), s);
            if (q == 5)
                return;
            if (q > best_q) {
                best_i = i;
                best_j = j;
                best_q = q;
            }
        }
    if (best_q == 0) {
        expand(g);
        applyz(g, s);
        return;
    }
    for (int k = 0; k != 2; ++k)
        for (int l = 0; l != 2; ++l)
            g[best_i+k][best_j+l] = s[k][l];
}

int main_terrain_generator(int argc, const char * argv[]) {
    const auto N = 2;
    std::vector<int> v(N);
    std::iota(v.begin(), v.end(), 0);
    matrix<int> s(2,2);
    matrix<int> current;
    expand(current);
    for (auto a : v)
        for (auto b: v)
            for (auto c : v)
                for (auto d : v) {
                    s[0][0] = a;
                    s[0][1] = b;
                    s[1][0] = c;
                    s[1][1] = d;
                    ::applyz(current, s);
                    
                }
    
    ((matrix_view<int>) current + 1).print();
    
    return 0;
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
