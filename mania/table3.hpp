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
#include "maybe.hpp"
#include "raw_vector.hpp"

#include "filter_iterator.hpp"
#include "transform_iterator.hpp"

namespace manic {
    
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
            
            ~entry() = default;
            
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
                assert(_hash == _table_hash(hash(k)));
            }
            
            // we choose not to zero ._hash in the destructor, which would be a
            // pessimization for some paths
            ~_raw_entry() = default;
            
        };
        
        struct _raw_entry_predicate {
            bool operator()(const _raw_entry& r) const {
                return static_cast<bool>(r._hash);
            }
        };
        
        using _raw_entry_iterator = filter_iterator<_raw_entry*, _raw_entry_predicate>;
        using _const_raw_entry_iterator = filter_iterator<const _raw_entry*, _raw_entry_predicate>;
        
        struct _raw_entry_select {
            entry& operator()(_raw_entry& r) const {
                return r._entry;
            }
            const entry& operator()(const _raw_entry& r) const {
                return r._entry;
            }
        };
        
        using iterator = transform_iterator<_raw_entry_iterator, _raw_entry_select>;
        using const_iterator = transform_iterator<_const_raw_entry_iterator, _raw_entry_select>;
        
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

        
        
        raw_vector<_raw_entry> _vector;
        usize _occupants;
        
        usize capacity() { return _vector._capacity; }
        usize size() { return _occupants; }
        
        u64 _mask() const { return _vector._capacity - 1; }
        
        static u64 _table_hash(u64 already_hashed) {
            if (!already_hashed)
                already_hashed = ~already_hashed;
            return already_hashed;
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
        
        void _assert_invariant() const {
            assert(std::ispow2(_vector._capacity));
            assert(!_vector._capacity || (_occupants < _vector._capacity));
            usize n = 0;
            for (usize i = 0; i != _vector._capacity; ++i) {
                std::cout << i << " : " << (_hash_at(i) & _mask()) << std::endl;
                if (_hash_at(i)) {
                    ++n;
                    assert(_hash_at(i) == _table_hash(hash(_key_at(i))));
                    if (_hash_at(i - 1))
                        assert(_displacement_at(i) <= (_displacement_at(i - 1) + 1));
                    else
                        assert(_displacement_at(i) == 0);
                }
            }
            assert(_occupants == n);
        }

        usize _capacity_for_occupants(usize n) {
            return n ? std::ceil2(n + (n >> 1) + 1) : 0;
        }
        
        table3()
        : _vector()
        , _occupants(0) {
        }
        
        table3(const table3&);
        
        table3(table3&& r)
        : table3() {
            r.swap(*this);
        }
        
        void _zero_all() {
            // would block memset be better?
            for (_raw_entry& e : _vector)
                e._hash = 0;
        }
        
        explicit table3(usize n)
        : _vector(_capacity_for_occupants(n))
        , _occupants(0) {
            _zero_all();
        }

        
        void _destroy_all() {
            for (_raw_entry& e : _vector)
                if (e._hash)
                    e.~_raw_entry();
        }
        
        ~table3() {
            _destroy_all();
        }
        
        table3& operator=(const table3& r) {
            // todo: prefer to use existing allocation
            table3(r).swap(*this);
            return *this;
        }
        
        table3& operator=(table3&& r) {
            table3(std::move(r)).swap(*this);
            return *this;
        }

        void swap(table3& r) {
            using std::swap;
            swap(_vector, r._vector);
            swap(_occupants, r._occupants);
        }

        void clear() {
            for (_raw_entry& e : _vector)
                if (e._hash) {
                    e.~_raw_entry();
                    e = 0; // would block memset be better?
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
                raw_vector<_raw_entry>(n).swap(_vector);
                _zero_all();
            }
        }
        
        void reserve(usize n) {
            n = _capacity_for_occupants(n);
            if (n > _vector._capacity) {
                raw_vector<_raw_entry> v(n);
                v.swap(_vector);
                _zero_all();
                _occupants = 0;
                for (auto& e : v)
                    if (e._hash)
                        _insert_relocate(e);
            }
        }
        
        template<typename Keylike>
        bool contains(const Keylike& k) {
            u64 h = _table_hash(hash(k));
            u64 i = h;
            for (;;) {
                if (_hash_at(i) == 0) {
                    return false;
                } else if ((_hash_at(i) == h) && (_key_at(i) == k)) {
                    return true;
                } else if (_displacement_at(i) < (i - h)) {
                    return false;
                }
                ++i;
            }
        }
        
        template<typename Keylike>
        value_type& get(const Keylike& k) {
            _assert_invariant();
            u64 h = _table_hash(hash(k));
            u64 i = h;
            for (;;) {
                if (_hash_at(i) == 0)
                    assert(false);
                if (_displacement_at(i) < (i - h))
                    assert(false);
                if ((_hash_at(i) == h) && (_key_at(i) == k))
                    return _value_at(i);
                ++i;
            }
        }

        _raw_entry& _insert_relocate(_raw_entry& e) {
            assert(e._hash == _table_hash(hash(e._entry.key)));
            u64 i = e._hash;
            for (;;) {
                if (_hash_at(i) == 0) {
                    std::memcpy(&_raw_entry_at(i), &e, sizeof(_raw_entry));
                    ++_occupants;
                    return _raw_entry_at(i);
                } else if ((_hash_at(i) == e._hash) && (_key_at(i) == e._entry.key)) {
                    using std::swap;
                    _raw_entry_at(i).~_raw_entry();
                    std::memcpy(&_raw_entry_at(i), &e, sizeof(_raw_entry));
                    return _raw_entry_at(i);
                } else if (_displacement_at(i) < (i - e._hash)) {
                    using std::swap;
                    swap(e, _raw_entry_at(i));
                }
                ++i;
            }
        }
        
        template<typename Keylike, typename Valuelike>
        entry& insert(Keylike&& k, Valuelike&& v) {
            reserve(_occupants + 1);
            maybe<_raw_entry> e;
            e.emplace(std::forward<Keylike>(k), std::forward<Valuelike>(v));
            return _insert_relocate(e.get())._entry;
            // e is not destroyed because it has been relocated
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
        
        struct _function {
            
            template<typename R>
            decltype(auto) operator()(R&& r) const {
                return std::forward<R>(r).key;
            }
            
        };
        
        auto begin() {
            return transform_iterator(_perfect_capture.begin(), _function());
        }
        
        auto end() {
            return transform_iterator(_perfect_capture.end(), _function());
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
        
        struct _function {
            
            template<typename R>
            decltype(auto) operator()(R&& r) const {
                return std::forward<R>(r).value;
            }
            
        };
        
        auto begin() {
            return transform_iterator(_perfect_capture.begin(), _function());
        }
        
        auto end() {
            return transform_iterator(_perfect_capture.end(), _function());
        }
        
    };
    
    template<typename T>
    _value_range<T> values(T&& r) {
        return _value_range<T>(std::forward<T>(r));
    }
    
    
} // namespace manic

#endif /* table3_hpp */
