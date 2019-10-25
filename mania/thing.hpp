//
//  thing.hpp
//  mania
//
//  Created by Antony Searle on 22/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef thing_hpp
#define thing_hpp

#include "matrix.hpp"
#include "vector.hpp"

namespace manic {

struct mcu {
    
    // MCU has a 2d location in a plane of memory cells, instead of a function
    // pointer into 1d memory
    
    i64 x;
    i64 y;
    
    // MCU has a single register currently.  Last two bits encode direction it
    // moves (NESW).  One register (and 8 bit opcodes) may not be sufficient
    
    u64 d;
    
    // Each tick, the MCU reads its instruction from the cell it is "at".  The
    // instruction may address only the diagonally adjacent cells for read write
    // etc.
    //
    // Code and data are mingled together, but diagonal access means it is easy
    // to keep them locally separate on two complementary grids.
    
    mcu(u64 x_, u64 y_, u64 d_)
    : x(x_)
    , y(y_)
    , d(d_) {
    }
    
};

inline std::ostream& operator<<(std::ostream& s, const manic::mcu& x) {
    return (s << "{ " << (int) x.x << ", " << (int)x.y << ", " << (int) x.d << " }");
    }
    
struct world {
    
    // State of world
    
    matrix<u64> _board; // 2d grid of memory cells
    vector<mcu> _mcus; // list of entities
    
    // The world is a 2d grid of 64-bit memory locations.
    
    // Impl. details
    
    vector<size_t> _died; // indices of entities marked for death
    vector<mcu> _born; // entities born this tick

        
    world();
    
    void exec(mcu& x);
    
    void tick();
    
    void print_world();
    
    void simulate();
        
}; // struct world


} // namespace manic



#endif /* thing_hpp */
