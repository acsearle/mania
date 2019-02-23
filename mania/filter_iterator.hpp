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

namespace manic {
    
    // Filters the underlying vector to skip over empty elements.
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

    
    template<typename Iterator, typename Predicate>
    struct filter_iterator {
        
        using value_type = typename std::iterator_traits<Iterator>::value_type;
        using reference = typename std::iterator_traits<Iterator>::reference;
        using pointer = typename std::iterator_traits<Iterator>::pointer;
        using iterator_category = std::common_type_t<std::bidirectional_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>;
        using difference_type = typename std::iterator_traits<Iterator>::difference_type;
        
        Iterator _begin;
        Iterator _end;
        Predicate _predicate;
        
        filter_iterator() = default;
        
        template<typename It1, typename It2, typename Pr>
        filter_iterator(It1&& first,
                        It2&& last,
                        Pr&& pred)
        : _begin(std::forward<It1>(first))
        , _end(std::forward<It2>(last))
        , _predicate(std::forward<Pr>(pred)) {
        }
        
        void _fast_forward() {
            while ((_begin != _end) && !_predicate(*_begin))
                ++_begin;
        }
        
        void _rewind() {
            assert(_begin != _end);
            while (!_predicate(*_begin))
                --_begin;
        }
        
        filter_iterator& operator++() {
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
            return *_begin;;
        }
        
        pointer operator->() const {
            return &*_begin;
        }

    }; // struct filter_iterator
    
    template<typename It1, typename Pr1, typename It2, typename Pr2>
    bool operator==(const filter_iterator<It1, Pr1>& a,
                    const filter_iterator<It2, Pr2>& b) {
        return a._begin == b._begin;
    }

    template<typename It1, typename Pr1, typename It2, typename Pr2>
    bool operator!=(const filter_iterator<It1, Pr1>& a,
                    const filter_iterator<It2, Pr2>& b) {
        return a._begin != b._begin;
    }
    
}

#endif /* filter_iterator_hpp */
