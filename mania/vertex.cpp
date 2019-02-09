//
//  vertex.cpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "vertex.hpp"

namespace gl {
    
    void vertex::bind() {
        glVertexAttribPointer((GLuint) attribute::position, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*) offsetof(vertex, position));
        glVertexAttribPointer((GLuint) attribute::texCoord, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*) offsetof(vertex, texCoord));
        glEnableVertexAttribArray((GLuint) attribute::position);
        glEnableVertexAttribArray((GLuint) attribute::texCoord);
    }
    
    // number of components of a vector type

    template<typename T>
    constexpr GLint size(T) {
        return 1;
    }
    
    template<typename T, std::size_t N>
    constexpr GLint size(vec<T, N>) {
        static_assert((1 <= N) && (N <= 4), "Only 1-4 components accepted");
        return (GLint) N;
    }
    
    // GLenum value for a type (or the type of the elements of a vector)
    
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
    
    

} // namespace gl
