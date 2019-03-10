//
//  vertex.cpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "vertex.hpp"

namespace gl {
    
    void vertex::bind() {
        
        glVertexAttribPointer((GLuint) attribute::position,
                              size<decltype(vertex::position)>,
                              type<decltype(vertex::position)>,
                              GL_FALSE,
                              sizeof(vertex),
                              (void*) offsetof(vertex, position));
        
        glVertexAttribPointer((GLuint) attribute::texCoord,
                              size<decltype(vertex::texCoord)>,
                              type<decltype(vertex::texCoord)>,
                              GL_FALSE,
                              sizeof(vertex),
                              (void*) offsetof(vertex, texCoord));
        
        glEnableVertexAttribArray((GLuint) attribute::position);
        glEnableVertexAttribArray((GLuint) attribute::texCoord);
        
    }

} // namespace gl
