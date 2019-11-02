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
#include "space.hpp"
#include "vector.hpp"

#include "rleq.hpp"

namespace manic {

struct entity {
    
    u64 type; // make the type explicit rather than screw with vtbl id
    
    // MCU has a 2d location in a plane of memory cells, instead of a function
    // pointer into 1d memory
    
    i64 x;
    i64 y;
    
    // MCU has a 'microstate' that allows it to perform operations over multiple
    // cycles.  The most common is being obstructed, where it must wait for
    // the cell ahead to clear before it can move on.  It can also be used to
    // wait for conditions to be met (input cell nonzero, output cell zero)
    // that can be used to construct locks, barriers, wait_load, wait_store.
    // Never use the microstate to smuggle information between cells; this is
    // what the registers are for, and nonzero values have different meanings
    // in different cells / instructions anyway.  The microstate is always
    // zero when entering and leaving a new cell.  Without the microstate, the
    // MCU would always attempt to execute the whole instructon each cycle while
    // blocked.  This is not always desired.
    
    u64 s; // 0: travelling; 1: obstructed; other: instruction defined
    
    // Operations may wait for some conditions to be met, mutate their own state
    // and that of the world, and then wait for more conditions to be met before
    // moving on.  For example, a LOCK instruction waits until a cell is zero,
    // then writes one to it, then proceeds.  A BARRIER instruction decrements
    // a cell, then waits until the cell is zero, then proceeds.
    
    // If MCUs are physical, the operations must also reserve and release cells
    // as they travel through them.  On execution, if state is zero, release
    // the "from" cell indicated by D, perform instruction, then if not
    // waiting, acquire the "to" cell indicated by D.
    
    // MCU has multiple registers.  These let it carry state as it moves around
    //     A:  The accumulator register.  Instructions involving two locations
    //         usually include accumulator as one of them.  For example,
    //         addition is perfromed by adding a value from a memory cell or
    //         register to the accumulator, A = A + *addr
    //     B:  General-purpose register
    //     C:  General-purpose register
    //     D:  The direction register.  The last two bits control the direction
    //         the MCU is travelling (0123 -> NESW).  Conditional instructions
    //         like "<" write their output to the direction register, as in
    //         right-turn-if-less-than D += (A < *addr)
    
    u64 a; // accumulator
    u64 b;
    u64 c;
    u64 d; // direction
    
    // Each tick, the MCU reads its instruction from the cell it is "at".  The
    // instruction may address only the diagonally adjacent cells for read write
    // etc.
    //
    // Code and data are mingled together, but diagonal access means it is easy
    // to keep them locally separate on two complementary grids.
    
    u64 i; // identity, serial number
    
    static entity* make() {
        return (entity*) calloc(1, sizeof(entity));
    }
    
};

inline std::ostream& operator<<(std::ostream& s, const manic::entity& x) {
    return (s << std::hex << "entity{x=" << x.x << ",y=" << x.y << ",a=" << x.a << ",d=" << x.d << "}");
}

struct world {
    
    // State of world
    
    // The world is a 2D grid of memory cells.  Entities are "above" this plane,
    // terrain is "below" (and mostly cosmetic).
    // The interpretation of the value is complex.  It may either be a value
    // or an opcode.  One bit is reserved to indicate if an MCU may enter the
    // cell; MCUs lock and unlock cells using this mechanism as they move
    // around the board, preventing collisions (like Factorio, deadlock is
    // possible if there are N MCUs in a loop of N nodes; unlike Factorio, the
    // MCUs have unit extent and thus cannot deadlock #-shaped intersections)
    
    space<u64> _board;
    
    // MCUs take turns to act on the board.  A queue is the obvious data
    // structure.  However, we want to spread the computational load across
    // each frame, and we want MCUs to move at a predictable speed.
    // * If we exec all MCUs in one frame, load spike
    // * If we dispatch to another CPU, we have to clone data
    // * If we process some one MCU each frame, they get slower as there are
    //   more (even if CPU is not loaded)
    // * If we process some fraction each frame, adding more MCUs makes the
    //   existing ones jump forward a bit
    //
    // Idea: each MCU gets a turn every N (=64) ticks.  There are 64 queues,
    // each MCU lives in one, and each tick executes a whole queue in order.
    // A potential problem is if the queues become unbalanced; balancing them
    // constrains what we can guarantee about the relative order of forked
    // MCUs?
    
    vector<entity*> _entities[64];
    
    world();
    
    u64 counter = 0;
    
    void tick();
    
    void exec(entity&);
    
    
}; // struct world


namespace instruction {

// allocation of word:
// 63: obstruction flag
// 62: instruction flag
// 61: opcode
//   :
// 31: address
//   :
//  2: register flag
//  1: direction
//  0: direction

const u64 OBSTRUCTION_FLAG = 0x8000'0000'0000'0000ull;
const u64 OBSTRUCTION_MASK = 0x8000'0000'0000'0000ull;

const u64 INSTRUCTION_FLAG = 0x4000'0000'0000'0000ull;
const u64 NUMBER_MASK =      0x3FFF'FFFF'FFFF'FFFFull;

const u64 OPCODE_SHIFT = 32;
const u64 OPCODE_MASK = 0x3FFF'FFFF'0000'0000ull;

const u64 ADDRESS_SHIFT = 0x000ull;
const u64 ADDRESS_MASK = 0x0007ull;

const u64 REGISTER_FLAG = 0x0004ull;

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
    dump,
    halt,
    barrier,
    mutex,
    _opcode_enum_size
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

// state enum needs to nicely mesh with the opcode bits.  If the top two
// bits of the u64 are OBSTRUCTION_FLAG and INSTRUCTION_FLAG, neither are
// needed?
enum state_enum : u64 {
    entering = 0x0000'0000,
    waiting = 0x4000'0000,
    exiting = 0x8000'000,
    // newborn?
};

const u64 STATE_MASK = 0xC000'0000;

inline u64 opcode(opcode_enum op, address_enum ad = northeast) {
    return INSTRUCTION_FLAG | (op << OPCODE_SHIFT) | (ad << ADDRESS_SHIFT);
}

} // namespace instruction

} // namespace manic

#endif /* thing_hpp */
