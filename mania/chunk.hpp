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
#include "terrain.hpp"
#include "vec.hpp"

namespace manic {
    
    struct chunk {
        
        isize _i;
        isize _j;
        static const isize _rows = 16;
        static const isize _columns = 16;
        
        matrix<u8> _tiles;
        
        std::vector<gl::vec<f32, 2>> _entities;
        
        chunk(isize i_, isize j_);
        
    };
    
}


#endif /* chunk_hpp */
