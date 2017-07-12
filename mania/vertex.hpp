//
//  vertex.hpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef vertex_hpp
#define vertex_hpp

#include <OpenGL/gltypes.h>

#include "vec.hpp"

#include <OpenGL/gl3.h>

namespace gl {
    
    enum class attribute {
        position,
        normal,
        color,
        texCoord
    };
    
    struct vertex {
        vec2 position;
        vec4 color;
        static void bind();
    };
    
     
    
} // namespace gl

#endif /* vertex_hpp */
