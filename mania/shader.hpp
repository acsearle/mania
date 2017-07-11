//
//  shader.hpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef shader_hpp
#define shader_hpp

#include <string>
#include <OpenGL/gltypes.h>

namespace gl {
    
    class shader {
        
        GLuint _name;
        
    public:
        
        shader() = delete;
        shader(const shader&) = delete;
        shader(shader&& r);
        ~shader();
        shader& operator=(const shader&) = delete;
        shader& operator=(shader&& r);
        
        explicit shader(GLenum type);
        operator GLuint() const;
        
        shader& source(const char* s);
        shader& source(const std::string& s);
        
        shader& debug();
        
        shader& compile();
        
    }; // class shader
    
} // namespace gl


#endif /* shader_hpp */
