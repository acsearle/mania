//
//  string.hpp
//  mania
//
//  Created by Antony Searle on 28/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef string_hpp
#define string_hpp

#include <iostream>
#include "unicode.hpp"

namespace manic {

struct string_view {
    
    using const_iterator = utf8_iterator;
    using iterator = const_iterator;
    using value_type = u32;
    
    const_iterator a, b;
    
    string_view() : a(nullptr), b(nullptr) {}
    string_view(char const* z) : a(z), b(z + strlen(z)) {}
    string_view(char const* p, usize n) : a(p), b(p + n) {}
    string_view(char const* p, char const* q) : a(p), b(q) {}
    string_view(const_iterator p, const_iterator q) : a(p), b(q) {}
    string_view(string_view const&) = default;
    
    bool empty() const { return a == b; }
    
    const_iterator begin() const { return a; }
    const_iterator end() const { return b; }
    const_iterator cbegin() const { return a; }
    const_iterator cend() const { return b; }
    
    u32 front() const { return *a; }
    u32 back() const { utf8_iterator c(b); return *--c; }
    
    friend bool operator==(string_view a, string_view b) {
        return std::equal(a.a, a.b, b.a, b.b);
    }
    
    friend bool operator!=(string_view a, string_view b) { return !(a == b); }
    
    friend bool operator<(string_view a, string_view b) {
        return std::lexicographical_compare(a.a, a.b, b.a, b.b);
    }
    
    friend bool operator>(string_view a, string_view b) { return b < a; }
    friend bool operator<=(string_view a, string_view b) { return !(b < a); }
    friend bool operator>=(string_view a, string_view b) { return !(a < b); }
    
    // terse (dangerous?) operations useful for parsing
    u32 operator*() const { return *a; }
    string_view& operator++() { ++a; return *this; }
    string_view& operator--() { --b; return *this; }
    explicit operator bool() const { return a != b; }
    
    const_vector_view<u8> as_bytes() const {
        return const_vector_view<u8>(a._ptr, b._ptr);
    }
    
    };
    
    
    inline std::ostream& operator<<(std::ostream& a, string_view b) {
        a.write((char const*) b.a._ptr, b.b._ptr - b.a._ptr);
        return a;
    }
    
    
    
    
    
    struct string {
        
        // utf-8 string
        
        vector<u8> _bytes;
        
        using const_iterator = utf8_iterator;
        using iterator = const_iterator;
        using value_type = u32;
        
        string() = default;
        
        string(char const* z) {
            std::size_t n = std::strlen(z);
            _bytes.assign(z, z + n + 1);
            _bytes.pop_back();
        }
        
        string(char const* p, usize n) {
            _bytes.assign(p, p + n);
            _bytes.push_back(0);
            _bytes.pop_back();
        }
        
        string(char const* p, char const* q) {
            _bytes.assign(p, q);
            _bytes.push_back(0);
            _bytes.pop_back();
        }
        
        explicit string(string_view v) {
            _bytes = v.as_bytes();
            _bytes.push_back(0);
            _bytes.pop_back();
        }
        
        explicit string(iterator a, iterator b) {
            _bytes.assign(a._ptr, b._ptr);
            _bytes.push_back(0);
            _bytes.pop_back();
        }
        
        operator const_vector_view<u8>() const { return _bytes; }
        operator string_view() const { return string_view(begin(), end()); }
        
        utf8_iterator begin() const { return utf8_iterator(_bytes.begin()); }
        utf8_iterator end() const { return utf8_iterator(_bytes.end()); }
        
        u8 const* data() const { return _bytes.begin(); }
        
        char const* c_str() const { return (char const*) _bytes.begin(); }
        
        void push_back(u32 c) {
            if (c < 0x80) {
                _bytes.push_back(c);
            } else if (c < 0x800) {
                _bytes.push_back(0xC0 | ((c >>  6)       ));
                _bytes.push_back(0x80 | ((c      ) & 0x3F));
            } else if (c < 0x10000) {
                _bytes.push_back(0xC0 | ((c >> 12)       ));
                _bytes.push_back(0x80 | ((c >>  6) & 0x3F));
                _bytes.push_back(0x80 | ((c      ) & 0x3F));
            } else {
                _bytes.push_back(0xC0 | ((c >> 18)       ));
                _bytes.push_back(0x80 | ((c >> 12) & 0x3F));
                _bytes.push_back(0x80 | ((c >>  6) & 0x3F));
                _bytes.push_back(0x80 | ((c      ) & 0x3F));
            }
            _bytes.push_back(0);
            _bytes.pop_back();
        }
        
        u32 pop_back() {
            assert(!empty());
            iterator e = end();
            --e;
            u32 c = *e;
            _bytes._size = (e._ptr - _bytes._begin);
            _bytes._begin[_bytes._size] = 0;
            return c;
        }
        
        u32 pop_front() {
            assert(!empty());
            iterator b = begin();
            u32 c = *b;
            ++b;
            _bytes._size -= (b._ptr - _bytes._begin);
            _bytes._begin += (b._ptr - _bytes._begin);
            return c;
        }
        
        bool empty() const {
            return !_bytes._size;
        }
        
        void clear() {
            _bytes.clear();
            _bytes.push_back(0);
            _bytes.pop_back();
        }
        
        void append(const char* z) {
            auto n = strlen(z);
            _bytes.append((u8 const*) z, (u8 const*) z + n + 1);
            _bytes.pop_back();
        }
        
        void append(string const& s) {
            _bytes.append(s._bytes._begin, s._bytes._begin + s._bytes._size + 1);
            _bytes.pop_back();
        }
        
        friend bool operator==(string const& a, string const& b) {
            //return (a._bytes._size == b._bytes._size) && !std::memcmp(a._bytes._begin, b._bytes._begin, a._bytes._size);
            return std::equal(a.begin(), a.end(), b.begin(), b.end());
        }
        
        friend bool operator!=(string const& a, string const& b) {
            return !(a == b);
        }
        
    };
    
    inline string operator+(string_view a, char const* b) {
        string s(a);
        s.append(b);
        return s;
    }
        
        inline std::ostream& operator<<(std::ostream& a, string const& b) {
            a.write((char const*) b._bytes._begin, b._bytes._size);
            return a;
        }

}

#endif /* string_hpp */
