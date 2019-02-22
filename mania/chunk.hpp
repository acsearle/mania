//
//  chunk.hpp
//  mania
//
//  Created by Antony Searle on 20/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef chunk_hpp
#define chunk_hpp

#include "matrix.hpp"
#include "vec.hpp"

namespace manic {
    
    struct chunk
    : matrix<unsigned char> {
        
        chunk(ptrdiff_t i, ptrdiff_t j)
        : matrix<unsigned char>(16,16) {
            
        }
        
    };
    
}


#endif /* chunk_hpp */
