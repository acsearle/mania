//
//  world.hpp
//  mania
//
//  Created by Antony Searle on 26/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef world_hpp
#define world_hpp

#include "entity.hpp"
#include "space2.hpp"
#include "terrain2.hpp"
#include "vector.hpp"

namespace manic {

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
    
    space2<_space2_inline<u64>> _board;
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

}

#endif /* world_hpp */
