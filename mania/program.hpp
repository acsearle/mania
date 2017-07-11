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

#include <OpenGL/gltypes.h>

namespace gl {
    
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
        
    }; // program
    
} // namespace gl

#endif /* program_hpp */
