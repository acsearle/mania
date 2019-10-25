//
//  thing.cpp
//  mania
//
//  Created by Antony Searle on 22/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "thing.hpp"

namespace manic {

world::world() {
    
    _board.resize(16, 16);
    
    _mcus.push_back(mcu(8, 8, 0x3, 0x10)); // mcu at centre, heading north, primed for 4 loops
    
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
    
    // simulate();
    
    
    
}

void world::exec(mcu& x) {
    
    // instructions are opcode:target
    // target for operation may be the cell NE SE SW NW or the register
    // itself, or ignored
    
    using namespace instruction;
    
    i64 u = x.x;
    i64 v = x.y;
    u64 instruction_ = _board(u, v);
    u64 opcode_ = (instruction_ & OPCODE_MASK) >> OPCODE_SHIFT;
    u64 target = instruction_ & ADDRESS_MASK;
    u64* p = nullptr;
    
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
        case register_d:
            p = &x.d;
            break;
        default:
            
            break;
    }
    
    // perform the operation
    switch (opcode_) {
        case noop: // NOOP
            
            break;
        case load: // LOAD
            x.a = *p;
            break;
        case store: // STORE
            *p = x.a;
            break;
        case add: // ADD
            x.a += *p;
            break;
        case sub: // SUB
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
        case decrement_saturate: // SATURATING DECREMENT (TURN CCW IF DIRECTION NONZERO)
            if (*p)
                --*p;
            break;
        case increment_saturate: // SATURATING DECREMENT (TURN CCW IF DIRECTION NONZERO)
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
        case increment: // INC
            ++*p;
            break;
        case decrement: // DEC
            --*p;
            break;
        case kill: // DIE
            _died.push_back(&x - _mcus.begin());
            break;
        case fork: { // FORK INTO DIAGONALLY ADJACENT CELL
            mcu y(x); y.x = u; y.y = v; _born.push_back(y);
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
            
    }
}

void world::tick() {
    
    for (mcu& x : _mcus) {
        exec(x);
    }
    
    // process deaths
    while (_died.size()) {
        size_t i = _died.pop_back();
        _mcus.erase(i);
    }
    
    // process births
    while (_born.size()) {
        _mcus.push_back(_born.pop_front());
    }
    
    // move mcus
    for (mcu& x : _mcus) {
        switch (x.d & 3) {
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

void world::print_world() {
    for (ptrdiff_t j = 0; j != 16; ++j) {
        for (ptrdiff_t i = 0; i != 16; ++i) {
            printf("%2llX ", _board(i, j));
        }
        printf("\n");
    }
    
    
}

void world::simulate() {
    
    print_world();
    
    while (_mcus.size()) {
        
        for (auto& x : _mcus)
            std::cout << x;
        std::cout << std::endl;
        // breakpoint here
        tick();
        
    }
}

}
