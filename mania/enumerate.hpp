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

namespace manic {
        
    template<typename Iterator>
    class enumerator {
        
        Iterator _iterator;
        std::ptrdiff_t _index;
    
    public:
        
        using difference_type = typename std::iterator_traits<Iterator>::difference_type;
        using value_type = std::tuple<std::ptrdiff_t, typename std::iterator_traits<Iterator>::value_type>;
        using pointer = void;
        using reference = std::tuple<std::ptrdiff_t const&, typename std::iterator_traits<Iterator>::reference>;
        using iterator_category = typename std::iterator_traits<Iterator>::iterator_category;

        enumerator(Iterator&& x)
        : _iterator(std::move(x))
        , _index(0) {
        }
        
        enumerator& operator++() {
            ++_iterator;
            ++_index;
            return *this;
        }
        
        reference operator*() const {
            return capture(_index, *_iterator);
        }
        
        friend bool operator!=(enumerator a, enumerator b) {
            return a._iterator != b._iterator;
        }
        
    };
    
    template<typename Iterator>
    enumerator(Iterator&& iterator) -> enumerator<std::decay_t<Iterator>>;
    
    template<typename Iterable>
    class enumerate {
        
        Iterable _iterable;
        
    public:
        
        using value_type = std::tuple<std::ptrdiff_t, typename std::decay_t<Iterable>::value_type>;
        using reference = std::tuple<std::ptrdiff_t&, typename std::decay_t<Iterable>::reference>;
        using const_reference = std::tuple<const std::ptrdiff_t&, typename std::decay_t<Iterable>::const_reference>;
        using iterator = enumerator<typename std::decay_t<Iterable>::iterator>;
        using const_iterator = enumerator<typename std::decay_t<Iterable>::const_iterator>;
        using difference_type = typename std::decay_t<Iterable>::difference_type;
        using size_type = typename std::decay_t<Iterable>::size_type;
        
        enumerate(Iterable&& v)
        : _iterable(std::forward<Iterable>(v)) {
        }
        
        auto begin() {
            return enumerator(_iterable.begin());
        }
        
        auto end() {
            return enumerator(_iterable.end());
        }
        
    };
    
    template<typename Iterable>
    enumerate(Iterable&& v) -> enumerate<Iterable>;
    
} // namespace manic

#endif /* enumerate_hpp */
