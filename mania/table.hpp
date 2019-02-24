//
//  table.h
//  mania
//
//  Created by Antony Searle on 23/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef table_hpp
#define table_hpp

#include <stdint.h>
#include <memory>
#include <utility>
#include <algorithm>
#include <iostream>

#include "hash.hpp"
#include "filter_iterator.hpp"
#include "transform_iterator.hpp"
#include "vector.hpp"


namespace manic {
    
    // Hash table
    //
    // Uses Robin Hood hashing with backwards shift deletion.  Keys are hashed
    // to produce a position in the table, and we then walk forward until
    //
    // Requires a high-quality hash function.  In particular, libc++ std::hash
    // is inadequate.
    //
    // http://codecapsule.com/2013/11/17/robin-hood-hashing-backward-shift-deletion/
  
    template<typename Key, typename Value>
    struct table {
        
        using key_type = Key;
        
        struct entry {
            
            const Key key;
            Value value;
            
            template<typename KR, typename TR>
            entry(KR&& key_, TR&& value_)
            : key(std::forward<KR>(key_))
            , value(std::forward<TR>(value_)) {
            }
            
        };
        
        struct _bucket {
            uint64_t _hash;
            std::unique_ptr<entry> _entry;
        };
        
        struct _bucket_predicate {
            bool operator()(const _bucket& b) const {
                return static_cast<bool>(b._entry);
            }
        };
        
        struct _bucket_subscript {
            const entry& operator()(const _bucket& b) const {
                return *b._entry;
            }
            entry& operator()(_bucket& b) const {
                return *b._entry;
            }
        };

        using _bucket_iterator = filter_iterator<typename vector<_bucket>::iterator, _bucket_predicate>;
        using _const_bucket_iterator = filter_iterator<typename vector<_bucket>::const_iterator, _bucket_predicate>;

        using value_type = entry;
        using reference = value_type&;
        using const_reference = const value_type&;
        
        using iterator = transform_iterator<_bucket_iterator, _bucket_subscript>;
        using const_iterator = transform_iterator<_const_bucket_iterator, _bucket_subscript>;
        
        
        //std::vector<_bucket> _buckets;
        vector<_bucket> _buckets;
        uint64_t _occupants;
        
        table()
        : _buckets()
        , _occupants(0) {
        }
        
        table(const table& t) : table() {
            if (t._occupants) {
                size_t n = 1;
                while (4 * t._occupants >= 3 * n) {
                    n *= 2;
                }
                _buckets.resize(n);
                for (auto& x : t)
                    if (x)
                        put(x._entry->key, x._entry->value);
            }
        }
        
        table(table&& r) : table() {
            swap(r);
        }
        
        explicit table(uint64_t n)
        : _buckets(n)
        , _occupants(0) {
        }
        
        ~table() = default;
        
        table& operator=(const table& r) {
            table(r).swap(*this);
            return *this;
        }
        
        table& operator=(table&& r) {
            table(std::move(r)).swap(*this);
            return *this;
        }
        
        bool empty() const {
            return static_cast<bool>(_occupants);
        }
        
        ptrdiff_t size() const {
            return _occupants;
        }
        
        void swap(table& r) {
            using std::swap;
            swap(_buckets, r._buckets);
            swap(_occupants, r._occupants);
        }
        
        void clear() {
            _buckets.clear();
            _occupants = 0;
        }
        
        uint64_t _mask() const {
            return _buckets.size() - 1;
        }
        
        uint64_t displacement(uint64_t i) const {
            return (i - _buckets[i & _mask()]._hash) & _mask();
        }

        uint64_t distance(uint64_t a, uint64_t b) const {
            return (a - b) & _mask();
        }
        
        _bucket& _subscript(uint64_t a) {
            return _buckets[a & _mask()];
        }

        const _bucket& _subscript(uint64_t a) const {
            return _buckets[a & _mask()];
        }
        
        static bool _is_power_of_two(uint64_t x) {
            return !(x & (x - 1));
        }
        
        bool _invariant() const {
            
            assert(_is_power_of_two(_buckets.size()));
            assert(_occupants <= _buckets.size());
            
            for (uint64_t i = 0; i != _buckets.size(); ++i) {
                auto& b = _subscript(i);
                if (b) {
                    // Check the stored hash is the hash of the associated key
                    assert(b._hash == hash(b._entry->key));
                    // Get previous bucket
                    auto& a = _subscript(i - 1);
                    if (a) {
                        // We must be unable to reduce the max distance by
                        // swapping the elements
                        auto da = displacement(i - 1);
                        auto db = displacement(i);
                        // Either b is in the right place (db == 0), or we could
                        // swap them, incrementing da and decrementing db, but
                        // this doesn't decrease the maximum
                        assert((db == 0) || ((da + 1) >= db));
                    } else {
                        // We must be in the right bucket, or we would be in
                        // the free bucket ahead of us
                        assert(displacement(i) == 0);
                    }
                }
            }
            return true;
        }
        
        Value* get(const Key& key) {
            uint64_t h = hash(key);
            uint64_t i = h;
            uint64_t probes = 1;
            uint64_t idistance = 0;
            if (!_occupants)
                return nullptr;
            for (;;) {
                if (!_subscript(i)._entry) {
                    return nullptr;
                }
                if ((_subscript(i)._hash == h) && (_subscript(i)._entry->key == key)) {
                    return &_subscript(i)._entry->value;
                } else if (distance(i, _subscript(i)._hash) < idistance) {
                    // If we were inserting the element, it would go here.  But
                    // it isn't, so it mustn't exist.  QED?
                    return nullptr;
                }
                
                // If we encounter an element we would have appeared before, we
                // can give up; how to test/express this?
                
                ++i;
                ++probes;
                ++idistance;
            }
        }
        
        entry* _put(uint64_t h, std::unique_ptr<entry> e) {
            entry* pppp = e.get();
            uint64_t i = h;
            uint64_t idistance = 0;
            for (;;) {
                if (!_subscript(i)._entry) {
                    _subscript(i)._hash = h;
                    _subscript(i)._entry = std::move(e);
                    ++_occupants;
                    return pppp;
                } else if ((_subscript(i)._hash == h) && (_subscript(i)._entry->key == e->key)) {
                    // An equivalent key is already present
                    std::swap(e, _subscript(i)._entry);
                    return pppp;
                } else {
                    // see if we are more deserving of the slot
                    uint64_t jh = _subscript(i)._hash;
                    uint64_t jideal = jh;
                    uint64_t jdistance = distance(i, jideal);
                    if (jdistance < idistance) {
                        std::swap(h, _subscript(i)._hash);
                        std::swap(e, _subscript(i)._entry);
                        std::swap(idistance, jdistance);
                    }
                }
                i = (i + 1) % _buckets.size();
                ++idistance;
            }
        }
        
        entry* put(const Key& key, const Value& value) {
            if (_occupants * 4 >= _buckets.size() * 3) {
                resize();
            }
            return _put(hash(key), std::make_unique<entry>(key, value));
        }
        
        void remove(const Key& key) {
            uint64_t h = hash(key);
            uint64_t i = h;
            if (!_occupants)
                return;
            for (;;) {
                if (!_subscript(i)._entry) {
                    // Preferred slot is empty
                    return;
                } else if ((_subscript(i)._hash == h) && (_subscript(i)._entry->key == key)) {
                    --_occupants;
                    _subscript(i)._entry = nullptr;
                    for (;;) {
                        uint64_t j = i + 1;
                        if (_subscript(j)._entry) {
                            uint64_t k = _subscript(j)._hash;
                            if (distance(j, k) != 0) {
                                // Can benefit from moving forward.  However,
                                // if it could move forward more it would have
                                // displaced i, so we only need to consider
                                // moving forward by one element
                                using std::swap;
                                swap(_subscript(i), _subscript(j));
                            } else {
                                // Gap is where it should be
                                return;
                            }
                            i = j;
                        } else {
                            // We merged with an existing gap
                            return;
                        }
                    }
                            
                } else {
                    // Not found.  Continue searching.
                    ++i;
                }
            }

        }
        
        void resize() {
            table t(std::max(_buckets.size() * 2, _buckets.size() + 1));
            for (auto& x : _buckets) {
                if (x._entry) {
                    t._put(x._hash, std::move(x._entry));
                }
            }
            this->swap(t);
        }
        
        void statistics() const {
            assert(_invariant());
            vector<int> histogram;
            for (size_t i = 0; i != _buckets.size(); ++i) {
                auto& b = _buckets[i];
                if (b) {
                    auto j = distance(i, b._hash);
                    histogram.resize(std::max(histogram.size(), (ptrdiff_t) j + 1), 0);
                    ++histogram[j];
                }
            }
            std::cout << "occupancy\t: " << (_occupants * 100.0 / _buckets.size()) << "%\n";
            for (int j = 0; j != histogram.size(); ++j) {
                std::cout << j << "\t: " << (histogram[j] * 100.0 / _occupants) << "%\n";
            }
        }
        
        const_iterator begin() const {
            return const_iterator(_const_bucket_iterator(_buckets.begin(), _buckets.end()));
        };
        
        const_iterator end() const {
            return const_iterator(_const_bucket_iterator(_buckets.end(), _buckets.end()));
        };
        
        const_iterator cbegin() const { return begin(); }
        const_iterator cend() const { return end(); }
        
        iterator begin() {
            return iterator(_bucket_iterator(_buckets.begin(), _buckets.end()));
        };
        
        iterator end() {
            return iterator(_bucket_iterator(_buckets.end(), _buckets.end()));
        };
        
        void print() const {
            std::cout << "{\n";
            for (auto&& a : *this) {
                std::cout << "\t" << a.key << "\t:\t" << a.value << std::endl;
            }
            std::cout << "}\n";
            statistics();
        }

        Value& operator[](const Key& k) {
            return get_or_default(k);
        }
        
        Value& get_or_default(const Key& k) {
            Value* p = get(k);
            if (!p) {
                p = &put(k, Value())->value;
            }
            return *p;
        }
        
        Value& get_or_insert(const Key& k, const Value& v) {
            Value* p = get(k);
            if (!p) {
                p = &put(k, v)->value;
            }
            return *p;
        }
        
        template<typename F>
        Value& get_or_insert_with(const Key& k, F&& f) {
            // Gets the value if it exists, inserts the result of f() if it
            // does not.
            Value* p = get(k);
            if (!p) {
                p = &put(k, f())->value;
            }
            return *p;
        }
        
        template<typename... Args>
        Value& get_or_emplace(const Key& k, Args&&... args) {
            Value& p = get(k);
            if (!p) {
                p = &put(k, Value(std::forward<Args>(args)...))->value;
            }
            return *p;
        }

        
    }; // table
    
    template<typename K, typename V>
    void swap(table<K, V>& a, table<K, V>& b) {
        a.swap(b);
    }
    
};

#endif /* table_hpp */
