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
    
    struct vertex {
        vec2 position;
        vec4 color;
    };
    
    template<typename T>
    constexpr GLint size(T) {
        return 1;
    }
    
    template<typename T, std::size_t N>
    constexpr GLint size(vec<T, N>) {
        static_assert((1 <= N) && (N <= 4), "Only 1-4 components accepted");
        return (GLint) N;
    }
    
    constexpr GLenum type(unsigned char) {
        return GL_UNSIGNED_BYTE;
    }
    
    constexpr GLenum type(float) {
        return GL_FLOAT;
    }
    
    template<typename T, std::size_t N>
    constexpr GLenum type(vec<T, N>) {
        return type(T());
    }
    
    void foo() {
        glVertexAttribPointer(0, <#GLint size#>, <#GLenum type#>, <#GLboolean normalized#>, <#GLsizei stride#>, <#const GLvoid *pointer#>)
    }
    
} // namespace gl

#endif /* vertex_hpp */
