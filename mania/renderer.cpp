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
#include <string>

#include "asset.hpp"
#include "atlas.hpp"
#include "debug.hpp"
#include "mat.hpp"
#include "program.hpp"
#include "text.hpp"
#include "thing.hpp"
#include "unicode.hpp"

namespace manic {

class blenderer
: public renderer {
    
    gl::program _program;
    
    GLsizei _width, _height;
        
    // manic::atlas2<unsigned long> _font;
    // manic::table3<unsigned long, float> _advances;
    std::pair<table3<u32, std::pair<sprite, float>>, float> _font;
    
    // manic::atlas3 _tiles;
    atlas _atlas;
    table3<std::string, sprite> _tiles;
    
    world _thing;
    
    //vector<vector<int>> _grid;
    //vector<gl::vec<GLfloat, 2>> _entities;
        
    gl::vec<double, 2> _camera_position;
    double             _camera_zoom;
    std::string _text;
    
    short _lineheight;
        
public:
    
    blenderer();
    virtual ~blenderer() = default;
    void resize(GLsizei width, GLsizei height);
    void render();
    
    void scribe(std::string_view, gl::vec2 xy);

    void blit3(std::string_view v, float x, float y);


};

}
std::unique_ptr<renderer> renderer::make() {
    return std::make_unique<manic::blenderer>();
}
namespace manic {

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
: _program("basic")
, _atlas(1024) {

    // _tiles("/Users/acsearle/Downloads/textures/symbols")
    _font = build_font(_atlas);
    _tiles = load_asset("/Users/acsearle/Downloads/textures/symbols", _atlas);
    
    // _lineheight = manic::build_font(_font, _advances);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glBindAttribLocation(_program, (GLuint) gl::attribute::position, "position");
    glBindAttribLocation(_program, (GLuint) gl::attribute::texCoord, "texCoord");
    glBindAttribLocation(_program, (GLuint) gl::attribute::color, "color");
    _program.link();
    
    _program.validate();
    _program.use();
    
    _program.assign("sampler", 0);
        
    _camera_position = 0;
    _camera_zoom = 1.0; //4.56789;
    
    
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

void blenderer::blit3(std::string_view v, float x, float y) {
    if (!_tiles.contains(v))
        return;
    x -= _camera_position.x;
    y -= _camera_position.y;
    _atlas.push_sprite_translated(_tiles[v], gl::vec2(x, y));
    //scribe(std::string(v).c_str(), x, y);
}
 

void blenderer::scribe(std::string_view v, gl::vec2 xy) {
    auto start = xy;
    pixel col = { 255, 255, 255, 255 };
    // char const* p = v.data();
    auto p = utf8_iterator((u8 const*) v.data());
    while (p != utf8_iterator((u8 const*) v.data() + v.size())) {
        u32 c = *p; ++p;
        if (c == '\n') {
            xy.x = start.x;
            xy.y += _font.second;
        } else {
            if (auto* q = _font.first.try_get(c)) {
                sprite s = q->first;
                s.a.color = col;
                s.b.color = col;
                _atlas.push_sprite_translated(s, xy);
                xy.x += q->second;
            }
        }
    }
}
        


void blenderer::render() {

    auto old_t = mach_absolute_time();

    static int frame = 0;
    frame += 1;

    
    
    glViewport(0, 0, _width, _height);

    _camera_zoom = 2.0;
    
    GLfloat transform[16] = {
        (float) _camera_zoom/_width, 0, 0, 0,
        0, - (float) _camera_zoom/_height, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
        
    glUniformMatrix4fv(glGetUniformLocation(_program, "transform"), 1, GL_TRUE, transform);
    
    static double angle = 0.0;
    angle += 0.01;
    
    if (_keyboard_map.contains('w')) {
        _camera_position.y -= 2;
    }
    if (_keyboard_map.contains('a')) {
        _camera_position.x -= 2;
    }
    if (_keyboard_map.contains('s')) {
        _camera_position.y += 2;
    }
    if (_keyboard_map.contains('d')) {
        _camera_position.x += 2;
    }

    /*
     blit3("northwest",
        _mouse.x * 2 - _width / 2 + _camera_position.x,
        - _mouse.y * 2 + _height / 2 + _camera_position.y);
     */
    
    static const char* translate[] = {
        "noop",
        "load",
        "store",
        "add",
        "sub",
        "bitwise_and",
        "bitwise_or",
        "bitwise_xor",
        "decrement",
        "decrement_saturate",
        "increment",
        "increment_saturate",
        "flip_increment",
        "flip_decrement",
        "swap",
        "kill",
        "fork",
        "conservative_or",
        "conservative_and",
        "less_than",
        "equal_to",
        "clear",
        "compare",
        "and_complement_of",
        "opcode_enum_size",
        "0",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "northeast",
        "southeast",
        "southwest",
        "northwest",
        "register_a",
        "register_b",
        "register_c",
        "register_d",
    };
    
    for (i64 i = 0; i != _thing._board.rows(); ++i) {
        for (i64 j = 0; j != _thing._board.columns(); ++j) {
            u64 k = _thing._board(i, j);
            u64 z = k;
            using namespace instruction;
            if (k & INSTRUCTION_FLAG) {
                k = (k & OPCODE_MASK) >> OPCODE_SHIFT;
            } else {
                if (k) {
                    k = 25 + (k & 0xF);
                }
            }
            
            std::string_view v(translate[k]);
            blit3(v, i * 64 - 512, j * 64 - 512);
            if (z & INSTRUCTION_FLAG) {
                std::string_view u(translate[(z & 0x7) + 25 + 16]);
                blit3(u, i * 64 - 512, j * 64 - 512);
            }
            
        }
    }
    
    for (auto&& a : _thing._chests) {
        auto u = a.x * 64 - 512;
        auto v = a.y * 64 - 512;
        blit3("chest1", u - 64, v);
        blit3("chest2", u, v);
        blit3("chest3", u - 64, v + 64);
        blit3("chest4", u, v + 64);
        
    }
    
    for (auto&& a : _thing._mcus) {
        auto u = a.x * 64 - 512;
        auto v = a.y * 64 - 512;
        auto f = (-frame) & 63;
        switch (a.d & 3) {
            case 0: v += f; break;
            case 1: u -= f; break;
            case 2: v -= f; break;
            case 3: u += f; break;
        }
        blit3("house", u, v);
        char z[32];
        sprintf(z, "%llX", a.d);
        blit3(z, u+32, v+32);
        sprintf(z, "%llX", a.a);
        blit3(z, u-32, v-32);

    }
    
    //_atlas.push_texture();

     
    if (!(frame & 63))
        _thing.tick();
        
    glClearColor(0.5, 0.6, 0.4, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    auto n = _atlas._vertices.size() / 6;
    
    _atlas.commit();

    {
        char s[128];
        auto new_t = mach_absolute_time();
        sprintf(s, "%.2f ms | %lu quads\n%dx%d\nf%d", (new_t - old_t) * 1e-6, n, _width, _height, frame);
        scribe(s, {-_width/2 + _font.first[' '].second, -_height/2 + _font.second});
    }
    
    scribe("Falsches Üben von Xylophonmusik quält jeden größeren Zwerg", { 0, -_height/2 + _font.second });
}

} // namespace::manic

void renderer::key_up(manic::u32 c) {
    // we need a hash_set, clearly (or a bound on key things)
    if (_keyboard_map.contains(c)) {
        _keyboard_map.erase(c);
    }
}

void renderer::key_down(manic::u32 c) {
    _keyboard_map.insert(c, true);
}

void renderer::mouse_moved(double x, double y) {
    _mouse.x = x;
    _mouse.y = y;
}


