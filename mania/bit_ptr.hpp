//
//  bit.hpp
//  mania
//
//  Created by Antony Searle on 26/12/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef bit_hpp
#define bit_hpp

#include <cstddef>
#include <cstdint>
#include <utility>
#include <climits>

namespace mania {
    
    class bit_cptr;
    class bit_ptr;
    
    class bit_cref;
    class bit_ref;
    
    // Pointer and proxy reference to individual bits, packed into a single
    // int64_t by relying on redundancy of of high bits on extant 64-bit
    // systems.  Arithmetic on the pointers is fast, access to individual bits
    // is slow.
    
#define bit_sizeof(T) (sizeof(T) * CHAR_BIT);
    
    class bit_cptr {
        
        int64_t _ptr;
        
        explicit bit_cptr(int64_t ptr) : _ptr(ptr) {}
        
    public:
        
        bit_cptr() = default;
        bit_cptr(const bit_cptr&) = default;
        bit_cptr(bit_cptr&&) = default;
        ~bit_cptr() = default;
        bit_cptr& operator=(const bit_cptr&) = default;
        bit_cptr& operator=(bit_cptr&&) = default;
        
        template<typename T>
        bit_cptr(const T* ptr) : _ptr(reinterpret_cast<int64_t>(ptr) * CHAR_BIT) {}
        bit_cptr(std::nullptr_t) : _ptr(reinterpret_cast<int64_t>(nullptr)) {}
        template<typename T>
        bit_cptr(const T* ptr, ptrdiff_t off) : _ptr((reinterpret_cast<int64_t>(ptr) * CHAR_BIT) + off) {}
        
        bit_cptr(bit_ptr ptr);
        
        template<typename T>
        bit_cptr& operator=(const T* ptr) { _ptr = reinterpret_cast<int64_t>(ptr) * CHAR_BIT; return *this; }
        bit_cptr& operator=(bit_ptr ptr);
        
        explicit operator bool() const { return static_cast<bool>(_ptr); }
        
        bit_cref operator[](ptrdiff_t i) const;
        bit_cref operator*() const;
        
        bit_cptr operator++(int) { return bit_cptr(_ptr++); }
        bit_cptr operator--(int) { return bit_cptr(_ptr--); }

        bit_cptr& operator++() { ++_ptr; return *this; }
        bit_cptr& operator--() { --_ptr; return *this; }

        friend bit_cptr operator+(bit_cptr a, ptrdiff_t b) { return bit_cptr(a._ptr + b); }
        friend bit_cptr operator-(bit_cptr a, ptrdiff_t b) { return bit_cptr(a._ptr - b); }
        
        friend bit_cptr operator+(ptrdiff_t a, bit_cptr b) { return bit_cptr(a + b._ptr); }
        friend int64_t operator-(bit_cptr a, bit_cptr b) { return a._ptr - b._ptr; }

#define X(F)\
        friend bool operator F (bit_cptr a, bit_cptr b) { return a._ptr F b._ptr; }\

        X(<) X(<=) X(>) X(>=)
        X(==) X(!=)
        
#undef X
        
        bit_cptr& operator+=(ptrdiff_t i) { _ptr += i; return *this; }
        bit_cptr& operator-=(ptrdiff_t i) { _ptr -= i; return *this; }

        int64_t raw() const { return _ptr; }
        
        // Express as an (aligned) pointer and a shift
        template<typename T>
        std::pair<const T*, ptrdiff_t> get() const {
            constexpr int64_t mask = (alignof(T) * CHAR_BIT) - 1;
            const T* ptr = reinterpret_cast<const T*>((_ptr & ~mask) / CHAR_BIT);
            ptrdiff_t off = static_cast<ptrdiff_t>(_ptr & mask);
            return std::make_pair(ptr, off);
        }
        
    };
    
    
    
    
    class bit_ptr {
        
        int64_t _ptr;
        
        explicit bit_ptr(int64_t ptr) : _ptr(ptr) {}
        
    public:
        
        bit_ptr() = default;
        bit_ptr(const bit_ptr&) = default;
        bit_ptr(bit_ptr&&) = default;
        ~bit_ptr() = default;
        bit_ptr& operator=(const bit_ptr&) = default;
        bit_ptr& operator=(bit_ptr&&) = default;
        
        template<typename T, typename = typename std::enable_if<!std::is_const<T>::value>::type>
        explicit bit_ptr(T* ptr) : _ptr(reinterpret_cast<int64_t>(ptr) * CHAR_BIT) {}
        bit_ptr(std::nullptr_t) : _ptr(reinterpret_cast<int64_t>(nullptr)) {}
        template<typename T, typename = typename std::enable_if<!std::is_const<T>::value>::type>
        bit_ptr(T* ptr, ptrdiff_t off) : _ptr((reinterpret_cast<int64_t>(ptr) * CHAR_BIT) + off) {}
        
        template<typename T, typename = typename std::enable_if<!std::is_const<T>::value>::type>
        bit_ptr& operator=(T* ptr) { _ptr = reinterpret_cast<int64_t>(ptr) * CHAR_BIT; return *this; }
        
        explicit operator bool() const { return static_cast<bool>(_ptr); }
        
        bit_ref operator[](ptrdiff_t i) const;
        bit_ref operator*() const;
        
        bit_ptr operator++(int) { return bit_ptr(_ptr++); }
        bit_ptr operator--(int) { return bit_ptr(_ptr--); }
        
        bit_ptr& operator++() { ++_ptr; return *this; }
        bit_ptr& operator--() { --_ptr; return *this; }
        
        friend bit_ptr operator+(bit_ptr a, ptrdiff_t b) { return bit_ptr(a._ptr + b); }
        friend bit_ptr operator-(bit_ptr a, ptrdiff_t b) { return bit_ptr(a._ptr - b); }
        
        friend bit_ptr operator+(ptrdiff_t a, bit_ptr b) { return bit_ptr(a + b._ptr); }
        friend int64_t operator-(bit_ptr a, bit_ptr b) { return a._ptr - b._ptr; }
        
#define X(F)\
friend bool operator F (bit_ptr a, bit_ptr b) { return a._ptr F b._ptr; }\

        X(<) X(<=) X(>) X(>=)
        X(==) X(!=)
        
#undef X
        
        bit_ptr& operator+=(ptrdiff_t i) { _ptr += i; return *this; }
        bit_ptr& operator-=(ptrdiff_t i) { _ptr -= i; return *this; }
        
        int64_t raw() const { return _ptr; }

        // Express as an (aligned) pointer and a shift
        template<typename T>
        std::pair<T*, ptrdiff_t> get() const {
            constexpr int64_t mask = (alignof(T) * CHAR_BIT) - 1;
            T* ptr = reinterpret_cast<T*>((_ptr & ~mask) / CHAR_BIT);
            ptrdiff_t off = static_cast<ptrdiff_t>(_ptr & mask);
            return std::make_pair(ptr, off);
        }
        
    };
    
    
    
    
    
    
    
    class bit_cref {
        
        bit_cptr _ptr;
        
    public:
        
        bit_cref() = delete;
        bit_cref(const bit_cref&) = default;
        bit_cref(bit_cref&&) = default;
        ~bit_cref() = default;
        bit_cref& operator=(const bit_cref&) = delete;
        bit_cref& operator=(bit_cref&&) = delete;
        
        bit_cref(bit_cptr ptr);
        bit_cref(bit_ref);

        operator bool() const;
        bit_cptr operator&() const;
        
    };

    class bit_ref {
        
        bit_ptr _ptr;
        
    public:
        
        bit_ref() = delete;
        bit_ref(const bit_ref&) = default;
        bit_ref(bit_ref&&) = default;
        ~bit_ref() = default;
        bit_ref& operator=(const bit_ref&);
        bit_ref& operator=(bit_ref&&);
        
        bit_ref(bit_ptr ptr);
        
        operator bool() const;
        bit_ptr operator&() const;
        
        bit_ref& operator=(bool x);
        bit_ref& operator&=(bool x);
        bit_ref& operator|=(bool x);
        bit_ref& operator^=(bool x);
        
    };

    
    
    inline bit_cref bit_cptr::operator[](ptrdiff_t i) const {
        return bit_cref(*this + i);
    }
    
    inline bit_cref bit_cptr::operator*() const {
        return bit_cref(*this);
    }
    
    inline bit_cref::bit_cref(bit_cptr ptr)
    : _ptr(ptr) {
    };
    
    inline bit_cref::operator bool() const {
        auto a = _ptr.get<unsigned char>();
        return ((*a.first) >> (a.second)) & 1;
    }
    
    inline bit_cptr bit_cref::operator&() const {
        return _ptr;
    }
    
    inline bit_ref& bit_ref::operator=(bool x) {
        auto a = _ptr.get<unsigned char>();
        if (x) {
            *a.first |= 1 << a.second;
        } else {
            *a.first &= ~(1 << a.second);
        }
        return *this;
    }
    
    inline bit_ref& bit_ref::operator|=(bool x) {
        auto a = _ptr.get<unsigned char>();
        *a.first |= x << a.second;
        return *this;
    }
    
    inline bit_ref& bit_ref::operator&=(bool x) {
        auto a = _ptr.get<unsigned char>();
        *a.first &= ~(!x << a.second);
        return *this;
    }
    
    inline bit_ref& bit_ref::operator^=(bool x) {
        auto a = _ptr.get<unsigned char>();
        *a.first ^= x << a.second;
        return *this;
    }
    
} // namespace mania

namespace std {
    
    mania::bit_ptr copy(mania::bit_cptr first, mania::bit_cptr last, mania::bit_ptr d_first) {
        
        auto n = last.raw() - first.raw();
        auto a = last.raw() ^ first.raw();
        
        
        return d_first;
    }
    
}

#endif /* bit_hpp */
