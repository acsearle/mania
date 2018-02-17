//
//  bit.cpp
//  mania
//
//  Created by Antony Searle on 26/12/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include <cassert>

#include "bit.hpp"

namespace mania {
    
    struct dfgh {
        dfgh();
    } ghh;
    
    dfgh::dfgh() {
        
        int a = 0xAAAAAAAA;
        
        bit_cptr b = &a;
        
        assert(b[0] == !b[1]);
        
        
        
    }
    
    
    
} // namespace mania
