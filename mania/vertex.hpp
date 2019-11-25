//
//  vertex.hpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef vertex_hpp
#define vertex_hpp
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gltypes.h>

#include "vec.hpp"
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

namespace gl {

using namespace manic;

enum class attribute {
    position,
    normal,
    color,
    texCoord
};

struct vertex {
    vec2 position;
    vec2 texCoord;
    vec<GLubyte, 4> color;
    static void bind();
};

// Infer format from a struct

template<typename T> constexpr GLenum format = GL_RED;
template<typename T> constexpr GLenum format<vec<T, 1>> = GL_RED;
template<typename T> constexpr GLenum format<vec<T, 2>> = GL_RG;
template<typename T> constexpr GLenum format<vec<T, 3>> = GL_RGB;
template<typename T> constexpr GLenum format<vec<T, 4>> = GL_RGBA;


// Infer type from a struct

template<typename T> GLenum type;

template<> constexpr GLenum type<GLubyte> = GL_UNSIGNED_BYTE;
template<std::size_t N> constexpr GLenum type<vec<GLubyte, N>> = GL_UNSIGNED_BYTE;

template<> constexpr GLenum type<GLfloat> = GL_FLOAT;
template<std::size_t N> constexpr GLenum type<vec<GLfloat, N>> = GL_FLOAT;


// Infer size from a struct

template<typename T> constexpr GLint size = 1;
template<typename T, std::size_t N> constexpr GLint size<vec<T, N>> = (GLint) N;

inline vertex operator+(const vertex& a, const vertex& b) {
    return vertex{
        a.position + b.position,
        a.texCoord + b.texCoord,
        a.color
    };
}

inline vertex operator/(const vertex& a, float b) {
    return vertex{
        a.position / b,
        a.texCoord / b,
        a.color
    };
}


} // namespace gl

#endif /* vertex_hpp */
