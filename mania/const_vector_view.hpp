//
//  const_vector_view.hpp
//  mania
//
//  Created by Antony Searle on 22/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef const_vector_view_hpp
#define const_vector_view_hpp

namespace manic {
    
    // Views contiguous objects.  Reference semantics, so assignment copies
    // the viewed elements
    
    template<typename T>
    struct const_vector_view {
        
        using value_type = T;
        using size_type = ptrdiff_t;
        using difference_type = ptrdiff_t;
        using reference = const T&;
        using const_reference = const T&;
        using iterator = const T*;
        using const_iterator = const T*;
        
        
        T* _begin;
        ptrdiff_t _size;
        
        const_vector_view() = delete;
        const_vector_view(const const_vector_view& r) = default;
        const_vector_view(const_vector_view&& r) = default;
        
        const_vector_view(std::nullptr_t, ptrdiff_t n)
        : _begin(nullptr)
        , _size(n) {
        }
        
        const_vector_view(const T* ptr, ptrdiff_t n)
        : _begin(const_cast<T*>(ptr))
        , _size(n) {
        }
        
        const_vector_view(const T* first, const T* last)
        : _begin(const_cast<T*>(first))
        , _size(last - first) {
        }
        
        const_vector_view(const std::vector<T>& v)
        : _begin(const_cast<T*>(v.data()))
        , _size(v.size()) {
        }
        
        ~const_vector_view() = default;
        
        const_vector_view& operator=(const const_vector_view&) = delete;
        const_vector_view& operator=(const_vector_view&&) = delete;
        
        const T* begin() const { return _begin; }
        const T* end() const { return _begin + _size; }
        ptrdiff_t size() const { return _size; }
        ptrdiff_t columns() const { return _size; }
        const T& operator[](ptrdiff_t i) const { return _begin[i]; }
        const T& operator()(ptrdiff_t i) const { return _begin[i]; }
        const T& front() const { return *_begin; }
        const T& back() const { return _begin[_size - 1]; }
        
        const_vector_view sub(ptrdiff_t i, ptrdiff_t n) const {
            return const_vector_view(_begin + i, n);
        }
        
        void print() const {
            for (auto&& a : *this)
                std::cout << a << ", ";
            std::cout << std::endl;
        }
        
    };
    
    
    // Undefined specialization prevents instantiation with const T
    template<typename T> struct const_vector_view<const T>;
    
    template<typename T, typename U>
    auto dot(const_vector_view<T> a, const_vector_view<U> b) {
        return std::accumulate(a.begin(), a.end(), b.begin(), decltype(std::declval<T>() * std::declval<U>())(0));
    }
    
    template<typename T>
    T sum(const_vector_view<T> a, T b = 0.0) {
        for (ptrdiff_t i = 0; i != a.size(); ++i)
            b += a(i);
        return b;
    }
    
}

#endif /* const_vector_view_hpp */
