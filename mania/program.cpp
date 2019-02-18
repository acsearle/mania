//
//  program.cpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "program.hpp"

#include <cassert>
#include <iostream>
#include <utility>
#include <vector>

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

#include "shader.hpp"
#include "renderer.hpp"

namespace gl {
    
    program::program()
    : _name(glCreateProgram()) {
    }
    
    program::program(program&& r)
    : _name(std::exchange(r._name, 0)) {
    }
    
    program::~program() {
        glDeleteProgram(_name);
    }
    
    program& program::operator=(program&& r) {
        std::swap(_name, r._name);
        return *this;
    }
    
    program::program(const std::string& name)
    : program() {
        attach(shader(GL_VERTEX_SHADER).source(load(name, "vsh")).compile());
        attach(shader(GL_FRAGMENT_SHADER).source(load(name, "fsh")).compile());
        link();        
    }
    
    program::operator GLuint() const {
        return _name;
    }
    
    program& program::attach(const shader& s) {
        assert(_name);
        glAttachShader(_name, s);
        return *this;
    }
    
    program& program::debug() {
        assert(_name);
        GLint infoLogLength;
        glGetProgramiv(_name, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength) {
            std::vector<GLchar> infoLog(infoLogLength);
            GLsizei length;
            glGetProgramInfoLog(_name, (GLsizei) infoLog.size(), &length, infoLog.data());
            std::clog << infoLog.data() << std::endl;
        }
        return *this;
    }
    
    program& program::link() {
        assert(_name);
        glLinkProgram(_name);
        debug();
        GLint linkStatus;
        glGetProgramiv(_name, GL_LINK_STATUS, &linkStatus);
        assert(linkStatus);
        return *this;
    }
    
    program& program::validate() {
        assert(_name);
        glValidateProgram(_name);
        debug();
        GLint validateStatus;
        glGetProgramiv(_name, GL_VALIDATE_STATUS, &validateStatus);
        assert(validateStatus);
        return *this;
    }
    
    program& program::use() {
        assert(_name);
        glUseProgram(_name);
        return *this;
    }
    
    // attributes and uniforms
    
} // namespace gl
