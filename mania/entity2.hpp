//
//  entity2.hpp
//  mania
//
//  Created by Antony Searle on 25/12/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef entity2_hpp
#define entity2_hpp

#include "common.hpp"
#include "rleq.hpp"

namespace manic {

struct world;

struct entity2 {
  
    i64 x;
    i64 y;
    u64 t;

    ~entity2() = delete;
    
    void tick(world&);

    enum : u64 {
        TRUCK = 1,
        MINE,
        SILO,
        SMELTER,
    };
        
    u64 discriminant;
    
    void tick_truck(world&);
    void tick_mine(world&);
    void tick_silo(world&);
    void tick_smelter(world&);
    
    union {

        struct { // TRUCK
            
            u64 s;
            u64 a, b, c, d;
            
        };
        
        struct { // MINE
            
            u64 m;
            
        };
        
        struct { // SILO
            
            rleq<u64> _queue;
            
        };
        
        struct { // SMELTER
        };
        
    };
        
};


} // namespace manic

#endif /* entity2_hpp */
