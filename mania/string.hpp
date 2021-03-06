//
//  string.hpp
//  mania
//
//  Created by Antony Searle on 28/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef string_hpp
#define string_hpp

#include "string_view.hpp"
#include "vector.hpp"

namespace manic {

struct string {
    
    // utf-8 string
    //
    // stored in a vector of u8.  to permit efficient c_str(), we guarantee
    // that the allocation extends at least one u8 beyond end(), and that
    // *end() == 0; this often leads to a
    //
    //     _bytes.push_back(0)
    //     _bytes.pop_back()
    //
    // idiom
    
    vector<u8> _bytes;
    
    using const_iterator = utf8_iterator;
    using iterator = const_iterator;
    using value_type = u32;
    
    string() = default;
    
    string(char const* z) {
        assert(z);
        std::size_t n = std::strlen(z);
        _bytes.assign(z, z + n + 1);
        _bytes.pop_back();
    }
    
    string(char const* p, usize n) {
        _bytes.reserve(n + 1);
        _bytes.assign(p, p + n);
        _bytes.push_back(0);
        _bytes.pop_back();
    }
    
    string(char const* p, char const* q) {
        _bytes.reserve(q - p + 1);
        _bytes.assign(p, q);
        _bytes.push_back(0);
        _bytes.pop_back();
    }
    
     string(string_view v) {
        _bytes.reserve(v.as_bytes().size() + 1);
        _bytes = v.as_bytes();
        _bytes.push_back(0);
        _bytes.pop_back();
    }
    
    explicit string(const_iterator a, const_iterator b) {
        _bytes.reserve(b._ptr - a._ptr + 1);
        _bytes.assign(a._ptr, b._ptr);
        _bytes.push_back(0);
        _bytes.pop_back();
    }
    
    explicit string(vector<u8>&& bytes) : _bytes(bytes) {
        _bytes.push_back(0);
        _bytes.pop_back();
    }
    
    operator const_vector_view<u8>() const { return _bytes; }
    operator string_view() const { return string_view(begin(), end()); }
    
    const_iterator begin() const { return utf8_iterator{_bytes.begin()}; }
    const_iterator end() const { return utf8_iterator{_bytes.end()}; }
    
    u8 const* data() const { return _bytes.begin(); }
    
    const_vector_view<u8> as_bytes() const {
        return _bytes;
    }
    
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
        // this is a good argument for const_vector_view having _end rather than
        // _size
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
    
    /*
    friend bool operator==(string const& a, string const& b) {
        return std::equal(a.begin(), a.end(), b.begin(), b.end());
    }
    
    friend bool operator!=(string const& a, string const& b) {
        return !(a == b);
    }
    */
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
    
    template<typename Deserializer>
    inline auto deserialize(placeholder<string>, Deserializer& d) {
        return string{deserialize<vector<u8>>(d)};
    }


struct immutable_string {
    
    // When a string is used as a hash table key, it is important to
    // keep the inline size small.  It will only be accessed for strcmp
    // to check hash collisions, and never mutated.  We can dispense
    // with _data and _capacity, place _end at the beginning of the
    // allocation, place the string after it, and infer _begin from
    // the allocation.
    
    // This is the simplest of several ways we can lay out the data, allowing
    // increasing mutability.  The downside is that accessing the iterators
    // requires pointer chasing, making use cases where we access the metadata
    // but not the data slower.
    
    struct implementation {
        u8* _end;
        u8 _begin[];
        
        // or,
        // u8* _begin
        // u8* _end
        // u8* _capacity
        // u8 _data[]
        
        static implementation* make(string_view v) {
            auto n = v.as_bytes().size();
            implementation* p = (implementation*) malloc(sizeof(implementation) + n + 1);
            p->_end = p->_begin + n;
            std::memcpy(p->_begin, v.as_bytes().begin(), n);
            *p->_end = 0;
            return p;
        }
        
        static implementation* make(implementation* q) {
            auto n = q->_end - q->_begin;
            implementation* p = (implementation*) malloc(sizeof(implementation) + n + 1);
            p->_end = p->_begin + n;
            std::memcpy(p->_begin, q->_begin, n + 1);
            return p;
        }
        
    };
    
    implementation* _body;
    
    immutable_string() : _body(nullptr) {}
    
    immutable_string(immutable_string const&) = delete;
    
    immutable_string(immutable_string&& s)
    : _body(std::exchange(s._body, nullptr)) {}
    
    ~immutable_string() {
        free(_body);
    }
    
    immutable_string& operator=(immutable_string&& s) {
        immutable_string r(std::move(s));
        std::swap(_body, r._body);
        return *this;
    }
    
    immutable_string clone() const {
        immutable_string r;
        if (_body)
            r._body = implementation::make(_body);
        return r;
    }
    
    explicit immutable_string(string_view v)
    : _body(implementation::make(v)) {
    }
    
    using const_iterator = utf8_iterator;
    using iterator = utf8_iterator;
    
    const_iterator begin() const {
        return iterator{_body ? _body->_begin : nullptr};
    }
    
    const_iterator end() const {
        return iterator{_body ? _body->_end : nullptr};
    }
    
    char const* c_str() const {
        return _body ? (char const*) _body->_begin : nullptr;
    }
    
    u8 const* data() const {
        return _body ? (u8 const*) _body->_begin : nullptr;
    }
    
    const_vector_view<u8> as_bytes() const {
        return const_vector_view<u8>(_body ? _body->_begin : nullptr,
                                     _body ? _body->_end : nullptr);
    }
    
    operator string_view() const {
        return string_view(begin(), end());
    }
    
    bool empty() const {
        return !(_body && (_body->_end - _body->_begin));
    }
    
    
};



} // namespace manic




#endif /* string_hpp */
