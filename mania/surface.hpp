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

#include "vec.hpp"
#include "chunk.hpp"

namespace manic {
    
    struct surface {
        
        std::map<gl::vec<ptrdiff_t, 2>, std::unique_ptr<chunk>> _chunks;
        
        chunk* operator()(ptrdiff_t i, ptrdiff_t j) const {
            auto p = _chunks.find(gl::vec<ptrdiff_t, 2>(i, j));
            return p != _chunks.end() ? p->second.get() : nullptr;
        }
        
    };
    
};

#endif /* surface_hpp */
