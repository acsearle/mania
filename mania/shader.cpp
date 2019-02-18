//
//  shader.cpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "shader.hpp"

#include <cassert>
#include <iostream>
#include <utility>
#include <vector>
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

namespace gl {

    shader::shader(shader&& r)
    : _name(std::exchange(r._name, 0)) {
    }
    
    shader::~shader() {
        glDeleteShader(_name);
    }
    
    shader& shader::operator=(shader&& r) {
        std::swap(_name, r._name);
        return *this;
    }
    
    shader::shader(GLenum type)
    : _name(glCreateShader(type)) {
    }
    
    shader::operator GLuint() const {
        return _name;
    }
    
    shader& shader::source(const char* s) {
        glShaderSource(_name, 1, &s, nullptr);
        return *this;
    }
    
    shader& shader::source(const std::string& s) {
        auto t = s.data();
        auto n = (GLint) s.size();
        glShaderSource(_name, 1, &t, &n);
        return *this;
    }
    
    shader& shader::debug() {
        assert(_name);
        GLint infoLogLength;
        glGetShaderiv(_name, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength) {
            std::vector<GLchar> infoLog(infoLogLength);
            GLsizei length;
            glGetShaderInfoLog(_name, (GLsizei) infoLog.size(), &length, infoLog.data());
            std::clog << infoLog.data() << std::endl;
        }
        return *this;
    }
    
    shader& shader::compile() {
        assert(_name);
        glCompileShader(_name);
        debug();
        GLint compileStatus;
        glGetShaderiv(_name, GL_COMPILE_STATUS, &compileStatus);
        assert(compileStatus);
        return *this;
    }
    
} // namespace gl
