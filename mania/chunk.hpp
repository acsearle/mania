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
        
        ptrdiff_t _i;
        ptrdiff_t _j;
        static const ptrdiff_t _rows = 16;
        static const ptrdiff_t _columns = 16;
        
        matrix<uint8_t> _tiles;
        
        std::vector<gl::vec<float, 2>> _entities;
        
        chunk(ptrdiff_t i_, ptrdiff_t j_);
        
    };
    
}


#endif /* chunk_hpp */
