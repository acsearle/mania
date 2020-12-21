//
//  bytes.hpp
//  mania
//
//  Created by Antony Searle on 4/3/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#ifndef bytes_hpp
#define bytes_hpp

#include <cassert> // assert
#include <compare> // strong_ordering
#include <cstdlib> // malloc
#include <cstring> // memcpy
#include <utility> // exchange

#include "common.hpp"

#define let auto
#define self (*this)
#define Self std::remove_pointer_t<decltype(this)>

namespace manic {

class const_bytes_view {
    
    byte const* _begin;
    byte const* _end;
    
public:
    
    const_bytes_view() : _begin(nullptr), _end(nullptr) {}
    
    const_bytes_view(const_bytes_view const&) = default;
    
    const_bytes_view(byte const* a, byte const* b)
    : _begin(a)
    , _end(b) {
        assert(a <= b);
    }
    
    const_bytes_view(byte const* p, usize n)
    : _begin(p)
    , _end(p + n) {
    }
    
    ~const_bytes_view() = default;
    
    const_bytes_view& operator=(const_bytes_view const&) = delete;
        
    byte const* begin() const { return _begin; }
    byte const* end() const { return _end; }

    byte const* data() const { return _begin; }
    usize size() const { return _end - _begin; }
    
    bool is_empty() const { return _begin == _end; }
    
    byte const* will_read(usize n) {
        assert(size() >= n);
        let old = begin();
        _begin += n;
        return old;
    }
    
    template<typename T, std::enable_if_t<std::is_trivial_v<T>, int> = 0>
    T read() {
        T x;
        std::memcpy(&x, will_read(sizeof(T)), sizeof(T));
        return x;
    }

}; // class const_bytes_view

class bytes_view {
    
    byte* _begin;
    byte* _end;
    
public:
    
    bytes_view() : _begin(nullptr), _end(nullptr) {}
    
    bytes_view(bytes_view const&) = default;
    
    bytes_view(byte* a, byte* b)
    : _begin(a)
    , _end(b) {
        assert(a <= b);
    }
    
    bytes_view(byte* p, usize n)
    : _begin(p)
    , _end(p + n) {
    }

    ~bytes_view() = default;
    
    bytes_view& operator=(bytes_view const& other) {
        assert(self.size() == other.size());
        std::memcpy(self.data(), other.data(), other.size());
        return *this;
    }

    bytes_view& operator=(const_bytes_view other) {
        assert(self.size() == other.size());
        std::memcpy(self.data(), other.data(), other.size());
        return *this;
    }
    
    operator const_bytes_view() const {
        return const_bytes_view(_begin, _end);
    }

    byte* begin() const { return _begin; }
    byte* end() const { return _end; }
    
    byte* data() const { return _begin; }
    usize size() const { return _end - _begin; }
    
    bool is_empty() const { return _begin == _end; }

    byte const* will_read(usize n) {
        assert(size() >= n);
        let old = begin();
        _begin += n;
        return old;
    }
    
    template<typename T, std::enable_if_t<std::is_trivial_v<T>, int> = 0>
    T read() {
        T x;
        std::memcpy(&x, will_read(sizeof(T)), sizeof(T));
        return x;
    }

}; // class bytes_view

/*


// block-transfer-oriented contiguous byte queue


using std::exchange;

// A noncopy pointer that has the correct semantics to be a member of a
// noncopy data structure; like unique_ptr but without destruction
template<typename T>
class move_ptr {
    
    usize value;
    
    explicit move_ptr(std::uintptr_t ptr) : value(ptr) {}
    
public:
    
    move_ptr() : value(0) {}
    move_ptr(move_ptr&& other) : value(exchange(other.value, 0)) {}
    move_ptr(move_ptr const&) = delete;

    ~move_ptr() = default;

    void swap(move_ptr& other) { using std::swap; swap(self.value, other.value); }
    
    explicit move_ptr(T* ptr) : value(reinterpret_cast<usize>(ptr)) {}
    
    T* as_ptr() const { return reinterpret_cast<T*>(self.value);}
    
    T& as_ref() const {
        assert(self.value);
        return reinterpret_cast<T*>(self.value);
    }
    
    move_ptr<T> clone() const { return Self(self.value); }
    
    usize as_usize() const { return self.value; }
    explicit operator T*() const { return self.as_ptr(); }
    explicit operator bool() const { return self.value; }

    move_ptr<T> operator++(int) {
        let old = self;
        ++self;
        return old;
    }

    move_ptr<T> operator--(int) {
        let old = self;
        --self;
        return old;
    }
    
    T& operator[](usize n) const { return self.as_ptr()[n]; }
    T* operator->() const { return self.as_ptr(); }
    
    move_ptr<T>& operator++() {
        if constexpr (std::is_void_v<T>) {
            ++(self.value);
        } else {
            self.value += sizeof(T);
        }
        return self;
    }

    move_ptr<T>& operator--() {
        if constexpr (std::is_void_v<T>) {
            --(self.value);
        } else {
            self.value -= sizeof(T);
        }
        return self;
    }
    
    bool operator!() const { return !self.value; }
    T& operator*() const { return self.as_ref(); }
    
    // unlike T*, it is not undefined behaviour to compare across different
    // allocations
    auto operator<=>(move_ptr const&) const = default;
    
    move_ptr& operator=(move_ptr&& other) {
        self.value = exchange(other.value, 0);
        return self;
    }
    
    move_ptr& operator=(move_ptr const&) = delete;
    
    move_ptr& operator=(T* ptr) {
        self.value = reinterpret_cast<usize>(ptr);
        return self;
    }

    move_ptr operator+=(isize n) {
        if constexpr (std::is_void_v<T>) {
            self.value += n;
        } else {
            self.value += sizeof(T) * n;
        }
        return self;
    }

    move_ptr operator-=(isize n) {
        if constexpr (std::is_void_v<T>) {
            self.value -= n;
        } else {
            self.value -= sizeof(T) * n;
        }
        return self;
    }

};

template<typename T>
move_ptr<T> operator+(move_ptr<T> left, isize right) {
    left += right;
    return left;
}

template<typename T>
move_ptr<T> operator+(isize left, move_ptr<T> right) {
    right += left;
    return right;
}

template<typename T>
move_ptr<T> operator-(move_ptr<T> left, isize right) {
    left -= right;
    return left;
}

template<typename T>
isize operator-(move_ptr<T> const& left, move_ptr<T> const& right) {
    if constexpr (std::is_void_v<T>) {
        return left.as_usize() - right.as_usize();
    } else {
        return left.as_ptr() - right.as_ptr();
    }
}

template<typename T>
void swap(move_ptr<T>& first, move_ptr<T>& second) {
    first.swap(second);
}


// graph of conversions
//
// const_bytes_view
//       bytes_view
//       bytes
//
// we must not be able to construct a bytes_view from

struct bytes {
    
    move_ptr<byte> _begin;
    move_ptr<byte> _end;
    
    usize size() const {
        return _end - _begin;
    }
    
    ~bytes() {
        free(_begin.as_ptr());
    }
    
    void swap(bytes& other) {
        self._begin.swap(other._begin);
        self._end.swap(other._end);
    }
    
    byte* begin() { return _begin.as_ptr(); }
    byte* end() { return _end.as_ptr(); }

    bytes clone() {
        let ptr = (byte*) std::malloc(self.size());
        std::memcpy(ptr, self.begin(), self.size());
        return Self { move_ptr { ptr }, move_ptr { ptr + self.size() } };
    }
    
};


struct bytes_raw {
    
    byte* _begin;
    byte* _end;
    
    usize size() const {
        return _end - _begin;
    }
    
    bytes_raw() : _begin(nullptr), _end(nullptr) {}
    
    bytes_raw(bytes_raw&& other)
    : _begin(std::exchange(other._begin, nullptr))
    , _end(std::exchange(other._end, nullptr)) {
    }
    
    bytes_raw(bytes_raw& other) = delete;
    
    bytes_raw(byte* a, byte* b) : _begin(a), _end(b) {}
    
    ~bytes_raw() {
        free(_begin);
    }
    
    void swap(bytes_raw& other) {
        using std::swap;
        swap(_begin, other._begin);
        swap(_end, other._end);
    }
    
    bytes_raw& operator=(bytes_raw&& other) {
        bytes_raw(std::move(other)).swap(*this);
        return *this;
    }
    
    bytes_raw& operator=(bytes_raw const&) = delete;
    
    static bytes_raw with_size(usize n) {
        let ptr = (byte*) malloc(n);
        return bytes_raw(ptr, ptr + n);
    }
    
    bytes_raw clone() const {
        let other = Self::with_size(self.size());
        std::memcpy(other._begin, self._begin, self.size());
        return other;
    }
    
};



struct bytez : bytes_view {
    
    byte* _allocation;
    byte* _capacity;
        
    usize capacity() const {
        return _capacity - _allocation;
    }
    
    bytes(byte* b, byte* e, byte* a, byte* c)
    : _begin(b)
    , _end(e)
    , _allocation(a)
    , _capacity(c) {
    }
    
    bytes()
    : _begin(nullptr)
    , _end(nullptr)
    , _allocation(nullptr)
    , _capacity(nullptr) {
    }
    
    bytes(bytes const&) = delete;
    
    bytes(bytes&& other)
    : bytes(std::exchange(other._begin, nullptr),
            std::exchange(other._end, nullptr),
            std::exchange(other._allocation, nullptr),
            std::exchange(other._capacity, nullptr)) {
    }
    
    ~bytes() {
        free(_allocation);
    }
    
    void swap(bytes& other) {
        using std::swap;
        swap(_begin, other._begin);
        swap(_end, other._end);
        swap(_allocation, other._allocation);
        swap(_capacity, other._capacity);
    }
        
    bytes& operator=(bytes const&) = delete;

    bytes& operator=(bytes&& other) {
        bytes(std::move(other)).swap(*this);
        return *this;
    }
        
    bytes clone() const {
        auto n = size();
        auto p = (byte*) malloc(n);
        std::memcpy(p, _begin, n);
        return bytes(p, p + n, p, p + n);
    }
    
    void clone_from(bytes const& other) {
        if (capacity() < other.size()) {
            free(_capacity);
            _allocation = (byte*) malloc(other.size());
            _capacity = _allocation + other.size();
        }
        std::memcpy(_allocation, other._begin, other.size());
        _begin = _allocation;
        _end = _begin + other.size();
    }

    byte* may_write(usize n) {
        assert(n <= (_capacity - _end));
        return _end;
    }
    
    void did_write(usize n) {
        assert(n <= (_capacity - _end));
        _end += n;
    }
    
    byte* will_write(usize n) {
        auto ptr = may_write(n);
        did_write(n);
        return ptr;
    }
    
    template<typename T, std::enable_if_t<std::is_trivial_v<T>, int> = 0>
    bytes& write(T x) {
        std::memcpy(will_write(sizeof(T)), &x, sizeof(T));
        return *this;
    }

}; // struct bytes
*/

} // namespace manic

#endif /* bytes_hpp */
