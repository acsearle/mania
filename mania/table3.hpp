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
#include "debug.hpp"

namespace manic {

// table3 relies on manic::hash being high-quality; it is not defensive
// against bad hashes.  We do not prevent users from mutating keys, which
// violates the invariant.

//template<typename K, typename V>
//struct table3 {
//
//    struct entry { K key; V value; };
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
//    bool contains(K) const;
//    
//    V* try_get();
//    V const* try_get() const;
//
//    V& get();
//    V const& get() const;
//
//    V& operator[](K);
//    V const& operator[](K) const;
//
//    void insert(K, V);
//
//    void erase(K);
//
//    iterator begin();
//    iterator end();
//
//};



template<typename K, typename V>
struct table3 {
    
    using key_type = std::add_const_t<K>;
    using value_type = V;
    
    static constexpr u64 _KEY_BIT = u64(1) << 63;
    static constexpr u64 _VALUE_BIT = u64(1) << 62;
    static constexpr u64 _HASH_MASK = (u64(1) << 62) - 1;
    
    struct _entry_type {
        
        K _key;
        value_type _value;
        u64 _hash;
        
        void _clear() {
            if (_hash & _KEY_BIT) {
                _key.~K();
                if (_hash & _VALUE_BIT)
                    _value.~value_type();
                _hash = 0;
            }
        }
        
    };
    
    struct entry_type {
        
        key_type key;
        value_type value;
        
        u64& _hash() { return reinterpret_cast<_entry_type&>(*this)._hash; }
        
        template<typename F>
        entry_type& and_modify(F&& f) {
            assert(_hash() & _KEY_BIT);
            if (_hash() & _VALUE_BIT)
                std::forward<F>(f)(value);
            return *this;
        }
        
        template<typename X>
        value_type& and_insert(X&& v) {
            assert(_hash() & _KEY_BIT);
            if (_hash() & _VALUE_BIT)
                value = std::forward<X>(v);
            else {
                new (&value) value_type(std::forward<X>(v));
                _hash() |= _VALUE_BIT;
            }
            return value;
        }
        
        value_type& or_default() {
            assert(_hash() & _KEY_BIT);
            if (!(_hash() & _VALUE_BIT)) {
                new (&value) value_type;
            }
        }
        
        value_type& or_insert(value_type const& v) {
            assert(_hash() & _KEY_BIT);
            if (!(_hash() & _VALUE_BIT)) {
                new (&value) value_type(v);
                _hash() |= _VALUE_BIT;
            }
            return value;
        }
        
        value_type& or_insert(value_type&& v) {
            assert(_hash() & _KEY_BIT);
            if (!(_hash() & _VALUE_BIT)) {
                new (&value) value_type(std::move(v));
                _hash() |= _VALUE_BIT;
            }
            return value;
        }
        
        template<typename... Args>
        value_type& or_emplace(Args&&... args) {
            assert(_hash() & _KEY_BIT);
            if (!(_hash() & _VALUE_BIT)) {
                new (&value) value_type(std::forward<Args>(args)...);
                _hash() |= _VALUE_BIT;
            }
            return value;
        }
        
        template<typename F>
        value_type& or_insert_with(F&& f) {
            assert(_hash() & _KEY_BIT);
            if (!(_hash() & _VALUE_BIT)) {
                new (&value) value_type(std::forward<F>(f)());
                _hash() |= _VALUE_BIT;
            }
            return value;
        }
        
    }; // struct entry_type
    
    struct _is_occupied {
        bool operator()(_entry_type const& e) const {
            return e._hash & _KEY_BIT;
        }
    };
    
    using _iterator = filter_iterator<_entry_type*, _entry_type*, _is_occupied>;
    using _const_iterator = filter_iterator<_entry_type const*, _entry_type const*, _is_occupied>;
    
    struct _entry_cast {
        entry_type& operator()(_entry_type& e) const {
            return reinterpret_cast<entry_type&>(e);
        }
        entry_type const& operator()(_entry_type const& e) const {
            return reinterpret_cast<entry_type const&>(e);
        }
    };
    
    using iterator = transform_iterator<_iterator, _entry_cast>;
    using const_iterator = transform_iterator<_const_iterator, _entry_cast>;
    
    raw_vector<_entry_type> _vector;
    usize _occupants;
    
    u64 _mask() const { return _vector._capacity - 1; }
    
    _entry_type& _entry_at(u64 i) const {
        return _vector[i & _mask()];
    }
    
    u64& _hash_at(u64 i) const {
        return _entry_at(i)._hash;
    }
    
    bool _is_key_at(u64 i) const {
        return _hash_at(i) & _KEY_BIT;
    }
    
    K& _key_at(u64 i) const {
        return _entry_at(i)._key;
    }
    
    value_type& _value_at(u64 i) const {
        return _entry_at(i)._value;
    }
    
    u64 _displacement_at(u64 i) const {
        return (i - _hash_at(i)) & _mask();
    }
    
    u64 _displacement_of(u64 h, u64 i) const {
        return (i - h) & _mask();
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
            if (_hash_at(i) & _KEY_BIT) {
                assert(_hash_at(i) & _VALUE_BIT);
                ++n;
                assert((_hash_at(i) & _HASH_MASK) == (hash(_key_at(i)) & _HASH_MASK));
                if (_hash_at(i - 1)) {
                    assert(_displacement_at(i) <= (_displacement_at(i - 1) + 1));
                } else {
                    assert(_displacement_at(i) == 0);
                }
            }
        }
        assert(_occupants == n);
    }
    
    void _destroy_all() {
        for (_entry_type& e : _vector)
            e._clear();
    }
    
    table3()
    : _vector()
    , _occupants(0) {
    }
    
    void swap(table3& y) {
        using std::swap;
        swap(_vector, y._vector);
        swap(_occupants, y._occupants);
    }
    
    table3(const table3&) = delete;
    
    table3(table3&& y)
    : table3() {
        this->swap(y);
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
    
    void clear() {
        _destroy_all();
        _occupants = 0;
    }
    
    void clear_and_reserve(usize n) {
        n = _capacity_for_occupants(n);
        if (n <= _vector._capacity) {
            clear();
        } else {
            _destroy_all();
            _occupants = 0;
            _vector = raw_vector<_entry_type>(n);
        }
    }
    
    void reserve(usize n) {
        n = _capacity_for_occupants(n);
        if (n > _vector._capacity) {
            raw_vector<_entry_type> v(n);
            v.swap(_vector);
            _occupants = 0;
            for (_entry_type& src : v)
                if (src._hash & _KEY_BIT) {
                    _entry_type& dest = _entry_unsafe(src._key, src._hash);
                    assert(!(dest._hash & _KEY_BIT));
                    std::memcpy(&dest, &src, sizeof(_entry_type));
                }
        }
    }
    
    void shrink_to_fit() {
        usize n = _capacity_for_occupants(_occupants);
        if (n < _vector._capacity) {
            raw_vector<_entry_type> v(n);
            v.swap(_vector);
            _occupants = 0;
            for (_entry_type& src : v)
                if (src._hash & _KEY_BIT) {
                    _entry_type& dest = _entry_unsafe(src._key, src._hash);
                    assert(!(dest._hash & _KEY_BIT));
                    std::memcpy(&dest, &src, sizeof(_entry_type));
                }
        }
    }
    
    
    template<typename Q>
    bool contains(Q&& k) {
        return contains(std::forward<Q>(k), hash(k));
    }
    
    template<typename Q>
    bool contains(Q&& k, u64 h) {
        if (!_occupants)
            return false;
        h |= (_KEY_BIT | _VALUE_BIT);
        u64 i = h;
        for (;;) {
            if ((_hash_at(i) == h) && (_key_at(i) == k))
                return true;
            if (!_is_key_at(i) || (_displacement_at(i) < _displacement_of(h, i)))
                return false;
            ++i;
        }

    }

    template<typename Q>
    value_type* try_get(Q&& k, u64 h) {
        if (!_occupants)
            return nullptr;
        h |= (_KEY_BIT | _VALUE_BIT);
        u64 i = h;
        for (;;) {
            if ((_hash_at(i) == h) && (_key_at(i) == k))
                return &_value_at(i);
            if (!_is_key_at(i) || (_displacement_at(i) < _displacement_of(h, i)))
                return nullptr;
            ++i;
        }
    }
    
    template<typename Q>
    value_type* try_get(Q&& k) {
        return try_get(std::forward<Q>(k), hash(k));
    }

    // preconditions: contains(k), h == hash(k)
    template<typename Q>
    value_type& _get_unsafe(Q&& k, u64 h) const {
        assert(_occupants);
        h |= (_KEY_BIT | _VALUE_BIT);
        u64 i = h;
        while ((_hash_at(i) != h) || (_key_at(i) != k)) {
            ++i;
        }
        return _value_at(i);
    }

    // preconditions: contains(k), h == hash(k)
    template<typename Q>
    value_type const& get(Q&& k, u64 h) const {
        return _get_unsafe(std::forward<Q>(k), h);
    }

    // preconditions: contains(k)
    template<typename Q>
    value_type& get(Q&& k) {
        return const_cast<value_type&>(_get_unsafe(std::forward<Q>(k), hash(k)));
    }

    template<typename Q>
    value_type const& get(Q&& k) const {
        return get(std::forward<Q>(k), hash(k));
    }

    // precondition: contains(k)
    template<typename Q>
    value_type& operator[](Q&& k) {
        return get(std::forward<Q>(k));
    }
    
    // precondition: contains(k)
    template<typename Q>
    value_type const& operator[](Q&& k) const {
        return get(std::forward<Q>(k));
    }
    
    
    template<typename Q>
    void erase(Q&& k, u64 h) {
        if (!_occupants)
            return;
        h |= (_KEY_BIT | _VALUE_BIT);
        u64 i = h;
        for (;;) {
            if (((_hash_at(i) | _VALUE_BIT) == h) && (_key_at(i) == k)) {
                _key_at(i).~K();
                if (_hash_at(i) & _VALUE_BIT)
                    _value_at(i).~value_type();
                while (_is_key_at(i + 1) && _displacement_at(i + 1)) {
                    std::memcpy(&_entry_at(i), &_entry_at(i + 1), sizeof(_entry_type));
                    ++i;
                }
                _hash_at(i) = 0;
                --_occupants;
                return;
            }
            if (!_is_key_at(i) || (_displacement_at(i) < _displacement_of(h, i)))
                return;
            ++i;
        }
    }
    
    template<typename Q>
    void erase(Q&& k) {
        erase(std::forward<Q>(k), hash(k));
    }

    // postcondition: the hash map invariant has potentially been violated and
    // if the entry is unoccupied a valid entry *must* be constructed in it
    template<typename Q>
    _entry_type& _entry_unsafe(Q&& k, u64 h) {
        reserve(_occupants + 1);
        h |= (_KEY_BIT | _VALUE_BIT);
        u64 i = h;
        for (;;) {
            if ((_hash_at(i) == h) && (_key_at(i) == k))
                return _entry_at(i);
            if (!_is_key_at(i)) {
                ++_occupants;
                return _entry_at(i);
            }
            if (_is_key_at(i) && (_displacement_at(i) < _displacement_of(h, i))) {
                // rob from the rich and give to the poor
                // find next free slot
                u64 j = i;
                do ++j; while (_is_key_at(j));
                // move entries back one
                while (j != i) {
                    std::memcpy(&_entry_at(j), &_entry_at(j - 1), sizeof(_entry_type));
                    --j;
                }
                // blank the slot
                _hash_at(i) = 0;
                // we have inserted a blank slot where k will go, but we have
                // not put it there yet; the invariant of the hash table is
                // broken and must be repaired by setting _hash and constructing
                // key and value
                ++_occupants;
                return _entry_at(i);
            }
            ++i;
        }
    }
    
    template<typename Q>
    entry_type& entry(Q&& k, u64 h) {
        _entry_type& e = _entry_unsafe(k, h);
        if (!(e._hash & _KEY_BIT)) {
            e._hash = (h | _KEY_BIT) & ~_VALUE_BIT;
            new (&e._key) K(std::forward<Q>(k));
        }
        return reinterpret_cast<entry_type&>(e);
    }
    
    // postcondition: the invariant may be broken and to restore it we *must*
    // call one of .or_insert, .or_emplace, .or_insert_with on the entry before
    // performing any other operations
    template<typename Q>
    entry_type& entry(Q&& k) {
        return entry(std::forward<Q>(k), hash(k));
    }
        
    template<typename Q, typename X>
    void insert(Q&& k, u64 h, X&& value) {
        entry(std::forward<Q>(k), h).and_insert(std::forward<X>(value));
    }

    // add or update the entry for k.  if a key comparing equal to k is already
    // present, the old key is not updated
    template<typename Q, typename X>
    void insert(Q&& k, X&& value) {
        insert(std::forward<Q>(k), hash(k), std::forward<X>(value));
    }

    table3<u64, u64> _histogram() const {
        table3<u64, u64> x;
        for (u64 i = 0; i != _vector._capacity; ++i)
            if (_is_key_at(i))
                ++x.entry(_displacement_at(i)).or_insert(0);
        return x;
    }
    
    iterator begin() {
        return iterator(_iterator(_vector.begin(), _vector.end()));
    }
    
    iterator end() {
        return iterator(_iterator(_vector.end(), _vector.end()));
    }
    
    const_iterator begin() const {
        return const_iterator(_const_iterator(_vector.begin(), _vector.end()));
    }
    
    const_iterator end() const {
        return const_iterator(_const_iterator(_vector.end(), _vector.end()));
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
