//
//  table4.hpp
//  mania
//
//  Created by Antony Searle on 28/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef table4_hpp
#define table4_hpp

#include "raw_vector.hpp"
#include "hash.hpp"
#include "bit.hpp"

namespace manic {

template<typename Key, typename Value>
struct table4 {
    
    using key_type = std::add_const<Key>;
    using value_type = Value;
    
    static constexpr u64 _KEY_BIT = u64(1) << 63;
    static constexpr u64 _VALUE_BIT = u64(1) << 62;
    static constexpr u64 _HASH_MASK = (u64(1) << 62) - 1;
    
    struct entry_type {
        
        Key _key;
        value_type _value;
        u64 _hash;
        
        ~entry_type() = delete;
        
        template<typename F>
        entry_type& and_modify(F&& f) {
            assert(_hash & _KEY_BIT);
            if (_hash & _VALUE_BIT)
                std::forward<F>(f)(_value);
            return *this;
        }
        
        key_type& key() const {
            assert(_hash & _KEY_BIT);
            return _key;
        }

        value_type& or_default() {
            assert(_hash & _KEY_BIT);
            if (!(_hash & _VALUE_BIT)) {
                new (&_value) value_type;
            }
        }

        value_type& or_insert(value_type const& value) {
            assert(_hash & _KEY_BIT);
            if (!(_hash & _VALUE_BIT)) {
                new (&_value) value_type(value);
                _hash |= _VALUE_BIT;
            }
            return _value;
        }

        value_type& or_insert(value_type&& value) {
            assert(_hash & _KEY_BIT);
            if (!(_hash & _VALUE_BIT)) {
                new (&_value) value_type(std::move(value));
                _hash |= _VALUE_BIT;
            }
            return _value;
        }

        template<typename... Args>
        value_type& or_emplace(Args&&... args) {
            assert(_hash & _KEY_BIT);
            if (!(_hash & _VALUE_BIT)) {
                new (&_value) value_type(std::forward<Args>(args)...);
                _hash |= _VALUE_BIT;
            }
            return _value;
        }
        
        template<typename F>
        value_type& or_insert_with(F&& f) {
            assert(_hash & _KEY_BIT);
            if (!(_hash & _VALUE_BIT)) {
                new (&_value) value_type(std::forward<F>(f)());
                _hash |= _VALUE_BIT;
            }
            return _value;
        }
        
        void _clear() {
            if (_hash & _KEY_BIT) {
                _key.~Key();
                if (_hash & _VALUE_BIT)
                    _value.~value_type();
                _hash = 0;
            }
        }
                                        
    }; // struct entry_type
        
    raw_vector<entry_type> _vector;
    usize _size;
    
    u64 _index_mask() const { return _vector._capacity - 1; }
        
    entry_type& _entry_at(u64 i) const {
        return _vector[i & _index_mask()];
    }

    Key& _key_at(u64 i) const {
        return _entry_at(i)._key;
    }
    
    value_type& _value_at(u64 i) const {
        return _entry_at(i)._value;
    }

    u64& _hash_at(u64 i) const {
        return _entry_at(i)._hash;
    }
    
    u64 _displacement_at(u64 i) const {
        return (i - _hash_at(i)) & _index_mask();
    }
    
    u64 _displacement_of(u64 h, u64 i) const {
        return (i - h) & _index_mask();
    }
        
    bool _is_key_at(u64 i) const {
        return _key_at(i) & _KEY_BIT;
    }
    
    table4() : _vector(0), _size(0) {}
    
    friend void swap(table4& a, table4& b) {
        using std::swap;
        swap(a._vector, b._vector);
        swap(a._size, b._size);
    }
    
    /*
    u64 _capacity_for_size(u64 n) {
        return std::max<u64>(std::ceil2((n * 4 + 3) / 3), 16);
    }
    
    u64 _size_for_capacity(u64 n) {
        return n * 3 / 4;
    }
     */
    
    table4(table4 const& x)
    : _vector(_capacity_for_size(x._size))
    , _size(0) {
        for (entry_type const& e : x._vector)
            if (e._hash)
                entry(e._key, e._hash).or_insert(e._value);
    }
    
    table4(table4&& x)
    : _vector(std::exchange(x._vector, raw_vector<entry_type>{}))
    , _size(std::exchange(x._size, 0)) {
    }
    

    void clear() {
        for (entry_type& e : _vector)
            e._clear();
        _size = 0;
    }
    
    ~table4() {
        clear();
    }
    
    table4& operator=(table4 const& x) {
        table4 y{x};
        swap(*this, y);
        return *this;
    }
    
    table4& operator=(table4&& x) {
        table4 y{std::move(x)};
        swap(*this, y);
        return *this;
    }
    
    void _reserve_one() {
        // FIXME: Need to ensure edge cases handled properly
        if (_size >= _size_for_capacity(_vector._capacity)) {
            table4 y;
            raw_vector<entry_type>{(isize) _capacity_for_size(_size + 1)}.swap(y._vector);
            for (entry_type& e : _vector)
                entry(std::move(e._key), e._hash).or_insert(std::move(e._value));
            swap(*this, y);
            raw_vector<entry_type>{}.swap(y._vector);
        }
    }
    
    usize size() const { return _size; }
    
    template<typename Q>
    bool contains(Q&& k) {
        return contains(std::forward<Q>(k), hash(k));
    }
    
    template<typename Q>
    bool contains(Q&& k, u64 h) {
        if (!_size)
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
    value_type& get(Q&& k, u64 h) {
        assert(_size);
        h |= (_KEY_BIT | _VALUE_BIT);
        u64 i = h;
        while ((_hash_at(i) != h) || (_key_at(i) != k)) {
            ++i;
        }
        return _value_at(i);
    }
    
    template<typename Q>
    value_type& get(Q&& k) {
        return get(std::forward<Q>(k), hash(k));
    }
    
    template<typename Q>
    value_type* try_get(Q&& k, u64 h) {
        if (!_size)
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

    template<typename Q>
    void erase(Q&& k, u64 h) {
        if (!_size)
            return;
        h |= (_KEY_BIT | _VALUE_BIT);
        u64 i = h;
        for (;;) {
            if (((_hash_at(i) | _VALUE_BIT) == h) && (_key_at(i) == k)) {
                _key_at(i).~Key();
                if (_hash_at(i) & _VALUE_BIT)
                    _value_at(i).~value_type();
                while (_is_key_at(i + 1) && _displacement_at(i + 1)) {
                    std::memcpy(&_entry_at(i), &_entry_at(i + 1), sizeof(entry_type));
                    ++i;
                }
                _hash_at(i) = 0;
                --_size;
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

    
    template<typename Q>
    entry_type& entry(Q&& k, u64 h) {
        _reserve_one();
        h |= _KEY_BIT;
        h &= ~_VALUE_BIT;
        u64 i = h;
        for (;;) {
            if (((_hash_at(i) & ~_VALUE_BIT) == h) && (_key_at(i) == k))
                return _entry_at(i);
            if (!_is_key_at(i))
                goto emplace;
            if (_is_key_at(i) && (_displacement_at(i) < _displacement_of(h, i))) {
                // rob from the rich and give to the poor
                // find next free slot
                u64 j = i;
                do ++j; while (_is_key_at(j));
                // move entries back one
                while (j != i) { std::memcpy(&_entry_at(j), &_entry_at(j - 1), sizeof(entry_type)); --j; }
                goto emplace;
            }
            ++i;
        }
    emplace:
        _hash_at(i) = h;
        new (&_key_at(i)) Key(std::forward<Q>(k));
        ++_size;
        return _entry_at(i);
    }
    
    template<typename Q>
    entry_type& entry(Q&& k) {
        return entry(std::forward<Q>(k), hash(k));
    }
    
    template<typename Q, typename V>
    void insert(Q&& k, V&& value) {
        insert(std::forward<Q>(k), hash(k), std::forward<V>(value));
    }
    
    template<typename Q, typename V>
    void insert(Q&& k, u64 h, V&& value) {
        entry_type& e = entry(std::forward<Q>(k), h);
        if (e._hash & _VALUE_BIT)
            e._value = std::forward<V>(value);
        else {
            new (&e._value) value_type(std::forward<V>(value));
            e._hash |= _VALUE_BIT;
        }            
    }
    

    
    

}; // struct table4

} // namespace manic

#endif /* table4_hpp */
