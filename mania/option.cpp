//
//  option.cpp
//  mania
//
//  Created by Antony Searle on 9/3/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "option.hpp"

#include "maybe.hpp"

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
                _value.emplace(*r);
        }
        
        option(option&& r)
        : _value(std::move(r._value))
        , _has(std::exchange(r._has, false)) {
        }
        
        ~option() {
            if (_has)
                _value.destroy();
        }
        
        void swap(option& r) {
            using std::swap;
            swap(_value, r._value);
            swap(_has, r._has);
        }
        
        option& operator=(const option& r) {
            option(r).swap(*this);
            return *this;
        }
        
        option& operator=(option&& r) {
            option(std::move(r)).swap(*this);
            return *this;
        }
        
        explicit operator bool() const { return _has; }
        
        T& operator*() { assert(_has); return *_value; }
        const T& operator*() const { assert(_has); return *_value; }
        
        T* operator->() { assert(_has); return _value.operator->(); }
        const T* operator->() const { assert(_has); return _value.operator->(); }
        
    };
    
    template<typename T>
    void swap(option<T>& a, option<T>& b) {
        a.swap(b);
    }
    
    template<typename T>
    class option<T&> {
        
        T* _ptr;
        
    public:
        
    };
    
    
}
