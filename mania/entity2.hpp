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
    
    enum : u64 {
        TRUCK = 1,
        MINE,
        SILO,
        SMELTER,
    };
        
    u64 discriminant;
    
    entity2() {
        std::memset(this, 0, sizeof(entity2));
    }
    
    ~entity2() {
        switch (discriminant) {
            case SILO:
                _queue.~rleq<u64>();
                break;
            default:
                break;
        }
    }

    void tick(world&);
    
    void tick_truck(world&);
    void tick_mine(world&);
    void tick_silo(world&);
    void tick_smelter(world&);
    
    
    union {

        struct { // TRUCK
            
            u64 s; // microstate
            u64 a, b, c, d; // registers
            
        };
        
        struct { // MINE
            
            u64 m; // material
            
        };
        
        struct { // SILO
            
            rleq<u64> _queue; // store
            
        };
        
        struct { // SMELTER
            // stateless
        };
        
    };
        
};


} // namespace manic

#endif /* entity2_hpp */
