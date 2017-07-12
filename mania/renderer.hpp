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
#include <string>

#include <OpenGL/gltypes.h>

std::string load(std::string name, std::string ext);

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
};

#endif /* renderer_hpp */
