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

template<typename T>
class maybe {
    
    alignas(alignof(T)) unsigned char _[sizeof(T)];
    
public:
    
    template<typename... Args>
    void emplace(Args&&... args) {
        new (this) T(std::forward<Args>(args)...);
    }

    T* get() { return reinterpret_cast<T*>(_); }
    T const* get() const { return reinterpret_cast<T const*>(_); }
    
    T* operator->() { return get(); }
    T const* operator->() const { return get(); }
    
    T& operator*() { return *get(); }
    T const& operator*() const { return *get(); }

    void destroy() const {
        get()->~T();
    }

}; // class maybe<T>



    
} // namespace maybe

#endif /* maybe_h */
