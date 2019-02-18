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
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

#include <mach/mach_time.h>

#include "program.hpp"
#include "vao.hpp"
#include "vbo.hpp"
#include "vertex.hpp"
#include "vec.hpp"
#include "mat.hpp"
#include "image.hpp"
#include "cg.hpp"
#include "texture.hpp"
#include "atlas.hpp"

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
    
    manic::atlas _atlas;
    
    vector<vector<int>> _grid;
    vector<gl::vec<GLfloat, 2>> _entities;
    
    
    
public:
    
    blenderer();
    virtual ~blenderer() = default;
    void resize(GLsizei width, GLsizei height);
    void render();
    void blit(ptrdiff_t i, ptrdiff_t x, ptrdiff_t y);
    void blit_twist(ptrdiff_t i, ptrdiff_t x, ptrdiff_t y, double radians);

};

std::unique_ptr<renderer> renderer::make() {
    return std::make_unique<blenderer>();
}

blenderer::blenderer()
: _program("basic"), _atlas(1024) {

    //auto pattern2 = manic::image::from_png("/Users/acsearle/Downloads/basn6a08.png");
    //auto pattern = manic::image::from_png("/Users/acsearle/Downloads/tbrn2c08.png");
    
    for (auto s : {
        "/Users/acsearle/Downloads/textures/sand.png",
        "/Users/acsearle/Downloads/textures/water.png",
        "/Users/acsearle/Downloads/textures/grass.png",
        "/Users/acsearle/Downloads/textures/right.png",
        "/Users/acsearle/Downloads/textures/up.png",
        "/Users/acsearle/Downloads/textures/left.png",
        "/Users/acsearle/Downloads/textures/down.png",
        "/Users/acsearle/Downloads/textures/plate.png",
    }) {
        auto pattern = manic::image::from_png(s);
        _atlas.push(pattern);
    }
    
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

    
    _program.validate();
    _program.use();
    
    _program.assign("sampler", 0);
    
    auto N = 30;
    
    for (int i = 0; i != N * N; ++i)
        _entities.emplace_back(rand() % (N * 32), rand() % (N * 18));
    
    for (int i = 0; i != N; ++i) {
        vector<int> v;
        for (int j = 0; j != N; ++j) {
            int k = 0;
            if (i > j) {
                if ((i + j) > N) {
                    k = 3;
                } else {
                    k = 6;
                }
            } else {
                if ((i + j) > N) {
                    k = 4;
                } else {
                    k = 5;
                }
            }
            if (!(rand() % N))
                k = rand() % 7;
            v.push_back(k);
        }
        _grid.push_back(v);
    }
    
    
    
    glPointSize(10.0);
    
    
}

void blenderer::resize(GLsizei width, GLsizei height) {
    
    _width = width;
    _height = height;
    
}

void blenderer::blit(ptrdiff_t i, ptrdiff_t x, ptrdiff_t y) {
    auto& r = _atlas[i];
    vertex a = r.a;
    vertex b = r.b;
    a.position.x += x;
    a.position.y += y;
    b.position.x += x;
    b.position.y += y;
    vertex c = a;
    vertex d = b;
    // a - c
    // | / |
    // d - b
    using std::swap;
    swap(c.position.x, d.position.x);
    swap(c.texCoord.x, d.texCoord.x);
    _vertices.push_back(a);
    _vertices.push_back(c);
    _vertices.push_back(d);
    _vertices.push_back(d);
    _vertices.push_back(c);
    _vertices.push_back(b);
}

void blenderer::blit_twist(ptrdiff_t i, ptrdiff_t x, ptrdiff_t y, double radians) {
    auto& r = _atlas[i];
    
    vertex a = r.a;
    vertex b = r.b;
    vertex c = a;
    vertex d = b;
    using std::swap;
    swap(c.position.x, d.position.x);
    swap(c.texCoord.x, d.texCoord.x);
    // a - c
    // | / |
    // d - b

    float cos_ = cos(radians);
    float sin_ = sin(radians);
    // rotate in ground plane
    mat<GLfloat, 2> m(cos_, -sin_ * 16.0/9.0,
                      sin_ * 9.0/16.0, cos_);
    a.position = dot(m, a.position);
    b.position = dot(m, b.position);
    c.position = dot(m, c.position);
    d.position = dot(m, d.position);
    

    a.position.x += x;
    a.position.y += y;
    b.position.x += x;
    b.position.y += y;
    c.position.x += x;
    c.position.y += y;
    d.position.x += x;
    d.position.y += y;



    _vertices.push_back(a);
    _vertices.push_back(c);
    _vertices.push_back(d);
    _vertices.push_back(d);
    _vertices.push_back(c);
    _vertices.push_back(b);
}



void blenderer::render() {
    
    static int frame = 0;
    
    static auto old_t = mach_absolute_time();
    auto new_t = mach_absolute_time();
    std::cout << 1e9/(new_t - old_t) << std::endl;
    old_t = new_t;
    
    static gl::vec<GLfloat, 2> deltas[] = {
        {0, 0},
        {0, 0},
        {0, 0},
        {2, 0},
        {0, -1},
        {-2, 0},
        {0, +1},
    };
    

    glViewport(0, 0, _width, _height);
    
    GLfloat transform[16] = {
        4.0f/_width, 0, 0, -1,
        0, -4.0f/_height, 0, 1,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    std::cout << _width << ", " << _height << std::endl;
    
    glUniformMatrix4fv(glGetUniformLocation(_program, "transform"), 1, GL_TRUE, transform);
    
    static double angle = 0.0;
    angle += 0.01;
    
    
    _vertices.clear();

    for (int j = 0; j != _grid.size(); ++j)
        for (int i = 0; i != _grid[j].size(); ++i)
            blit_twist(_grid[j][i], i * 32, j * 18, angle);
    
    for (int i = 0; i != _entities.size(); ++i) {
        blit(7, _entities[i].x, _entities[i].y);
    }
    
    for (int i = 0; i != _entities.size(); ++i) {
        int x = _entities[i].x / 32 + 0.5;
        int y = _entities[i].y / 18 + 0.5;
        y = std::clamp<int>(y, 0, _grid.size() - 1);
        x = std::clamp<int>(x, 0, _grid[y].size() - 1);
        auto dd = deltas[_grid[y][x]];
        auto nu = _entities[i] + dd;
        /*
        int j = 0;
        for (j = 0; j != _entities.size(); ++j) {
            if (j == i)
                continue;
            auto del = nu - _entities[j];
            if (del.x * del.x + del.y * del.y < 100) {
                // We are collided but are we obstructing?
                if (dd.x * del.x + dd.y * del.y <= 0)
                    break;
            }
        }
        if (j == _entities.size())*/
            _entities[i] = nu;
        
    }
    std::stable_sort(_entities.begin(), _entities.end(), [](auto a, auto b) {
        return a.y > b.y;
    });
    
    
    //glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    gl::vbo::assign(GL_ARRAY_BUFFER, _vertices, GL_STREAM_DRAW);
    
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei) _vertices.size());
    
    
}
