//
//  cell.hpp
//  aarc
//
//  Created by Antony Searle on 10/9/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#ifndef cell_hpp
#define cell_hpp

#include <cstdlib>
#include <utility>

#include "common.hpp"

namespace manic {

template<typename T>
class cell {
    
    mutable T _value;
    
public:
    
    cell(T value) : _value(std::move(value)) {
    }
    
    cell& operator=(T value) {
        _value = std::move(value);
    }
    
    T get() const {
        return _value;
    }
    
    T& get_mut() {
        return _value;
    }
    
};

template<typename T>
class ref_cell {
    
    mutable T _value;
    mutable isize _borrows;
    
public:
    
    ref_cell()
    : _value()
    , _borrows(0) {
    }
        
    ref_cell(ref_cell const& other)
    : _value(other._value)
    , _borrows(0) {
        assert(other._borrows >= 0);
    }
    
    ref_cell(ref_cell&& other)
    : _value(std::move(other._value))
    , _borrows(0) {
        assert(other._borrows == 0);
    }
    
    ~ref_cell() {
        assert(_borrows == 0);
    }
    
    class ref {
        
        ref_cell const* _ptr;
        
        explicit ref(ref_cell const* ptr)
        : _ptr(ptr) {
            if (_ptr) {
                assert(_ptr->_borrows >= 0);
                ++ptr->_borrows;
            }
        }
        
    public:
        
        ref(ref const& other)
        : _ptr(other._ptr) {
            if (_ptr) {
                assert(_ptr->_borrows > 0);
                ++_ptr->_borrows;
            }
        }
        
        ref(ref&& other)
        : _ptr(std::exchange(other._ptr, nullptr)) {
            if (_ptr) {
                assert(_ptr->_borrows > 0);
            }
        }

        ~ref() {
            if (_ptr) {
                assert(_ptr->_borrows > 0);
                --_ptr->_borrows;
            }
        }
        
    };
    
    class ref_mut {
        
        ref_cell const* _ptr;
        
        explicit ref_mut(ref_cell const* ptr) : _ptr(ptr) {
            assert(ptr->_borrows == 0);
            ptr->_borrows = -1;
        }

    public:
        
        ~ref_mut() {
            if (_ptr) {
                assert(_ptr->_borrows == -1);
                _ptr->_borrows = 0;
            }
        }

    };
    
    ref borrow() const {
        ref(this);
    }
    
    ref_mut borrow_mut() const {
        ref_mut(this);
    }
    
    operator T&() {
        assert(_borrows == 0);
        return _value;
    }
    
    void swap(ref_cell const& other) const {
        assert((_borrows == 0) && (other._borrows == 0));
        using std::swap;
        swap(_value, other._value);
    }
    
    T take() const {
        replace(T());
    }
    
    T replace(T value) const {
        assert(_borrows == 0);
        using std::swap;
        swap(_value, value);
        return value;
    }
    
    T into_inner() && {
        assert(_borrows == 0);
        return std::move(_value);
    }
    
};

} // namespace manic

#endif /* cell_hpp */
