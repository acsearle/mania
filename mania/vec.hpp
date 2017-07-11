//
//  vec.hpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef vec_hpp
#define vec_hpp

#include <cstddef>

namespace gl {
    
    template<typename T, std::size_t N>
    class vec {
        
        T _[N];
        
    public:
        
        using size_type = std::size_t;
        using value_type = T;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = pointer;
        using const_iterator = const_pointer;
        
        constexpr reference operator[](size_type i) { return _[i]; }
        constexpr const_reference operator[](size_type i) const { return _[i]; }
        
        constexpr reference front() { return _[0]; }
        constexpr const_reference front() const { return _[0]; }
        
        constexpr reference back() { return _[N - 1]; }
        constexpr const_reference back() const { return _[N - 1]; }
        
        constexpr pointer data() { return _; }
        constexpr const_pointer data() const { return _; }
        
        constexpr iterator begin() { return _; }
        constexpr const_iterator begin() const { return _; }
        constexpr const_iterator cbegin() const { return _; }
        
        constexpr iterator end() { return _ + N; }
        constexpr const_iterator end() const { return _ + N; }
        constexpr const_iterator cend() const { return _ + N; }
        
        constexpr bool empty() const { return N; }
        constexpr size_type size() const { return N; }
        
    };
    
    using vec2 = vec<float, 2>;
    using vec3 = vec<float, 3>;
    using vec4 = vec<float, 4>;
    
}

#endif /* vec_hpp */
