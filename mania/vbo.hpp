//
//  vbo.hpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef vbo_hpp
#define vbo_hpp

#include <vector>
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gltypes.h>

namespace gl {
    
    class vbo {
        
        GLuint _name;
        
    public:
        
        vbo();
        vbo(const vbo&) = delete;
        vbo(vbo&&);
        ~vbo();
        vbo& operator=(const vbo&) = delete;
        vbo& operator=(vbo&&);
        
        operator GLuint() const;
        
        vbo& bind(GLenum target);
        static void unbind(GLenum target);
        
        template<typename T>
        static void assign(GLenum target, std::vector<T> data, GLenum usage) {
            glBufferData(target,
                         data.size() * sizeof(T),
                         data.data(),
                         usage);
        }
    }; // class vbo
    
} // namespace gl

#endif /* vbo_hpp */
