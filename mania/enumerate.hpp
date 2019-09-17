//
//  enumerate.hpp
//  mania
//
//  Created by Antony Searle on 7/8/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef enumerate_hpp
#define enumerate_hpp

#include <iterator>
#include <tuple>
#include <type_traits>

#include "capture.hpp"
#include "indirect.hpp"

namespace manic {
    
    // for (auto&& [i, x] = enumerate(v))
    //     assert(x == v[i]);
        
    template<typename Index, typename Iterator>
    class enumerate_iterator {
        
    public:
        
        using difference_type = typename std::iterator_traits<Iterator>::difference_type;
        using value_type = std::tuple<Index, typename std::iterator_traits<Iterator>::value_type>;
        using reference = std::tuple<const Index&, typename std::iterator_traits<Iterator>::reference>;
        using pointer = indirect<reference>;
        using iterator_category = typename std::iterator_traits<Iterator>::iterator_category;
        
        enumerate_iterator(Index&& i, Iterator&& x)
        : _index(std::move(i))
        , _iterator(std::move(x)) {
        }

        enumerate_iterator& operator++() {
            ++_index;
            ++_iterator;
            return *this;
        }
        
        reference operator*() const {
            return reference(_index, *_iterator);
        }
        
        indirect<reference> operator->() const {
            return indirect<reference>(operator*());
        }
        
        friend enumerate_iterator operator+(const enumerate_iterator& p, Index n) {
            return enumerate_iterator(p._index + n, p._iterator + n);
        }

        friend enumerate_iterator operator+(Index n, const enumerate_iterator& p) {
            return enumerate_iterator(n + p._index, n + p._iterator);
        }

        friend difference_type operator-(const enumerate_iterator& p, const enumerate_iterator& q) {
            return p._iterator - q._iterator;
        }
        
        friend bool operator!=(const enumerate_iterator& a, const enumerate_iterator& b) {
            return a._iterator != b._iterator;
        }
        
    private:
        
        Index _index;
        Iterator _iterator;
        
    };
    
    template<typename Index, typename Iterator>
    enumerate_iterator(Index&& index, Iterator&& iterator) -> enumerate_iterator<std::decay_t<Index>, std::decay_t<Iterator>>;
    
    template<typename Index, typename Iterable>
    class enumerable {
        
    public:
        
        using value_type = std::tuple<Index, typename std::decay_t<Iterable>::value_type>;
        using reference = std::tuple<const Index&, typename std::decay_t<Iterable>::reference>;
        using const_reference = std::tuple<const Index&, typename std::decay_t<Iterable>::const_reference>;
        using iterator = enumerate_iterator<Index, typename std::decay_t<Iterable>::iterator>;
        using const_iterator = enumerate_iterator<Index, typename std::decay_t<Iterable>::const_iterator>;
        using difference_type = typename std::decay_t<Iterable>::difference_type;
        using size_type = typename std::decay_t<Iterable>::size_type;
        
        enumerable(Iterable&& v)
        : _iterable(std::forward<Iterable>(v)) {
        }
        
        iterator begin() {
            return iterator(0, _iterable.begin());
        }
        
        iterator end() {
            return iterator(_iterable.size(), _iterable.end());
        }
        
        const_iterator begin() const {
            return const_iterator(0, _iterable.begin());
        }
        
        const_iterator end() const {
            return const_iterator(_iterable.size(), _iterable.end());
        }

        const_iterator cbegin() const {
            return const_iterator(0, _iterable.begin());
        }
        
        const_iterator cend() const {
            return const_iterator(_iterable.size(), _iterable.end());
        }

        
    private:
        
        Iterable _iterable;
        
    };
    

    // enumerable(v) uses decltype(v)::size_type as index
    // enumerable<Integral>(v) uses Integral type as index

    struct _iterable_size_type {};
    
    template<typename Index = _iterable_size_type,
             typename Iterable,
             typename Index_ = std::conditional_t<std::is_same_v<Index,
                                                                 _iterable_size_type>,
                               typename std::decay_t<Iterable>::size_type,
                               Index>>
    enumerable<Index_, Iterable> enumerate(Iterable&& iterable) {
        return enumerable<Index_, Iterable>(std::forward<Iterable>(iterable));
    }
            
    
} // namespace manic

#endif /* enumerate_hpp */
