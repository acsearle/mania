//
//  transform_iterator.hpp
//  mania
//
//  Created by Antony Searle on 23/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef transform_iterator_hpp
#define transform_iterator_hpp

namespace manic {
    
    template<typename Iterator, typename UnaryFunction>
    struct transform_iterator {
        
        using reference = decltype((std::declval<const UnaryFunction&>()(*std::declval<const Iterator&>())));
        using value_type = std::decay_t<reference>;
        using pointer = void;
        using iterator_category = typename std::iterator_traits<Iterator>::iterator_category;
        using difference_type = typename std::iterator_traits<Iterator>::difference_type;
        
        Iterator _iterator;
        UnaryFunction _function;
        
        transform_iterator() = default;
        
        template<typename It, typename Uf>
        transform_iterator(It&& it,
                        Uf&& ufn)
        : _iterator(std::forward<It>(it))
        , _function(std::forward<Uf>(ufn)) {
        }
        
        reference operator*() const {
            return _function(*_iterator);
        }
        
        reference operator[](difference_type i) const {
            return _function(_iterator[i]);
        }
        
        transform_iterator& operator++() {
            ++_iterator;
            return *this;
        }
        
        transform_iterator& operator--() {
            --_iterator;
            return *this;
        }
        
        transform_iterator operator++(int) {
            transform_iterator a(*this);
            ++*this;
            return a;
        }
        
        transform_iterator operator--(int) {
            transform_iterator a(*this);
            --*this;
            return a;
        }
        
        transform_iterator& operator+=(difference_type i) {
            _iterator += i;
            return *this;
        }

        transform_iterator& operator-=(difference_type i) {
            _iterator -= i;
            return *this;
        }

        
    }; // struct filter_iterator
    
    template<typename It1, typename Pr1, typename It2, typename Pr2>
    bool operator==(const transform_iterator<It1, Pr1>& a,
                    const transform_iterator<It2, Pr2>& b) {
        return a._iterator == b._iterator;
    }
    
    template<typename It1, typename Pr1, typename It2, typename Pr2>
    bool operator!=(const transform_iterator<It1, Pr1>& a,
                    const transform_iterator<It2, Pr2>& b) {
        return a._iterator != b._iterator;
    }
    
}


#endif /* transform_iterator_h */
