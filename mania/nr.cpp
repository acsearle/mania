//
//  nr.cpp
//  mania
//
//  Created by Antony Searle on 17/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "nr.hpp"

namespace nr {
    
    /*
    template<typename T>
    class NRvector {
    private:
        int nn;
        T* v;
    public:
        NRvector();
        explicit NRvector(int n);
        NRvector(int n, const T& a);
        NRvector(int n, const T* a);
        NRvector(const NRvector& rhs);
        NRvector(NRvector&& rhs);
        ~NRvector();
        NRvector& operator=(const NRvector& rhs);
        NRvector& operator=(NRvector&& rhs);
        typedef T value_type;
        T& operator[](const int i);
        const T& operator[](const int i) const;
        int size() const;
        void resize(int newn);
        void assign(int newn, const T& a);
    }; // NRvector<T>
     */
    
    template<typename T>
    using NRvector = std::vector<T>;
    
    template<typename T>
    class NRmatrix {
        int nn; // rows
        int mm; // columns
        T **v; // todo: if we don't actually use this, replace with mul (benchmark i guess?)
    public:
        NRmatrix() : nn(0), mm(0), v(nullptr) {}
        NRmatrix(int n, int m) : nn(n), mm(m), v(n > 0 ? new T*[n] : nullptr) {
            int i, nel = n * m;
            if (v)
                v[0] = nel > 0 ? new T[nel] : nullptr;
            for (i = 1; i < n; i++)
                v[i] = v[i - 1] + m;
        }
        NRmatrix(int n, int m, const T& a);
        NRmatrix(int n, int m, const T* a);
        NRmatrix(const NRmatrix& rhs)
        : NRmatrix(rhs.nn, rhs.mm) {
            int nel = nn * mm;
            std::memcpy(v[0], rhs.v[0], nel * sizeof(T));
        }
        // todo: rvalues
        ~NRmatrix() {
            if (v)
                delete[] v[0];
            delete[] v;
        }
        NRmatrix& operator=(const NRmatrix& rhs) {
            NRmatrix(rhs).swap(*this);
            return *this;
        }
        
        void swap(NRmatrix& rhs) {
            using std::swap;
            swap(nn, rhs.nn);
            swap(mm, rhs.mm);
            swap(v, rhs.v);
        }
        
        typedef T value_type;
        T* operator[](const int i) { return v[i]; }
        const T* operator[](const int i) const { return v[i]; }
        int nrows() const { return nn; }
        int ncols() const { return mm; }
        void resize(int newn, int newm) {
            NRmatrix(newn, newm).swap(*this);
        }
        void assign(int newn, int newm, const T& a);
    }; // NRmatrix<T>
    
    using Int = int;
    using MatDoub = NRmatrix<double>;
    using VecInt = NRvector<int>;
    using Doub = double;
    using VecDoub = NRvector<double>;
    
    
    struct LUdcmp {
        /*
         const int n = ...
         MatDoub a(n,n);
         VecDoub b(n), x(n);
         ...
         LUdcmp alu(a);
         alu.solve(b, x);
         alu.solve(b2, x2);
         alu.solve(b3, x3);
         */
        Int n;
        MatDoub lu;
        VecInt indx;
        Doub d;
        LUdcmp(const MatDoub& a);
        void solve(const VecDoub& b, VecDoub& x) const;
        void solve(const MatDoub& b, MatDoub& x) const;
        void inverse(MatDoub& ainv) const;
        Doub det() const;
        
        void mprove(const VecDoub& b, VecDoub& x) const;
        const MatDoub& aref;
    };
    
    
    LUdcmp::LUdcmp(const MatDoub& a)
    : n(a.nrows())
    , lu(a)
    , aref(a)
    , indx(n) {
        using std::abs;
        const Doub TINY = 1.0e-40; // A small number
        Int i, imax = 0, j, k;
        Doub big, temp;
        VecDoub vv(n); // vv stores the implict scaling of each row
        d = 1.0; // no row interchanges yet
        for (int i = 0; i < n; i++) { // loop over rows to get implicit scaling information
            big = 0.0;
            for (j = 0; j < n; j++)
                if ((temp = abs(lu[i][j])) > big)
                    big = temp;
            if (big == 0.0)
                assert(false);
            // we now have scaling information for the _row_
            vv[i] = 1.0 / big; // save the scaling
        }
        
        // saved scalings
        
        for (k = 0; k < n; k++) { // this is the outermost kij loop
            big = 0.0;  // Initialize the search for the largest pivot element
            imax = k;
            for (i = k; i < n; i++) {
                temp = vv[i] * abs(lu[i][k]); // i.e. apply scaling before comparing
                if (temp > big) { // is figure of merit better?
                    big = temp; // big if true
                    imax = i;
                }
            }
            // we've found the biggest pivot in the _column_, taking into account _row_ scalings
        
            // interchange rows to put pivot on diagonal
            // (todo: why not do this in O(1) with the matrix's ability to swizzle rows?)
            
            if (k != imax) { // do we need to interchange rows?
                for (j = 0; j < n; j++) {
                    // todo: implict swap
                    // todo: std::swap
                    temp = lu[imax][j]; // interchange
                    lu[imax][j] = lu[k][j];
                    lu[k][j] = temp;
                }
                d = -d; // change parity
                
                // also interchange the scale factor
                vv[imax] = vv[k]; // half of swap
                // vv[k] now wrong, but we only refer to indices > k from
                // now on
            }
            indx[k] = imax;
            if (lu[k][k] == 0.0)
                lu[k][k] = TINY;
            // If the pivot element is zero, the matrix is singular (at least
            // to precision of the algrorithm).  For some applications on
            // singular matrices it is desirable to substitute TINY for zero
            for (i = k + 1; i < n; i++) {
                temp = lu[i][k] /= lu[k][k];
                for (j = k+1; j < n; j++)
                    lu[i][j] -= temp*lu[k][j];
            }
        } // for k
    }
    
    void LUdcmp::solve(const VecDoub& b, VecDoub& x) const {
        Int i, ii=0, ip, j;
        Doub sum;
        assert (b.size() == n && x.size() == n);
        for (i= 0; i < n; i++)
            x[i] = b[i];
        for (i = 0; i < n; i++) { // when ii is set to a positive value it
            ip = indx[i]; // becomes the index of first nonvanishing element of
            sum=x[ip]; // b.  we now do the forward substitution, unscrambling as we go
            x[ip] = x[i];
            if (ii != 0)
                for (j = ii-1; j < i; j++)
                    sum -= lu[i][j]*x[j];
            else
                if (sum != 0.0) // nonzero element encountered in b
                    ii = i + 1; // from now on we do the sum above
            x[i]=sum;
        }
        for (i = n-1; i >= 0; i--) { // now do backsubstitution
            sum = x[i];
            for (j = i+1; j < n; j++)
                sum -= lu[i][j] * x[j];
            x[i] = sum / lu[i][i]; // store solution
        }
    }
    
    void LUdcmp::solve(const MatDoub& b, MatDoub& x) const {
        int i,j,m=b.ncols();
        assert(b.nrows() == n);
        assert(x.nrows() == n);
        assert(b.ncols() == x.ncols());
        VecDoub xx(n);
        for (j=0;j<m;j++) { // Copy and solve each column in turn
            for(i=0; i < n; i++)
                xx[i] = b[i][j];
            solve(xx,xx);
            for(i = 0; i <n;i++)
                x[i][j] = xx[i];
        }
    }
    
    void LUdcmp::inverse(MatDoub& ainv) const {
        Int i, j;
        ainv.resize(n, n);
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++)
                ainv[i][j] = 0.0;
            ainv[i][i] = 1.0;
        }
        solve(ainv, ainv);
    }
    
    Doub LUdcmp::det() const {
        Doub dd = d; // seed with parity
        for (Int i = 0; i < n; i++)
            dd *= lu[i][i];
        return dd;
    }
    
    
    MatDoub mul(const MatDoub& a, const MatDoub& b) {
        assert(a.ncols() == b.nrows());
        MatDoub c(a.nrows(), b.ncols());
        for (int i = 0; i != a.nrows(); ++i) {
            for (int k = 0; k != b.ncols(); ++k) {
                c[i][k] = 0.0;
                for (int j = 0; j != a.ncols(); ++j) {
                    c[i][k] += a[i][j] * b[j][k];
                }
            }
        }
        return c;
    }
    
    void print(const MatDoub& a) {
        for (int i = 0; i != a.nrows(); ++i) {
            for (int j = 0; j != a.ncols(); ++j) {
                std::cout << a[i][j] << ' ';
            }
            std::cout << '\n';
        }
    }
    
    
    void test_func() {
        
        srand(1234567);
        
        int n = 4;
        MatDoub a(n, n);
        MatDoub z(n, n);
        
        for (int i = 0; i != n; ++i) {
            for (int j = 0; j != n; ++j) {
                a[i][j] = rand() % 100 - 50;
                z[i][j] = 0.0;
            }
            //a[i][i] = 1.0;
        }
        
        LUdcmp lu(a);
        
        lu.inverse(z);
        
        print(a);
        std::cout << '\n';
        print(lu.lu);
        std::cout << '\n';
        
        
        
        print(z);
        std::cout << '\n';
        print(mul(a, z));
        
        exit(0);
        
    }
    
    
    struct test {
        
        test() {
            test_func();
        }
        
    } t;
    

} // namespace nr

