//
//  program.hpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef program_hpp
#define program_hpp

#include <string>

#include <OpenGL/gl3.h>

#include "vec.hpp"

namespace gl {

    inline void glUniform(GLuint location, GLuint v0) {
        glUniform1ui(location, v0);
    }

    inline void glUniform(GLuint location, GLfloat v0) {
        glUniform1f(location, v0);
    }
    
    inline void glUniform(GLuint location, const vec<GLfloat, 4>& value) {
        glUniform4fv(location, 1, value.data());
    }
    
    class shader;
    
    class program {
        
        GLuint _name;
        
    public:
        
        program();
        program(const program&) = delete;
        program(program&& r);
        ~program();
        program& operator=(const program&) = delete;
        program& operator=(program&& r);
        
        explicit program(const std::string& name);
        operator GLuint() const;
        
        program& attach(const shader& s);
        
        program& debug();
        
        program& link();
        
        program& validate();
        
        program& use();
        
        // attributes and uniforms
        
        void assign(const GLchar* name, vec<float, 4> v) {
            glUniform(glGetUniformLocation(_name, name), v);
        }
        
    }; // program
    
} // namespace gl

#endif /* program_hpp */
