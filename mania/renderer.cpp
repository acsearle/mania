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

#include "application.hpp"
#include "asset.hpp"
#include "atlas.hpp"
#include "debug.hpp"
#include "mat.hpp"
#include "program.hpp"
#include "text.hpp"
#include "thing.hpp"

namespace manic {

struct game : application {
    
    gl::program _program;
    
    usize _width, _height;
        
    font _font;
    
    // manic::atlas3 _tiles;
    atlas _atlas;
    table3<string, sprite> _tiles;
    
    world _thing;
    
    gl::vec<i64, 2> _camera_position;
    string _text;
    
    short _lineheight;
    
    u64 _selected_opcode;
        
    game();
    virtual ~game() = default;
    void resize(usize width, usize height) override;
    void draw() override;
    
    void scribe(string_view, gl::vec2 xy);

    void blit3(string_view v, gl::vec2 xy);
    void blit4(string_view v, gl::vec2 xy);
    void blit5(string_view v, gl::vec2 xy);

    virtual void key_down(u32) override;
    virtual void mouse_up(u64) override;
    virtual void mouse_down(u64) override;
    virtual void scrolled(double x, double y) override;

    gl::vec2 bound(string_view v);

    sprite _solid;

}; // struct game

application& application::get() {
    static game x;
    return x;
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

game::game()
: _program("basic")
, _atlas(1024) {

    _tiles = load_asset("/Users/acsearle/Downloads/textures/symbols", _atlas);
    _font = build_font(_atlas);
    {
        // Put a white pixel on the texture so we can render solid blocks of color
        pixel p{255, 255, 255, 255};
        _solid = _atlas.place(const_matrix_view<pixel>(&p, 1, 1, 1));
        auto midpoint = (_solid.a.texCoord + _solid.b.texCoord) / 2.0f;
        _solid.a.texCoord = _solid.b.texCoord = midpoint;
    }
    
    std::cout << load("enum", "hpp") << std::endl;
    
    
    
    // _lineheight = manic::build_font(_font, _advances);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
    _selected_opcode = 0;
    
    
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



void game::resize(usize width, usize height) {
    
    _width = width;
    _height = height;
    
}

void game::blit3(string_view v, gl::vec2 xy) {
    if (auto p = _tiles.try_get(v)) {
        xy.x -= _camera_position.x;
        xy.y -= _camera_position.y;
        _atlas.push_sprite_translated(*p, xy);
    }
}

void game::blit4(string_view v, gl::vec2 xy) {
    if (auto p = _tiles.try_get(v))
        _atlas.push_sprite_translated(*p, xy);
}

void game::blit5(string_view v, gl::vec2 xy) {
    if (auto p = _tiles.try_get(v)) {
        auto s = *p;
        s.a.color *= 0.5f;
        s.b.color *= 0.5f;
        _atlas.push_sprite_translated(s, xy);
    }
}


void game::scribe(string_view v, gl::vec2 xy) {
    {
        _solid.a.position = xy;
        _solid.b.position = xy + bound(v);;
        _solid.a.position.y -= _font.ascender;
        _solid.b.position.y -= _font.ascender;
        _solid.a.color = pixel{0, 0, 0, 64};
        _solid.b.color = pixel{0, 0, 0, 192};
        _atlas.push_sprite(_solid);
    }
    auto start = xy;
    pixel col = { 255, 255, 255, 255 };
    // char const* p = v.data();
    while (v) {
        u32 c = *v; ++v;
        if (c == '\n') {
            xy.x = start.x;
            xy.y += _font.height;
        } else {
            if (auto* q = _font.charmap.try_get(c)) {
                sprite s = q->sprite_;
                s.a.color = col;
                s.b.color = col;
                _atlas.push_sprite_translated(s, xy);
                xy.x += q->advance;
            }
        }
    }
}

gl::vec2 game::bound(string_view v) {
    gl::vec2 xy{0, _font.ascender - _font.descender};
    float x = 0.0f;
    while (v) {
        u32 c = *v; ++v;
        if (c != '\n') {
            if (auto* p = _font.charmap.try_get(c)) {
                x += p->advance;
            }
        } else {
            xy.x = std::max<float>(xy.x, x);
            x = 0.0f;
            xy.y += _font.height;
        }
    }
    xy.x = std::max<float>(xy.x, x);
    return xy;
}


void game::draw() {

    auto old_t = mach_absolute_time();

    static int frame = 0;
    frame += 1;
    
    glViewport(0, 0, (GLsizei) _width, (GLsizei) _height);
    
    GLfloat transform[16] = {
        (float) 2.0f/_width, 0, 0, -1,
        0, - (float) 2.0f/_height, 0, +1,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
        
    glUniformMatrix4fv(glGetUniformLocation(_program, "transform"), 1, GL_TRUE, transform);
    
    static double angle = 0.0;
    angle += 0.01;
    
    if (_keyboard_state.contains('w')) {
        _camera_position.y -= 2;
    }
    if (_keyboard_state.contains('a')) {
        _camera_position.x -= 2;
    }
    if (_keyboard_state.contains('s')) {
        _camera_position.y += 2;
    }
    if (_keyboard_state.contains('d')) {
        _camera_position.x += 2;
    }

    gl::vec2 world_mouse{
        _mouse_window.x * 2 + _camera_position.x,
        - _mouse_window.y * 2 + _height + _camera_position.y};
    
    gl::vec2 selectee(((i64) world_mouse.x) >> 6, ((i64) world_mouse.y) >> 6);
        
    static const char* _translate[] = {
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
    
    auto translate = [&](u64 x) {
        using namespace instruction;
        if (x & INSTRUCTION_FLAG) {
            x = (x & OPCODE_MASK) >> OPCODE_SHIFT;
            if (x < _opcode_enum_size) {
                return _translate[x];
            } else {
                return "missing";
            }
        } else {
            return _translate[(x & 0xF) + _opcode_enum_size];
        }
    };
    
    {
        /*
        auto a = std::max<i64>(_camera_position.x >> 6, 0);
        auto b = std::max<i64>(_camera_position.y >> 6, 0);
        auto c = std::min<i64>((_camera_position.x + _width + 64) >> 6, _thing._board.rows());
        auto d = std::min<i64>((_camera_position.y + _height + 64) >> 6, _thing._board.columns());
        
        for (i64 i = a; i != c; ++i) {
            for (i64 j = b; j != d; ++j) {
                u64 k = _thing._board(i, j);
                blit3(translate(k), {i * 64, j * 64});
                if (k & instruction::INSTRUCTION_FLAG) {
                    string_view u(_translate[(k & 0x7) + instruction::_opcode_enum_size + 16]);
                    blit3(u, {i * 64, j * 64});
                }
            }
        }
         */
        
        for (auto&& z : _thing._board) {
            auto key = z.key;
            if (
                ( key.x       * 64 < _width  + _camera_position.x) &&
                ((key.x + 16) * 64 >           _camera_position.x) &&
                ( key.y       * 64 < _height + _camera_position.y) &&
                ((key.y + 16) * 64 >           _camera_position.y)) {
                for (i64 i = 0; i != 16; ++i) {
                    for (i64 j = 0; j != 16; ++j) {
                        u64 k = z.value(i, j);
                        blit3(translate(k), {(i + key.x) * 64, (j + key.y) * 64});
                        if (k & instruction::INSTRUCTION_FLAG) {
                            string_view u(_translate[(k & 0x7) + instruction::_opcode_enum_size + 16]);
                            blit3(u, {(i + key.x) * 64, (j + key.y) * 64});
                        }
                    }
                }
            }
        }
        
    }
    
    
    for (auto&& a : _thing._chests) {
        auto u = a.x * 64;
        auto v = a.y * 64;
        blit3("chest1", {u - 64, v});
        blit3("chest2", {u, v});
        blit3("chest3", {u - 64, v + 64});
        blit3("chest4", {u, v + 64});
        
    }
    
    for (auto&& a : _thing._mcus) {
        auto u = a.x * 64;
        auto v = a.y * 64;
        auto f = (-frame) & 63;
        switch (a.d & 3) {
            case 0: v += f; break;
            case 1: u -= f; break;
            case 2: v -= f; break;
            case 3: u += f; break;
        }
        blit3("house", {u, v});
        char z[32];
        sprintf(z, "%llX", a.a);
        blit3(z, {u-32, v-32});
        sprintf(z, "%llX", a.b);
        blit3(z, {u+32, v-32});
        sprintf(z, "%llX", a.c);
        blit3(z, {u+32, v+32});
        sprintf(z, "%llX", a.d);
        blit3(z, {u-32, v+32});

    }

    if (_selected_opcode) {
        gl::vec2 offset(-16, -16);
        using namespace instruction;
        blit5(translate(_selected_opcode), selectee * 64 - _camera_position);
        blit5(_translate[((_selected_opcode) & 7) + _opcode_enum_size + 16], selectee * 64 - _camera_position);
        
    } else {
        blit5("reticule", selectee * 64 - _camera_position);
    }

    for (i64 i = 0; i != instruction::_opcode_enum_size; ++i) {
        string_view v = translate(instruction::opcode((instruction::opcode_enum) i));
        blit4("button", {i * 64, _height - 64});
        blit4(v, {i * 64, _height - 64});
    }
             
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
        scribe(s, {_font.charmap[' '].advance, _font.height});
    }
    
    scribe("Falsches Üben von Xylophonmusik quält jeden größeren Zwerg", { _width / 2, + _font.height });
}

void game::key_down(u32 c) {
    application::key_down(c);
    
    gl::vec2 world_mouse{
        _mouse_window.x * 2 + _camera_position.x,
        - _mouse_window.y * 2 + _height + _camera_position.y};
    
    gl::vec2 selectee(((i64) world_mouse.x) >> 6, ((i64) world_mouse.y) >> 6);
    i64 i = selectee.x;
    i64 j = selectee.y;
    
    switch (c) {
        case 'r':
            if (_selected_opcode) {
                ++_selected_opcode;
            } else {
                ++_thing._board(i, j);
            }
            break;
        case 'R':
            if (_selected_opcode) {
                --_selected_opcode;
            } else {
                --_thing._board(i, j);
            }
            break;
        case 'q': {
            mcu m(selectee.x, selectee.y, 0, 0);
            _thing._mcus.push_back(m);
        }
            
        default:
            break;
    }
}



void game::mouse_down(u64 x) {
    application::mouse_down(x);
    if (x != 1)
        return;
    if (_selected_opcode) {
        _selected_opcode = 0;
    } else {
        gl::vec2 world_mouse(_mouse_window.x * 2 + _camera_position.x,
                             - _mouse_window.y * 2 + _height + _camera_position.y);
        gl::vec2 selectee(((i64) world_mouse.x) >> 6, ((i64) world_mouse.y) >> 6);
        _thing._board(selectee.x, selectee.y) = 0;
    }
}

void game::mouse_up(u64 x) {
    application::mouse_up(x);
    if (x != 0)
        return;
    gl::vec2 _screen_mouse(_mouse_window.x * 2,
                           - _mouse_window.y * 2 + _height);
    if (_screen_mouse.y > _height - 64) {
        i64 j = _screen_mouse.x;
        j >>= 6;
        if (j < instruction::_opcode_enum_size) {
            _selected_opcode = instruction::opcode((instruction::opcode_enum) j);
        }
    } else {
        if (_selected_opcode) {
            gl::vec2 world_mouse(_mouse_window.x * 2 + _camera_position.x,
                                 - _mouse_window.y * 2 + _height + _camera_position.y);
            gl::vec2 selectee(((i64) world_mouse.x) >> 6, ((i64) world_mouse.y) >> 6);
            _thing._board(selectee.x, selectee.y) = _selected_opcode;
            std::cout << "writing" << std::hex << _selected_opcode << std::endl;
        }
    }
}



void game::scrolled(double x, double y) {
    application::scrolled(x, y);
    _camera_position.x -= x;
    _camera_position.y -= y;
}

} // namespace manic
