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
    
    u64 a;
    u64 d;
    
    // Each tick, the MCU reads its instruction from the cell it is "at".  The
    // instruction may address only the diagonally adjacent cells for read write
    // etc.
    //
    // Code and data are mingled together, but diagonal access means it is easy
    // to keep them locally separate on two complementary grids.
    
    mcu(u64 x_, u64 y_, u64 a_, u64 d_)
    : x(x_)
    , y(y_)
    , a(a_)
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


namespace instruction {

// opcodes
//
// opcode_flag     = 0x800000000;
// opcode_cell     = 0x800000000; // ..3
// opcode_register = 0x800000004; // ..7
// opcode_register_a = 0x80000004;
// opcode_register_d = 0x80000007;
// opcode_noop     = 0x800000000;
// opcode_swap
// opcode_sat_dec_d = 0xA0000007;
// fork
// dec d
// inc d

const auto INSTRUCTION_FLAG = 0x8000000000000000ull;
const auto NUMBER_MASK = ~INSTRUCTION_FLAG;

const auto OPCODE_SHIFT = 32;
const auto OPCODE_MASK = ((~0ull) << OPCODE_SHIFT) ^ INSTRUCTION_FLAG;

const auto ADDRESS_SHIFT = 0;
const auto ADDRESS_MASK = ~0ull ^ (OPCODE_MASK | INSTRUCTION_FLAG);

enum opcode_enum : u64 {
    noop,
    load,
    store,
    add,
    sub,
    bitwise_and,
    bitwise_or,
    bitwise_xor,
    decrement,
    decrement_saturate,
    increment,
    increment_saturate,
    flip_increment,
    flip_decrement,
    swap,
    kill,
    fork,
    conservative_or,
    conservative_and,
    less_than,
    equal_to,
    clear,
    compare,
    and_complement_of,
    opcode_enum_size,
};

enum address_enum : u64 {
    northeast = 0,
    southeast,
    southwest,
    northwest,
    register_a,
    register_b,
    register_c,
    register_d,
};
    
inline u64 opcode(opcode_enum op, address_enum ad = northeast) {
    return INSTRUCTION_FLAG | (op << OPCODE_SHIFT) | (ad << ADDRESS_SHIFT);
}

} // namespace instruction


} // namespace manic



#endif /* thing_hpp */
