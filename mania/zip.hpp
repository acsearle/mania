//
//  zip.hpp
//  mania
//
//  Created by Antony Searle on 14/3/18.
//  Copyright © 2018 Antony Searle. All rights reserved.
//

#ifndef zip_hpp
#define zip_hpp

#include <tuple>
#include <iterator>

namespace manic {
    
    using std::forward;
    using std::get;
    using std::apply;
    using std::tuple;
    
    // Zip iterator and zipped iterables
    //
    // for (auto& [a, b] : zip(u, v))
    //     ...
    
    // Perfect captures arguments in a tuple, as if with auto&& for each
    
    template<typename... Args>
    tuple<Args...> capture_as_tuple(Args&&... args) {
        return tuple<Args...>(std::forward<Args>(args)...);
    }
    
    template<typename... Iterators>
    class zip_iterator {
        tuple<Iterators...> _iterators;
    public:
        
        using difference_type = std::common_type_t<typename std::iterator_traits<Iterators>::difference_type...>;
        using value_type = tuple<typename std::iterator_traits<Iterators>::value_type...>;
        using pointer = void;
        using reference = tuple<typename std::iterator_traits<Iterators>::value_type...>;
        using iterator_category = std::common_type_t<typename std::iterator_traits<Iterators>::iterator_category...>;
        
        
        template<typename... Args>
        zip_iterator(Args&&... args)
        : _iterators(forward<Args>(args)...) {
        }
        
        zip_iterator& operator++() {
            apply([] (auto&&... args) {
                (..., ++args);
            }, _iterators);
            return *this;
        }
        
        auto operator++(int) {
            return apply([] (auto&&... args) {
                return zip_iterator(args++...);
            }, _iterators);
        }
        
        auto operator*() const {
            return apply([] (auto&&... args) {
                return capture_as_tuple(*forward<decltype(args)>(args)...);
            }, _iterators);
        }
        
        template<typename I>
        auto operator[](I&& i) const {
            return apply([&i](auto&&... args) {
                return capture_as_tuple(forward<decltype(args)>(args)[i]...);
            }, _iterators);
        }
        
        friend bool operator==(const zip_iterator& a, const zip_iterator& b) {
            return get<0>(a._iterators) == get<0>(a._iterators);
        }

        friend bool operator!=(const zip_iterator& a, const zip_iterator& b) {
            return get<0>(a._iterators) != get<0>(a._iterators);
        }

    }; // class zip_iterator<Iterators...>
    
    template<typename... Args>
    zip_iterator<std::decay_t<Args>...> make_zip_iterator(Args&&... args) {
        return zip_iterator<std::decay_t<Args>...>(forward<Args>(args)...);
    }
    
    template<typename... Iterables>
    class zip_iterable {
        
        tuple<Iterables...> _iterables;
        
    public:
        
        using value_type = tuple<typename Iterables::value_type...>;
        using reference = tuple<typename Iterables::reference...>;
        using const_reference = tuple<typename Iterables::const_reference...>;
        using iterator = zip_iterable<typename Iterables::iterator...>;
        using const_iterator = zip_iterable<typename Iterables::const_iterator...>;
        using difference_type = std::common_type_t<typename Iterables::difference_type...>;
        using size_type = std::common_type_t<typename Iterables::size_type...>;
        
        template<typename... IterablesR>
        zip_iterable(IterablesR&&... iterables)
        : _iterables(forward<IterablesR>(iterables)...) {
        }
        
        auto begin() {
            return std::apply([](auto&&... x){
                return make_zip_iterator(forward<decltype(x)>(x).begin()...);
            }, _iterables);
        }
        
        auto begin() const {
            return std::apply([](auto&&... x){
                return make_zip_iterator(forward<decltype(x)>(x).begin()...);
            }, _iterables);
        }
        
        auto end() {
            return std::apply([](auto&&... x){
                return make_zip_iterator(forward<decltype(x)>(x).end()...);
            }, _iterables);
        }
        
        auto end() const {
            return std::apply([](auto&&... x){
                return make_zip_iterator(forward<decltype(x)>(x).end()...);
            }, _iterables);
        }
        
        auto cbegin() const {
            return std::apply([](auto&&... x){
                return make_zip_iterator(forward<decltype(x)>(x).cbegin()...);
            }, _iterables);
        }
        
        auto cend() const {
            return std::apply([](auto&&... x){
                return make_zip_iterator(forward<decltype(x)>(x).cend()...);
            }, _iterables);
        }
        
    }; // class zip_iterable<Iterables...>
    
    template<typename... Iterables>
    zip_iterable<std::decay_t<Iterables>...> zip(Iterables&&... iterables) {
        return zip_iterable<std::decay_t<Iterables>...>(forward<Iterables>(iterables)...);
    }
    
} // namespace zip

#endif /* zip_hpp */
