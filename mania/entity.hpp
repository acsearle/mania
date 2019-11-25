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
#include "space2.hpp"
#include "vector.hpp"
#include "terrain2.hpp"

#include "rleq.hpp"

#include "instruction.hpp"

namespace manic {

// Base class for things that have a location and tick (will anything not have
// a location, but tick?)

struct world;

struct entity {
            
    i64 x;
    i64 y;

    entity(i64 x_, i64 y_)
    : x(x_)
    , y(y_) {
    }
    
    virtual ~entity() = default;
    
    virtual void tick(world&) = 0;
    
    // Consier if virtual functions are good enough, or if we want to implement
    // an explicit system of switch-on-type, or if we want to look at an entity
    // component system

};

struct mcu : entity {
    
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
    
    u64 s;
    
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
    
    mcu(i64 x, i64 y, u64 d_) : entity(x, y), d(d_) {
        s = instruction::newborn;
        a = 0;
        b = 0;
        c = 0;
    }
    
    virtual void tick(world&) override;
    
};

struct mine : entity {
    
    virtual void tick(world&) override;
    
    u64 m;
    
    mine(i64 x, i64 y, u64 m) : entity(x, y), m(m) {}
    
};

struct smelter : entity {
    
    virtual void tick(world&) override;
    
    smelter(i64 x, i64 y) : entity(x, y) {}
    
};

struct silo : entity {
    
    rleq<u64> _queue;
    
    virtual void tick(world&) override;
    
    silo(i64 x, i64 y) : entity(x, y) {}

};

inline std::ostream& operator<<(std::ostream& s, const manic::mcu& x) {
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
    // possible if there are N MCUs in a loop of N nodes, with 2 and 4 node
    // loops (head-on collision and two-lane road intersections) possible)
    
    space2<_space2_default<u64>> _board;
    terrain2 _terrain;
    
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
    usize _next_insert;
    
    // Idea 2: entities spend most of their time waiting (travelling is
    // implemented as waiting and then jumping), wiating either until a point
    // in the future (as when travelling or processing) or waiting on a cell
    // to take on a new value (as in a mutex, barrier, or a blocking read/write
    // to an empty/occupied cell).
    
    table3<u64, vector<entity*>> _waiting_on_time;
    table3<vec<i64, 2>, vector<entity*>> _waiting_on_write;
    
    table3<vec<i64, 2>, vector<entity*>> _entities_in_chunk;
    
    // When waiting on a value, only a few local entities can be waiting?  If
    // many entities are waiting on a value, does it make sense to break down
    // the wait types (wait-for-zero, wait-for-zero-and-will-set-nonzero,
    // wait-for-nonzero, wait-for-nonzero-and-will-set-zero) to avoid stampede
    
    // Make a unified chunk that contains terrain, cells, waiters on cells
    // and entities-within?  Depends on access patterns.  Terrain accesed by
    // rendering but not by entity code.
    
    // Can we execute entities by chunk without losing deterministic ordering?
    
    u64 read(vec<i64, 2> xy);
    void write(vec<i64, 2> xy);
    
    
    void save();
    void load();
    
    
    
    world();
    
    u64 counter = 0;
    
    void tick();
    
    void exec(entity&);
    
    void push_back(entity*);
    
    void did_exit(i64 i, i64 j, u64 d);
    
}; // struct world

} // namespace manic

#endif /* thing_hpp */
