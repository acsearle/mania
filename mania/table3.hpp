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
#include "serialize.hpp"

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
    
    static const u64 HASH_BIT = 0x8000'0000'0000'0000;
    
    using key_type = std::add_const_t<K>;
    using value_type = V;
    
    struct _entry_type {
        
        u64 _hash;
        struct et {
            K _key;
            value_type _value;
            
            template<typename Q, typename U>
            et(Q&& k, U&& v)
            : _key(std::forward<Q>(k))
            , _value(std::forward<U>(v)) {
            }
            
            template<typename Q>
            explicit et(Q&& k)
            : _key(std::forward<Q>(k))
            , _value() {
            }

        } _p;
        
        template<typename Q, typename U>
        _entry_type(u64 h, Q&& k, U&& v)
        : _hash{h}
        , _p(std::forward<Q>(k),
             std::forward<U>(v)) {
        }
        
        template<typename Q>
        _entry_type(u64 h, Q&& k)
        : _hash{h}
        , _p(std::forward<Q>(k)) {
        }

        
    };

    struct entry_type {
        key_type key;
        value_type value;
    };
    
    struct _is_occupied {
        bool operator()(_entry_type const& e) const {
            return e._hash;
        }
    };
    
    using _iterator = filter_iterator<_entry_type*, _entry_type*, _is_occupied>;
    using _const_iterator = filter_iterator<_entry_type const*, _entry_type const*, _is_occupied>;
    
    struct _entry_cast {
        entry_type& operator()(_entry_type& e) const {
            return reinterpret_cast<entry_type&>(e._p._key);
        }
        entry_type const& operator()(_entry_type const& e) const {
            return reinterpret_cast<entry_type const&>(e._p._key);
        }
    };
    
    using iterator = transform_iterator<_iterator, _entry_cast>;
    using const_iterator = transform_iterator<_const_iterator, _entry_cast>;
    
    raw_vector<_entry_type> _vector;
    usize _occupants;
    
    u64 _mask() const {
        return _vector._capacity - 1;
    }
    
    _entry_type& _entry_at(u64 i) const {
        return _vector[i & _mask()];
    }
    
    u64& _hash_at(u64 i) const {
        return _entry_at(i)._hash;
    }
    
    bool _occupied_at(u64 i) const {
        return _hash_at(i);
    }
    
    K& _key_at(u64 i) const {
        return _entry_at(i)._p._key;
    }
    
    value_type& _value_at(u64 i) const {
        return _entry_at(i)._p._value;
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
    
    template<typename Q>
    static u64 _table_hash(Q&& k) {
        return hash(std::forward<Q>(k)) | HASH_BIT;
    }
    
    void _assert_invariant() const {
        assert(std::ispow2(_vector._capacity));
        assert(!_vector._capacity || (_occupants < _vector._capacity));
        usize n = 0;
        for (usize i = 0; i != _vector._capacity; ++i) {
            if (_hash_at(i)) {
                ++n;
                assert((_hash_at(i)) == _table_hash(_key_at(i)));
                if (_hash_at(i - 1)) {
                    assert(_displacement_at(i) <= (_displacement_at(i - 1) + 1));
                } else {
                    assert(_displacement_at(i) == 0);
                }
            }
        }
        assert(_occupants == n);
    }
    
    void _destroy_one(_entry_type& e) {
        if (e._hash) {
            e._p.~et();
            e._hash = 0;
        }
    }
    
    void _destroy_all() {
        for (_entry_type& e : _vector)
            _destroy_one(e);
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

    // Rob from the rich: move all entries back one slot until a free slot is
    // found
    void _rob(u64 i) {
        u64 j = i;
        while (_hash_at(++j))
            ;
        do {
            std::memcpy(&_entry_at(j),
                        &_entry_at(j - 1),
                        sizeof(_entry_type));
        } while (--j != i);
        // _entry_at(i) is now uninitialized garbage
    }
    
    _entry_type& _prepare_insert(u64 h) {
        // return the location to insert an element that is not present
        assert(h & HASH_BIT);
        for (u64 i = h;; ++i) {
            if (!_hash_at(i)) {
                return _entry_at(i);
            } else if (_displacement_at(i) < _displacement_of(h, i)) {
                _rob(i);
                return _entry_at(i);
            }
            
        }
        
    }
    
    void reserve(usize n) {
        n = _capacity_for_occupants(n);
        if (n > _vector._capacity) {
            raw_vector<_entry_type> v(n);
            v.swap(_vector);
            for (_entry_type& src : v)
                if (src._hash) {
                    std::memcpy(&_prepare_insert(src._hash),
                                &src,
                                sizeof(_entry_type));
                }
        }
    }
    
    void shrink_to_fit() {
        usize n = _capacity_for_occupants(_occupants);
        if (n < _vector._capacity) {
            raw_vector<_entry_type> v(n);
            v.swap(_vector);
            for (_entry_type& src : v)
                if (src._hash) {
                    std::memcpy(&_prepare_insert(src._hash),
                                &src,
                                sizeof(_entry_type));
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
        h |= HASH_BIT;
        for (u64 i = h;; ++i) {
            if ((_hash_at(i) == h) && (_key_at(i) == k))
                return true;
            if (!_hash_at(i) || (_displacement_at(i) < _displacement_of(h, i)))
                return false;
        }

    }

    template<typename Q>
    value_type* try_get(Q&& k, u64 h) {
        if (!_occupants)
            return nullptr;
        h |= HASH_BIT;
        for (u64 i = h;; ++i) {
            if ((_hash_at(i) == h) && (_key_at(i) == k))
                return &_value_at(i);
            if (!_hash_at(i) || (_displacement_at(i) < _displacement_of(h, i)))
                return nullptr;
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
        h |= HASH_BIT;
        u64 i = h;
        while ((_hash_at(i) != h) || (_key_at(i) != k)) {
            assert(_hash_at(i) && (_displacement_at(i) >= _displacement_of(h, i)));
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
        h |= HASH_BIT;
        u64 i = h;
        for (;;) {
            if ((_hash_at(i) == h) && (_key_at(i) == k)) {
                _key_at(i).~K();
                _value_at(i).~value_type();
                while (_hash_at(i + 1) && _displacement_at(i + 1)) {
                    std::memcpy(&_entry_at(i), &_entry_at(i + 1), sizeof(_entry_type));
                    ++i;
                }
                _hash_at(i) = 0;
                --_occupants;
                return;
            }
            if (!_hash_at(i) || (_displacement_at(i) < _displacement_of(h, i)))
                return;
            ++i;
        }
    }
    
    template<typename Q>
    void erase(Q&& k) {
        erase(std::forward<Q>(k), hash(k));
    }

    template<typename Q, typename U>
    value_type& insert(Q&& k, u64 h, U&& u) {
        reserve(_occupants + 1);
        assert(h & HASH_BIT);
        for (u64 i = h;; ++i) {
            if ((_hash_at(i) == h) && (_key_at(i) == k)) {
                _value_at(i) = std::forward<U>(u);
                return _value_at(i);
            }
            if (!_hash_at(i)) {
            thing:
                new (&_entry_at(i)) _entry_type{
                    h,
                    std::forward<Q>(k),
                    std::forward<U>(u)
                };
                ++_occupants;
                return _value_at(i);
            }
            if (_displacement_at(i) < _displacement_of(h, i)) {
                _rob(i);
                goto thing;
            }
        }
    }
    
    template<typename Q, typename U>
    value_type& insert(Q&& k, U&& v) {
        return insert(std::forward<Q>(k),
                      _table_hash(k),
                      std::forward<U>(v));
    }

    
    // Entry API
    
    enum _entry_tag : u64 {
        VACANT,
        PRESENT,
        OCCUPIED
    };

    // expose the find-element logic and the three possible results
    //   * element is found in slot
    //   * element not found in slot
    //   * element not found and slot is occupied by a richer entry
    template<typename Q>
    std::pair<u64, _entry_tag> _find_entry(Q const& k, u64 h) {
        assert(h & HASH_BIT);
        reserve(_occupants + 1);
        for (u64 i = h; ; ++i) {
            if (!_hash_at(i))
                return std::make_pair(i, VACANT); // <-- vacant
            if ((_hash_at(i) == h) && (_key_at(i) == k))
                return std::make_pair(i, PRESENT); // <-- present
            if (_hash_at(i) && (_displacement_at(i) < _displacement_of(h, i)))
                return std::make_pair(i, OCCUPIED); // <-- occupied
        }
    }
        
    struct _deferred_entry {
        K _key;
        table3<K, V>* _target;
        u64 _hash;
        u64 _index;
        _entry_tag _tag;
        
        template<typename F>
        _deferred_entry& and_modify(F&& f) {
            if (_tag == PRESENT)
                std::forward<F>(f)(_target->_value_at(_index));
        }
        
        template<typename U>
        value_type& insert(U&& u) {
            switch (_tag) {
                case OCCUPIED:
                    _target->_rob(_index);
                    // fallthrough
                case VACANT:
                    new (&_target->_entry_at(_index)) _entry_type{
                        _hash,
                        {
                            std::move(_key),
                            std::forward<U>(u),
                        },
                    };
                    ++_target->_occupants;
                    return _target->_value_at(_index);
                case PRESENT:
                    return _target->_value_at(_index) = std::forward<U>(u);
            }
        }
        
        key_type& key() const {
            return _key;
        }
        
        template<typename U>
        value_type& or_insert(U&& u) {
            switch (_tag) {
                case OCCUPIED:
                    _target->_rob(_index);
                    // fallthrough
                case VACANT:
                    new (&_target->_entry_at(_index))
                    _entry_type(_hash,
                                std::move(_key),
                                std::forward<U>(u));
                    ++_target->_occupants;
                    // fallthrough
                case PRESENT:
                    return _target->_value_at(_index);
            }
        }

        template<typename F>
        value_type& or_insert_with(F&& f) {
            switch (_tag) {
                case OCCUPIED:
                    _target->_rob(_index);
                    // fallthrough
                case VACANT:
                    new (&_target->_entry_at(_index))
                    _entry_type(_hash,
                                std::move(_key),
                                std::forward<F>(f)());
                    ++_target->_occupants;
                    // fallthrough
                case PRESENT:
                    return _target->_value_at(_index);
            }
        }
        
        value_type& or_default() {
            switch (_tag) {
                case OCCUPIED:
                    _target->_rob(_index);
                    // fallthrough
                case VACANT:
                    new (&_target->_entry_at(_index)) _entry_type{
                        _hash,
                        {
                            std::move(_key),
                            // value is default constructed
                        },
                    };
                    ++_target->_occupants;
                    // fallthrough
                case PRESENT:
                    return _target->_value_at(_index);
            }
        }

    }; // entry type
    
    template<typename Q>
    _deferred_entry entry(Q&& q) {
        u64 h = hash(q) | HASH_BIT;
        auto [i, t] = _find_entry(q, h);
        return _deferred_entry{
            std::forward<Q>(q),
            this,
            h,
            i,
            t,
        };
    }
    
    // entry(k).
    //          or_insert(V) -> V&
    //          or_insert_with(F) -> V&
    //          key() -> key_type&
    //          and_modify(F) -> entry
    //          insert(V) -> occupied_entry
    //          or_default() -> V&
    //
    // occupied_entry::
    //                 key() -> key_type&
    //                 remove_entry() -> (K, V)
    //                 get() -> V&
    //                 insert(V) -> V
    //                 remove() -> V
    //                 replace_entry(V) -> (K, V)
    //                 replace_key() -> K
    //
    // vacant_entry::
    //               key() -> key_type&
    //               insert(V) -> V&
    
    /*
    struct union_entry_type {
        table3<K, V>* _table;
        union {
            _entry_type* _entry;
            K _key;
        };
    };

    struct occupied_entry_type {
        union_entry_type _inner;
    };
    
    struct vacant_entry_type {
        union_entry_type _inner;
    };
     */
    
    table3<u64, u64> _histogram() const {
        table3<u64, u64> x;
        for (u64 i = 0; i != _vector._capacity; ++i)
            if (_hash_at(i))
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
    
    template<typename K, typename V>
    bool operator==(table3<K, V> const& a, table3<K, V> const& b) {
        if (a.size() != b.size())
            return false;
        for (auto&& [k, v] : a) {
            auto p = const_cast<table3<K, V>&>(b).try_get(k);
            if (!p || (*p != v))
                return false;
        }
        return true;
    }

template<typename K, typename V>
bool operator!=(table3<K, V> const& a, table3<K, V> const& b) {
    return !(a == b);
}
    
    
    template<typename K, typename V, typename Serializer>
    void serialize(table3<K, V> const& x, Serializer& s) {
        serialize(x.size(), s);
        for (auto&& [k, v] : x) {
            serialize(k, s);
            serialize(v, s);
        }
    }
        
    template<typename K, typename V, typename Deserializer>
    auto deserialize(placeholder<table3<K, V>>, Deserializer& d) {
        auto n = deserialize<usize>(d);
        table3<K, V> x;
        x.reserve(n);
        while (n--) {
            auto k = deserialize<K>(d);
            auto v = deserialize<V>(d);
            x.insert(k, v);
        }
        return x;
    }
    
    
    
} // namespace manic

#endif /* table3_hpp */
