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
    
    // _board.resize(256, 256);
    
    entity* m = entity::make();
    m->x = 8; m->y = 8; m->a = 0x3; m->d = 0x10;
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
    
    if (x.s == 0) {
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
        *q &= ~OBSTRUCTION_FLAG;
    }
    
    // if we are not obstructed, execute the command
    if (x.s == 0) {
        i64 u = x.x;
        i64 v = x.y;
        u64 instruction_ = _board(u, v);
        u64 opcode_ = (instruction_ & OPCODE_MASK) >> OPCODE_SHIFT;
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
        
        // Here, consider switching on the quantity opcode_ | state_
        // Most opcodes want to exec on first runthrough
        // Some opcodes have special behaviour when waiting or blocked
        
        // perform the operation
        switch (opcode_) {
            case noop: // NOOP
                
                break;
            case load: // LOAD target into accumulator
                x.a = *p;
                break;
            case store: // STORE accumulator into target
                *p = x.a;
                break;
            case add: // ADD target to accumulator
                x.a += *p;
                break;
            case sub: // SUB target from accumulator
                x.a -= *p;
                break;
            case bitwise_and: // AND
                x.a &= *p;
                break;
            case bitwise_or: // OR
                x.a |= *p;
                break;
            case bitwise_xor: // XOR
                x.a ^= *p;
                break;
            case decrement: // DEC
                --*p;
                break;
            case decrement_saturate: // SATURATING DECREMENT (TURN CCW IF DIRECTION NONZERO)
                if (*p)
                    --*p;
                break;
            case increment: // INC
                std::cout << "incrementing" << std::endl;
                ++*p;
                break;
            case increment_saturate: // SATURATING INCREMENT (TURN CW IF DIRECTION NONZERO)
                if (~*p)
                    ++*p;
                break;
            case and_complement_of:
                x.a &= ~*p;
                break;
            case flip_decrement: // flip
                --*p;
                _board(x.x, x.y) = opcode(flip_increment, register_d);
                break;
            case flip_increment: // flip
                ++*p;
                _board(x.x, x.y) = opcode(flip_decrement, register_d);
                break;
            case instruction::swap:
                std::swap(x.a, *p);
                break;
            case kill: // DIE
                // _died.push_back(&x - _mcus.begin());
                x.s = 2;
                return; // without moving
                break;
            case fork: { // FORK INTO DIAGONALLY ADJACENT CELL
                // mcu y(x); y.x = u; y.y = v; _born.push_back(y);
            } break;
            case conservative_or:
                x.a |= std::exchange(*p, x.a & *p);
                break;
            case conservative_and:
                x.a &= std::exchange(*p, x.a | *p);
                break;
            case less_than:
                x.d = x.a < *p;
                break;
            case equal_to:
                x.d = (x.a == *p);
                break;
            case clear:
                *p = 0;
                break;
            case compare:
                x.d = (*p < x.a) - (x.a < *p);
                break;
            case dump:
                *q = x.a;
                x.a = 0;
                x.d += 2;
                break;
            case halt:
                x.s = 2;
            default:
                
                break;
                
        }
    } else /* x.s != 0 */ {
        // This is not our first attempt at this block.  For most operations,
        // this means our exit was obstructed by another entity, so we do
        // nothing.  Some operations are blocking, and will enter this state
        // and wait until their condition is satisfied
        
        i64 u = x.x;
        i64 v = x.y;
        u64 instruction_ = _board(u, v);
        u64 opcode_ = (instruction_ & OPCODE_MASK) >> OPCODE_SHIFT;
        switch (opcode_) {
            default:
                // do nothing by default
                break;
        }

        
    }
    
    if (x.s < 2) {
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
        if (*q & OBSTRUCTION_MASK) {
            // The cell is already claimed; set our state to OBSTRUCTED and
            // try again next turn
            x.s = 1;
        } else {
            // step forward
            x.s = 0; // set travelling state
            *q |= OBSTRUCTION_FLAG; // claim the destination cell
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
