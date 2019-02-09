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

float norm(gl::vec2 v) {
    return v[0] * v[0] + v[1] * v[1];
}

gl::vec2 disk() {
    gl::vec2 v;
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
    //auto pattern = manic::image::from_png("/Users/acsearle/Downloads/basn6a08.png");
    auto pattern = manic::image::from_png("/Users/acsearle/Downloads/tbrn2c08.png");
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
    
    gl::vec2 gravity;
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
    
    
    std::vector<gl::vec<double, 2>> vx;
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
