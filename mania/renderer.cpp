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
#include "world.hpp"
#include "pane.hpp"

namespace manic {

struct game : pane {
    
    struct draw_proxy : manic::draw_proxy {
        
        vec2 bound(string_view v);

        gl::program _program;
        font _font;
        atlas _atlas;
        table3<string, sprite> _assets;
        vector<sprite> _animation_h;
        vector<sprite> _animation_v;
        vector<sprite> _buildings;
        vector<sprite> _terrain;
        sprite _ui_rect;
        sprite _solid;
        
        draw_proxy();

        virtual ~draw_proxy() = default;
        
        virtual void draw_frame(rect<f32>) override;
        virtual void draw_text(rect<f32>, string_view v) override;

        virtual void draw_asset(vec2, string_view) override;
        
    } _draw_proxy;
    
    table3<u64, string> _periodic;

    world _thing;
    
    vec<i64, 2> _camera_position;
    string _text;
    
    u64 _selected_opcode;
        
    game();
    virtual ~game() = default;
    //void resize(rect<f32>) override;
    virtual void draw(rect<f32>, manic::draw_proxy*) override;
    
    void scribe(string_view, vec2 xy);

    void blit3(string_view v, vec2 xy);
    void blit4(string_view v, vec2 xy);
    void blit5(string_view v, vec2 xy);

    virtual bool key_down(u32, event_proxy*) override;
    virtual bool mouse_up(rect<f32>, u64, event_proxy*) override;
    virtual bool mouse_down(rect<f32>, u64, event_proxy*) override;
    virtual bool scrolled(vec2, event_proxy*) override;

    /*
    struct draw_proxy : manic::draw_proxy {
        
        explicit draw_proxy(game* p) : _ptr(p) {}
        
        game* _ptr;
        void draw_frame(rect<f32> x) override {
            assert(_ptr);
            _ptr->_draw_frame(x);
        }
        
        virtual void draw_text(rect<f32> x, string_view v) override {
            assert(_ptr);
            _ptr->scribe(v, x.a + vec2{0, _ptr->_font.height});
        }

    };
     */
    
    
}; // struct game

application& application::get() {
    // static game x;
    static application* x = [](){
        application* a = new application;
        a->_pane = new game();
        return a;
    }();
    return *x;
}

game::draw_proxy::draw_proxy()
: _program("basic")
, _atlas(2048) {
    _assets = load_asset("/Users/acsearle/Downloads/textures/symbols", _atlas);
    _font = build_font(_atlas);
    {
        // Put a white pixel on the texture so we can render solid blocks of color
        pixel p{255, 255, 255, 255};
        _solid = _atlas.place(const_matrix_view<pixel>(&p, 1, 1, 1));
        auto midpoint = (_solid.a.texCoord + _solid.b.texCoord) / 2.0f;
        _solid.a.texCoord = _solid.b.texCoord = midpoint;
    }
    _ui_rect = load_image("/Users/acsearle/Downloads/textures/frame", _atlas);
    //std::cout << load("enum", "hpp") << std::endl;
    
    _animation_h = load_animation(_atlas, "/Users/acsearle/Documents/pov/stepper", {1, 0}, 32);
    _animation_v = load_animation(_atlas, "/Users/acsearle/Documents/pov/stepperv", {0, (float)(1.0/sqrt(2.0))}, 32);
    
    _buildings = load_animation(_atlas, "/Users/acsearle/Documents/pov/silo", {0,0}, 5);
    _terrain = load_animation(_atlas, "/Users/acsearle/Documents/pov/terrain", {0, 0}, 16);

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

    
}

game::game() {
        
    using namespace manic;
        
    _camera_position = 0;
    _selected_opcode = 0;
    
    using namespace element;
    _periodic.insert(carbon, "coal");
    _periodic.insert(hematite, "iron_ore");
    _periodic.insert(iron, "iron");

}

//void game::resize(rect<f32> x) {
//    _ext = x;
//}

void game::blit3(string_view v, vec2 x) {
    x -= _camera_position;
    if (auto p = _draw_proxy._assets.try_get(v)) {
        _draw_proxy._atlas.push_sprite_translated(*p, x);
    }
}

void game::blit4(string_view v, vec2 xy) {
    if (auto p = _draw_proxy._assets.try_get(v))
        _draw_proxy._atlas.push_sprite_translated(*p, xy);
}

void game::blit5(string_view v, vec2 xy) {
    if (auto p = _draw_proxy._assets.try_get(v)) {
        auto s = *p;
        s.a.color *= 0.5f;
        s.b.color *= 0.5f;
        _draw_proxy._atlas.push_sprite_translated(s, xy);
    }
}

void game::draw_proxy::draw_asset(vec2 xy, string_view v) {
    if (auto p = _assets.try_get(v)) {
        _atlas.push_sprite_translated(*p, xy);
    }
}


void game::draw_proxy::draw_text(rect<f32> x, string_view v) { // ::scribe(string_view v, vec2 xy) {
    /*
    {
        _solid.a.position = xy;
        _solid.b.position = xy + bound(v);;
        _solid.a.position.y -= _font.ascender;
        _solid.b.position.y -= _font.ascender;
        _solid.a.color = pixel{0, 0, 0, 64};
        _solid.b.color = pixel{0, 0, 0, 192};
        _atlas.push_sprite(_solid);
    }
     */
    auto xy = x.a;
    pixel col = { 255, 255, 255, 255 };
    // char const* p = v.data();
    while (v) {
        u32 c = *v; ++v;
        if (c == '\n') {
            xy.x = x.a.x;
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

vec2 game::draw_proxy::bound(string_view v) {
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


void game::draw(rect<f32> _ext, manic::draw_proxy*) {
    
    static auto old_t = mach_absolute_time();

    static int frame = 0;
    frame += 1;
    
    //_width = 2048;
    //_height = 2048;
    
    glViewport(0, 0, (GLsizei) _ext.b.x, (GLsizei) _ext.b.y);
    
    GLfloat transform[16] = {
        (float) 2.0f/_ext.b.x, 0, 0, -1,
        0, - (float) 2.0f/_ext.b.y, 0, +1,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    //_camera_position.x += sin(frame * 0.1);
    //_camera_position.y += cos(frame * 0.1);

    
    glUniformMatrix4fv(glGetUniformLocation(_draw_proxy._program, "transform"), 1, GL_TRUE, transform);
    
    static double angle = 0.0;
    angle += 0.01;
    
    /*
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
     */

    /*
    vec2 world_mouse{
        _mouse_window.x * 2 + _camera_position.x,
        - _mouse_window.y * 2 + _height + _camera_position.y};
    
    vec2 selectee(((i64) world_mouse.x) >> 6, ((i64) world_mouse.y) >> 6);
      */
    
    // This section is a series of nasty hacks.  In a polished version we want
    // to go directly from integer terrain, instruction etc. codes to sprite
    // indices; vs current intermediate conversion to strings.  To do this it
    // will help to have enum reflection.
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
        //
        // Importantly, this scales with the number of tiles drawn, not the
        // number of chunks instantiated
        i64 x_lo = ((i64) _camera_position.x) >> 6;
        i64 x_hi = ((i64) (_camera_position.x + (i64) _ext.b.x + 63)) >> 6;
        i64 y_lo = ((i64) _camera_position.y) >> 6;
        i64 y_hi = ((i64) (_camera_position.y + (i64) _ext.b.y + 63)) >> 6;
        // Perf: we can lookup chunks and then render subsets of them to save
        // hashtable lookups
        for (i64 x = x_lo; x != x_hi; ++x) {
            for (i64 y = y_lo; y != y_hi; ++y) {
                //const char* z = nullptr;
                /*
                switch (_thing._terrain({x, y})) {
                    case 0: z = "tile0"; break;
                    case 1: z = "tileF"; break;
                    case 2: z = "tracks_h"; break;
                    case 3: z = "tracks_v"; break;
                    default:
                        assert(false);
                }
                 */
                // blit3(z, {x*64,y*64});
                u64 i = hash(vec<i64, 2>{x, y});
                sprite s = _draw_proxy._terrain[i & 15];
                u8 c = _thing._terrain.get({x, y});
                s.b.color = s.a.color = vec4{ c , 128, 0, 255};
                _draw_proxy._atlas.push_sprite_translated(s, vec2{x*64-32, y*64-32} - _camera_position);
            }
        }
    }
    
    
    {
        // Draw cell layer
        //
        // Scales with number of cells, not number of instantiated chunks
        i64 x_lo = ((i64) _camera_position.x) >> 6;
        i64 x_hi = ((i64) (_camera_position.x + _ext.b.x + 63)) >> 6;
        i64 y_lo = ((i64) _camera_position.y) >> 6;
        i64 y_hi = ((i64) (_camera_position.y + _ext.b.y + 63)) >> 6;
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


    {
        // Draw entities
        //
        // Problems:
        //
        // Iterates every entity.  Should only do work proportional to number
        // on screen.
        //
        // Doesn't draw them in correct order.  Should draw by y (for
        // occultation) then by x (for shadows).
        
        int zz = 0;
        for (entity* q : _thing._entities) {
            
            // clip this list to screen
            // and draw in proper order (top to bottom, left to right?)
            
            auto u = q->x * 64;
            auto v = q->y * 64;
            
            if (mcu* p = dynamic_cast<mcu*>(q)) {
                
                u64 k = 0;
                if (!p->s) {
                    auto f = (p->t - _thing.counter) & 63;
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
                    _draw_proxy._atlas.push_sprite_translated(_draw_proxy._animation_h[k & 31], {u - 96 - _camera_position.x, v - 96 - _camera_position.y});
                } else {
                    _draw_proxy._atlas.push_sprite_translated(_draw_proxy._animation_v[k & 31], {u - 96 - _camera_position.x, v - 96 - _camera_position.y});
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
                //blit3("house", {u, v});
                _draw_proxy._atlas.push_sprite_translated(_draw_proxy._buildings[zz % _draw_proxy._buildings.size()], {u - _camera_position.x - 256 + 32, v - _camera_position.y - 256 + 32});
                ++zz;
            }
        }
        
    }
        
    
    
    
    
    {
        /*
        // Draw whatever mouse is holding
        if (_selected_opcode) {
            vec2 offset(-16, -16);
            using namespace instruction;
            blit5(translate(_selected_opcode), selectee * 64 - _camera_position);
            blit5(_translate_address[((_selected_opcode) & 7)], selectee * 64 - _camera_position);
            
        } else {
            blit5("reticule", selectee * 64 - _camera_position);
        }
         */
    }

    /*
    {
        // Draw opcode selector
        for (i64 i = 0; i != _ext.b.x / 64; ++i) {
            string_view v = translate(instruction::opcode((instruction::opcode_enum) (i << instruction::OPCODE_SHIFT)));
            // blit4("button", {i * 64, _height - 64});
            _draw_proxy.draw_frame({{i * 64 + 2, _ext.b.y - 62},{i * 64 + 62, _ext.b.x}});
            blit4(v, {i * 64, _ext.b.y - 64});
        }
    }
     */
    
    
    //_atlas.push_sprite_translated(_animation[frame & 31], {100, 100});
    /*
    {
        pane_text p{rect<f32>{{100,100},{200,200}}, "hello"};
        draw_proxy c(this);
        p.draw(&c);
    }
*/
    /*
    {
        pane_palette p{rect<f32>{{256,256},{256+640,256+64}}};
        p._stuff.resize(10, 1);
        p._stuff(1,0) = "oxygen";
        p._stride = {64, 64};
        p.draw(&_draw_proxy);
    }
    */
    
    
    // With total redraw, clearing may or may not be necessary
    //glClearColor(0.1, 0.0, 0.1, 0.0);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    
    /*
    if (_keyboard_state.contains('p')) {
        _atlas.discard();
        _atlas.push_atlas_translated(_camera_position);
        glClearColor(0.7, 0.5, 0.3, 0.5);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    */
    
    
    for (int i = 0; i != 1; ++i) {
        _thing.tick();
    }

    auto n = _draw_proxy._atlas._vertices.size() / 6;

    {
        char s[128];
        auto new_t = mach_absolute_time();
        sprintf(s, "%.2f ms | %lu quads\n%gxw%g\nf%d", (new_t - old_t) * 1e-6, n, _ext.b.x, _ext.b.y, frame);
        _draw_proxy.draw_text({{_draw_proxy._font.charmap[' '].advance, _draw_proxy._font.height},{_ext.b}}, s);
        old_t = new_t;
    }

    _draw_proxy._atlas.commit();


}

void game::draw_proxy::draw_frame(rect<float> r) {
    
    // Draw UI
    int x = r.a.x;
    int y = r.a.y;
    r.b -= r.a;
    int w = r.b.x;
    int h = r.b.y;
    sprite s = _ui_rect;
    
    float c;
    c = (s.a.position.x + s.b.position.x) / 2.0f;
    float vx[] = { s.a.position.x, c, c + w, s.b.position.x + w };
    c = (s.a.position.y + s.b.position.y) / 2.0f;
    float vy[] = { s.a.position.y, c, c + h, s.b.position.y + h };
    c = (s.a.texCoord.x + s.b.texCoord.x) / 2.0f;
    float vu[] = { s.a.texCoord.x, c, c, s.b.texCoord.x };
    c = (s.a.texCoord.y + s.b.texCoord.y) / 2.0f;
    float vv[] = { s.a.texCoord.y, c, c, s.b.texCoord.y };
    
    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++ j) {
            sprite t = {
                {{vx[i], vy[j]}, {vu[i], vv[j]}, s.a.color},
                {{vx[i+1], vy[j+1]}, {vu[i+1], vv[j+1]}, s.a.color}
            };
            _atlas.push_sprite_translated(t, {x, y});
        }
    
}

bool game::key_down(u32 c, event_proxy* e) {
    // application::key_down(c);
    
    vec2 world_mouse = e->mouse() + _camera_position;

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
            return false;
    }
    return true;
}



bool game::mouse_down(rect<f32> _ext, u64 x, event_proxy* e) {
    if (x != 1)
        return false;
    if (_selected_opcode) {
        _selected_opcode = 0;
    } else {
        vec2 world_mouse = e->mouse() + _camera_position;
        vec2 selectee(((i64) world_mouse.x) >> 6, ((i64) world_mouse.y) >> 6);
        _thing._board({selectee.x, selectee.y}) = 0;
    }
    return true;
}

bool game::mouse_up(rect<f32> _ext, u64 x, event_proxy* e) {
    // application::mouse_up(x);
    if (x != 0)
        return false;
    if (e->mouse().y > _ext.b.y - 64) {
        i64 j = e->mouse().x;
        j >>= 6;
        _selected_opcode = instruction::opcode((instruction::opcode_enum) (j << instruction::OPCODE_SHIFT));
    } else {
        if (_selected_opcode) {
            vec2 world_mouse = e->mouse() + _camera_position;
            vec2 selectee(((i64) world_mouse.x) >> 6, ((i64) world_mouse.y) >> 6);
            _thing._board({selectee.x, selectee.y}) = _selected_opcode;
            std::cout << "writing" << std::hex << _selected_opcode << std::endl;
        }
    }
    return true;
}



bool game::scrolled(vec2 x, event_proxy*) {
    _camera_position -= x;
    return true;
}

} // namespace manic
