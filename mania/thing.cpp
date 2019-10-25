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
    
    _mcus.push_back(mcu(8, 8, 0x10)); // mcu at centre, heading north, primed for 4 loops
    
    _board(8, 7) = 0xF0; // fork
    _board(8, 6) = 0x80; // conditional ccw
    _board(6, 6) = 0x41; // sub sw, turning ccw
    _board(7, 7) = 0x01; // value to sub
    _board(6, 9) = 0x41; // sub se, turning ccw
    _board(7, 10) = 0x01; // value to sub
    _board(8, 9) = 0x42; // sub sw, turning ccw
    
    _board(8, 1) = 0xD0; // terminate after escaping loop
    
    _board(9, 4) = 0xE0; // flip-flop branches the spawns
    _board(7, 4) = 0xBD; // turn
    _board(7, 3) = 0x32; // add
    _board(6, 4) = 0x08; // add
    _board(7, 2) = 0x21; // store
    _board(7, 1) = 0xCD; // turn
    
    _board(7, 1) = 0xD0; // terminate spawns
    _board(14, 4) = 0xD0; // terminate spawns
    
    _board(12, 3) = 0x020; // store
    
    _board(9, 2) = 0x02; // store
    
    _board(14, 6) = 0xD0; // terminate last wild run
    
    
    
    
    
}

void world::exec(mcu& x) {
    
    // instructions are opcode:target
    // target for operation may be the cell NE SE SW NW or the register
    // itself, or ignored
    
    u8 u = x.x & 0xF;
    u8 v = x.y & 0xF;
    u8 instruction = _board(u, v);
    u8 opcode = instruction >> 4;
    u8 target = instruction & 15;
    u8* p = nullptr;
    
    // resolve what we are operating on - a nearby cell or a register
    switch (target) {
            // a diagonally adjacent cell of the board
        case 0x0:
            ++u; --v; p = &_board(u, v); // NE
            break;
        case 0x1:
            ++u; ++v; p = &_board(u, v); // SE
            break;
        case 0x2:
            --u; ++v; p = &_board(u, v); // SW
            break;
        case 0x3:
            --u; --v; p = &_board(u, v); // NW
            break;
        case 0xD:
            p = &x.d;
            break;
        default:
            
            break;
    }
    
    // perform the operation
    switch (opcode) {
        case 0x0: // NOOP
            
            break;
        case 0x1: // LOAD
            x.d = *p;
            break;
        case 0x2: // STORE
            *p = x.d;
            break;
        case 0x3: // ADD
            x.d += *p;
            break;
        case 0x4: // SUB
            x.d -= *p;
            break;
        case 0x5: // AND
            x.d &= *p;
            break;
        case 0x6: // OR
            x.d |= *p;
            break;
        case 0x7: // XOR
            x.d ^= *p;
            break;
        case 0x8: // SATURATING DECREMENT (TURN CCW IF DIRECTION NONZERO)
            if (x.d)
                --x.d;
            break;
        case 0x9: // flip
            ++x.d;
            _board(x.x & 0xF, x.y & 0xF) = 0xE0;
            break;
        case 0xA: // SWAP
            std::swap(x.d, *p);
            break;
        case 0xB: // INC
            ++*p;
            break;
        case 0xC: // DEC
            --*p;
            break;
        case 0xD: // DIE
            _died.push_back(&x - _mcus.begin());
            break;
        case 0xE: // flop
            --x.d;
            _board(x.x & 0xF, x.y & 0xF) = 0x90;
            break;
        case 0xF: { // FORK INTO DIAGONALLY ADJACENT CELL
            mcu y(x); y.x = u; y.y = v; _born.push_back(y);
        } break;
            
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
            printf("%2X ", _board(i, j));
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
