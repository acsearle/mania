//
//  memory.hpp
//  mania
//
//  Created by Antony Searle on 27/10/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef memory_hpp
#define memory_hpp

#include <utility>

#include "optional.hpp"

namespace manic {
    
    
    
    template<typename T>
    class ref {
        
        const T* _ptr;
        
    public:
        
        ref() = delete;
        ref(const ref& r)
        : _ptr(r._ptr) {
            borrow(r);
        }
        ref(ref&& r)
        : _ptr(std::exchange(r._ptr, nullptr)) {
        }
        
        ref& operator=(const ref&) = delete;
        ref& operator=(ref&&) = delete;
        
        const T* get() const { return _ptr; }
        
        
        
        
    };
    
    
    template<typename T>
    class ref_mut;
    
    
    
    
    
    
    
    template<typename T>
    class cell {
        
        mutable T _value;
        
    public:
        
        cell() = default;
        cell(const cell&) = default;
        cell(cell&&) = default;
        ~cell() = default;
        cell& operator=(const cell&) = default;
        cell& operator=(cell&&) = default;
        
        cell(T&& x) : _value(std::move(x)) {}
        
        T get() const { return _value; }
        T& get_mut() { return _value; }
        void set(T&& x) const { _value = x; }
        void swap(const cell& r) const {
            std::swap(_value, r._value);
        }
        void replace(T&& x) const {
            std::swap(_value, x);
            return std::move(x);
        }
        T into_inner() && { return std::move(_value); }
        T take() const { T tmp; std::swap(tmp, _value); return tmp; }
        
    };
    
    template<typename T>
    class refcell {
        
        mutable T _value;
        int _borrows;
        
    public:
        
        refcell(T&& value) : _value(std::move(value)) {}
        T into_inner() && { return std::move(_value); }
        T replace(T&& value) const {
            assert(_borrows == 0);
            std::swap(_value, value);
            return std::move(value);
        }
        void swap(const refcell<T>& r) {
            asert((_borrows == 0) && (r._borrows == 0));
            std::swap(_value, r._value);
        }
        ref<T> borrow() const;
        ref_mut<T> borrow_mut() const;
        
        
    };
    
    template<typename T>
    class box {
        
        T* _ptr;
        
    public:
        
        box() = delete;
        box(const box&) = delete;
        box(box&& r) : _ptr(std::exchange(r._ptr, nullptr)) {}
        ~box() { delete _ptr; }
        box& operator=(const box&) = delete;
        box& operator=(box&& r) { std::swap(_ptr, r._ptr); return *this; }

        box(T&& x) : _ptr(new T(x)) {}

    };
    
    template<typename T>
    class arc {
    
    public:
        
        arc(T&& data);
        
    };
    
    template<typename T>
    T into_raw(arc<T>&& r);
    
    template<typename T>
    T& make_mut(arc<T>& r);
    
    template<typename T>
    optional<T&> get_mut(arc<T>& r);
    
    template<typename T>
    void drop(arc<T>&& r) {
        arc<T> tmp(std::move(r));
    }
    
    
    
};

#endif /* memory_hpp */
