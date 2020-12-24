//
//  vertex.hpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef vertex_hpp
#define vertex_hpp

#include "vec.hpp"

namespace gl {
    
    using namespace manic;
    
    struct alignas(8) subvert {
        vec2 position;
        vec2 texCoord;
    };
    
    struct alignas(8) vertex {
        subvert v;
        vec<std::uint8_t, 4> color;
        static void bind();
    };
    
    static_assert(sizeof(vertex) == 24);
    /*
    inline vertex operator+(const vertex& a, const vertex& b) {
        return vertex{
            a.v.position + b.v.position,
            a.v.texCoord + b.v.texCoord,
            a.color
        };
    }
    
    inline vertex operator/(const vertex& a, float b) {
        return vertex{
            a.v.position / b,
            a.v.texCoord / b,
            a.color
        };
    }
     */
    
    
} // namespace gl

#endif /* vertex_hpp */
