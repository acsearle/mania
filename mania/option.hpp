//
//  option.hpp
//  mania
//
//  Created by Antony Searle on 9/3/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef option_hpp
#define option_hpp

#include "maybe.hpp"
#include "relocate.hpp"

namespace manic {

template<typename T>
class Option;

struct _None_type {
    
    template<typename T>
    operator Option<T>() const {
        return Option<T>{};
    }
    
};

inline _None_type None;

template<typename T>
Option<T> Some(T&& x) {
    return Option<T>{std::forward<T>(x)};
}

template<typename T>
class Option {
    
    maybe<T> _value;
    bool _is_some = false;
    
public:

    Option() = default;
    
    template<typename U>
    Option(Option<U> const& x)
    : _value{}
    , _is_some{x._is_some} {
        _value.emplace(*x._value);
    }

    template<typename U>
    Option(Option<U>& x)
    : _value{}
    , _is_some{x._is_some} {
        _value.emplace(*x._value);
    }
    
    Option(Option&& x)
    : _value{x._value}
    , _is_some{std::exchange(x._is_some, false)} {
    }

    template<typename U>
    Option(Option<U>&& x)
    : _value{}
    , _is_some{std::exchange(x._is_some, false)} {
        _value.emplace(std::move(*_x._value));
    }

    template<typename U>
    explicit Option(U&& x)
    : _value()
    , _is_some(true) {
        _value.emplace(std::forward<U>(x));
    }
    
    ~Option() {
        if (_is_some)
            _value.destroy();
    }
    
    template<typename U>
    Option& operator=(Option<U> const& x) {
        if (_is_some) {
            if (x._is_some)
                *_value = *x._value;
            else
                _value.destroy();
        } else
            if (x._is_some)
                _value.emplace(*x._value);
        _is_some = x._is_some;
        return *this;
    }
    
    template<typename U>
    Option& operator=(Option<U>& x) {
        if (_is_some) {
            if (x._is_some)
                *_value = *x._value;
            else
                _value.destroy();
        } else
            if (x._is_some)
                _value.emplace(*x._value);
        _is_some = x._is_some;
        return *this;
    }
    
    template<typename U>
    Option& operator=(Option<U>&& x) {
        if (_is_some) {
            if (x._is_some) {
                *_value = std::move(*x._value);
                x._value.destroy();
            } else {
                _value.destroy();
            }
        } else {
            if (x._is_some) {
                _value.emplace(std::move(*x._value));
                x._value.destroy();
            }
        }
        _is_some = x._is_some;
        x._is_some = false;
        return *this;
    }
    
    template<typename U>
    Option& operator=(U&& x) {
        if (_is_some) {
            *_value = std::forward<U>(x);
        } else {
            _value.emplace(std::forward<U>(x));
        }
        _is_some = true;
        return *this;
    }
    
    explicit operator bool() const { return _is_some; }
    
    T& operator*() & { assert(_is_some); return *_value; }
    T const& operator*() const& { assert(_is_some); return *_value; }
    T&& operator*() && { assert(_is_some); return *_value; }
    
    T* operator->() { assert(_is_some); return _value.get(); }
    T const* operator->() const { assert(_is_some); return _value.get(); }
    
    bool is_some() const { return _is_some; }
    bool is_none() const { return !_is_some; }
    
    Option<T const&> as_ref() const {
        return is_some ? Some(*_value) : None;
    }
    
    Option<T&> as_mut() const {
        return is_some ? Some(*_value) : None;
    }
    
    T unwrap() && {
        assert(_is_some);
        _is_some = false;
        return T{std::move(*_value)};
    }
    
    template<typename F>
    auto map(F&& f) {
        return _is_some ? Some(map(*_value)) : None;
    }
    
    template<typename F>
    auto map(F&& f) const {
        return _is_some ? Some(map(*_value)) : None;
    }
    
    template<typename F>
    auto map(F&& f) &&;
    
    
    
    
    
        
        
        
        
        

};


template<typename T>
class Option<T&> {
    
    T* _ptr = nullptr;
    
public:
    
    Option() = default;
    
    template<typename U>
    Option(Option<U&> const& x)
    : _ptr(x._ptr) {
    }

    template<typename U>
    Option(Option<U&>& x)
    : _ptr(x._ptr) {
    }
    
    template<typename U>
    Option(U& x)
    : x._ptr(&x) {
    }
    
    ~Option() = default;
    
    Option& operator=(Option<U&> const& x) {
        _ptr = x._ptr;
        return *this;
    }

    Option& operator=(Option<U&>& x) {
        _ptr = x._ptr;
        return *this;
    }
    
    Option& operator=(U& x) {
        _ptr = &x;
        return *this;
    }
    
    explicit operator bool() const { return _ptr; }
    
    T& operator*() const { assert(_ptr); return *_ptr; }
    T* operator->() const { assert(_ptr); return _ptr; }
    
};


#endif /* option_hpp */
