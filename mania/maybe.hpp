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
    
    // Aligned bytes where there may or may not be a T, tracked by some
    // external mechanism.  The interface is necessarily unsafe.  It is useful
    // for building option and hash_map::entry.
    //
    // Type must be relocatable; that is, a move-constructed object must be
    // a bitwise copy of its source, and a moved-from object must be in a state
    // for which the destructor is a no-op.  Objects can then be moved by
    // a memcpy into an uninitialized destination, and forgetting the source.
    // (This is technically undefined behaviour?)
    //
    // All movable STL types are relocatable (I think).  Non-relocatable types
    // typically contain pointers to themselves.  Note that assign-move-by-swap
    // is incompatible with relocation as the source, holding the old value of
    // the destination, is never deleted.
    //
    // We can thus move-construct, move-assign (to an empty dest), and swap
    // maybes without knowing if the sources are constructed.
    
    template<typename T>
    class maybe {
        
        alignas(alignof(T)) unsigned char _[sizeof(T)];
        
    public:
        
        // Postcondition: there is no object
        maybe() = default;
        
        // Precondition: there is no object
        ~maybe() = default;

        // Deleted: there is no useful implementation
        maybe(const maybe&) = delete;

        // Postcondition: there is no object at source
        maybe(maybe&&) = default;

        // Deleted: there is no useful implementation
        maybe& operator=(const maybe&) = delete;

        // Precondition: there is no object at destination
        // Postcondition: there is no object at source
        maybe& operator=(maybe&&) = default;

        // Swap any combination of states.
        void swap(maybe& r) {
            std::swap_ranges(_, _ + sizeof(T), r._);
        }
        
        // Precondition: there is no object
        template<typename... Args>
        void emplace(Args&&... args) {
            new (this) T(std::forward<Args>(args)...);
        }
        
        // Precondition: there is an object
        void destroy() const {
            operator*().~T();
        }
        
        // Precondition: there is an object
        T& operator*() {
            return reinterpret_cast<T&>(*this);
        }
        
        // Precondition: there is an object
        const T& operator*() const {
            return reinterpret_cast<const T&>(*this);
        }
        
        // Precondition: there is an object
        T* operator->() {
            return reinterpret_cast<T*>(this);
        }
        
        // Precondition: there is an object
        const T* operator->() const {
            return reinterpret_cast<const T*>(this);
        }
        
    }; // class maybe<T>
    
    template<typename T>
    void swap(maybe<T>& a, maybe<T>& b) {
        a.swap(b);
    }
    
    
    // A region of memory that may contain no object, or any object of
    // compatible size or alignment.  Unsafe.  Useful for sum types.
    //
    // We can swap and relocate without knowing the type (technically undefined
    // behaviour?)
    
    template<typename T>
    class disarray {
        
        alignas(alignof(T)) unsigned char _[sizeof(T)];
        
    public:
        
        disarray() = default;
        disarray(const disarray&) = delete;
        disarray(disarray&&) = default;
        ~disarray() = default;
        disarray& operator=(const disarray&) = delete;
        disarray& operator=(disarray&&) = default;
        
        void swap(disarray& r) {
            std::swap_ranges(_, _ + sizeof(T), r._);
        }
        
        template<typename U, typename... Args, typename = std::enable_if_t<!(alignof(T) % alignof(U)) && (sizeof(U) <= sizeof(T))>>
        void emplace(Args&&... args) {
            new (_) U(std::forward<Args>(args)...);
        }
        
        template<typename U>
        void destroy() {
            get<U>().~U();
        }
        
        template<typename U>
        U& get() {
            return reinterpret_cast<U&>(*this);
        }

        template<typename U>
        const U& get() const {
            return reinterpret_cast<U&>(*this);
        }

        
        
    };
    
    
} // namespace maybe

#endif /* maybe_h */
