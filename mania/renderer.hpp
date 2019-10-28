//
//  renderer.hpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef renderer_hpp
#define renderer_hpp

#include <memory>
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gltypes.h>
#include "common.hpp"
#include "table3.hpp"
#include "vec.hpp"
#include "string.hpp"

manic::string load(manic::string_view name, manic::string_view ext);

class renderer {
public:
    renderer() = default;
    renderer(const renderer&) = delete;
    renderer(renderer&&) = delete;
    virtual ~renderer() = default;
    renderer& operator=(const renderer&) = delete;
    renderer& operator=(renderer&&) = delete;
    virtual void resize(GLsizei width, GLsizei height) = 0;
    virtual void render() = 0;
    static std::unique_ptr<renderer> make();
    
    void key_down(manic::u32);
    void key_up(manic::u32);
    
    manic::table3<manic::u32, bool> _keyboard_map;
    
    void mouse_moved(double x, double y);
    gl::vec<double, 2> _mouse;
    
};

#endif /* renderer_hpp */
