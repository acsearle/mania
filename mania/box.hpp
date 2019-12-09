//
//  box.hpp
//  mania
//
//  Created by Antony Searle on 4/12/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef box_hpp
#define box_hpp

#include <utility>

#include "hash.hpp"

namespace manic {

template<typename T>
struct box {
    
    T* _ptr;
    
    box() : _ptr(nullptr) {}
    
    box(box const&) = delete;
    box(box&& x) : _ptr(std::exchange(x._ptr, nullptr)) {}
    
    ~box() { delete _ptr; }
    
    box& operator=(box const&) = delete;
    box& operator=(box&& x);
    
    explicit box(T* p) : _ptr(p) {}
    
    template<typename... Args>
    static box from(Args&&... args) {
        box a;
        a._ptr = new T{std::forward<Args>(args)...};
        return a;
    }
    
    static box from_raw(T* p) { return box(p); }
    
    [[nodiscard]] T* into_raw() { return std::exchange(_ptr, nullptr); }

    T* operator->() const {
        assert(_ptr);
        return _ptr;
    }
    
    T& operator*() const {
        assert(_ptr);
        return _ptr;
    }
    
    T& leak() { return *std::exchange(_ptr, nullptr); }
    
}; // struct box

template<typename T>
box<T>& box<T>::operator=(box<T>&& x) {
    box y{std::move(x)};
    using std::swap;
    swap(_ptr, y._ptr);
}

template<typename T>
bool operator==(box<T> const& a, box<T> const& b) {
    return *a == *b;
}

template<typename T>
bool operator!=(box<T> const& a, box<T> const& b) {
    return *a != *b;
}

template<typename T>
u64 hash(box<T> const& x) {
    return hash(*x);
}



} // namespace manic

#endif /* box_hpp */
