//
//  draw_proxy.cpp
//  mania
//
//  Created by Antony Searle on 26/12/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "animation.hpp"
#include "asset.hpp"
#include "atlas.hpp"
#include "draw_proxy.hpp"
#include "program.hpp"
#include "string.hpp"
#include "text.hpp"
#include "vector.hpp"

namespace manic {

struct draw_proxy_concrete : draw_proxy {
    
    id<MTLDevice> _device;
    // gl::program _program;
    font _font;
    atlas _atlas;
    table3<string, sprite> _assets;
    vector<sprite> _animation_h;
    vector<sprite> _animation_v;
    vector<sprite> _buildings;
    vector<sprite> _terrain;
    sprite _ui_rect;
    sprite _solid;
    
    explicit draw_proxy_concrete(id<MTLDevice> device);

    virtual ~draw_proxy_concrete() = default;
    
    virtual void draw_frame(rect<f32>) override;
    virtual void draw_text(rect<f32>, string_view v) override;
    virtual rect<f32> bound_text(string_view v) override;

    virtual void draw_asset(vec2, string_view) override;
    virtual void draw_terrain(vec2, int i) override;
    virtual void draw_animation_h(vec2, int i) override;
    virtual void draw_animation_v(vec2, int i) override;

    virtual void draw_sprite(vec2, sprite s) override;

    virtual void presize(vec2) override;
    virtual void commit(id<MTLRenderCommandEncoder> renderEncoder) override;
    
    virtual void signal() override;
    
};

draw_proxy& draw_proxy::get(id<MTLDevice> device) {
    static draw_proxy_concrete x(device);
    return x;
}


draw_proxy_concrete::draw_proxy_concrete(id<MTLDevice> device)
: _device(device)
    // , _program("basic")
, _atlas(4192, device) {
    _assets = load_asset("symbols", _atlas);
    _font = build_font(_atlas);
    {
        // Put a white pixel on the texture so we can render solid blocks of color
        pixel p{255, 255, 255, 255};
        _solid = _atlas.place(const_matrix_view<pixel>(&p, 1, 1, 1));
        auto midpoint = (_solid.a.texCoord + _solid.b.texCoord) / 2.0f;
        _solid.a.texCoord = _solid.b.texCoord = midpoint;
    }
    _ui_rect = load_image("frame", _atlas);
    //std::cout << load("enum", "hpp") << std::endl;
    
    _animation_h = load_animation(_atlas, "stepper", {1, 0}, 32);
    _animation_v = load_animation(_atlas, "stepperv", {0, (float)(1.0/sqrt(2.0))}, 32);
    
    _buildings = load_animation(_atlas, "silo", {0,0}, 5);
    _terrain = load_animation(_atlas, "terrain", {0, 0}, 16);

    _background = load_image("curioso-photography", _atlas);
    
    /*
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
*/
    
}




void draw_proxy_concrete::draw_text(rect<f32> x, string_view v) { // ::scribe(string_view v, vec2 xy) {
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
    //x.a.x = 0; //+= _font.charmap[' '].advance;
    //x.a.y = 0; //+= _font.height;
    auto xy = x.a;

    pixel col = { 255, 255, 255, 255 };
    // char const* p = v.data();
    while (v) {
        u32 c = *v; ++v;
        //if (c == '\n') {
        //    xy.x = x.a.x;
        //    xy.y += _font.height;
        //} else {
            if (auto* q = _font.charmap.try_get(c)) {
                sprite s = q->sprite_;
                s.a.color = col;
                s.b.color = col;
                _atlas.push_sprite_translated(s, xy);
                xy.x += q->advance;
            }
        //}
    }
}

rect<f32> draw_proxy_concrete::bound_text(string_view v) {
    /*
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
     */
    
    // This is a loose bound that prefers vertical stability to tightness;
    
    rect<f32> xy{{0, -_font.ascender},{0, -_font.descender}};
    while (v) {
        u32 c = *v; ++v;
        if (auto* p = _font.charmap.try_get(c)) {
            xy.b.x += p->advance;
        }
    }
    return xy;
}

void draw_proxy_concrete::draw_asset(vec2 xy, string_view v) {
    if (auto p = _assets.try_get(v)) {
        _atlas.push_sprite_translated(*p, xy);
    }
}

void draw_proxy_concrete::draw_terrain(vec2 xy, int i) {
    _atlas.push_sprite_translated(_terrain[i], xy);
}

void draw_proxy_concrete::draw_sprite(vec2 v, sprite s) {
    _atlas.push_sprite_translated(s, v);
}


void draw_proxy_concrete::draw_animation_h(vec2 xy, int i) {
    _atlas.push_sprite_translated(_animation_h[i], xy);
}

void draw_proxy_concrete::draw_animation_v(vec2 xy, int i) {
    _atlas.push_sprite_translated(_animation_v[i], xy);
}

void draw_proxy_concrete::presize(vec2 ext) {
    // glViewport(0, 0, (GLsizei) ext.x, (GLsizei) ext.y);
    
    GLfloat transform[16] = {
        (float) 2.0f/ext.x, 0, 0, -1,
        0, - (float) 2.0f/ext.y, 0, +1,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    //_camera_position.x += sin(frame * 0.1);
    //_camera_position.y += cos(frame * 0.1);

    // glUniformMatrix4fv(glGetUniformLocation(_program, "transform"), 1, GL_TRUE, transform);

}

void draw_proxy_concrete::commit(id<MTLRenderCommandEncoder> renderEncoder) {
    _atlas.commit(renderEncoder);
}



void draw_proxy_concrete::draw_frame(rect<float> r) {
    
    r.a += 2.0f;
    r.b -= 2.0f;
    
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

    void draw_proxy_concrete::signal() {
        dispatch_semaphore_signal(_atlas._semaphore);
    }
    
} // namespace manic


