//
//  world.cpp
//  mania
//
//  Created by Antony Searle on 26/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "elements.hpp"
#include "world.hpp"

namespace manic {

world::world()
: _next_insert(0)
, _terrain(_terrain_generator(0)) {

    push_back(new mine(4, 8, element::carbon));
    push_back(new mine(8, 8, element::hematite));
    push_back(new smelter(12, 8));
    push_back(new silo(16, 8));
    
}

void world::tick() {
    for (entity* p : _entities[counter & 63]) {
        assert(p);
        p->tick(*this);
    }
    ++counter;
}

void world::push_back(entity* p) {
    instruction::occupy(_board({p->x, p->y}));
    _entities[_next_insert & 63].push_back(p);
    _next_insert += 5 * 5;
}

void world::did_exit(i64 i, i64 j, u64 d) {
    _terrain({i, j}) = 2 + !(d & 1);
}


}
