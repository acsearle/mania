//
//  vec.hpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef vec_hpp
#define vec_hpp

#include <cmath>
#include <iostream>
#include <utility>

namespace gl {
    
    template<typename T, std::size_t N>
    class vec;
    
    template<typename T>
    class vec<T, 0> {
    public:
        
    };
    
    template<typename T, std::size_t N>
    class vec {
        
        // TriviallyCopyable allows us to move around by clobbering data
        static_assert(std::is_trivially_copyable<T>::value, "T must be TriviallyCopyable");
        
        T _[N];
        
    public:
        
        using size_type = std::size_t;
        using value_type = T;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = pointer;
        using const_iterator = const_pointer;

        constexpr vec() = default;
        constexpr vec(const vec&) = default;
        constexpr vec(vec&&) = default;
        ~vec() = default;
        constexpr vec& operator=(const vec&) = default;
        constexpr vec& operator=(vec&&) = default;
        
        template<typename U, typename = decltype(std::declval<T&>() = std::declval<const U&>())>
        constexpr explicit vec(const U& r) {
            for (size_type i = 0; i != N; ++i)
                _[i] = r;
        }
        
        template<typename U, typename = decltype(std::declval<T&>() = std::declval<const U&>())>
        constexpr vec(const vec<U, N>& r) {
            for (size_type i = 0; i != N; ++i)
                _[i] = r[i];
        }
        
        template<typename U, typename... Args, typename = decltype(std::declval<T&>() = std::declval<const U&>())>
        constexpr vec(const U& r, const Args&... args) {
            _[0] = r;
            new (_ + 1) vec<T, N - 1>(args...);
        }
        
        template<typename U, std::size_t M, typename... Args, typename = decltype(std::declval<T&>() = std::declval<const U&>())>
        constexpr vec(const vec<U, M>& r, const Args&... args) {
            static_assert(M < N, "too many initializers");
            new (_) vec<T, M>(r);
            new (_ + M) vec<T, N - M>(args...);
        }

        template<typename U, typename = decltype(std::declval<T&>() = std::declval<const U&>())>
        constexpr vec& operator=(const U& r) {
            for (size_type i = 0; i != N; ++i)
                _[i] = r;
            return *this;
        }
        
        template<typename U, typename = decltype(std::declval<T&>() = std::declval<const U&>())>
        constexpr vec& operator=(const vec<U, N>& r) {
            for (size_type i = 0; i != N; ++i)
                _[i] = r[i];
            return *this;
        }
        
        constexpr reference operator[](size_type i) { return _[i]; }
        constexpr const_reference operator[](size_type i) const { return _[i]; }
        
        constexpr reference front() { return _[0]; }
        constexpr const_reference front() const { return _[0]; }
        
        constexpr reference back() { return _[N - 1]; }
        constexpr const_reference back() const { return _[N - 1]; }
        
        constexpr pointer data() { return _; }
        constexpr const_pointer data() const { return _; }
        
        constexpr iterator begin() { return _; }
        constexpr const_iterator begin() const { return _; }
        constexpr const_iterator cbegin() const { return _; }
        
        constexpr iterator end() { return _ + N; }
        constexpr const_iterator end() const { return _ + N; }
        constexpr const_iterator cend() const { return _ + N; }
        
        constexpr bool empty() const { return !N; }
        constexpr size_type size() const { return N; }
        
    };
    
    
#define UNARY(OP)\
template<typename T, std::size_t N> auto operator OP (const vec<T, N>& a) {\
vec<decltype( OP std::declval<const T&>()), N> c;\
for (std::size_t i = 0; i != N; ++i) c[i] = OP a[i];\
return c;\
}\

    UNARY(+)
    UNARY(-)
    UNARY(~)
    UNARY(!)
    
#undef UNARY
    
#define BINARY(OP)\
template<typename T, std::size_t N, typename U, typename R = decltype(std::declval<const T&>() OP std::declval<const U&>())>\
vec<R, N> operator OP (const vec<T, N>& a, const vec<U, N>& b) {\
vec<R, N> c;\
for (std::size_t i = 0; i != N; ++i) c[i] = a[i] OP b[i];\
return c;\
}\
template<typename T, std::size_t N, typename U, typename R = decltype(std::declval<const T&>() OP std::declval<const U&>())>\
vec<R, N> operator OP (const vec<T, N>& a, const U& b) {\
vec<R, N> c;\
for (std::size_t i = 0; i != N; ++i) c[i] = a[i] OP b;\
return c;\
}\
template<typename T, std::size_t N, typename U, typename R = decltype(std::declval<const T&>() OP std::declval<const U&>())>\
vec<R, N> operator OP (const T& a, const vec<U, N>& b) {\
vec<R, N> c;\
for (std::size_t i = 0; i != N; ++i) c[i] = a OP b[i];\
return c;\
}\
template<typename T, std::size_t N, typename U>\
vec<T, N>& operator OP##=(vec<T, N>& a, const vec<U, N>& b) {\
for (std::size_t i = 0; i != N; ++i) a[i] OP##= b[i];\
return a;\
}\
template<typename T, std::size_t N, typename U>\
vec<T, N>& operator OP##=(vec<T, N>& a, const U& b) {\
for (std::size_t i = 0; i != N; ++i) a[i] OP##= b;\
return a;\
}
    
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
    
    
    template<typename T, std::size_t N, typename U>
    auto dot(const vec<T, N>& a, const vec<U, N>& b) {
        static_assert(N, "");
        auto c = a[0] * b[0];
        for (std::size_t i = 1; i != N; ++i)
            c += a[i] * b[i];
        return c;
    }

    template<typename T, std::size_t N>
    auto length(const vec<T, N>& a) {
        return sqrt(dot(a, a));
    }
    
    // Specialize when std::hypot is available
    template<typename T>
    auto length(const vec<T, 2>& a) {
        using std::hypot;
        return hypot(a[0], a[1]);
    }
    
    template<typename T, std::size_t N>
    auto distance(const vec<T, N>& a, const vec<T, N>& b) {
        return length(a - b);
    }
    
    template<typename T, typename U>
    auto cross(const vec<T, 3>& a, const vec<U, 3>& b) {
        using R = decltype(std::declval<const T&>() * std::declval<const U&>());
        return vec<R, 3>(a[1] * b[2] - a[2] * b[1],
                         a[2] * b[0] - a[0] * b[2],
                         a[0] * b[1] - a[1] * b[0]);
    }
    
    template<typename T, std::size_t N>
    auto normalize(const vec<T, N>& a) {
        auto b = length(a);
        return b ? a / b : (decltype(a / b)) a;
    }

    template<typename T, typename U>
    auto cross(const vec<T, 2>& a, const vec<U, 2>& b) {
        return a[0] * b[1] - a[1] * b[0];
    }
    
    template<typename T>
    vec<T, 2> perp(const vec<T, 2>& a) {
        return vec<T, 2>(-a[1], a[0]);
    }
    
    template<typename T, std::size_t N>
    std::ostream& operator<<(std::ostream&, const vec<T, N>& a) {
        std::cout << '[';
        for (std::size_t i = 0; i != N; ++i) {
            std::cout << a[i];
            if (i != N - 1)
                std::cout << ", ";
        }
        return std::cout << ']';
    }
    
    static_assert(sizeof(int) == 4, "int is not 32 bit");
    using uint = uint32_t;
    
    using vec2 = vec<float, 2>;
    using vec3 = vec<float, 3>;
    using vec4 = vec<float, 4>;
    using dvec2 = vec<double, 2>;
    using dvec3 = vec<double, 3>;
    using dvec4 = vec<double, 4>;
    using bvec2 = vec<bool, 2>;
    using bvec3 = vec<bool, 3>;
    using bvec4 = vec<bool, 4>;
    using ivec2 = vec<int, 2>;
    using ivec3 = vec<int, 3>;
    using ivec4 = vec<int, 4>;
    using uvec2 = vec<uint, 2>;
    using uvec3 = vec<uint, 3>;
    using uvec4 = vec<uint, 4>;



    
}

#endif /* vec_hpp */
