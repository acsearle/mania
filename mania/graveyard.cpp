//
//  aaa.cpp
//  mania
//
//  Created by Antony Searle on 2/3/18.
//  Copyright © 2018 Antony Searle. All rights reserved.
//

#include <cstddef>
#include <utility>
#include <memory>

#include "aaa.hpp"
#include <vector>
#include <iostream>

namespace manic {
    
    using std::forward;
    using std::exchange;
    using std::move;
    
    using std::memcpy;
    using std::memset;
    
    void* memswp(void* a, void* b, std::size_t count) {
        unsigned char* c = (unsigned char*) a;
        unsigned char* d = (unsigned char*) b;
        while (count--) {
            char e = *c;
            *c = *d;
            *d = e;
            ++c;
            ++d;
        }
        return a;
    }
    
    template<typename T>
    void drop(T&& value) {
        std::decay_t<T> a(move(value));
    }
    
    
    // objects can be
    //     immovable -> mutex
    //     trivially copyable -> int, observer_ptr
    //     trivially movable -> unique_ptr
    //     explicitly cloned -> shared_ptr
    //     never nontrivially copied?
    
    // trivial move constructor: copy bytes then zero source
    
    // trivially movable:
    // default constructable
    // move constructor with postcondition that source is empty
    // no copy constructor
    // destructor must do nothing on zero state
    // no copy assignment operator
    // move assignment with postcondition that it must be swap
    // operator bool showing empty state ("optional thing")
    
    // we can default construct with memset
    // we can move-construct with memcpy + memset
    // we can move-assign with "memswp"
    // we can neglect to destroy move-constructed-from objects
    
    // trivially
    
    // useful concepts
    // iterator
    //     real references vs generated temporaries
    //     one-shot vs reusable
    // iterable -> has begin
    // range -> has begin and end
    // container -> can change number of elements
    
    // replace std::array and std::vector
    // size optionally known at compile time
    // minimal member functions, preferring free functions
    
    // problem: ability to resize differs across things, so one is a "container"
    // and the other isn't.  This is a wart in the standard library too, where
    // std::array is a SequenceContainer but gets special dispensation to
    // not support insert and erase
    
    // alt model: two kinds of thing, one fixed size and one flexible size
    // with on-stack being an optimization
    
    using index = std::ptrdiff_t;
    
    template<typename T, index N>
    constexpr index length(T (&a)[N]) { return N; }
    
    template<typename T, index N>
    constexpr T* begin(T (&a)[N]) { return a; }
    
    template<typename T, index N>
    constexpr T* end(T (&a)[N]) { return a + N; }
    
    class bit {
        bool _value;
    public:
        bit() = default;
        bit(const bit&) = default;
        bit(bit&&) = default;
        ~bit() = default;
        bit& operator=(const bit&) = default;
        bit& operator=(bit&&) = default;
        operator bool() const { return _value; }
    };
    
    class byte {
        std::uint8_t _value;
    public:
        byte() = default;
        byte(const byte&) = default;
        byte(byte&&) = default;
        ~byte() = default;
        byte& operator=(const byte&) = default;
        byte& operator=(byte&&) = default;
        byte(int value) : _value(value) {}
        operator bool() const { return static_cast<bool>(_value); }
        byte operator~() { return byte(~_value); }
        friend byte operator&(byte a, byte b) { return byte(a._value & b._value); }
        friend byte operator|(byte a, byte b) { return byte(a._value | b._value); }
        friend byte operator^(byte a, byte b) { return byte(a._value ^ b._value); }
        byte& operator&=(byte a) { _value &= a._value; return *this; }
        byte& operator|=(byte a) { _value |= a._value; return *this; }
        byte& operator^=(byte a) { _value ^= a._value; return *this; }
    };
    
    
    template<typename T>
    T clone(const T& value) { return value; }
    
    template<typename T>
    class span {
        T* _begin;
        T* _end;
    public:
        T& operator[](index i) { return _begin[i]; }
        operator bool() const { return _end != _begin; }
        friend index length(const span<T>& a) { return a._end - a._begin; }
        friend T* begin(span<T>& a) { return a._begin; }
        friend const T* begin(const span<T>& a) { return a._begin; }
        friend T* end(span<T>& a) { return a._end; }
        friend const T* end(const span<T>& a) { return a._end; }
    };
    
    template<typename T>
    class box {
        T* _ptr;
        explicit box(T* ptr) : _ptr(ptr) {}
        T* release() { return exchange(_ptr, nullptr); }
    public:
        box() : _ptr(nullptr) {}
        box(const box&) = delete;
        box(box&& r) : _ptr(exchange(r._ptr, nullptr)) {}
        ~box() { delete _ptr; }
        box& operator=(const box&) = delete;
        box& operator=(box&& r) { swap(*this, r); return *this; }
        explicit operator bool() const { return _ptr; }
        T& operator*() & { return *_ptr; }
        T&& operator*() && { return *_ptr; }
        const T& operator*() const& { return *_ptr; }
        const T&& operator*() const&& { return *_ptr; }
        const T* operator->() const { return _ptr; }
        T* operator->() { return _ptr; }
        friend void swap(box& a, box& b) {
            using std::swap;
            swap(a._ptr, b._ptr);
        }
        friend box clone(const box& r) {
            return r._ptr ? box(new T(clone(*r._ptr))) : box();
        }
        template<typename... Args>
        static box make(Args&&... args) {
            return box(new T(forward<Args>(args)...));
        }
        friend bool operator==(const box& a, const box& b) {
            return a._ptr == b._ptr;
        }
        friend bool operator!=(const box& a, const box& b) {
            return a._ptr != b._ptr;
        }
    }; // box<T>
    
    template<typename T>
    class rc {
        class control {
        public:
            control(T&& value) : _value(move(value)) {}
            T _value;
            index _strong;
            index _weak;
        };
        control* _ptr;
    public:
        rc() : _ptr(nullptr) {}
        rc(const rc&) = delete;
        rc(rc&& r) : _ptr(exchange(r._ptr, nullptr)) {}
        ~rc() { if (_ptr) if (_ptr--) delete _ptr; }
        rc& operator=(const rc&) = delete;
        rc& operator=(rc&& r) { using std::swap; swap(_ptr, r._ptr); return *this; }
        const T& operator*() const { return _ptr->_value; }
    };
    
    
    
    
    
    
    // differs from std::optional by behaving basically like box - moves leave it empty, can't be copied
    
    template<typename T> // require bytewise move
    class option {
        std::aligned_storage_t<sizeof(T), alignof(T)> _value;
        bool _has;
    public:
        option() : _has(false) {}
        option(const option&) = delete;
        option(option&& r)
        : _has(exchange(r._has, false)) {
            if (_has)
                new (&_value) T(move(r._value));
        }
        ~option() {
            if (_has)
                reinterpret_cast<T&>(_value).~T();
        }
        option& operator=(const option&) = delete;
        option& operator=(option&& r) {
            swap(*this, r);
            return *this;
        }
        option(T&& value)
        : _has(true) {
            new (&_value) T(move(value));
        }
        option(std::nullptr_t)
        : _has(false) {
        }
        option& operator=(T&& value) {
            if (_has)
                swap(reinterpret_cast<T&>(_value), value);
            else
                new (&_value) T(value);
            return *this;
        }
        option& operator=(std::nullptr_t) {
            if (_has) {
                reinterpret_cast<T&>(_value).~T();
                _has = false;
            }
            return *this;
        }
        friend void swap(option& a, option& b) {
            using std::swap;
            swap(a._value, b._value); // <- relies on bytewise swap
            swap(a._has, b._has);
        }
        explicit operator bool() const { return _has; }
        T& operator*() & { return reinterpret_cast<T&>(_value); }
        T&& operator*() && { return reinterpret_cast<T&>(_value); }
        const T& operator*() const& { return reinterpret_cast<const T&>(_value); }
        const T&& operator*() const&& { return reinterpret_cast<const T&>(_value); }
        const T* operator->() const { return reinterpret_cast<const T*>(&_value); }
        T* operator->() { return reinterpret_cast<T*>(&_value); }
        
        friend option clone(const option& a) {
            return a ? option(*a) : option();
        }
        
    };
    
    template<typename T>
    class array {
        
        T* _begin;
        T* _end;
        T* _allocation;
        T* _capacity;
        
        array(T* begin_, T* end_, T* allocation_, T* capacity_)
        : _begin(begin_)
        , _end(end_)
        , _allocation(allocation_)
        , _capacity(capacity_) {
        }
        
    public:
        
        array()
        : _begin(nullptr)
        , _end(nullptr)
        , _allocation(nullptr)
        , _capacity(nullptr) {
        }
        
        array(const array&) = delete;
        
        array(array&& a)
        : _begin(exchange(a._begin, nullptr))
        , _end(exchange(a._end, nullptr))
        , _allocation(exchange(a._allocation, nullptr))
        , _capacity(exchange(a._capacity, nullptr)) {
        }
        
        ~array() {
            for (auto a = _begin; a != _end; ++a)
                a->~T();
            operator delete(_allocation);
        }
        
        array& operator=(const array&) = delete;
        
        array& operator=(array&& a) {
            swap(*this, a);
            return *this;
        }
        
        friend index length(const array& a) {
            return a._end - a._begin;
        }
        
        friend void swap(array& a, array& b) {
            using std::swap;
            swap(a._begin, b._begin);
            swap(a._end, b._end);
            swap(a._allocation, b._allocation);
            swap(a._capacity, b._capacity);
        }
        
        friend void swap(array& a, array&& b) { swap(a, b); }
        friend void swap(array&& a, array& b) { swap(a, b); }
        friend void swap(array&& a, array&& b) { swap(a, b); }
        
        explicit operator bool() const { return _end != _begin; }
        
        friend T* begin(array& a) { return a._begin; }
        friend T* end(array& a) { return a._end; }
        
        friend const T* begin(const array& a) { return a._begin; }
        friend const T* end(const array& a) { return a._end; }
        
        friend array clone(array& a) {
            array b;
            for (auto&& c : a)
                b.push_back(clone(c));
            return b;
        }
        
        option<T> pop_back() {
            if (_begin != _end) {
                --_end;
                T value(move(*_end));
                _end->~T();
                return option<T>(move(value));
            } else {
                return option<T>();
            }
        }
        
        option<T> pop_front() {
            if (_begin != _end) {
                T value(move(*_begin));
                _begin->~T();
                ++_begin;
                return option<T>(move(value));
            } else {
                return option<T>();
            }
        }
        
        void push_back(T&& value) {
            if (_end == _capacity)
                ;
            new (_end++) T(move(value));
        }
        
        void push_front(T&& value) {
            if (_begin == _allocation)
                ;
            new (--_begin) T(move(value));
        }
        
        template<typename... Args>
        static array make(Args&&... args) {
            array a;
            //(..., a.push_back(forward<Args>(args))); // not yet supported
            // Make a variadic lambda that does nothing so we can expand the
            // parameter pack in its arguments
            [](auto&&...) {} ((a.push_back(forward<Args>(args)), 0)...);
            return a;
        }
        
    }; // array<T>
    
    void bloop() {
        auto b = array<double>::make(1,2,3);
        for (auto&& x : b)
            std::cout << x << std::endl;
    }
    
    struct qwe {
        qwe() {
            //bloop();
        }
    } _;
    
    
    /*
     
     
     template<typename T, std::size_t N = 0>
     class array {
     T _values[N];
     public:
     template<typename... Args, typename = std::enable_if_t<sizeof...(Args) == N>>
     array(Args&&... args) : _values{std::forward<Args>(args)...} {}
     constexpr T& operator[](std::ptrdiff_t i) { return _values[i]; }
     constexpr const T& operator[](std::ptrdiff_t i) const { return _values[i]; }
     friend constexpr std::ptrdiff_t length(const array&) { return N; }
     friend constexpr T* begin(array& a) { return a._values; }
     friend constexpr const T* begin(const array& a) { return a._values; }
     friend constexpr T* end(array& a) { return a._values + N; }
     friend constexpr const T* end(const array& a) { return a._values + N; }
     };
     
     template<typename T>
     class array<T, 0> {
     T* _begin;
     T* _end;
     T* _capacity;
     public:
     const T& operator[](std::ptrdiff_t i) const { return _begin[i]; }
     T& operator[](std::ptrdiff_t i) { return _begin[i]; }
     friend std::ptrdiff_t length(const array& a) { return a._end - a._begin; }
     };
     
     template<typename T>
     T* get(T* p) {
     return p;
     }
     
     template<typename T>
     T& get(T& r) {
     return r;
     }
     
     template<typename NullablePointer>
     class nonnull {
     NullablePointer _ptr;
     bool _invariant() {
     return static_cast<bool>(_ptr);
     }
     public:
     nonnull() = delete;
     nonnull(const nonnull&) = default;
     nonnull(nonnull&& r) : nonnull(r) {}
     ~nonnull() = default;
     nonnull& operator=(const nonnull&) = default;
     nonnull& operator=(nonnull&& r) { return operator=(r); }
     nonnull(std::nullptr_t) = delete;
     nonnull& operator=(std::nullptr_t) = delete;
     template<typename U>
     nonnull(U&& ptr)
     : _ptr(std::forward<U>(ptr)) {
     if (!_ptr) throw nullptr;
     }
     NullablePointer operator->() const { return _ptr; }
     decltype(auto) operator*() const { return *_ptr; }
     operator bool() const = delete;
     };
     */
    
}



//
//  renderer.cpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "renderer.hpp"

#include <cmath>
#include <iostream>
#include <cstdlib>
#include <vector>

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

#include "program.hpp"
#include "vao.hpp"
#include "vbo.hpp"
#include "vertex.hpp"
#include "vec.hpp"
#include "mat.hpp"
#include "image.hpp"
#include "cg.hpp"
#include "texture.hpp"

using namespace gl;
using namespace std;

typedef vec<GLushort, 2> edge;
typedef vec<GLushort, 3> triangle;


class blenderer
: public renderer {
    
    gl::program _program;
    gl::vao _vao;
    gl::vbo _vbo;
    gl::vbo _ebo;
    
    vector<vertex> _vertices;
    vector<edge> _edges;
    vector<triangle> _triangles;
    vector<vec2> _velocities;
    
    vector<double> _lengths;
    vector<double> _areas;
    
    GLsizei _width, _height;
    
    
    
public:
    
    blenderer();
    virtual ~blenderer() = default;
    void resize(GLsizei width, GLsizei height);
    void render();
    
};

std::unique_ptr<renderer> renderer::make() {
    return std::make_unique<blenderer>();
}

double uniform() {
    return rand() / (double) RAND_MAX;
}

float norm(vec2 v) {
    return v[0] * v[0] + v[1] * v[1];
}

vec2 disk() {
    vec2 v;
    do {
        v = vec2(uniform(), uniform()) * 2 - 1;
    } while (norm(v) > 1.0);
    return v;
}

double area(vec2 a, vec2 b, vec2 c) {
    return 0.5 * cross(a - b, b - c);
}

vec2 normal(vec2 a) {
    vec2 b;
    b[0] = a[1];
    b[1] = -a[0];
    return b;
}

vec2 un(vec2 a) {
    vec2 b;
    double c = hypot(a[0], a[1]);
    b[0] = -a[1] / c;
    b[1] = a[0] / c;
    return b;
}

blenderer::blenderer()
: _program("basic") {
    
    //png_thing();
    //auto pattern = manic::image::from_png_and_multiply_alpha("/Users/acsearle/Downloads/basn6a08.png");
    auto pattern = manic::image::from_png_and_multiply_alpha("/Users/acsearle/Downloads/tbrn2c08.png");
    // Premultiply alpha:
    auto p = pattern._data;
    for (int i = 0; i != pattern._width * pattern._height; ++i) {
        p[i].r = p[i].r * p[i].a / 255;
        p[i].g = p[i].g * p[i].a / 255;
        p[i].b = p[i].b * p[i].a / 255;
    }
    
    assert(pattern._width == 32);
    
    auto tex = new texture();
    tex->bind(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, pattern._data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    vec2 a;
    a.x = 7;
    
    glBindAttribLocation(_program, (GLuint) gl::attribute::position, "position");
    glBindAttribLocation(_program, (GLuint) gl::attribute::color, "color");
    _program.link();
    
    _vao.bind();
    _vbo.bind(GL_ARRAY_BUFFER);
    _ebo.bind(GL_ELEMENT_ARRAY_BUFFER);
    
    gl::vertex::bind();
    
    const int N = 12;
    
    /*
     for (int i = 0; i != N; ++i) {
     gl::vertex v;
     v.position = disk();
     v.color[0] = uniform();
     v.color[1] = uniform();
     v.color[2] = uniform();
     v.color[3] = 1.0;
     _vertices.push_back(v);
     _velocities.push_back(disk() * 0.01f);
     
     
     }
     */
    
    for (size_t i = 0; i != N; ++i) {
        gl::vertex v;
        //v.color[0] = uniform();
        //v.color[1] = uniform();
        v.color[0] = i & 1;
        v.color[1] = (i >> 1) & 1;
        v.color[2] = uniform();
        v.color[3] = 1;
        v.position[0] = 0.75 * cos(i * 0.5);
        v.position[1] = 0.75 * sin(i * 0.5);
        _vertices.push_back(v);
        v.position[0] = 0.5 * cos(i * 0.5 + 0.25);
        v.position[1] = 0.5* sin(i * 0.5 + 0.25);
        _vertices.push_back(v);
    }
    
    for (size_t i = 0; i != _vertices.size(); ++i) {
        edge e;
        e[1] = i;
        if (i > 1) {
            e[0] = i-2;
            _edges.push_back(e);
        }
        if (i > 0) {
            e[0] = i-1;
            _edges.push_back(e);
        }
    }
    
    for (size_t i = 2; i != _vertices.size(); ++i) {
        triangle t;
        t[0] = i - 2;
        t[1] = i - 1;
        t[2] = i;
        _triangles.push_back(t);
    }
    
    for (size_t i = 0; i != _edges.size(); ++i) {
        auto& e = _edges[i];
        _lengths.push_back(distance(_vertices[e[0]].position, _vertices[e[1]].position));
    }
    
    for (auto&& a : _triangles) {
        auto b = area(_vertices[a[0]].position,
                      _vertices[a[1]].position,
                      _vertices[a[2]].position);
        if (b < 0) {
            b = -b;
            std::swap(a[0], a[1]);
        }
        _areas.push_back(b);
    }
    
    {
        vec2 v;
        v[0] = 0;
        v[1] = 0;
        _velocities.resize(_vertices.size(), v);
    }
    
    {
        auto& c = _vertices[_edges[15][0]].color;
        c[0] = 1.0;
        auto& d = _vertices[_edges[15][1]].color;
        d[0] = 1.0;
    }
    
    
    _program.validate();
    _program.use();
    
    
    
    glPointSize(10.0);
    
}

void blenderer::resize(GLsizei width, GLsizei height) {
    
    _width = width;
    _height = height;
    
}

void blenderer::render() {
    
    glViewport(0, 0, std::min(_width, _height), std::min(_width, _height));
    
    vec2 gravity;
    gravity[0] = 0;
    gravity[1] = -9.8/60.0/60.0/64.0;
    
    static double t = 0;
    t += 0.04;
    
    
    /*
     
     Make bubbles:
     
     random points
     triangulate
     assign mass of gas to each triangle
     * mass of vertex is weighted average of triangles (is it?)
     forces are surface tension in, pressure out
     * how are these applied? at vertices? along edges?
     
     next: heat? apply to given point (needs lookup)
     
     */
    
    
    
    
    auto b = normalize(ivec2(1,1));
    
    //std::cout << b << std::endl;
    
    _lengths[15] = 0.25 + 0.125 * sin(t);
    
    
    for (size_t i = 0; i != _edges.size(); ++i) {
        auto j = _edges[i][0];
        auto k = _edges[i][1];
        vec2 d = _vertices[j].position - _vertices[k].position;
        
        
        
        // the stick damps relative velocity along its length
        vec2 common = (_velocities[j] + _velocities[k]) * 0.5;
        double l = length(d);
        vec2 u = 0.3 * d / -l;
        _velocities[j] -= u * dot(u, _velocities[j] - common);
        _velocities[k] -= u * dot(u, _velocities[k] - common);
        
        // surface tension
        
        _velocities[j] -= d / 600.0;
        _velocities[k] += d / 600.0;
        
        
        // Stiffest spring produces impulse to return to rest immediately
        
        /*
         double f = 0.5 * (l - _lengths[i]) / l;
         _velocities[j] -= d * f;
         _velocities[k] += d * f;
         */
        
        // Creep -- rods slowly adapt to their new length
        //(_lengths[i] *= 0.99) += 0.01 * l;
        
        
        
        
        
    }
    
    
    
    for (size_t i = 0; i != _triangles.size(); ++i) {
        
        {
            auto x0 = _vertices[_triangles[i][0]].position;
            auto x1 = _vertices[_triangles[i][1]].position;
            auto x2 = _vertices[_triangles[i][2]].position;
            
            auto& v0 = _velocities[_triangles[i][0]];
            auto& v1 = _velocities[_triangles[i][1]];
            auto& v2 = _velocities[_triangles[i][2]];
            
            auto area = -cross(x0 - x1, x2 - x1);
            auto p = 1e-4 / area;
            
            // animate one
            if (i == 10) {
                p = (1e-4 + (1 + sin(t)) * 1e-3) / area;
            }
            auto f = perp(x0 - x1) * p;
            v0 += f;
            v1 += f;
            f = perp(x1 - x2) * p;
            v1 += f;
            v2 += f;
            f = perp(x2 - x0) * p;
            v2 += f;
            v0 += f;
            
            
            
            
            /*
             auto a = (v0 + v1 + v2) / 3.0f;
             v0 -= a;
             v1 -= a;
             v2 -= a;
             v0 *= 0.5f;
             v1 *= 0.5f;
             v2 *= 0.5f;
             v0 += a;
             v1 += a;
             v2 += a;
             
             auto w = (cross(v0, x0) + cross(v1, x1) + cross(v1, x1)) / 3.0f;
             */
            /*
             v0 = w * normal(x0) / length(x0) + a;
             v1 = w * normal(x1) / length(x1) + a;
             v2 = w * normal(x2) / length(x2) + a;
             */
            
            
            // V = A . X
            // V . X^-1 = A
            /*
             mat2 X;
             X[0] = y1; X[1] = y2;
             mat2 invX = inv(X);
             mat2 V;
             V[0] = u1; V[1] = u2;
             mat2 A = dot(V, invX);
             
             auto curl = A[0][1] - A[1][0];
             auto div = A[0][0] + A[1][1];
             auto shear1 = A[0][1] + A[0][1];
             auto shear2 = A[0][0] - A[1][1];
             
             A[0][0] = 0;
             A[0][1] = curl / 2;
             A[1][0] = -curl / 2;
             A[1][1] = 0;
             V = dot(A, X);
             
             auto d1 = V[0] - u1;
             auto d2 = V[1] - u2;
             v1 += d1 * 0.1f;
             v2 += d2 * 0.1f;
             v0 -= (d1 + d2) * 0.1f;
             
             //std::cout << (V[0] - u1)[0] << std::endl;
             */
            
            
        }
        /*
         {
         auto a = _vertices[_triangles[i][0]].position;
         auto b = _vertices[_triangles[i][1]].position;
         auto c = _vertices[_triangles[i][2]].position;
         
         auto d = area(a, b, c);
         
         auto e = d - _areas[i];
         
         auto ab = a-b;
         auto bc = b-c;
         auto ca = c-a;
         
         auto f = length(ab) + length(bc) + length(ca);
         
         auto g = e / f * 0.9;
         
         _velocities[_triangles[i][0]] -= (un(ab) + un(ca)) * (float) g;
         _velocities[_triangles[i][1]] -= (un(bc) + un(ab)) * (float) g;
         _velocities[_triangles[i][2]] -= (un(ca) + un(bc)) * (float) g;
         }
         */
        
        
    }
    
    for (size_t i = 0; i != _vertices.size(); ++i) {
        
        auto& x = _vertices[i].position;
        auto& v = _velocities[i];
        
        v += gravity;
        x += v;
        
        if (x[0] < -1.0) {
            x[0] = -1.0;
            v[0] = -v[0];
            v *= 0.5;
        }
        
        if (x[0] > 1.0) {
            x[0] = 1.0;
            v[0] = -v[0];
            v *= 0.5;
        }
        
        if (x[1] < -1.0) {
            x[1] = -1.0;
            v[1] = -v[1];
            v *= 0.5;
        }
        
        if (x[1] > 1.0) {
            x[1] = 1.0;
            v[1] = -v[1];
            v *= 0.5;
        }
        
        
    }
    
    
    std::vector<vec<double, 2>> vx;
    vx.reserve(_vertices.size());
    for (auto& a : _vertices)
        vx.push_back(a.position);
    auto vy = manic::Delaunay(vx, 0).triangles();
    
    _vertices.clear();
    _vertices.push_back({{0,0},{0,0,0,0}});
    _vertices.push_back({{0,1},{0,1,0,0}});
    _vertices.push_back({{1,0},{1,0,0,0}});
    _vertices.push_back({{0,1},{0,1,0,0}});
    _vertices.push_back({{1,0},{1,0,0,0}});
    _vertices.push_back({{1,1},{1,1,0,0}});
    
    vec4 black(0,0,1,1);
    vec4 white(1,1,1,1);
    
    
    glClearColor(0.5, 0.25, 0.75, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    gl::vbo::assign(GL_ARRAY_BUFFER, _vertices, GL_STREAM_DRAW);
    
    static int j = 0;
    static int i = 0;
    if (j++ == 15) {
        j = 0;
        ++i;
    }
    
    
    _program.assign("mColor", white);
    //gl::vbo::assign(GL_ELEMENT_ARRAY_BUFFER, _triangles, GL_STREAM_DRAW);
    //glDrawElements(GL_TRIANGLES, (GLsizei) _triangles.size() * 3, GL_UNSIGNED_SHORT, (void*) 0);
    //gl::vbo::assign(GL_ELEMENT_ARRAY_BUFFER, vy, GL_STREAM_DRAW);
    //glDrawElements(GL_TRIANGLES, (GLsizei) /*(i % (*/vy.size()/* + 1))*/  * 3, GL_UNSIGNED_INT, (void*) 0);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei) _vertices.size());
    
    
    //vbo::assign(GL_ELEMENT_ARRAY_BUFFER, _edges, GL_STATIC_DRAW);
    //_program.assign("mColor", black);
    //glDrawElements(GL_LINES, (GLsizei) _edges.size() * 2, GL_UNSIGNED_SHORT, (void*) 0);
    
    
    //glDrawArrays(GL_POINTS, 0, (GLsizei) _vertices.size());
    
    
    
    
}




/*
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
 
 */

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

/*
 int main_table(int argc, char** argv) {
 
 table<uint64_t, uint64_t> t;
 
 for (uint64_t i = 0; i != 20000000; ++i)
 t[i] = i;
 
 //t.print();
 
 return 0;
 }
 */

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
    vec<double, 3> mid(0.5, 0.5, 0.5);
    vec<double, 3> red(1.0, 0.0, 0.0);
    vec<double, 3> green = cross(mid, red);
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
