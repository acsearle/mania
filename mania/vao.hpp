//
//  vao.hpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef vao_hpp
#define vao_hpp

#include <OpenGL/gltypes.h>

namespace gl {
    
    class vao {
        
        GLuint _name;
        
    public:
        
        vao();
        vao(const vao&) = delete;
        vao(vao&&);
        ~vao();
        vao& operator=(const vao&) = delete;
        vao& operator=(vao&&);
        
        operator GLuint() const;
        
        vao& bind();
        static void unbind();
        
    }; // class vao
    
} // namespace gl

#endif /* vao_hpp */
