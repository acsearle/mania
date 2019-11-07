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

#include "animation.hpp"
#include "application.hpp"
#include "asset.hpp"
#include "atlas.hpp"
#include "debug.hpp"
#include "mat.hpp"
#include "program.hpp"
#include "text.hpp"
#include "entity.hpp"
#include "instruction.hpp"
#include "elements.hpp"
#include "terrain2.hpp"

namespace manic {

struct game : application {
    
    gl::program _program;
    
    usize _width, _height;
        
    font _font;
    
    // manic::atlas3 _tiles;
    atlas _atlas;
    table3<string, sprite> _tiles;
    vector<sprite> _animation_h;
    vector<sprite> _animation_v;
    
    table3<u64, string> _periodic;

    world _thing;
    
    vec<i64, 2> _camera_position;
    string _text;
    
    short _lineheight;
        
    
    u64 _selected_opcode;
        
    game();
    virtual ~game() = default;
    void resize(usize width, usize height) override;
    void draw() override;
    
    void scribe(string_view, vec2 xy);

    void blit3(string_view v, vec2 xy);
    void blit4(string_view v, vec2 xy);
    void blit5(string_view v, vec2 xy);

    virtual void key_down(u32) override;
    virtual void mouse_up(u64) override;
    virtual void mouse_down(u64) override;
    virtual void scrolled(double x, double y) override;

    vec2 bound(string_view v);

    sprite _solid;

}; // struct game

application& application::get() {
    static game x;
    return x;
}

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
    
    //std::cout << load("enum", "hpp") << std::endl;
    
    _animation_h = load_animation(_atlas, "/Users/acsearle/Documents/pov/stepper", {1, 0});
    _animation_v = load_animation(_atlas, "/Users/acsearle/Documents/pov/stepperv", {0, (float)(1.0/sqrt(2.0))});

    
    // _lineheight = manic::build_font(_font, _advances);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glEnable(GL_FRAMEBUFFER_SRGB);
    
    glBindAttribLocation(_program, (GLuint) gl::attribute::position, "position");
    glBindAttribLocation(_program, (GLuint) gl::attribute::texCoord, "texCoord");
    glBindAttribLocation(_program, (GLuint) gl::attribute::color, "color");
    _program.link();
    
    _program.validate();
    _program.use();
    
    _program.assign("sampler", 0);
        
    _camera_position = 0;
    _selected_opcode = 0;
    
    using namespace element;
    _periodic.insert(carbon, "coal");
    _periodic.insert(hematite, "iron_ore");
    _periodic.insert(iron, "iron");

}

void game::resize(usize width, usize height) {
    
    _width = width;
    _height = height;
    
}

void game::blit3(string_view v, vec2 xy) {
    if (auto p = _tiles.try_get(v)) {
        xy.x -= _camera_position.x;
        xy.y -= _camera_position.y;
        _atlas.push_sprite_translated(*p, xy);
    }
}

void game::blit4(string_view v, vec2 xy) {
    if (auto p = _tiles.try_get(v))
        _atlas.push_sprite_translated(*p, xy);
}

void game::blit5(string_view v, vec2 xy) {
    if (auto p = _tiles.try_get(v)) {
        auto s = *p;
        s.a.color *= 0.5f;
        s.b.color *= 0.5f;
        _atlas.push_sprite_translated(s, xy);
    }
}


void game::scribe(string_view v, vec2 xy) {
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

vec2 game::bound(string_view v) {
    vec2 xy{0, _font.ascender - _font.descender};
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
    
    static auto old_t = mach_absolute_time();

    static int frame = 0;
    frame += 1;
    
    //_width = 2048;
    //_height = 2048;
    
    glViewport(0, 0, (GLsizei) _width, (GLsizei) _height);
    
    GLfloat transform[16] = {
        (float) 2.0f/_width, 0, 0, -1,
        0, - (float) 2.0f/_height, 0, +1,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    //_camera_position.x += sin(frame * 0.1);
    //_camera_position.y += cos(frame * 0.1);

    
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

    vec2 world_mouse{
        _mouse_window.x * 2 + _camera_position.x,
        - _mouse_window.y * 2 + _height + _camera_position.y};
    
    vec2 selectee(((i64) world_mouse.x) >> 6, ((i64) world_mouse.y) >> 6);
        
    static const char* _translate_opcode[] = {
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
        "dump",
        "halt",
        "barrier",
        "mutex",
        "greater_than",
        "less_than_or_equal_to",
        "greater_than_or_equal_to",
        "not_equal_to",
        "complement",
        "negate",
        "shift_left",
        "shift_right",
    };

    static const char* _translate_value[] = {
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
    };
    
    static const char* _translate_address[] = {
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
        if (is_instruction(x)) {
            x = opcode_of(x) >> OPCODE_SHIFT;
            return _translate_opcode[x];
        } if (is_item(x)) {
            auto* p = _periodic.try_get(x);
            return p ? p->c_str() : "0";
        } else {
            return _translate_value[(x & 0xF)];
        }
    };
    
    {
        // Draw terrain layer
        i64 x_lo = ((i64) _camera_position.x) >> 6;
        i64 x_hi = ((i64) (_camera_position.x + (i64) _width + 63)) >> 6;
        i64 y_lo = ((i64) _camera_position.y) >> 6;
        i64 y_hi = ((i64) (_camera_position.y + (i64) _height + 63)) >> 6;
        for (i64 x = x_lo; x != x_hi; ++x) {
            for (i64 y = y_lo; y != y_hi; ++y) {
                const char* z = nullptr;
                switch (_thing._terrain({x, y})) {
                    case 0: z = "tile0"; break;
                    case 1: z = "tileF"; break;
                    case 2: z = "tracks_h"; break;
                    case 3: z = "tracks_v"; break;
                    default:
                        assert(false);
                }
                blit3(z, {x*64,y*64});
            }
        }
    }
    
    
    {
        // Draw cell layer
        i64 x_lo = ((i64) _camera_position.x) >> 6;
        i64 x_hi = ((i64) (_camera_position.x + _width + 63)) >> 6;
        i64 y_lo = ((i64) _camera_position.y) >> 6;
        i64 y_hi = ((i64) (_camera_position.y + _width + 63)) >> 6;
        for (i64 x = x_lo; x != x_hi; ++x)
            for (i64 y= y_lo; y != y_hi; ++y) {
                // Perf: look up chunks once and then draw the block
                u64 k = _thing._board({x, y});
                blit3(translate(k), {x * 64, y * 64});
                if (instruction::is_instruction(k)) {
                    string_view u(_translate_address[(k & 0x7)]);
                    blit3(u, {x * 64, y * 64});
                }
            }
    }

    
    // Don't iterate through all MCUs, we only need those that are on visible
    // chunks
    
    // Maintain per-surface-chunk list of MCUs?
    for (u64 i = 0; i != 63; ++i) {
        for (entity* q : _thing._entities[i]) {
            
            // clip this list to screen
            // and draw in proper order (top to bottom, left to right?)

            auto u = q->x * 64;
            auto v = q->y * 64;

            if (mcu* p = dynamic_cast<mcu*>(q)) {
                
                u64 k = 0;
                if (!p->s) {
                    auto f = (i - _thing.counter) & 63;
                    switch (p->d & 3) {
                        case 0:
                            v += f;
                            k = -f;
                            break;
                        case 1:
                            u -= f;
                            k = f;
                            break;
                        case 2:
                            v -= f;
                            k = +f;
                            break;
                        case 3:
                            u += f;
                            k = -f;
                            break;
                    }
                }
                
                if (p->d & 1) {
                    _atlas.push_sprite_translated(_animation_h[k & 31], {u - 96 - _camera_position.x, v - 96 - _camera_position.y});
                } else {
                    _atlas.push_sprite_translated(_animation_v[k & 31], {u - 96 - _camera_position.x, v - 96 - _camera_position.y});
                }
                
                //char z[32];
                //sprintf(z, "house%llX", p->d & 3);
                //blit3(z, {u, v});
                //sprintf(z, "%llX", p->a);
                //blit3(z, {u-32, v-32});
                //sprintf(z, "%llX", p->b);
                //blit3(z, {u+32, v-32});
                //sprintf(z, "%llX", p->c);
                //blit3(z, {u+32, v+32});
                //sprintf(z, "%llX", p->d);
                //blit3(z, {u-32, v+32});
                blit3(translate(p->a), {u, v-24});
                
            } else {
                // some other kind of entity
                blit3("house", {u, v});
            }
        }
    }

    if (_selected_opcode) {
        vec2 offset(-16, -16);
        using namespace instruction;
        blit5(translate(_selected_opcode), selectee * 64 - _camera_position);
        blit5(_translate_address[((_selected_opcode) & 7)], selectee * 64 - _camera_position);
        
    } else {
        blit5("reticule", selectee * 64 - _camera_position);
    }

    for (i64 i = 0; i != _width / 64; ++i) {
        string_view v = translate(instruction::opcode((instruction::opcode_enum) (i << instruction::OPCODE_SHIFT)));
        blit4("button", {i * 64, _height - 64});
        blit4(v, {i * 64, _height - 64});
    }
    
    //_atlas.push_sprite_translated(_animation[frame & 31], {100, 100});

    // With total redraw, clearing may or may not be necessary
    //glClearColor(0.1, 0.0, 0.1, 0.0);
    glClearColor(1, 1, 1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    
    
    /*{
        _atlas.discard();
        _atlas.push_atlas_translated(_camera_position);
        glClearColor(0.5, 0.5, 0.5, 0.5);
        glClear(GL_COLOR_BUFFER_BIT);
    }*/
    
    
    
    for (int i = 0; i != 1; ++i) {
        _thing.tick();
    }

    auto n = _atlas._vertices.size() / 6;

    {
        char s[128];
        auto new_t = mach_absolute_time();
        sprintf(s, "%.2f ms | %lu quads\n%lux%lu\nf%d", (new_t - old_t) * 1e-6, n, _width, _height, frame);
        scribe(s, {_font.charmap[' '].advance, _font.height});
        old_t = new_t;
    }

    _atlas.commit();


}

void game::key_down(u32 c) {
    application::key_down(c);
    
    vec2 world_mouse{
        _mouse_window.x * 2 + _camera_position.x,
        - _mouse_window.y * 2 + _height + _camera_position.y};
    
    vec2 selectee(((i64) world_mouse.x) >> 6, ((i64) world_mouse.y) >> 6);
    i64 i = selectee.x;
    i64 j = selectee.y;
    
    switch (c) {
        case 'r':
            if (_selected_opcode) {
                ++_selected_opcode;
            } else {
                ++_thing._board({i, j});
            }
            break;
        case 'R':
            if (_selected_opcode) {
                --_selected_opcode;
            } else {
                --_thing._board({i, j});
            }
            break;
        case 'q': {
            using namespace instruction;
            entity* p = new mcu(selectee.x, selectee.y, 0);
            if (is_vacant(_thing._board({p->x, p->y}))) {
                _thing.push_back(p);
            }
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
        vec2 world_mouse(_mouse_window.x * 2 + _camera_position.x,
                             - _mouse_window.y * 2 + _height + _camera_position.y);
        vec2 selectee(((i64) world_mouse.x) >> 6, ((i64) world_mouse.y) >> 6);
        _thing._board({selectee.x, selectee.y}) = 0;
    }
}

void game::mouse_up(u64 x) {
    application::mouse_up(x);
    if (x != 0)
        return;
    vec2 _screen_mouse(_mouse_window.x * 2,
                           - _mouse_window.y * 2 + _height);
    if (_screen_mouse.y > _height - 64) {
        i64 j = _screen_mouse.x;
        j >>= 6;
        _selected_opcode = instruction::opcode((instruction::opcode_enum) (j << instruction::OPCODE_SHIFT));
    } else {
        if (_selected_opcode) {
            vec2 world_mouse(_mouse_window.x * 2 + _camera_position.x,
                                 - _mouse_window.y * 2 + _height + _camera_position.y);
            vec2 selectee(((i64) world_mouse.x) >> 6, ((i64) world_mouse.y) >> 6);
            _thing._board({selectee.x, selectee.y}) = _selected_opcode;
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
