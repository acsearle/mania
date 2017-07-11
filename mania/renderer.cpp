//
//  renderer.cpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "renderer.hpp"

#include <cstdlib>

#include <OpenGL/gl3.h>

#include "program.hpp"
#include "shader.hpp"

renderer::renderer() {
    gl::program p("basic");
    // standard bindings
    // set uniforms
    
    
    // vertex stream
    // vertex array object
    // attribute buffers
}

void renderer::resize(GLsizei width, GLsizei height) {
    glViewport(0, 0, width, height);
}

void renderer::render() {
    glClearColor(rand() / (double) RAND_MAX, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
}
