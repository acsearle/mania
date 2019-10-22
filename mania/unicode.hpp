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

u8* utf8advance(u8* p) {
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

bool utf8validatez(u8* p) {
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


unsigned char* utf8advance_unsafe(unsigned char* p) {
    
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

struct utf8iterator {
    
    u8 const* _ptr;
    
    bool _is_continuation_byte() const {
        return (*_ptr & 0xC0) == 0x80;
    }
    
    using difference_type = std::ptrdiff_t;
    using value_type = u32;
    using reference = u32;
    using pointer = void;
    using iterator_category = std::bidirectional_iterator_tag;
    
    utf8iterator() = default;
    
    explicit utf8iterator(void const* ptr)
    : _ptr((u8 const*) ptr) {
    }
    
    utf8iterator& operator++() {
        do {
            ++_ptr;
        } while (_is_continuation_byte());
        return *this;
    }
    
    u32 operator*() const {
        if ((_ptr[0] & 0x80) == 0x00)
            return (((_ptr[0]       )      ));
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
    
    
};



struct string {
    
    // utf-8 string
    
    vector<u8> _bytes;
    
    string() = default;
    
    explicit string(const char* utf8z) {
        std::size_t n = std::strlen(utf8z);
        _bytes.assign(utf8z, utf8z + n);
    }
    
    utf8iterator begin() const { return utf8iterator(_bytes.begin()); }
    utf8iterator end() const { return utf8iterator(_bytes.end()); }
    
    char const* c_str() {
        _bytes.push_back(0);
        _bytes.pop_back();
        return (char const*) _bytes.begin();
    }
    
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
    }
    
    void pop_back() {
        u8 c;
        do {
            c = _bytes.pop_back();
        } while ((c & 0xC0) == 0x80);
    }
    
    bool empty() const {
        return !_bytes.size();
    }
    
    void clear() {
        _bytes.clear();
    }

};

        
        
}
        
#endif /* unicode_hpp */
