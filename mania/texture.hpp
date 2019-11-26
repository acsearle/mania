//
//  texture.hpp
//  mania
//
//  Created by Antony Searle on 16/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef texture_hpp
#define texture_hpp

#include <utility>
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

namespace gl {
    
    class texture {
        
        GLuint _name;
        
    public:
        
        texture() {
            glGenTextures(1, &_name);
            assert(_name);
        }
        texture(const texture&) = delete;
        texture(texture&& r) : _name(std::exchange(r._name, 0)) {}
        ~texture() { glDeleteTextures(1, &_name); }
        texture& operator=(const texture&) = delete;
        texture& operator=(texture&& r) { std::swap(_name, r._name); return *this; }
        
        operator GLuint() const { return _name; }
        
        texture& bind(GLenum target) {
            glBindTexture(target, _name);
            return *this;
        }
        
    };
    
} // namespace gl

#endif /* texture_hpp */
