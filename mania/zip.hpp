//
//  zip.hpp
//  mania
//
//  Created by Antony Searle on 14/3/18.
//  Copyright © 2018 Antony Searle. All rights reserved.
//

#ifndef zip_hpp
#define zip_hpp

#include <iterator>
#include <tuple>

#include "capture.hpp"

namespace manic {
    
    // Zip iterator and zipped iterables
    //
    // for (auto& [a, b] : zip(u, v))
    //     ...
    
    template<typename... Iterators>
    class zip_iterator {
        
        std::tuple<Iterators...> _iterators;
    
    public:
        
        using difference_type = std::common_type_t<typename std::iterator_traits<Iterators>::difference_type...>;
        using value_type = std::tuple<typename std::iterator_traits<Iterators>::value_type...>;
        using pointer = void;
        using reference = std::tuple<typename std::iterator_traits<Iterators>::value_type...>;
        using iterator_category = std::common_type_t<typename std::iterator_traits<Iterators>::iterator_category...>;
        
        
        template<typename... Args>
        explicit zip_iterator(Args&&... args)
        : _iterators(std::forward<Args>(args)...) {
        }
        
        zip_iterator& operator++() {
            std::apply([] (auto&&... args) {
                (..., ++args);
            }, _iterators);
            return *this;
        }
        
        zip_iterator operator++(int) {
            return std::apply([] (auto&&... args) {
                return zip_iterator(args++...);
            }, _iterators);
        }
        
        reference operator*() const {
            return std::apply([] (auto&&... args) {
                return capture(*args...);
            }, _iterators);
        }
        
        template<typename I>
        reference operator[](I&& i) const {
            return std::apply([&i](auto&&... args) {
                return capture(args[i]...);
            }, _iterators);
        }
        
        friend bool operator==(const zip_iterator& a, const zip_iterator& b) {
            return std::get<0>(a._iterators) == std::get<0>(a._iterators);
        }

        friend bool operator!=(const zip_iterator& a, const zip_iterator& b) {
            return std::get<0>(a._iterators) != std::get<0>(a._iterators);
        }

    }; // class zip_iterator<Iterators...>
    
    template<typename... Args>
    zip_iterator(Args&&... args) -> zip_iterator<std::decay_t<Args>...>;
    
    
    template<typename... Iterables>
    class zip {
        
        std::tuple<Iterables...> _iterables;
        
    public:
        
        using value_type = std::tuple<typename std::decay_t<Iterables>::value_type...>;
        using reference = zip_iterator<std::conditional_t<std::is_const_v<std::remove_reference_t<Iterables>>,
            typename std::decay_t<Iterables>::const_reference,
            typename std::decay_t<Iterables>::reference>...>;
        using const_reference = std::tuple<typename std::decay_t<Iterables>::const_reference...>;
        using iterator = zip_iterator<std::conditional_t<std::is_const_v<std::remove_reference_t<Iterables>>,
            typename std::decay_t<Iterables>::const_iterator,
            typename std::decay_t<Iterables>::iterator>...>;
        using const_iterator = zip_iterator<typename std::decay_t<Iterables>::const_iterator...>;
        using difference_type = std::common_type_t<typename std::decay_t<Iterables>::difference_type...>;
        using size_type = std::common_type_t<typename std::decay_t<Iterables>::size_type...>;
        
        template<typename... IterablesR>
        explicit zip(IterablesR&&... iterables)
        : _iterables(std::forward<IterablesR>(iterables)...) {
        }
        
        iterator begin() {
            return std::apply([](auto&&... x){
                return zip_iterator(x.begin()...);
            }, _iterables);
        }
        
        const_iterator begin() const {
            return std::apply([](auto&&... x){
                return zip_iterator(x.begin()...);
            }, _iterables);
        }
        
        iterator end() {
            return std::apply([](auto&&... x){
                return zip_iterator(x.end()...);
            }, _iterables);
        }
        
        const_iterator end() const {
            return std::apply([](auto&&... x){
                return zip_iterator(x.end()...);
            }, _iterables);
        }
        
        const_iterator cbegin() const {
            return std::apply([](auto&&... x){
                return zip_iterator(x.cbegin()...);
            }, _iterables);
        }
        
        const_iterator cend() const {
            return std::apply([](auto&&... x){
                return zip_iterator(x.cend()...);
            }, _iterables);
        }
        
    }; // class zip<Iterables...>
    
    template<typename... Iterables>
    zip(Iterables&&... iterables) -> zip<Iterables...>;
    
} // namespace zip

#endif /* zip_hpp */
