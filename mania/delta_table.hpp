//
//  delta_table.hpp
//  mania
//
//  Created by Antony Searle on 27/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef delta_table_hpp
#define delta_table_hpp

#include <optional>

#include "table3.hpp"

namespace manic {

// A hash table that can be rolled back to a nominated past state.
//
// The intent is that nominated state is large and the tentative state is small

template<typename K, typename V>
struct delta_table {
    
    using value_type = V;
    
    // table storing committed state
    table3<K, V> _table;
    
    // table storing speculative state; erasures are marked as empty optionals
    table3<K, std::optional<V>> _delta;
    usize _size;
    bool _was_cleared;

    delta_table() : _size(0), _was_cleared(false) {}
    
    delta_table(delta_table&& x)
    : _table(std::move(x._table))
    , _delta(std::move(x._delta))
    , _was_cleared(std::exchange(x._was_cleared, false))
    , _size(std::exchange(x._size, 0)) {
    }
    
    ~delta_table() = default;

    delta_table& operator=(delta_table&& x) {
        using std::swap;
        swap(_table, x._table);
        swap(_delta, x._delta);
        swap(_was_cleared, x._was_cleared);
        swap(_size, x._size);
        x._table.clear();
        x._delta.clear();
        x._was_cleared = false;
        x._size = 0;
    }
    
    usize size() { return _size; }
    bool empty() { return !_size; }
    
    void clear() {
        _delta.clear();
        _size = 0;
        _was_cleared = true;
    }
    
    // TODO: Reuse hash calculation
    // TODO: Entry API would be helpful
    // TODO: Steal more hash bits to indicate missing value?
    
    template<typename Keylike>
    bool contains(Keylike&& key) {
        std::optional<value_type>* p =  _delta.try_get(key);
        if (_was_cleared)
            return false;
        return p ? p->has_value() : _table.contains(key);
    }
    
    template<typename Keylike>
    value_type* try_get(Keylike&& key) {
        std::optional<value_type>* p = _delta.try_get(key);
        if (p)
            return p->has_value() ? &**p : nullptr;
        if (_was_cleared)
            return nullptr;
        value_type* q = _table.try_get(key);
        if (!q)
            return nullptr;
        return &*_delta.insert(key, std::make_optional(*q)).value;
    }
    
    template<typename Keylike>
    value_type& get(Keylike&& key) {
        value_type* q = try_get(key);
        assert(q);
        return *q;
    }
    
    template<typename Keylike, typename Valuelike>
    void insert(Keylike&& key, Valuelike&& value) {
        std::optional<value_type>* p = _delta.try_get(key);
        if (p) {
            if (!p->has_value())
                ++_size;
            *p = std::forward<Valuelike>(value);
            return;
        }
        if (_was_cleared || !_table.contains(key))
            ++_size;
        _delta.insert(std::forward<Keylike>(key),
                      std::optional<value_type>(std::forward<Valuelike>(value)));
    }
    
    template<typename Keylike>
    void erase(Keylike&& key) {
        std::optional<value_type>* p = _delta.try_get(key);
        if (p) {
            if (p->has_value()) {
                p->reset();
                --_size;
            } // else already erased
            return;
        }
        if (_was_cleared)
            return;
        if (_table.contains(key))
            --_size;
        _delta.insert(std::forward<Keylike>(key),
                      std::optional<value_type>{});
    }
    
    template<typename Keylike, typename Valuelike>
    value_type& get_or_insert(Keylike&& key, Valuelike&& value) {
        std::optional<value_type>* p = _delta.try_get(key);
        if (p) {
            if (!p->has_value()) {
                ++_size;
                p->emplace(std::forward<Valuelike>(value));
            }
            return *p;
        }
        value_type* q = !_was_cleared ? _table.try_get(key) : nullptr;
        if (!q)
            ++_size;
        return *_delta.insert(std::forward<Keylike>(key), q
                               ? std::optional<value_type>{*q}
                               : std::optional<value_type>(std::forward<Valuelike>(value))
                               ).value;
    }
    
    template<typename Keylike, typename F>
    value_type& get_or_insert_with(Keylike&& key, F&& f) {
        std::optional<value_type>* p = _delta.try_get(key);
        if (p) {
            if (!p->has_value()) {
                ++_size;
                p->emplace(std::forward<F>(f)());
            }
            return **p;
        }
        value_type* q = !_was_cleared ? _table.try_get(key) : nullptr;
        if (!q)
            ++_size;
        return *_delta.insert(std::forward<Keylike>(key), q
                              ? std::optional<value_type>{*q}
                              : std::optional<value_type>(std::forward<F>(f)())
                              ).value;
    }
    
    void revert() {
        _delta.clear();
        _was_cleared = false;
        _size = _table.size();
    }

    void commit() {
        if (_was_cleared)
            _table.clear();
        for (auto&& kv : _delta) {
            if (kv.value.has_value())
                _table.insert(kv.key, std::move(*kv.value));
            else
                _table.erase(kv.key);
        }
        revert();
    }
    
};

} // namespace li

#endif /* delta_table_hpp */
