//
//  thing.cpp
//  mania
//
//  Created by Antony Searle on 22/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "entity.hpp"

namespace manic {

world::world() {
        
    entity* m = entity::make();
    m->x = 8; m->y = 8; m->a = 0x3; m->d = 0x10;
    instruction::occupy(_board(m->x, m->y));
    _entities[0].push_back(m); // mcu at centre, heading north, primed for 4 loops
    
    using namespace instruction;
    
    _board(8, 7) = opcode(fork, northeast); // fork
    _board(8, 6) = opcode(decrement_saturate, register_d);
    _board(6, 6) = opcode(decrement_saturate, register_d);
    _board(6, 9) = opcode(decrement_saturate, register_d);
    _board(8, 9) = opcode(decrement_saturate, register_d);
    
    _board(8, 1) = opcode(kill); // terminate after escaping loop
    
    _board(9, 4) = opcode(flip_decrement, register_d);
    
    //_board(7, 4) = opcode(load, register_d);
    _board(6, 4) = opcode(and_complement_of, southeast);
    _board(7, 5) = 0x1;
    //_board(5, 4) = opcode(store, register_d);
    
    _board(2, 4) = opcode(kill);
    _board(14, 4) = opcode(kill);
    
}

void world::exec(entity& x) {
    
    // instructions are opcode:target
    // target for operation may be the cell NE SE SW NW or the register
    // itself, or ignored
    
    using namespace instruction;
    
    if (x.s == entering) {
        // We are entering the cell for the first time; unobstruct our origin cell
        u64* q = nullptr;
        switch (x.d & 3) {
            case 0:
                q = &_board(x.x, x.y + 1);
                break;
            case 1:
                q = &_board(x.x - 1, x.y);
                break;
            case 2:
                q = &_board(x.x, x.y - 1);
                break;
            case 3:
                q = &_board(x.x + 1, x.y);
                break;
        }
        // bug: this will trigger when we spawn a new entity
        assert(is_occupied(*q)); // we had a lock on our old cell
        assert(!is_conserved(*q)); // we aren't colliding with a physical object
        vacate(*q);
    } else if (x.s == newborn) {
        x.s = entering;
    }
    

    if (x.s != exiting) {
        i64 u = x.x;
        i64 v = x.y;
        u64 instruction_ = _board(u, v);
        u64 opcode_ = instruction_ & OPCODE_MASK;
        u64 target = instruction_ & ADDRESS_MASK;
        u64* p = nullptr;
        u64* q = nullptr;
        
        // Get these addresses on demand from a functor rather than compute everywhere
        
        // resolve what we are operating on - a nearby cell or a register
        switch (target) {
                // a diagonally adjacent cell of the board
            case northeast:
                ++u; --v; p = &_board(u, v); // NE
                break;
            case southeast:
                ++u; ++v; p = &_board(u, v); // SE
                break;
            case southwest:
                --u; ++v; p = &_board(u, v); // SW
                break;
            case northwest:
                --u; --v; p = &_board(u, v); // NW
                break;
            case register_a:
                p = &x.a;
                break;
            case register_b:
                p = &x.b;
                break;
            case register_c:
                p = &x.c;
                break;
            case register_d:
                p = &x.d;
                break;
            default:
                
                break;
        }
        
        switch (x.d & 3) {
            case 0:
                q = &_board(x.x, x.y - 1);
                break;
            case 1:
                q = &_board(x.x + 1, x.y);
                break;
            case 2:
                q = &_board(x.x, x.y + 1);
                break;
            case 3:
                q = &_board(x.x - 1, x.y);
                break;
        }
        
        // perform the operation
        switch (opcode_ | (x.s & MICROSTATE_MASK)) {
                
            case noop:
            default:
                // most values are not opcodes
                // when the opcode is changed while an entity is waiting on it
                x.s = exiting;
                break;

            case load: // LOAD target into accumulator
                x.a = *p;
                x.s = exiting;
                break;

            case store: // STORE accumulator into target
                *p = x.a;
                x.s = exiting;
                break;
                
            case add: // ADD target to accumulator
                x.a += *p;
                x.s = exiting;
                break;
                
            case sub: // SUB target from accumulator
                x.a -= *p;
                x.s = exiting;
                break;
                
            case bitwise_and: // AND
                x.a &= *p;
                x.s = exiting;
                break;
                
            case bitwise_or: // OR
                x.a |= *p;
                x.s = exiting;
                break;
                
            case bitwise_xor: // XOR
                x.a ^= *p;
                x.s = exiting;
                break;
                
            case decrement: // DEC
                --*p;
                x.s = exiting;
                break;
                
            case decrement_saturate: // SATURATING DECREMENT (TURN CCW IF DIRECTION NONZERO)
                if (*p)
                    --*p;
                x.s = exiting;
                break;
                
            case increment: // INC
                ++*p;
                x.s = exiting;
                break;
                
            case increment_saturate: // SATURATING INCREMENT (TURN CW IF DIRECTION NONZERO)
                if (~*p)
                    ++*p;
                x.s = exiting;
                break;
                
            case and_complement_of:
                x.a &= ~*p;
                x.s = exiting;
                break;
                
            case flip_decrement: // flip
                --*p;
                _board(x.x, x.y) = opcode(flip_increment, register_d);
                x.s = exiting;
                break;
                
            case flip_increment: // flip
                ++*p;
                _board(x.x, x.y) = opcode(flip_decrement, register_d);
                break;

            case instruction::swap:
                std::swap(x.a, *p);
                x.s = exiting;
                break;
                
            case conservative_or: // "load bits"?
                x.a |= std::exchange(*p, x.a & *p);
                x.s = exiting;
                break;
                
            case conservative_and: // "store bits"?
                x.a &= std::exchange(*p, x.a | *p);
                x.s = exiting;
                break;
                
            case less_than:
                x.d += x.a < *p;
                x.s = exiting;
                break;

            case greater_than:
                x.d += x.a > *p;
                x.s = exiting;
                break;

            case less_than_or_equal_to:
                x.d += x.a <= *p;
                x.s = exiting;
                break;
                
            case greater_than_or_equal_to:
                x.d += x.a >= *p;
                x.s = exiting;
                break;

            case equal_to:
                x.d += (x.a == *p);
                x.s = exiting;
                break;

            case not_equal_to:
                x.d += (x.a != *p);
                x.s = exiting;
                break;

            case compare:
                x.d = (*p < x.a) - (x.a < *p); // which way around is best?
                x.s = exiting;
                break;
                
            case complement:
                *p = ~*p;
                x.s = exiting;
                break;
                
            case negate:
                *p = -*p;
                break;

            case clear:
                *p = 0;
                x.s = exiting;
                break;
            
            case shift_left:
                x.a <<= *p;
                x.s = exiting;
                break;

            case shift_right:
                x.a >>= *p;
                x.s = exiting;
                break;

            case dump | entering:
                // fallthrough
            case dump | waiting:
                if (*q) {
                    x.s = waiting;
                } else {
                    *q = x.a;
                    x.a = 0;
                    x.d += 2;
                    x.s = exiting;
                }
                break;
                
            case kill:
            case halt:
                x.s = waiting;
                // fallthrough
            case halt | waiting:
            case kill | waiting:
                break;
                
            case barrier | entering:
                if (*p)
                    --*p; // decrement the barrier
                else
                    *p = 1; // we have entered an expired barrier; reinitialize to (2 - 1)
                // fallthrough
            case barrier | waiting:
                x.s = *p ? waiting : exiting;
                break;
                
            case mutex | entering:
                //fallthrough
            case mutex | waiting:
                if (*p) {
                    x.s = waiting;
                } else {
                    *p = 1;
                    x.s = exiting;
                }
                break;
                                
        }
        
    }
    
    if (x.s == exiting) {
        // The operation is complete; attempt to exit the cell
        
        // Locate next cell
        u64* q = nullptr;
        switch (x.d & 3) {
            case 0:
                q = &_board(x.x, x.y - 1);
                break;
            case 1:
                q = &_board(x.x + 1, x.y);
                break;
            case 2:
                q = &_board(x.x, x.y + 1);
                break;
            case 3:
                q = &_board(x.x - 1, x.y);
                break;
        }
        // Check if we can claim it
        if (!is_occupied(*q)) {
            // step forward
            x.s = entering; // set travelling state
            occupy(*q);
            switch (x.d & 3) { // jump into it
                case 0:
                    x.y -= 1;
                    break;
                case 1:
                    x.x += 1;
                    break;
                case 2:
                    x.y += 1;
                    break;
                case 3:
                    x.x -= 1;
                    break;
            }
        }
    }
    
}

void world::tick() {
    for (entity* p : _entities[counter & 63]) {
        exec(*p);
    }
    ++counter;
}

} // namespace manic
