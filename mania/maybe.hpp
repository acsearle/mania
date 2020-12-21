//
//  maybe.hpp
//  mania
//
//  Created by Antony Searle on 24/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef maybe_h
#define maybe_h

#include <utility>

namespace manic {

// Aligned bytes where there may or may not be a T, tracked by some external
// mechanism.  The interface is necessarily unsafe.  It is useful for building
// types with unusual lifetimes, such as optional, or participants in
// relocation that are on the stack or embedded in other types.
//
// maybe<T> is a TrivialType
//
// Some operations require T is Relocatable

template<typename T>
class Maybe {
    
    alignas(alignof(T)) unsigned char _[sizeof(T)];
    
public:
    
    // Defaulted, unsafe, constructors and assignment
    
    template<typename... Args>
    void emplace(Args&&... args) {
        new (this) T(std::forward<Args>(args)...);
    }

    void destroy() const {
        (**this).~T();
    }

    T* get() { return reinterpret_cast<T&>(_); }
    T const* get() const { return reinterpret_cast<T const&>(_); }
    
    T* operator->() { return get(); }
    T const* operator->() const { return get(); }
    
    T& operator*() { return *get(); }
    T const& operator*() const { return *get(); }
    
    template<typename... Args>
    static Maybe from(Args&&... args) {
        Maybe x;
        x.emplace(std::forward<Args>(args)...);
        return x;
    }

}; // class maybe<T>


template<typename T>
union maybe {
    
    T value;
    
    maybe() {}
    maybe(maybe const&) = default;
    ~maybe() {}
    
    template<typename... Args>
    void emplace(Args&&... args) {
        new (&value) T(std::forward<Args>(args)...);
    }
    
    void erase() const noexcept {
        (&value)->~T();
    }
    
};


    
} // namespace maybe

#endif /* maybe_h */
