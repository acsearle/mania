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
#include <array>

#include "nr.hpp"
#include "matrix.hpp"

namespace nr {
    
    
    // Construct a pointer with a particular alignment and stride
    
    template<typename T, std::size_t stride>
    class stride_ptr {

        static_assert(!(stride % alignof(T)), "stride must respect alignment");
        
        using value = std::aligned_storage_t<stride, alignof(T)>;
        using pointer = value*;
        pointer _ptr;
        
        explicit stride_ptr(pointer* p) : _ptr(p) {}
        
    public:
        
        stride_ptr() = default;
        stride_ptr(const stride_ptr&) = default;
        stride_ptr(stride_ptr&&) = default;
        ~stride_ptr() = default;
        stride_ptr& operator=(const stride_ptr&) = default;
        stride_ptr& operator=(stride_ptr&&) = default;
        
        explicit stride_ptr(T* p)
        : _ptr(reinterpret_cast<pointer>(p)) {
        }
        
        T& operator*() const { *get(); }
        T& operator[](std::ptrdiff_t i) const { return *(_ptr + i).get(); }
        T* operator->() const { get(); }
        T* get() const { return reinterpret_cast<T*>(_ptr); }
        
        operator bool() const { return _ptr; }
        
        stride_ptr& operator++() { ++_ptr; }
        stride_ptr operator++(int) { return stride_ptr(_ptr++); }
        stride_ptr& operator+=(std::ptrdiff_t n) { _ptr += n; return *this; }
        stride_ptr operator+(std::ptrdiff_t n) const { return stride_ptr(_ptr + n); }
        
        stride_ptr& operator--() { --_ptr; }
        stride_ptr operator--(int) { return stride_ptr(_ptr--); }
        stride_ptr& operator-=(std::ptrdiff_t n) { _ptr -= n; return *this; }
        stride_ptr operator-(std::ptrdiff_t n) const { return stride_ptr(_ptr - n); }
        
        friend std::ptrdiff_t operator-(stride_ptr a, stride_ptr b) { return a._ptr - b._ptr; }
        
#define X(F)\
        friend bool operator F (stride_ptr a, stride_ptr b) { return a._ptr F b._ptr; }\

        X(<) X(<=) X(>) X(>=)
        X(==) X(!=)
        
        friend T* unwrap(stride_ptr a) { return a.get(); }
        
    }; // class stride_ptr
    
    
    // Non-owning pointer to a single object
    
    template<typename T>
    class observer_ptr {
        
        T* _ptr;
    
    public:
        
        observer_ptr() = default;
        observer_ptr(const observer_ptr&) = default;
        observer_ptr(observer_ptr&&) = default;
        ~observer_ptr() = default;
        observer_ptr& operator=(const observer_ptr&) = default;
        observer_ptr& operator=(observer_ptr&&) = default;

        observer_ptr(T* ptr) : _ptr(ptr) {}
        observer_ptr& operator=(T* ptr) { _ptr = ptr; return *this; }
        
        operator bool() const { return static_cast<bool>(_ptr); }
        T* operator->() const { return _ptr; }
        T& operator*() const { return *_ptr; }
        T* get() const { return _ptr; }
        
        friend bool operator==(observer_ptr a, observer_ptr b) { return a._ptr == b._ptr; }
        friend bool operator!=(observer_ptr a, observer_ptr b) { return a._ptr != b._ptr; }
        
        friend T* unwrap(observer_ptr ptr) { return ptr._ptr; }
        
    }; // class observer_ptr
    
    template<typename T>
    T* unwrap(T* ptr) { return ptr; }
    
    template<typename T>
    T& unwrap(T& ref) { return ref; }
    
    
    
    
    
    // Numerical Recipes derived code
    
    using std::vector;
    
    
    /*
    template<typename T>
    class matrix {
        size_t _rows, _columns; // rows, columns
        vector<T> v;
    public:
        matrix() : _rows(0), _columns(0), v() {}
        matrix(const matrix&) = default;
        matrix(matrix&&) = default;
        ~matrix() = default;
        matrix& operator=(const matrix&) = default;
        matrix& operator=(matrix&&) = default;

        matrix(size_t n, size_t m) : _rows(n), _columns(m), v(n * m) {}
        matrix(size_t n, size_t m, const T& a) : _rows(n), _columns(m), v(n * m, a) {}
        matrix(size_t n, size_t m, const T* a) : _rows(n), _columns(m), v(a, a + n * m) {}

        void swap(matrix& rhs) {
            using std::swap;
            swap(_rows, rhs._rows);
            swap(_columns, rhs._columns);
            swap(v, rhs.v);
        }
        
        typedef T value_type;
        T* operator[](size_t i) { assert(i < _rows); return v.data() + i * _columns; }
        const T* operator[](size_t i) const { assert(i < _rows); return v.data() + i * _columns; }
        size_t nrows() const { return _rows; }
        size_t ncols() const { return _columns; }
        void resize(size_t newn, size_t newm) {
            _rows = newn;
            _columns = newm;
            v.resize(_rows * _columns);
        }
        void assign(size_t newn, size_t newm, const T& a);
        
    }; // matrix<T>
    */
    
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
    //size_t n;
    
    using manic::matrix;

    struct LUdcmp {
        matrix<double> lu;
        vector<size_t> indx;
        double d;
        LUdcmp(const matrix<double>& a);
        void solve(const vector<double>& b, vector<double>& x) const;
        void solve(const matrix<double>& b, matrix<double>& x) const;
        void inverse(matrix<double>& ainv) const;
        double det() const;
        
        void mprove(const vector<double>& b, vector<double>& x) const;
        const matrix<double>& aref;
    };
    
    
    LUdcmp::LUdcmp(const matrix<double>& a)
    : //n(a.nrows())
    /*,*/ lu(a)
    , aref(a)
    , indx(a.rows()) {
        assert(a.rows() == a.columns());
        using std::abs;
        const auto n = a.rows();
        //const double TINY = 1.0e-40; // A small number
        //int /*i,*/ /*imax = 0,*/ j, k;
        //double big, temp;
        vector<double> vv(n); // vv stores the implict scaling of each row
        d = 1.0; // no row interchanges yet
        for (size_t i = 0; i != n; ++i) { // loop over rows to get implicit scaling information
            double big = 0.0;
            for (size_t j = 0; j != n; ++j) {
                auto temp = abs(lu[i][j]);
                if (temp > big)
                    big = temp;
            }
            if (big == 0.0)
                assert(false);
            // we now have scaling information for the _row_
            vv[i] = 1.0 / big; // save the scaling
        }
        
        // saved scalings
        
        for (size_t k = 0; k != n; ++k) { // this is the outermost kij loop
            double big = 0.0;  // Initialize the search for the largest pivot element
            size_t imax = k;
            for (size_t i = k; i != n; ++i) {
                double temp = vv[i] * abs(lu[i][k]); // i.e. apply scaling before comparing
                if (temp > big) { // is figure of merit better?
                    big = temp; // big if true
                    imax = i;
                }
            }
            // we've found the biggest pivot in the (unprocessed lower part of
            // the) _column_, taking into account _row_ scalings
        
            // interchange rows to put pivot on diagonal
            // (todo: why not do this in O(1) with the matrix's ability to swizzle rows?)
            
            if (k != imax) { // do we need to interchange rows?
                for (size_t j = 0; j != n; ++j) {
                    // todo: implict swap?
                    using std::swap;
                    swap(lu[imax][j], lu[k][j]);
                }
                d = -d; // change parity
                
                // also interchange the scale factor
                vv[imax] = vv[k]; // half of swap
                // vv[k] now wrong, but we only refer to indices > k from
                // now on
            }
            indx[k] = imax;
            if (lu[k][k] == 0.0) {
                assert(false); // wtf
                //lu[k][k] = TINY;
            }
            // If the pivot element is zero, the matrix is singular (at least
            // to precision of the algrorithm).  For some applications on
            // singular matrices it is desirable to substitute TINY for zero
            for (size_t i = k + 1; i != n; ++i) {
                double temp = lu[i][k] /= lu[k][k];
                for (size_t j = k + 1; j != n; ++j)
                    lu[i][j] -= temp*lu[k][j];
            }
        } // for k : [0, n)
    }
    
    void LUdcmp::solve(const vector<double>& b, vector<double>& x) const {
        size_t ii=0, ip, j;
        const auto n = lu.rows();
        double sum;
        assert (b.size() == n && x.size() == n);
        for (size_t i= 0; i < n; i++)
            x[i] = b[i];
        for (size_t i = 0; i < n; i++) { // when ii is set to a positive value it
            ip = indx[i]; // becomes the index of first nonvanishing element of
            sum=x[ip]; // b.  we now do the forward substitution, unscrambling as we go
            x[ip] = x[i];
            if (ii != 0)
                for (j = ii-1; j < i; j++)
                    sum -= lu[i][j]*x[j];
            else
                if (sum != 0.0) // nonzero element encountered in b
                    ii = i + 1; // from now on we do the sum above
            x[i] = sum;
        }
        for (size_t i = n; i--; ) { // backsubstitution
            sum = x[i];
            for (j = i+1; j < n; j++)
                sum -= lu[i][j] * x[j];
            x[i] = sum / lu[i][i]; // store solution
        }
    }
    
    void LUdcmp::solve(const matrix<double>& b, matrix<double>& x) const {
        auto m = b.columns();
        const auto n = lu.rows();
        assert(b.rows() == n);
        assert(x.rows() == n);
        assert(b.columns() == x.columns());
        vector<double> xx(n);
        for (size_t j = 0; j != m; ++j) { // Copy and solve each column in turn
            for(size_t i = 0; i != n; ++i)
                xx[i] = b[i][j];
            solve(xx,xx);
            for(size_t i = 0; i != n; ++i)
                x[i][j] = xx[i];
        }
    }
    
    void LUdcmp::inverse(matrix<double>& ainv) const {
        const auto n = lu.rows();
        ainv.resize(n, n);
        for (size_t i = 0; i != n; ++i) {
            for (size_t j = 0; j != n; ++j)
                ainv[i][j] = 0.0;
            ainv[i][i] = 1.0;
        }
        solve(ainv, ainv);
    }
    
    double LUdcmp::det() const {
        const auto n = lu.rows();
        double dd = d; // seed with parity
        for (size_t i = 0; i != n; ++i)
            dd *= lu[i][i];
        return dd;
    }
    
    
    matrix<double> mul(const matrix<double>& a, const matrix<double>& b) {
        assert(a.columns() == b.rows());
        matrix<double> c(a.rows(), b.columns());
        for (size_t i = 0; i != a.rows(); ++i) {
            for (size_t k = 0; k != b.columns(); ++k) {
                c[i][k] = 0.0;
                for (size_t j = 0; j != a.columns(); ++j) {
                    c[i][k] += a[i][j] * b[j][k];
                }
            }
        }
        return c;
    }
    
    void print(const matrix<double>& a) {
        for (size_t i = 0; i != a.rows(); ++i) {
            for (size_t j = 0; j != a.columns(); ++j)
                std::cout << a[i][j] << ' ';
            std::cout << '\n';
        }
    }
    
    
    void test_func() {
        
        srand(1234567);
        
        const size_t n = 4;
        matrix<double> a(n, n);
        matrix<double> z(n, n);
        
        for (size_t i = 0; i != n; ++i) {
            for (size_t j = 0; j != n; ++j) {
                a[i][j] = rand() % 100;
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
        
        //exit(0);
        
    }
    
    
    struct test {
        
        test() {
            //test_func();
        }
        
    } t;
    

} // namespace nr

