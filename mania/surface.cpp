//
//  surface.cpp
//  mania
//
//  Created by Antony Searle on 20/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "surface.hpp"

namespace manic {
    
    void surface::instantiate(ptrdiff_t i, ptrdiff_t j) {
        get_or_insert_with(key_type(i, j), [=]() { return chunk(i, j); });
    }
    
};
