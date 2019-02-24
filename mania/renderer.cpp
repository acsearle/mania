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
#include "texture.hpp"
#include "atlas.hpp"
#include "surface.hpp"


class blenderer
: public renderer {
    
    gl::program _program;
    gl::vao _vao;
    gl::vbo _vbo;
    
    std::vector<gl::vertex> _vertices;
    
    GLsizei _width, _height;
    
    manic::atlas _atlas;
    
    //vector<vector<int>> _grid;
    //vector<gl::vec<GLfloat, 2>> _entities;
    
    manic::surface _surface;
    
    gl::vec<double, 2> _camera_position;
    double             _camera_zoom;
    
public:
    
    blenderer();
    virtual ~blenderer() = default;
    void resize(GLsizei width, GLsizei height);
    void render();
    void blit(ptrdiff_t i, float x, float y);
    void blit_twist(ptrdiff_t i, ptrdiff_t x, ptrdiff_t y, double radians);

};

std::unique_ptr<renderer> renderer::make() {
    return std::make_unique<blenderer>();
}

// As a general principle, we want to use nested local coordinates as much as
// possible, because we access and copy lots of coordinates.  Using doubles
// everywhere is convenient but squanders memory bandwidth, and limits
// precision.  Obviously this is overkill for small worlds/projects.
// As we dig into structures, offsets can accumulate

// Is there a fundamental coordinate?  A pixel?  A sub-pixel?  If so, entities
// need float storage.  Are entities rendered at non-integer co-ordinates?  If
// so, we need GL_LINEAR, and we need to carefuly arrange tiles so they are
// always rendered in the same order, and have enough overlap to blend properly.
// * No, we just need to have border pixels supplied so GL_LINEAR works properly
//
// Conclusion: to support non-integer camera zooms, we need GL_LINEAR rendering.
// Tiles must supply border pixels as well to supply GL_LINEAR what it needs.

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
        auto pattern = manic::from_png(s);
        _atlas.push(pattern);
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
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
    
    
    _surface.instantiate(0, 0);
    _surface.instantiate(0, -1);
    _surface.instantiate(-1, -1);

    for (auto&& e : _surface) {
        auto& c = e.value;
        for (ptrdiff_t i = 0; i != c._rows * c._columns; ++i)
            c._entities.emplace_back(rand() / (double) RAND_MAX * c._rows, rand() / (double) RAND_MAX * c._columns);
    }
    
    glPointSize(10.0);
    
    _camera_position = 0;
    _camera_zoom = 4.56789;
    
    
}

// Rendering engine:
//
// Rendering unscaled asserts requires:
//
// Float x, float y
// Integer asset tag
//
// Implicit is
//
// The texture atlas to use
// The layer so populated / the ordering
//
// Everything
// - Layers 0..n
//   - Atlases 0..n
//     - (x, y, identifier)
//
// If we make the implicit atlas, layer and ordering explicit we have to do
// more work.  Fair tradeoff?
//
// Imagine trees in adjacent chunks; all at the same player, but all overdrawing
// each other.  Z-buffer solves this for 3d geometry but we want to use alpha
// heavily for shadows and smooth edges.  So we have to sort something,
// somewhere.  We will tend to emit large runs of sorted order, so the sort
// can benefit from this for the right algorithm.
//
// While the problem is acute when viewing along a coordinate axis, isometric
// still suffers from it: a large sprite in one chunk can overhang a small
// sprite in another.  Do we need to render chunks by row?
//
// When multiple atlases are in play, the pathological case occurs when
// entities from different atlases are stacked in front of each other.  i.e.
// every time a run of the same atlas breaks, we have to change texture and
// issue a new draw command, or we have to pass a texture id in with each
// vertex.
//
// We can ban this case by restricting layers to a single atlas, but this is
// still problematic--we can only have one layer for overlappy things like
// trees, power poles, buildings, i.e. things that live at the same z-level.
//
// tiles < ore < grass < belts < plates <
// trees, buildings, poles, creatures
// < robots < clouds
//
// Objects that don't overhang their exclusive footprint are precious; a set of
// these can be rendered in any order.  Is this why assemblers are rhomboid?
//
// So layers can either be nonoverlapping and multi-atlas, or overlapping and
// single atlas?  A weird restriction.
//
// Shadows are a devastating example of this.  They can't be cast properly
// onto non-flat surfaces anyway (terrain, tiles, plates layers).
//
// What if we render shadows separately, with a special shader, that accumulates
// them properly, and then renders them back onto main target?  Expensive but
// high quality.  A lone item shadows everything on the ground correctly (but
// also true of naive method), crowded objects don't deepen shadows (better)
// and don't cast on each other (vs. casting wrongly on each other).  Hmm.



void blenderer::resize(GLsizei width, GLsizei height) {
    
    _width = width;
    _height = height;
    
}

void blenderer::blit(ptrdiff_t i, float x, float y) {
    auto& r = _atlas[i];
    gl::vertex a = r.a;
    gl::vertex b = r.b;
    a.position.x += x;
    a.position.y += y;
    b.position.x += x;
    b.position.y += y;
    gl::vertex c = a;
    gl::vertex d = b;
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
        (float) _camera_zoom/_width, 0, 0, 0,
        0, - (float) _camera_zoom/_height, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    std::cout << _width << ", " << _height << std::endl;
    
    glUniformMatrix4fv(glGetUniformLocation(_program, "transform"), 1, GL_TRUE, transform);
    
    static double angle = 0.0;
    angle += 0.01;
    
    
    _vertices.clear();

    _camera_position.x = sin(new_t * 1e-9) * 32;
    _camera_position.y = cos(new_t * 1e-9) * 18;

    for (auto&& d : _surface) {
        auto&& c = d.value;
        for (ptrdiff_t i = 0; i != c._rows; ++i)
            for (ptrdiff_t j = 0; j != c._columns; ++j)
                blit(c._tiles(i, j),
                     (c._j * c._columns + j) * 32 - _camera_position.x,
                     (c._i * c._rows + i) * 18 - _camera_position.y);
        
        for (auto&& e : c._entities) {
            blit(7,
                 (c._j * c._columns + e.x) * 32  - _camera_position.x,
                 (c._i * c._rows + e.y) * 18 - _camera_position.y);
        }
    }
    
    

    
    //glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    gl::vbo::assign(GL_ARRAY_BUFFER, _vertices, GL_STREAM_DRAW);
    
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei) _vertices.size());
    
    
}






void blenderer::blit_twist(ptrdiff_t i, ptrdiff_t x, ptrdiff_t y, double radians) {
    auto& r = _atlas[i];
    
    gl::vertex a = r.a;
    gl::vertex b = r.b;
    gl::vertex c = a;
    gl::vertex d = b;
    using std::swap;
    swap(c.position.x, d.position.x);
    swap(c.texCoord.x, d.texCoord.x);
    // a - c
    // | / |
    // d - b
    
    float cos_ = cos(radians);
    float sin_ = sin(radians);
    // rotate in ground plane
    gl::mat<GLfloat, 2> m(cos_, -sin_ * 16.0/9.0,
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
