//
//  surface.hpp
//  mania
//
//  Created by Antony Searle on 20/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef surface_hpp
#define surface_hpp

#include <map>
#include <memory>

#include "chunk.hpp"
#include "table.hpp"
#include "vec.hpp"

namespace manic {
    
    struct surface
    : table<gl::vec<ptrdiff_t, 2>, chunk> {
        
        void instantiate(ptrdiff_t i, ptrdiff_t j);
        
    };
    
};

#endif /* surface_hpp */
