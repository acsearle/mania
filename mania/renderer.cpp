//
//  renderer.cpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "renderer.hpp"

#include <cmath>
#include <iostream>
#include <cstdlib>
#include <vector>

#include <OpenGL/gl3.h>

#include "program.hpp"
#include "vao.hpp"
#include "vbo.hpp"
#include "vertex.hpp"
#include "vec.hpp"
#include "mat.hpp"
#include "image.hpp"
#include "cg.hpp"
#include "texture.hpp"

using namespace gl;
using namespace std;

typedef vec<GLushort, 2> edge;
typedef vec<GLushort, 3> triangle;


class blenderer
: public renderer {
    
    gl::program _program;
    gl::vao _vao;
    gl::vbo _vbo;
    
    vector<vertex> _vertices;
    
    GLsizei _width, _height;
    
    
    
public:
    
    blenderer();
    virtual ~blenderer() = default;
    void resize(GLsizei width, GLsizei height);
    void render();
    
};

std::unique_ptr<renderer> renderer::make() {
    return std::make_unique<blenderer>();
}

blenderer::blenderer()
: _program("basic") {
    
    //png_thing();
    auto pattern2 = manic::image::from_png("/Users/acsearle/Downloads/basn6a08.png");
    auto pattern = manic::image::from_png("/Users/acsearle/Downloads/tbrn2c08.png");
    // Premultiply alpha:
    
    assert(pattern._width == 32);
    
    auto tex = new texture();
    tex->bind(GL_TEXTURE_2D);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, pattern._data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 32, 32, GL_RGBA, GL_UNSIGNED_BYTE, pattern._data);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 32, 32, 32, GL_RGBA, GL_UNSIGNED_BYTE, pattern2._data);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 32, 0, 32, 32, GL_RGBA, GL_UNSIGNED_BYTE, pattern2._data);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 33);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 32, 32, 32, 32, GL_RGBA, GL_UNSIGNED_BYTE, pattern._data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glBindAttribLocation(_program, (GLuint) gl::attribute::position, "position");
    glBindAttribLocation(_program, (GLuint) gl::attribute::texCoord, "texCoord");
    _program.link();
    
    _vao.bind();
    _vbo.bind(GL_ARRAY_BUFFER);
    
    gl::vertex::bind();
    
    _vertices.push_back({{0,0},{0,0}});
    _vertices.push_back({{0,-1},{0,1}});
    _vertices.push_back({{1,0},{1,0}});
    _vertices.push_back({{0,-1},{0,1}});
    _vertices.push_back({{1,-1},{1,1}});
    _vertices.push_back({{1,0},{1,0}});

    
    _program.validate();
    _program.use();
    
    _program.assign("sampler", 0);
    
    
    
    glPointSize(10.0);
    
}

void blenderer::resize(GLsizei width, GLsizei height) {
    
    _width = width;
    _height = height;
    
}

void blenderer::render() {
    
    glViewport(0, 0, std::min(_width, _height), std::min(_width, _height));
    
    
    glClearColor(0.5, 0.25, 0.75, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    gl::vbo::assign(GL_ARRAY_BUFFER, _vertices, GL_STREAM_DRAW);
    
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei) _vertices.size());
    
    
}
