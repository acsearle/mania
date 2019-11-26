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
: counter(0)
, _terrain(_terrain_generator(0)) {

    push_back(new mine(4, 8, element::carbon));
    push_back(new mine(8, 8, element::hematite));
    push_back(new smelter(12, 8));
    push_back(new silo(16, 8));
}

void world::tick() {
    if (vector<entity*>* a = _waiting_on_time.try_get(counter)) {
        vector<entity*> b;
        while (a->size()) {
            assert(b.empty());
            swap(*a, b);
            assert(a->empty());
            while (!b.empty())
                b.pop_front()->tick(*this);
            // entities may have enqueued themselves at the current time, so
            // we have more work to do, and they may have enqueued themselves
            // at other times, triggering a table resize, so we must find the
            // bin again
            a = &_waiting_on_time[counter];
        }
        // no more work at this time
        assert(_waiting_on_time[counter].empty());
        _waiting_on_time.erase(counter);
    }
    // advance the time
    ++counter;
}

void world::write(vec<i64, 2> xy, u64 v) {
    this->_board(xy) = v;
    vector<entity*>* a = _waiting_on_write.try_get(xy);
    if (a) {
        _waiting_on_time[counter].append(a->begin(), a->end());
        _waiting_on_write.erase(xy);
    }
}

u64 world::read(vec<i64, 2> xy) {
    return this->_board(xy);
}

void world::push_back(entity* p) {
    // register for drawing
    _entities.push_back(p);
    // register for immediate execution
    wait_on_time(counter, p);
    // occupy cell, potentially enqueuing entities waiting on that cell
    vec<i64, 2> vq = {p->x, p->y};
    u64 q = read(vq);
    instruction::occupy(q);
    write(vq, q);
}

void world::did_exit(i64 i, i64 j, u64 d) {
    // make tracks
    _terrain({i, j}) = 2 + !(d & 1);
}

void world::wait_on_time(u64 t, entity* p) {
    assert(p);
    this->_waiting_on_time.get_or_insert_with(t, []() {
        return vector<entity*>{};
    }).push_back(p);
    p->t = t;
}

void world::wait_on_write(vec<i64, 2> x, entity* p) {
    this->_waiting_on_write.get_or_insert_with(x, [](){
        return vector<entity*>{};
    }).push_back(p);
}


}