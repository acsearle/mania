//
//  table2.hpp
//  mania
//
//  Created by Antony Searle on 24/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef table2_h
#define table2_h


#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <utility>

#include "bit.hpp"
#include "common.hpp"
#include "maybe.hpp"

namespace manic {
    
    // Hash table
    //
    // Open addressing, linear probing, Robin Hood, attempting to be cache-
    // friendly and low-overhead when small and empty.
    //
    // Table size is always a power of two, including zero.
    //
    // We use a single allocation with 32-bit hashes at the front and key-value
    // pairs at the back.
    //
    // We only store hashes as u32, so as the table size approaches the maximum
    // of 2^31 we will encounter more effective hash collisions requiring full
    // tests against keys.
    //
    // How many remote regions of memory do we need to visit to perform an
    // operation?  One jump to get into the allocation.  Then we move forward
    // a few elements.  We need to access the item, which may be remote or not.
    //
    //
    // Linear probing is cache-friendly; we will typically only move forward,
    // by a few items.  sizeof(V) will typically be only a few pointers.
    //
    // Therefore: table 3! [(hash, (key, value))]
    
    template<typename K, typename V>
    class table2 {
    
    public:
        
        using key_type = K;
        using value_type = V;
        using item_type = std::pair<K, V>;
        
        void* _allocation;
        u32 _capacity;
        u32 _occupants;
        
        u32 _table_hash(u64 _already_hashed) {
            u32 a = static_cast<u32>(_already_hashed);
            if (!a)
                a = ~a;
            return a;
        }
        
        u32 _mask() const { return _capacity - 1; }
        usize size() const { return _occupants; }
        usize capacity() const { return _capacity; }
        
        table2()
        : _allocation(nullptr)
        , _capacity(0)
        , _occupants(0) {
        }
        
        u32* _hashes_begin() const {
            return reinterpret_cast<u32*>(_allocation);
        }
        
        u32* _hashes_end() const {
            return _hashes_begin() + _capacity;
        }
        
        item_type* _items_begin() const {
            return reinterpret_cast<item_type*>(_hashes_end());
        }
        
        item_type* _items_end() const {
            return _items_begin() + _capacity;
        }
        
        u32& _hash_at(u32 h) const {
            return _hashes_begin()[h & _mask()];
        }
        
        item_type& _item_at(u32 h) const {
            return _items_begin()[h & _mask()];
        }
        
        key_type& _key_at(u32 h) const {
            return _item_at(h).first;
        }

        value_type& _value_at(u32 h) const {
            return _item_at(h).value;
        }

        
        u32 _displacement_at(u32 h) {
            return (h - _hash_at(h)) & _mask();
        }
        
        void _assert_invariant() const {
            assert(_allocation || !_capacity);
            assert(ispow2(_capacity));
            assert(!_capacity || (_occupants < _capacity));
            usize n = 0;
            for (usize i = 0; i != _capacity; ++i)
                if (_hash_at(i)) {
                    ++n;
                    assert(_hash_at(i) == _table_hash(hash(_item_at(i).first)));
                    if (_hash_at(i - 1))
                        assert(_displacement_at(i) <= (_displacement_at(i - 1) + 1));
                    else
                        assert(_displacement_at(i) == 0);
                }
            assert(_occupants == n);
        }

        
        
        usize _capacity_for_occupants(usize n) {
            // Capacity has the following properties
            // * For zero occupants, use zero
            // * For small numbers of occupants, enough to properly align the
            //   items part of the allocation
            // * For large numbers of occupants, no more than 2/3 load factor
            return n ? std::ceil2(std::max(n + (n >> 1), alignof(item_type) / sizeof(u32))) : 0;
        }
        
        table2(const table2& r)
        : table2(r._occupants) {
            item_type* q = r._items_begin();
            for (u32* p = r._hashes_begin(); p != r._hashes_end(); ++p, ++q)
                if (*p)
                    _insert(*p, *q);
        }
        
        table2(table2&& r) : table2() { swap(r); }
        
        explicit table2(isize n)
        : table2() {
            _capacity = _capacity_for_occupants(n);
            _allocation = std::malloc(_capacity * (sizeof(u32) + sizeof(item_type)));
            std::memset(_allocation, 0, _capacity * sizeof(u32));
        }
        
        void _destroy_all() {
            item_type* q = _items_begin();
            for (u32* p = _hashes_begin(); p != _hashes_end(); ++p, ++q)
                if (*p)
                    q->~item_type();
        }
        
        ~table2() {
            _destroy_all();
            free(_allocation);
        }
        
        void clear_and_reserve(isize n) {
            _destroy_all();
            _occupants = 0;
            n = _capacity_for_occupants(n);
            if (_capacity < n) {
                _capacity = n;
                free(_allocation);
                std::malloc(_capacity * (sizeof(u32) + sizeof(item_type)));
            }
            std::memset(_hashes_begin(), 0, sizeof(u32) * _capacity);
        }
        
        table2& operator=(const table2& r) {
            assert(*this != &r);
            if (this != &r) {
                clear_and_reserve(r._occupants);
                item_type* q = _items_begin();
                for (u32* p = _hashes_begin(); p != _hashes_end(); ++p, ++q)
                    if (*p)
                        _insert(*p, *q);
            }
            return *this;
        }
        
        table2& operator=(table2&& r) {
            assert(this != &r);
            if (this != &r) {
                table2(std::move(r)).swap(*this);
            }
            return *this;
        }
        
        void clear() {
            _destroy_all();
            _occupants = 0;
            std::memset(_allocation, 0, sizeof(u32) * _capacity);
        }
        
        void reserve(usize n) {
            n = _capacity_for_occupants(n);
            if (_capacity < n) {
                void* a = _allocation;
                u32* p = _hashes_begin();
                u32* p_end = _hashes_end();
                item_type* q = _items_begin();
                _capacity = n;
                _occupants = 0;
                _allocation = std::malloc(_capacity * (sizeof(u32) + sizeof(item_type)));
                std::memset(_allocation, 0, _capacity * sizeof(u32));
                for (; p != p_end; ++p, ++q)
                    if (*p)
                        _insert_relocate(*p, q);
                std::free(a);
            }
        }
        
        bool empty() const {
            return static_cast<bool>(_occupants);
        }
        
        bool contains(const key_type& k) const {
            u32 h = _table_hash(hash(k));
            u32 i = h;
            for (;;) {
                u32 g = _hash_at(i);
                if (!g)
                    return false;
                if ((g == h) && (k == _key_at(i)))
                    return true;
                ++i;
            }
        }
        
        const value_type& operator[](const key_type& k) const {
            u32 h = _table_hash(hash(k));
            u32 i = h;
            for (;;) {
                u32 g = _hash_at(i);
                if (!g)
                    assert(false);
                if ((g == h) && (k == _key_at(i)))
                    return _value_at(i);
                ++i;
            }
        }
        
        value_type& operator[](const key_type& k) {
            u32 h = _table_hash(hash(k));
            u32 i = h;
            for (;;) {
                u32 g = _hash_at(i);
                if (!g)
                    assert(false);
                if ((g == h) && (k == _key_at(i)))
                    return _value_at(i);
                ++i;
            }
        }
        

        
        
        void insert(const key_type& k, const value_type& v);
        
        void _insert(u32 h, const item_type& x);
        void _insert_relocate(u32 h, item_type&);
        
        void swap(table2& r) {
            using std::swap;
            swap(_allocation, r._allocation);
            swap(_capacity, r._capacity);
            swap(_occupants, r._occupants);
        }
    
    }; // class table2<K, V>
    
    template<typename K, typename V>
    void swap(table2<K, V>& a, table2<K, V>& b) {
        a.swap(b);
    }
    
    
};

#endif /* table2_h */
