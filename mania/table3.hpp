//
//  table3.hpp
//  mania
//
//  Created by Antony Searle on 24/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef table3_hpp
#define table3_hpp

#include "bit.hpp" // <bit>
#include <iostream>
#include <utility>

#include "common.hpp"
#include "filter_iterator.hpp"
#include "hash.hpp"
#include "maybe.hpp"
#include "raw_vector.hpp"
#include "relocate.hpp"
#include "string.hpp"
#include "transform_iterator.hpp"

namespace manic {

// table3 relies on manic::hash being high-quality; it is not defensive
// against bad hashes.  We do not prevent users from mutating keys, which
// violates the invariant.

//template<typename Key, typename Value>
//struct table3 {
//
//    struct entry { Key key; Value value; };
//
//    explicit table3(usize n); // set capacity
//
//    usize capacity() const;
//    usize size() const;
//    bool empty() const;
//
//    void clear();
//    void reserve(usize n);
//    void clear_and_reserve(usize n);
//    void shrink_to_fit();
//
//    bool contains(Key) const;
//    
//    Value* try_get();
//    Value const* try_get() const;
//
//    Value& get();
//    Value const& get() const;
//
//    Value& operator[](Key);
//    Value const& operator[](Key) const;
//
//    void insert(Key, Value);
//
//    void erase(Key);
//
//    iterator begin();
//    iterator end();
//
//};
 


template<typename Key, typename Value>
struct table3 {
    
    using key_type = Key;
    using value_type = Value;
    
    struct entry {
        
        key_type key;
        value_type value;
        
        template<typename Keylike, typename Valuelike>
        entry(Keylike&& k, Valuelike&& v)
        : key(std::forward<Keylike>(k))
        , value(std::forward<Valuelike>(v)) {
        }
        
    };
    
    struct _raw_entry {
        
        u64 _hash;
        entry _entry;
        
        template<typename Keylike, typename Valuelike>
        _raw_entry(Keylike&& k, Valuelike&& v)
        : _raw_entry(_table_hash(hash(k)),
        std::forward<Keylike>(k),
                     std::forward<Valuelike>(v)) {
        }
        
        template<typename Keylike, typename Valuelike>
        _raw_entry(u64 table_hash_hash_k, Keylike&& k, Valuelike&& v)
        : _hash(table_hash_hash_k)
        , _entry(std::forward<Keylike>(k),
        std::forward<Valuelike>(v)) {
            // assert(_hash == _table_hash(hash(k)));
        }
        
    };
    
    struct _is_occupied {
        bool operator()(const _raw_entry& r) const {
            return static_cast<bool>(r._hash);
        }
    };
    
    using _raw_entry_iterator = filter_iterator<_raw_entry*, _raw_entry*, _is_occupied>;
    using _const_raw_entry_iterator = filter_iterator<const _raw_entry*, const _raw_entry*, _is_occupied>;
    
    struct _dot_entry {
        entry& operator()(_raw_entry& r) const {
            return r._entry;
        }
        const entry& operator()(const _raw_entry& r) const {
            return r._entry;
        }
    };
    
    using iterator = transform_iterator<_raw_entry_iterator, _dot_entry>;
    using const_iterator = transform_iterator<_const_raw_entry_iterator, _dot_entry>;
    
    
    raw_vector<_raw_entry> _vector;
    usize _occupants;
        
    u64 _mask() const { return _vector._capacity - 1; }
    
    static const u64 HIGH_BIT = u64(1) << 63;
    
    static u64 _table_hash(u64 already_hashed) {
        // We need a cheap way to rule out the hash ever being zero.  The
        // high bit will never contribute to the index, and merely doubles
        // the negligible rate of hash collisions.
        return already_hashed | HIGH_BIT;
    }
    
    _raw_entry& _raw_entry_at(u64 h) const {
        return _vector[h & _mask()];
    }
    
    u64 _hash_at(u64 h) const {
        return _raw_entry_at(h)._hash;
    }
    
    entry& _entry_at(u64 h) const {
        return _raw_entry_at(h)._entry;
    }
    
    key_type& _key_at(u64 h) const {
        return _entry_at(h).key;
    }
    
    value_type& _value_at(u64 h) const {
        return _entry_at(h).value;
    }
    
    u64 _displacement_at(u64 h) const {
        return (h - _hash_at(h)) & _mask();
    }
    
    usize _capacity_for_occupants(usize n) {
        // table resizes when 2/3 full.  tuning point.
        return n ? std::ceil2(n + (n >> 1) + 1) : 0;
    }
    
    void _assert_invariant() const {
        assert(std::ispow2(_vector._capacity));
        assert(!_vector._capacity || (_occupants < _vector._capacity));
        usize n = 0;
        for (usize i = 0; i != _vector._capacity; ++i) {
            if (_hash_at(i)) {
                ++n;
                assert(_hash_at(i) == _table_hash(hash(_key_at(i))));
                if (_hash_at(i - 1)) {
                    assert(_displacement_at(i) <= (_displacement_at(i - 1) + 1));
                } else {
                    assert(_displacement_at(i) == 0);
                }
            }
        }
        assert(_occupants == n);
    }
    
    _raw_entry& _insert_relocate(_raw_entry& e) {
        u64 i = e._hash;
        for (;;) {
            if (_hash_at(i) == 0) {
                _raw_entry* p = &_raw_entry_at(i);
                relocate(p, &e);
                ++_occupants;
                return *p;
            } else if ((_hash_at(i) == e._hash) && (_key_at(i) == e._entry.key)) {
                _raw_entry_at(i).~_raw_entry();
                relocate(&_raw_entry_at(i), &e);
                return _raw_entry_at(i);
            } else if (_displacement_at(i) < ((i - e._hash) & _mask())) {
                using std::swap;
                swap(e, _raw_entry_at(i));
            }
            ++i;
        }
    }
    
    void _destroy_all() {
        for (_raw_entry& e : _vector) {
            if (e._hash)
                e.~_raw_entry();
        }
    }
    
    
    table3()
    : _vector()
    , _occupants(0) {
    }
    
    table3(const table3&) = delete;
    
    table3(table3&& r)
    : table3() {
        r.swap(*this);
    }
    
    explicit table3(usize n)
    : _vector(_capacity_for_occupants(n))
    , _occupants(0) {
    }
    
    ~table3() {
        _destroy_all();
    }
    
    table3& operator=(const table3& r) = delete;
    
    table3& operator=(table3&& r) {
        table3(std::move(r)).swap(*this);
        return *this;
    }
    
    usize capacity() {
        constexpr usize a = 0x5555555555555555;
        return _vector._capacity ? std::max(a & _mask(), ~a & _mask()) : 0;
        
    }
    usize size() const { return _occupants; }
    bool empty() const { return !_occupants; }
    
    void swap(table3& r) {
        using std::swap;
        swap(_vector, r._vector);
        swap(_occupants, r._occupants);
    }
    
    void clear() {
        for (_raw_entry& e : _vector)
            if (e._hash) {
                e.~_raw_entry();
                e._hash = 0; // would block memset be better?
            }
        _occupants = 0;
    }
    
    void clear_and_reserve(usize n) {
        n = _capacity_for_occupants(n);
        if (n <= _vector._capacity) {
            clear();
        } else {
            _destroy_all();
            _occupants = 0;
            _vector = raw_vector<_raw_entry>(n);
        }
    }
    
    void reserve(usize n) {
        n = _capacity_for_occupants(n);
        if (n > _vector._capacity) {
            raw_vector<_raw_entry> v(n);
            v.swap(_vector);
            _occupants = 0;
            for (auto& e : v) {
                if (e._hash)
                    _insert_relocate(e);
            }
        }
    }
    
    void shrink_to_fit() {
        usize n = _capacity_for_occupants(_occupants);
        if (n < _vector._capacity) {
            raw_vector<_raw_entry> v(n);
            v.swap(_vector);
            _occupants = 0;
            for (auto& e : v) {
                if (e._hash)
                    _insert_relocate(e);
            }
        }
    }
    
    
    template<typename Keylike>
    bool contains(const Keylike& k) const {
        if (!_occupants)
            return false;
        const u64 h = _table_hash(hash(k));
        u64 i = h;
        for (;;) {
            if ((_hash_at(i) == h) && (_key_at(i) == k))
                return true;
            if ((_hash_at(i) == 0) || (_displacement_at(i) < ((i - h) & _mask())))
                return false;
            ++i;
        }
    }
    
    template<typename Keylike>
    value_type* try_get(const Keylike& k) const {
        if (!_occupants)
            return nullptr;
        const u64 h = _table_hash(hash(k));
        u64 i = h;
        for (;;) {
            if ((_hash_at(i) == h) && (_key_at(i) == k))
                return &_value_at(i);
            else if ((_hash_at(i) == 0) || (_displacement_at(i) < ((i - h) & _mask())))
                return nullptr;
            ++i;
        }
    }
    
    // precondition: contains(k)
    template<typename Keylike>
    value_type& get(const Keylike& k) const {
        assert(_occupants);
        const u64 h = _table_hash(hash(k));
        u64 i = h;
        while ((_hash_at(i) != h) || (_key_at(i) != k)) {
            //assert(!((_hash_at(i) == 0) || (_displacement_at(i) < ((i - h) & _mask()))));
            ++i;
        }
        return _value_at(i);
    }
    
    // precondition: contains(k)
    template<typename Keylike>
    value_type& operator[](const Keylike& k) {
        return get(k);
    }
    
    // precondition: contains(k)
    template<typename Keylike>
    const value_type& operator[](const Keylike& k) const {
        return get(k);
    }
    
    template<typename Keylike, typename Valuelike>
    entry& insert(Keylike&& k, Valuelike&& v) {
        reserve(_occupants + 1);
        maybe<_raw_entry> e;
        e.emplace(std::forward<Keylike>(k), std::forward<Valuelike>(v));
        return _insert_relocate(*e)._entry;
    }
        
    template<typename Keylike>
    void erase(Keylike&& k) {
        const u64 h = _table_hash(hash(k));
        u64 i = h;
        for (;;) {
            if ((_hash_at(i) == h) && (_key_at(i) == k)) {
                _raw_entry_at(i).~_raw_entry();
                while (_hash_at(i + 1) && _displacement_at(i + 1)) {
                    relocate(&_raw_entry_at(i), &_raw_entry_at(i + 1));
                    ++i;
                }
                _raw_entry_at(i)._hash = 0;
                --_occupants;
                return;
            } else if ((_hash_at(i) == 0) || (_displacement_at(i) < ((i - h) & _mask()))) {
                return;
            }
            ++i;
        }
    }
    
    template<typename Keylike, typename Valuelike>
    value_type& get_or_insert(Keylike&& k, Valuelike&& v) {
        reserve(_occupants + 1);
        const u64 h = _table_hash(hash(k));
        u64 i = h;
        for (;;) {
            if (_hash_at(i) == 0) {
                new (&_raw_entry_at(i)) _raw_entry(h, std::forward<Keylike>(k), std::forward<Valuelike>(v));
                ++_occupants;
                return _value_at(i);
            } else if (_hash_at(i) && _key_at(i) == k) {
                return _value_at(i);
            } else if (_displacement_at(i) < ((i - h) & _mask())) {
                // We are a better fit for this slot, kick everybody until the
                // next free slot back one place
                u64 j = i;
                do {
                    ++j;
                } while (_hash_at(j));
                // This is almost always a block memcpy except when j has
                // wrapped across the boundary; do it piecewise to handle
                // this case
                while (j != i) {
                    memcpy(_raw_entry_at(j), _raw_entry_at(j - 1), sizeof(_raw_entry));
                    --j;
                }
                new (&_raw_entry_at(i)) _raw_entry(h, std::forward<Keylike>(k), std::forward<Valuelike>(v));
                ++_occupants;
                return _value_at(i);
            }
            ++i;
        }
        
    }
    
    template<typename Keylike, typename F>
    value_type& get_or_insert_with(Keylike&& k, F&& f) {
        reserve(_occupants + 1);
        const u64 h = _table_hash(hash(k));
        u64 i = h;
        for (;;) {
            if (_hash_at(i) == 0) {
                new (&_raw_entry_at(i)) _raw_entry(h, std::forward<Keylike>(k), std::forward<F>(f)());
                ++_occupants;
                return _value_at(i);
            } else if (_hash_at(i) && _key_at(i) == k) {
                return _value_at(i);
            } else if (_displacement_at(i) < ((i - h) & _mask())) {
                // We are a better fit for this slot, kick everybody until the
                // next free slot back one place
                u64 j = i;
                do {
                    ++j;
                } while (_hash_at(j));
                // This is almost always a block memcpy except when j has
                // wrapped across the boundary; do it piecewise to handle
                // this case
                while (j != i) {
                    memcpy(&_raw_entry_at(j), &_raw_entry_at(j - 1), sizeof(_raw_entry));
                    --j;
                }
                new (&_raw_entry_at(i)) _raw_entry(h, std::forward<Keylike>(k), std::forward<F>(f)());
                ++_occupants;
                return _value_at(i);
            }
            ++i;
        }
        
    }
    
    table3<u64, u64> _histogram() const {
        table3<u64, u64> x;
        for (u64 i = 0; i != _vector._capacity; ++i) {
            if (_hash_at(i)) {
                u64 j = _displacement_at(i);
                if (x.contains(j))
                    ++x.get(j);
                else
                    x.insert(j, 1);
            }
        }
        return x;
    }
    
    iterator begin() {
        return iterator(_raw_entry_iterator(_vector.begin(), _vector.end()));
    }
    
    iterator end() {
        return iterator(_raw_entry_iterator(_vector.end(), _vector.end()));
    }
    
    const_iterator begin() const {
        return const_iterator(_const_raw_entry_iterator(_vector.begin(), _vector.end()));
    }
    
    const_iterator end() const {
        return const_iterator(_const_raw_entry_iterator(_vector.end(), _vector.end()));
    }
    
    const_iterator cbegin() const {
        return begin();
    }
    
    const_iterator cend() const {
        return end();
    }
    
    }; // class table3
    
    template<typename K, typename V>
    void swap(table3<K, V>& a, table3<K, V>& b) {
        a.swap(b);
    }
    
    template<typename T>
    class _key_range {
        
        T _perfect_capture;
        
    public:
        
        _key_range(T&& r)
        : _perfect_capture(r) {
        }
        
        struct _dot_key {
            
            template<typename R>
            decltype(auto) operator()(R&& r) const {
                return std::forward<R>(r).key;
            }
            
        };
        
        auto begin() {
            return transform_iterator(_perfect_capture.begin(), _dot_key());
        }
        
        auto end() {
            return transform_iterator(_perfect_capture.end(), _dot_key());
        }
        
    };
    
    template<typename T>
    _key_range<T> keys(T&& r) {
        return _key_range<T>(std::forward<T>(r));
    }
    
    template<typename T>
    class _value_range {
        
        T _perfect_capture;
        
    public:
        
        _value_range(T&& r)
        : _perfect_capture(r) {
        }
        
        struct _dot_value {
            
            template<typename R>
            decltype(auto) operator()(R&& r) const {
                return std::forward<R>(r).value;
            }
            
        };
        
        auto begin() {
            return transform_iterator(_perfect_capture.begin(), _dot_value());
        }
        
        auto end() {
            return transform_iterator(_perfect_capture.end(), _dot_value());
        }
        
    };
    
    template<typename T>
    _value_range<T> values(T&& r) {
        return _value_range<T>(std::forward<T>(r));
    }
    
    
    
    
    
} // namespace manic

#endif /* table3_hpp */
