//
//  renderer.hpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef renderer_hpp
#define renderer_hpp

#include <string>
#include <OpenGL/gltypes.h>

std::string load(std::string name, std::string ext);

class renderer {
public:
    renderer();
    void resize(GLsizei width, GLsizei height);
    void render();
};

#endif /* renderer_hpp */
