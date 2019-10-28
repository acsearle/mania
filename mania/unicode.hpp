//
//  unicode.hpp
//  mania
//
//  Created by Antony Searle on 20/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef unicode_hpp
#define unicode_hpp

#include <cstddef>

#include <iterator>

#include "vector.hpp"

namespace manic {

inline const u8* utf8advance(const u8* p) {
    if (((p[0] & 0b10000000) == 0b00000000))
        return p + 1;
    if (((p[0] & 0b11100000) == 0b11000000) &&
        ((p[1] & 0b11000000) == 0b10000000))
        return p + 2;
    if (((p[0] & 0b11110000) == 0b11100000) &&
        ((p[1] & 0b11000000) == 0b10000000) &&
        ((p[2] & 0b11000000) == 0b10000000))
        return p + 3;
    if (((p[0] & 0b11111000) == 0b11110000) &&
        ((p[1] & 0b11000000) == 0b10000000) &&
        ((p[2] & 0b11000000) == 0b10000000) &&
        ((p[3] & 0b11000000) == 0b10000000))
        return p + 4;
    abort();
}

inline bool utf8validatez(u8* p) {
    for (;;) {
        if (p[0] == 0) {
            return true;
        } else if (((p[0] & 0b10000000) == 0b00000000)) {
            ++p;
        } else if (((p[0] & 0b11100000) == 0b11000000) &&
                   ((p[1] & 0b11000000) == 0b10000000)) {
            p += 2;
        } else if (((p[0] & 0b11110000) == 0b11100000) &&
                   ((p[1] & 0b11000000) == 0b10000000) &&
                   ((p[2] & 0b11000000) == 0b10000000)) {
            p += 3;
        } else if (((p[0] & 0b11111000) == 0b11110000) &&
                   ((p[1] & 0b11000000) == 0b10000000) &&
                   ((p[2] & 0b11000000) == 0b10000000) &&
                   ((p[3] & 0b11000000) == 0b10000000)) {
            p += 4;
        } else {
            return false;
        }
    }
    abort();
}

inline u8* utf8_encode(u32 a, u8 b[4]) {
    if (a < 0x8F) {
        b[0] = a;
        return b + 1;
    } else if (a < 0x800) {
        b[0] = 0b11000000 | (a >> 6);
        b[1] = 0b10000000 | (a & 0b00111111);
        return b + 2;
    } else if (a < 0x10000) {
        b[0] = 0b11100000 | (a >> 12);
        b[1] = 0b10000000 | ((a >> 6) & 0b00111111);
        b[2] = 0b10000000 | (a & 0b00111111);
        return b + 3;
    } else if (a < 0x110000) {
        b[0] = 0b11110000 | (a >> 18);
        b[1] = 0b10000000 | ((a >> 12) & 0b00111111);
        b[2] = 0b10000000 | ((a >> 6) & 0b00111111);
        b[3] = 0b10000000 | (a & 0b00111111);
        return b + 4;
    } else {
        abort();
    }
}


inline unsigned char* utf8advance_unsafe(unsigned char* p) {
    
    // Advance to the next code point in a valid UFT-8 sequence
    
    // The top 4 bits of the first byte distinguish between 1, 2, 3 and 4 byte
    // encodings
    //
    //     0b0******* -> 1
    //     0b10****** -> invalid
    //     0b110***** -> 2
    //     0b1110**** -> 3
    //     0b11110*** -> 4
    //
    // In the form of a table indexed by the top 4 bits
    //
    //     [ 1, 1, 1, 1, 1, 1, 1, 1, *, *, *, *, 2, 2, 3, 4 ].
    //
    // We can pack the table into the half-bytes of a 64-bit integer
    
    static constexpr std::uint64_t delta = 0x4322000011111111;
    
    // and then index the table by shifting and masking
    
    return p + ((delta >> ((*p >> 2) & 0b111100)) & 0b1111);
    
    // Bytes of the form 0b10****** cannot begin a valid UTF-8 code point.  The
    // table value we choose for these affords us some limited error handling
    // options.  We may choose 1, permissive, to advance one byte and attempt
    // to continue on the sequence.  Or we may choose 0, strict, refusing to
    // advance the pointer and likely hanging the process.
    
    // Other forms of invalid UTF-8 are not detectable in this scheme.
    
}

struct utf8_iterator {
    
    u8 const* _ptr;
    
    bool _is_continuation_byte() const {
        return (*_ptr & 0xC0) == 0x80;
    }
    
    using difference_type = std::ptrdiff_t;
    using value_type = u32;
    using reference = u32;
    using pointer = void;
    using iterator_category = std::bidirectional_iterator_tag;
    
    utf8_iterator() = default;
    
    explicit utf8_iterator(void const* ptr)
    : _ptr((u8 const*) ptr) {
    }
    
    explicit utf8_iterator(std::nullptr_t) : _ptr(nullptr) {}
    
    explicit operator bool() const { return _ptr; }
    bool operator!() const { return !_ptr; }
    
    utf8_iterator& operator++() {
        do { ++_ptr; } while (_is_continuation_byte());
        return *this;
    }
    
    utf8_iterator operator++(int) {
        utf8_iterator a(*this); operator++(); return a;
    }
    
    utf8_iterator& operator--() {
        do { --_ptr; } while (_is_continuation_byte());
        return *this;
    }
    
    utf8_iterator operator--(int) {
        utf8_iterator a(*this); operator--(); return a;
    }
    
    u32 operator*() const {
        if (!(_ptr[0] & 0x80))
            return *_ptr;
        return _deref_multibyte();
    }
    
    u32 _deref_multibyte() const {
        if ((_ptr[0] & 0xE0) == 0xC0)
            return (((_ptr[0] & 0x1F) <<  6) |
                    ((_ptr[1] & 0x3F)      ));
        if ((_ptr[0] & 0xF0) == 0xE0)
            return (((_ptr[0] & 0x0F) << 12) |
                    ((_ptr[1] & 0x3F) <<  6) |
                    ((_ptr[2] & 0x3F)      ));
        if ((_ptr[0] & 0xF8) == 0xF0)
            return (((_ptr[0] & 0x07) << 18) |
                    ((_ptr[1] & 0x3F) << 12) |
                    ((_ptr[2] & 0x3F) <<  6) |
                    ((_ptr[3] & 0x3F)      ));
        abort();
    }
    
    friend bool operator==(utf8_iterator a, utf8_iterator b) {
        return a._ptr == b._ptr;
    }
    
    friend bool operator!=(utf8_iterator a, utf8_iterator b) {
        return a._ptr != b._ptr;
    }
    
    friend bool operator==(utf8_iterator a, std::nullptr_t) {
        return a._ptr == nullptr;
    }
    
    friend bool operator==(std::nullptr_t, utf8_iterator b) {
        return nullptr == b._ptr;
    }
    
    friend bool operator!=(utf8_iterator a, std::nullptr_t) {
        return a._ptr != nullptr;
    }
    
    friend bool operator!=(std::nullptr_t, utf8_iterator b) {
        return nullptr != b._ptr;
    }
    
    
    };
    
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

        friend bool operator!=(string_view a, string_view b) { return a == b; }
        
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

    };
    
    
    
    
    
    
    struct string {
        
        // utf-8 string

        vector<u8> _bytes;

        using const_iterator = utf8_iterator;
        using iterator = const_iterator;
        using value_type = u32;
                
        string() = default;
        
        explicit string(char const* z) {
            std::size_t n = std::strlen(z);
            _bytes.assign(z, z + n + 1);
            _bytes.push_back(0);
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
        
        explicit operator const_vector_view<u8>() const { return _bytes; }
        explicit operator string_view() const { return string_view(begin(), end()); }

        utf8_iterator begin() const { return utf8_iterator(_bytes.begin()); }
        utf8_iterator end() const { return utf8_iterator(_bytes.end()); }

        u8 const* data() const { return _bytes.begin(); }
        
        char const* c_str() { return (char const*) _bytes.begin(); }
        
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
        
    };
    
    
}

#endif /* unicode_hpp */
