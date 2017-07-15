//
//  mat.hpp
//  mania
//
//  Created by Antony Searle on 14/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef mat_hpp
#define mat_hpp

#include <cstddef>
#include <experimental/optional>

#include "vec.hpp"

namespace gl {
    
    template<typename T, std::size_t M, std::size_t N = M>
    struct mat {
        
        vec<T, M * N> _;
        
        using size_type = std::size_t;
        using value_type = vec<T, M>;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using reference = value_type&;
        using const_reference = const value_type&;
        using iterator = pointer;
        using const_iterator = const_pointer;
        
        constexpr mat() = default;
        constexpr mat(const mat&) = default;
        constexpr mat(mat&&) = default;
        ~mat();
        constexpr mat& operator=(const mat&);
        constexpr mat& operator=(mat&&);
        
        template<typename U, typename = decltype(std::declval<T&>() = std::declval<const U&>())>
        constexpr explicit mat(const U& r) : _(r) {}
        
        template<typename U, typename = decltype(std::declval<T&>() = std::declval<const U&>())>
        constexpr explicit mat(const mat<U, M, N>& r) : _(r._) {}
        
        template<typename... Args> constexpr explicit mat(const Args&... args) : _(args...) {}
        
        template<typename U, typename = decltype(std::declval<T&>() = std::declval<const U&>())>
        constexpr mat& operator=(const U& x) {
            _ = x; return *this;
        }
        
        template<typename U, typename = decltype(std::declval<T&>() = std::declval<const U&>())>
        constexpr mat& operator=(const mat<U, M, N>& r) {
            _ = r._; return *this;
        }
        
        constexpr reference operator[](size_type i) {
            return reinterpret_cast<reference>(_[i * N]);
        }
        constexpr const_reference operator[](size_type i) const {
            return reinterpret_cast<const_reference>(_[i * N]);
        }
        
        constexpr iterator begin() { return reinterpret_cast<iterator>(_); }
        constexpr const_iterator begin() const { reinterpret_cast<const_iterator>(_); }
        constexpr const_iterator cbegin() const { reinterpret_cast<const_iterator>(_); }
        
        constexpr iterator end() { return reinterpret_cast<iterator>(_) + M; }
        constexpr const_iterator end() const { reinterpret_cast<const_iterator>(_) + M; }
        constexpr const_iterator cend() const { reinterpret_cast<const_iterator>(_) + M; }
        
        constexpr bool empty() const { return !M; }
        constexpr size_type size() const { return M; }
        
        template<typename U, std::size_t K, std::size_t L>
        friend class mat;
        
    };

    // Componentwise operators
    //
    // Unlike GLSL, operator* is componentwise.  Use dot( , ) to perform
    // matrix-matrix and matrix-vector multiplies
    
#define UNARY(OP)\
template<typename T, std::size_t M, std::size_t N> auto operator OP (const mat<T, M, N>& a) {\
return mat<decltype( OP std::declval<const T&>()), M, N>( OP a._);\
}\

    UNARY(+)
    UNARY(-)
    UNARY(~)
    UNARY(!)
    
#undef UNARY

    
#define BINARY(OP)\
template<typename T, std::size_t M, std::size_t N, typename U>\
auto operator OP (const mat<T, M, N>& a, const mat<U, M, N>& b) {\
return mat<decltype(std::declval<const T&>() OP std::declval<const U&>()), M, N>(a._ OP b._);\
}\
template<typename T, std::size_t M, std::size_t N, typename U>\
auto operator OP (const mat<T, M, N>& a, const U& b) {\
return mat<decltype(std::declval<const T&>() OP std::declval<const U&>()), M, N>(a._ OP b);\
}\
template<typename T, std::size_t M, std::size_t N, typename U>\
auto operator OP (const T& a, const mat<U, M, N>& b) {\
return mat<decltype(std::declval<const T&>() OP std::declval<const U&>()), M, N>(a OP b._);\
}\
template<typename T, std::size_t M, std::size_t N, typename U>\
auto operator OP##= (mat<T, M, N>& a, const mat<U, M, N>& b) {\
a._ OP##= b._;\
return a;\
}\
template<typename T, std::size_t M, std::size_t N, typename U>\
auto operator OP##= (mat<T, M, N>& a, const U& b) {\
a._ OP##= b;\
return a;\
}\

    BINARY(+)
    BINARY(-)
    BINARY(*)
    BINARY(/)
    BINARY(%)
    BINARY(^)
    BINARY(&)
    BINARY(|)
    BINARY(<<)
    BINARY(>>)
    
#undef BINARY

    template<typename T, std::size_t L, std::size_t M, typename U, std::size_t N>
    auto hcat(const mat<T, L, M>& a, mat<U, L, N>& b) {
        using R = decltype(std::declval<const T&>() * std::declval<const U&>());
        mat<R, L, M + N> c;
        for (size_t i = 0; i != L; ++i) {
            reinterpret_cast<vec<R, M>&>(c[i][0]) = a[i];
            reinterpret_cast<vec<R, N>&>(c[i][M]) = b[i];
        }
        return c;
    }

    template<typename T, std::size_t L, std::size_t M, typename U, std::size_t N>
    auto vcat(const mat<T, L, M>& a, mat<U, N, M>& b) {
        using R = decltype(std::declval<const T&>() * std::declval<const U&>());
        mat<R, L + N, M> c;
        for (size_t i = 0; i != L; ++i)
            c[i] = a[i];
        for (size_t i = 0; i != N; ++i)
            c[i + L] = b[i];
        return c;
    }

    template<typename T, std::size_t N> auto eye() {
        mat<T, N> c;
        for (size_t i = 0; i != N; ++i)
            for (size_t j = 0; j != N; ++j)
                c[i][j] = (i == j) ? 1 : 0;
        return c;
    }

    template<typename T, std::size_t L, std::size_t M, typename U, std::size_t N>
    auto dot(const mat<T, L, M>& a, const mat<U, M, N>& b) {
        mat<decltype(std::declval<const T&>() * std::declval<const U&>()), L, N> c(0);
        for (size_t i = 0; i != L; ++i)
            for (size_t k = 0; k != M; ++k)
                c[i] += a[i][k] * b[k];
        return c;
    }

    template<typename T, std::size_t M, std::size_t N, typename U>
    auto dot(const mat<T, M, N>& a, const vec<U, N>& b) {
        vec<decltype(std::declval<const T&>() * std::declval<const U&>()), M> c;
        for (size_t i = 0; i != M; ++i)
                c[i] += dot(a[i], b);
        return c;
    }
    
    template<typename T, std::size_t M, typename U, std::size_t N>
    auto dot(const vec<T, M>& a, const mat<U, M, N>& b) {
        vec<decltype(std::declval<const T&>() * std::declval<const U&>()), N> c(0);
        for (size_t i = 0; i != N; ++i)
            for (size_t j = 0; j != M; ++j)
                c[i] += a[j] * b[j][i];
        return c;
    }
    
    template<typename T, std::size_t M, typename U, std::size_t N>
    auto outer(const vec<T, M>& a, const vec<T, N>& b) {
        mat<decltype(std::declval<const T&>() * std::declval<const U&>()), M, N> c;
        for (size_t i = 0; i != M; ++i)
                c[i] = a[i] * b;
    }
    template<typename T, std::size_t M, std::size_t N>
    mat<T, N, M> transpose(const mat<T, M, N>& a) {
        mat<T, N, M> c;
        for (int i = 0; i != N; ++i)
            for (int j = 0; j != M; ++j)
                c[i][j] = a[j][i];
    }


    template<typename T>
    auto determinant(const mat<T, 2>& a) {
        return
        + a[0][0] * a[1][1]
        - a[0][1] * a[1][0]
        ;
    }
    
    template<typename T>
    auto determinant(const mat<T, 3>& a) {
        return
        + a[0][0] * a[1][1] * a[2][2]
        + a[0][1] * a[1][2] * a[2][0]
        + a[0][2] * a[1][0] * a[2][1]
        - a[0][2] * a[1][1] * a[2][0]
        - a[0][0] * a[1][2] * a[2][1]
        - a[0][1] * a[1][0] * a[2][2]
        ;
    }
    
    template<typename T>
    auto inv(const mat<T, 2>& x) {
        return mat<T, 2>(x[1][1], -x[0][1], -x[1][0], x[0][0]) / determinant(x);
    }
    
    using mat2 = mat<float, 2>;
    using mat3 = mat<float, 2>;
    using mat4 = mat<float, 2>;

    using mat2x2 = mat<float, 2, 2>;
    using mat2x3 = mat<float, 2, 3>;
    using mat2x4 = mat<float, 2, 4>;

    using mat3x2 = mat<float, 3, 2>;
    using mat3x3 = mat<float, 3, 3>;
    using mat3x4 = mat<float, 3, 4>;

    using mat4x2 = mat<float, 4, 2>;
    using mat4x3 = mat<float, 4, 3>;
    using mat4x4 = mat<float, 4, 4>;

    using dmat2 = mat<double, 2>;
    using dmat3 = mat<double, 2>;
    using dmat4 = mat<double, 2>;
    
    using dmat2x2 = mat<double, 2, 2>;
    using dmat2x3 = mat<double, 2, 3>;
    using dmat2x4 = mat<double, 2, 4>;
    
    using dmat3x2 = mat<double, 3, 2>;
    using dmat3x3 = mat<double, 3, 3>;
    using dmat3x4 = mat<double, 3, 4>;
    
    using dmat4x2 = mat<double, 4, 2>;
    using dmat4x3 = mat<double, 4, 3>;
    using dmat4x4 = mat<double, 4, 4>;

}

#endif /* mat_hpp */
