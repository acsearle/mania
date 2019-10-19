//
//  option.cpp
//  mania
//
//  Created by Antony Searle on 9/3/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "option.hpp"

#include "maybe.hpp"
#include "relocate.hpp"

namespace manic {


template<typename T>
class option {
    
    maybe<T> _value;
    bool _has;
    
public:
    
    option()
    : _value()
    , _has(false) {
    }
    
    option(const option& r)
    : _has(r) {
        if (r)
            _value.emplace(*r._value.get());
    }
    
    option(option&& r)
    : _has(std::exchange(r._has, false)) {
        if (_has)
            relocate(_value.get(), r._value.get());
    }
    
    explicit option(T const& x) : _has(true) {
        _value.emplace(x);
    }
    
    explicit option(T&& x) : _has(true) {
        _value.emplace(std::move(x));
    }
    
    ~option() {
        if (_has)
            _value.destroy();
    }
    
#define OE operator= // Xcode indentation is broken
    
    option& OE(const option &x) {
        option y(x);
        using std::swap;
        swap(y, *this);
        return *this;
    }
    
    option& OE(option &&x) {
        option y{std::move(x)};
        swap(y, *this);
        return *this;
    }
    
    friend void swap(option &a, option &b) {
        using std::swap;
        swap(a._value, b._value);
        swap(a._has, b._has);
    }
    
    explicit operator bool() const { return _has; }
    
    T& operator*() { assert(_has); return *_value; }
    const T& operator*() const { assert(_has); return *_value; }
    
    T* operator->() { assert(_has); return _value.operator->(); }
    const T* operator->() const { assert(_has); return _value.operator->(); }
    
    T unwrap() {
        assert(_has);
        T x{std::move(*_value.get())};
        _value.destroy();
        _has = false;
        return x;
    }
    
    template<typename U>
    T unwrap_or(U&& x) {
        if (_has) {
            T y{std::move(*_value.get())};
            _value.destroy();
            _has = false;
            return y;
        } else {
            return std::forward<U>(x);
        }
    }
};

template<typename T>
class option<T&> {
    
    T* _ptr;
    
public:
    
    option() : _ptr(nullptr) {}
    
    option(option const&) = default;
    
    option(option&& x) : _ptr(std::exchange(x._ptr, nullptr)) {}
    
    explicit option(T& x) : _ptr(&x) {}
    
    ~option() = default;
    
    option& operator=(option const&) = default;
    
    option& operator=(option&& x) {
        std::exchange(_ptr, std::exchange(x._ptr, nullptr));
        return *this;
    }
    
    friend void swap(option& a, option& b) {
        using std::swap;
        swap(a._ptr, b._ptr);
    }
    
    
    explicit operator bool() const { return _ptr; }
    
    T& operator*() const { assert(_ptr); return *_ptr; }
    T* operator->() const { assert(_ptr); return *_ptr; }
    
};


template<typename T>
struct Box {
    
    T* _ptr;
    
    Box() = delete;
    
    Box(const Box&) = delete;
    
    Box(Box&&) = delete;

    ~Box() {
        delete _ptr;
    }
    
    explicit Box(T&& t) : _ptr(new T(std::move(t))) {}
    
    Box& operator=(const Box&) = delete;

    
};






}
