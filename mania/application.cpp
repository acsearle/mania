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
    glClearColor(1.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
}

void application::key_up(manic::u32 c) {
    std::cout << "key_up: '" << c << "'" << std::endl;
    if (_keyboard_state.contains(c)) {
        _keyboard_state.erase(c);
    }
}

void application::key_down(manic::u32 c) {
    std::cout << "key_down: '" << c << "'" << std::endl;
    _keyboard_state.insert(c, true);
}

void application::mouse_moved(double x, double y) {
    std::cout << "mouse_moved: " << x << ", " << y << std::endl;
    _mouse_window.x = x;
    _mouse_window.y = y;
}

void application::mouse_down(manic::u64 x) {
    std::cout << "mouse_down: " << x << std::endl;
}

void application::mouse_up(manic::u64 x) {
    std::cout << "mouse_up: " << x << std::endl;
}

void application::scrolled(double x, double y) {
    std::cout << "scrolled: " << x << ", " << y << std::endl;
}

}
