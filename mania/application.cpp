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

void application::draw() {
    _pane->draw(rect<f32>{{0,0},{1920,1080}}, nullptr);
    
}

void application::key_up(manic::u32 c) {
    if (_event_proxy._keyboard_state.contains(c))
        _event_proxy._keyboard_state.erase(c);
    _pane->key_up(c, &_event_proxy);
}

void application::key_down(manic::u32 c) {
    _event_proxy._keyboard_state.insert(c, true);
    _pane->key_down(c, &_event_proxy);
}

void application::mouse_moved(double x, double y) {
    _event_proxy._mouse_window.x = x * 2;
    _event_proxy._mouse_window.y = 1080 - y * 2;
    _pane->mouse_move(rect<f32>{{0,0},{1920,1080}},&_event_proxy);
}

void application::mouse_down(manic::u64 x) {
    _pane->mouse_down(rect<f32>{{0,0},{1920,1080}}, x, &_event_proxy);
}

void application::mouse_up(manic::u64 x) {
    _pane->mouse_up(rect<f32>{{0,0},{1920,1080}},x, &_event_proxy);
}

void application::scrolled(double x, double y) {
    _pane->scrolled({x, y}, &_event_proxy);
}

void application::resize(usize width, usize height) {
    //_pane->resize(rect<f32>{{0,0},{width,height}});
}


}
