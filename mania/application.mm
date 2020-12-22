//
//  application.cpp
//  mania
//
//  Created by Antony Searle on 30/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

#include "application.hpp"

namespace manic {

void application::draw(id <MTLRenderCommandEncoder> renderEncoder) {
    draw_proxy::get().presize({_width, _height});
    _pane->draw(rect<f32>{{0,0},{_width,_height}}, &draw_proxy::get());
    draw_proxy::get().commit(renderEncoder);

    
}

void application::key_up(manic::u32 c) {
    if (_event_proxy._keyboard_state.contains(c))
        _event_proxy._keyboard_state.erase(c);
    // _pane->key_up(c, &_event_proxy);
    auto b = box<event::key_up>::from(c);
    rect<f32> e = {{0,0},{_width,_height}};
    _pane->handle_event(e, b._ptr);
}

void application::key_down(manic::u32 c) {
    _event_proxy._keyboard_state.insert(c, true);
    auto b = box<event::key_down>::from(c);
    rect<f32> e = {{0,0},{_width,_height}};
    _pane->handle_event(e, b._ptr);
}

void application::mouse_moved(double x, double y) {
    _event_proxy._mouse_window.x = x * 2;
    _event_proxy._mouse_window.y = 1080 - y * 2;
    //_pane->mouse_move(rect<f32>{{0,0},{1920,1080}},&_event_proxy);
    auto b = box<event::mouse_moved>::from(vec2{x, y});
    rect<f32> e = {{0,0},{_width,_height}};
    _pane->handle_event(e, b._ptr);

}

void application::mouse_down(manic::u64 x) {
    //_pane->mouse_down(rect<f32>{{0,0},{1920,1080}}, x, &_event_proxy);
    auto b = box<event::mouse_down>::from(x);
    rect<f32> e = {{0,0},{_width,_height}};
    _pane->handle_event(e, b._ptr);

}

void application::mouse_up(manic::u64 x) {
    //_pane->mouse_up(rect<f32>{{0,0},{1920,1080}},x, &_event_proxy);
    auto b = box<event::mouse_up>::from(x);
    rect<f32> e = {{0,0},{_width,_height}};
    _pane->handle_event(e, b._ptr);
}

void application::scrolled(double x, double y) {
    //_pane->scrolled({x, y}, &_event_proxy);
    auto b = box<event::scrolled>::from(vec2{x, y});
    rect<f32> e = {{0,0},{_width,_height}};
    _pane->handle_event(e, b._ptr);
}

void application::resize(usize width, usize height) {
    _width = width;
    _height = height;
    draw_proxy::get().presize(vec2(_width, _height));
}


}
