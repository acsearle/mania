//
//  world.hpp
//  mania
//
//  Created by Antony Searle on 26/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef world_hpp
#define world_hpp

#include "entity2.hpp"
#include "space2.hpp"
#include "terrain2.hpp"
#include "vector.hpp"

namespace manic {


// Abstract away if we are mutating the consensus state or the predicted state

struct world_interface {
    
    world_interface() = default;
    world_interface(world_interface const&) = delete;
    virtual ~world_interface() = default;
    world_interface& operator=(world_interface const&) = delete;
    
    virtual u64 read(vec<i64, 2>) const = 0;
    virtual void write(vec<i64, 2>, u64) = 0;
    
    virtual void wait_on_write(vec<i64, 2>, entity2*);
    virtual void wait_on_time(u64, entity2*);
    
};

struct world {

    // Values in mutable cells, cells often empty
    // Values represent numbers, lock/occupancy, etc.
    space2<_space2_inline<u64>> _board;

    // Underlying terrain, every tile occupied
    terrain2 _terrain;
    
    // Bag of entities.
    vector<entity2*> _entities;
    //usize _next_insert;
    
    // Idea 2: entities spend most of their time waiting (travelling is
    // implemented as waiting and then jumping), waiting either until a point
    // in the future (as when travelling or processing) or waiting on a cell
    // to take on a new value (as in a mutex, barrier, or a blocking read/write
    // to an empty/occupied cell).
    
    u64 counter = 0;
    
    table3<u64, vector<entity2*>> _waiting_on_time;
    table3<vec<i64, 2>, vector<entity2*>> _waiting_on_write;
    
    void wait_on_write(vec<i64, 2>, entity2*);
    void wait_on_time(u64, entity2*);

    // table3<vec<i64, 2>, vector<entity2*>> _entities_in_chunk;
    
    // When waiting on a value, only a few local entities can be waiting?  If
    // many entities are waiting on a value, does it make sense to break down
    // the wait types (wait-for-zero, wait-for-zero-and-will-set-nonzero,
    // wait-for-nonzero, wait-for-nonzero-and-will-set-zero) to avoid stampede
    
    // Make a unified chunk that contains terrain, cells, waiters on cells
    // and entities-within?  Depends on access patterns.  Terrain accesed by
    // rendering but not by entity2 code.
    
    // Can we execute entities by chunk without losing deterministic ordering?
    
    u64 read(vec<i64, 2> xy);
    void write(vec<i64, 2> xy, u64 v);
    void _did_write(vec<i64, 2> xy); // hack until we clean up access
    
    world();
        
    void tick();
    
    void exec(entity2&);
    
    void push_back(entity2*);
    
    void did_exit(i64 i, i64 j, u64 d);
    
}; // struct world

template<typename Serializer>
void serialize(world const& x, Serializer& s) {
    serialize(x._board, s);
    serialize(x._terrain, s);
    // entities are hard
    serialize(x.counter, s);
}

template<typename Deserializer>
auto deserialize(placeholder<world>, Deserializer& d) {
    world x;
    x._board = deserialize<decltype(x._board)>(d);
    x._terrain = deserialize<decltype(x._terrain)>(d);

    
    
    x.counter = deserialize<u64>(d);
    return x;
}


}

#endif /* world_hpp */
