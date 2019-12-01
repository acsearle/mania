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
    //glClearColor(1.0, 0.0, 1.0, 1.0);
    //glClear(GL_COLOR_BUFFER_BIT);
    _pane->draw(nullptr);
    
}

void application::key_up(manic::u32 c) {
    //std::cout << "key_up: '" << c << "'" << std::endl;
    if (_event_proxy._keyboard_state.contains(c))
        _event_proxy._keyboard_state.erase(c);
    _pane->key_up(c, &_event_proxy);
}

void application::key_down(manic::u32 c) {
    //std::cout << "key_down: '" << c << "'" << std::endl;
    _event_proxy._keyboard_state.insert(c, true);
    _pane->key_down(c, &_event_proxy);
}

void application::mouse_moved(double x, double y) {
    //std::cout << "mouse_moved: " << x << ", " << y << std::endl;
    _event_proxy._mouse_window.x = x * 2;
    _event_proxy._mouse_window.y = _pane->_ext.b.y - y * 2;
    _pane->mouse_move(&_event_proxy);
}

void application::mouse_down(manic::u64 x) {
    //std::cout << "mouse_down: " << x << std::endl;
    _pane->mouse_down(x, &_event_proxy);
}

void application::mouse_up(manic::u64 x) {
    //std::cout << "mouse_up: " << x << std::endl;
    _pane->mouse_up(x, &_event_proxy);
}

void application::scrolled(double x, double y) {
    // std::cout << "scrolled: " << x << ", " << y << std::endl;
    _pane->scrolled({x, y}, &_event_proxy);
}

void application::resize(usize width, usize height) {
    _pane->resize(rect<f32>{{0,0},{width,height}});
}


}
