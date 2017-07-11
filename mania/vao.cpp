//
//  vao.cpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "vao.hpp"

#include <cassert>
#include <utility>

#include <OpenGL/gl3.h>

namespace gl {
    
    vao::vao()
    : _name(0) {
        glGenVertexArrays(1, &_name);
        assert(_name);
    }
    
    vao::vao(vao&& r)
    : _name(std::exchange(r._name, 0)) {
        
    }
    
    vao::~vao() {
        glDeleteVertexArrays(1, &_name);
    }
    
    vao& vao::operator=(vao&& r) {
        std::swap(_name, r._name);
        return *this;
    }
    
    vao& vao::bind() {
        assert(_name);
        glBindVertexArray(_name);
        return *this;
    }
    
    void vao::unbind() {
        glBindVertexArray(0);
    }
    
    
} // namespace gl
