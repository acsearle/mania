//
//  filter_iterator.hpp
//  mania
//
//  Created by Antony Searle on 23/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef filter_iterator_hpp
#define filter_iterator_hpp

#include <iterator>

#include "sentinel.h"

namespace manic {

// Filters the underlying vector to skip over empty elements.
//
// The iterator always points to either an element for which the predicate
// is true, or to the end of the underlying sequence.  It is undefined
// behaviour to increment an iterator equal to end(), or to decrement an
// iterator equal to begin().
//
// The iterator is bidirectional.  Note that begin() may only
// be dereferenced if begin() != end(), and
//
//     auto it = begin(); --it;
//
// is undefined.  To iterate backwards,
//
// for (auto it = end(); it != begin(); ) {
//     f(*--it);
// }
//
// We inherit from Predicate to facilitate empty base optimization in
// the common case where it is stateless.
//
// BUG: When the wrapped iterator is smart, it will be dereferenced
// multiple times under typical usage.  Fix this by buffering the result,
// which will be tricky to make work in all cases.

// This implementation appropriate for dumb iterators

template<typename Iterator, typename Iterator2, typename Predicate>
struct filter_iterator : private Predicate {
    
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    using reference = typename std::iterator_traits<Iterator>::reference;
    using pointer = typename std::iterator_traits<Iterator>::pointer;
    using iterator_category = std::common_type_t<std::bidirectional_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>;
    using difference_type = typename std::iterator_traits<Iterator>::difference_type;
    
    Iterator _begin;
    Iterator2 _end;
    
    filter_iterator() = default;
    
    template<typename It1, typename It2>
    filter_iterator(It1&& first,
                    It2&& last)
    : _begin(std::forward<It1>(first))
    , _end(std::forward<It2>(last)) {
        _fast_forward();
    }
    
    template<typename It1, typename It2, typename Pr>
    filter_iterator(It1&& first,
                    It2&& last,
                    Pr&& pred)
    : Predicate(std::forward<Pr>(pred))
    , _begin(std::forward<It1>(first))
    , _end(std::forward<It2>(last)) {
        _fast_forward();
    }
    
    void _fast_forward() {
        while ((_begin != _end) && !Predicate::operator()(*_begin))
            ++_begin;
    }
    
    void _rewind() {
        assert(_begin != _end);
        while (!Predicate::operator()(*_begin))
            --_begin;
    }
    
    filter_iterator& operator++() {
        assert(_begin != _end);
        ++_begin;
        _fast_forward();
        return *this;
    }
    
    filter_iterator& operator--() {
        --_begin;
        _rewind();
        return *this;
    }
    
    filter_iterator operator++(int) {
        filter_iterator a(*this);
        ++*this;
        return a;
    }
    
    filter_iterator operator--(int) {
        filter_iterator a(*this);
        --*this;
        return a;
    }
    
    reference operator*() const {
        return *_begin;
    }
    
    pointer operator->() const {
        return &*_begin;
    }
    
}; // struct filter_iterator

template<typename It1, typename It2, typename Pr>
filter_iterator(It1&&, It2&&, Pr&&) -> filter_iterator<std::decay_t<It1>, std::decay_t<It2>, std::decay_t<Pr>>;

template<typename It1, typename It1e, typename Pr1, typename It2, typename It2e, typename Pr2>
bool operator==(const filter_iterator<It1, It1e, Pr1>& a,
                const filter_iterator<It2, It2e, Pr2>& b) {
    return a._begin == b._begin;
}

template<typename It1, typename It1e, typename Pr1, typename It2, typename It2e, typename Pr2>
bool operator!=(const filter_iterator<It1, It1e, Pr1>& a,
                const filter_iterator<It2, It2e, Pr2>& b) {
    return a._begin != b._begin;
}

template<typename A, typename B, typename C>
bool operator==(filter_iterator<A, B, C> const& a, sentinel_t) {
    return a._begin == a._end;
}

template<typename A, typename B, typename C>
bool operator!=(filter_iterator<A, B, C> const& a, sentinel_t) {
    return a._begin != a._end;
}


// filter a range with a predicate

template<typename T, typename F>
class filter : F {
    
    T _t;
    
public:
    
    filter(T&& t, F&& f)
    : F(std::forward<F>(f))
    , _t(std::forward<T>(t)) {
    }
    
    auto begin() {
        return filter_iterator(_t.begin(), *this);
    }
    
};

}

#endif /* filter_iterator_hpp */
