//
//  vbo.cpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "vbo.hpp"

#include <cassert>
#include <utility>

#include <OpenGL/gl3.h>

namespace gl {
    
    vbo::vbo()
    : _name(0) {
        glGenBuffers(1, &_name);
        assert(_name);
    }
    
    vbo::vbo(vbo&& r)
    : _name(std::exchange(r._name, 0)) {
    }
    
    vbo::~vbo() {
        glDeleteBuffers(1, &_name);
    }
    
    vbo& vbo::operator=(vbo&& r) {
        std::swap(_name, r._name);
        return *this;
    }
    
    vbo::operator GLuint() const {
        return _name;
    }
    
    vbo& vbo::bind(GLenum target) {
        glBindBuffer(target, _name);
        return *this;
    }
    
    void vbo::unbind(GLenum target) {
        glBindBuffer(target, 0);
    }
    
    template<typename T>
    void vbo::assign(GLenum target, const T* first, const T* last, GLenum usage) {
        glBufferData(target,
                     (last - first) * sizeof(T),
                     first,
                     usage);
    }
    
    
} // namespace gl
