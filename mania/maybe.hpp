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
    
    // Aligned bytes where there may or may not be an object
    //
    // Type must be relocatable; that is, a move-constructed object must be
    // a bitwise copy of its source, and a moved-from object must be in a state
    // for which the destructor is a no-op.  This is true of all movable STL
    // types (I think).
    
    template<typename T>
    class maybe {
        
        alignas(alignof(T)) unsigned char _[sizeof(T)];
        
    public:
        
        maybe() = default;
        
        maybe(const maybe&) = delete;
        maybe(maybe&&) = default;
        
        ~maybe() = default;
        
        maybe& operator=(const maybe&) = delete;
        maybe& operator=(maybe&&) = default;
        
        template<typename... Args>
        T& emplace(Args&&... args) {
            new (this) T(std::forward<Args>(args)...);
            return get();
        }
        
        void destroy() const {
            get().~T();
        }
        
        T& get() {
            return reinterpret_cast<T&>(*this);
        }
        
        const T& get() const {
            return reinterpret_cast<const T&>(*this);
        }
        
        T take() const {
            return T(std::move(get()));
        }
        
    };
    
}

#endif /* maybe_h */
