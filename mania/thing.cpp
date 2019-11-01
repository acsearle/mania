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
    
    // _board.resize(256, 256);
    
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

    {
        chest c;
        c.x = 12;
        c.y = 12;
        c._queue.push_back(1);
        c._queue.push_back(2);
        c._queue.push_back(3);
        c._queue.push_back(4);
        c._queue.push_back(5);
        _chests.push_back(c);
        
        _board(13, 11) = opcode(decrement, register_d);
        _board(10, 11) = opcode(decrement, register_d);
        _board(10, 14) = opcode(decrement, register_d);
        _board(13, 14) = opcode(decrement, register_d);
        
        _mcus.push_back(mcu{13, 12, 0, 0});
        
        _board(11, 11) = opcode(instruction::swap, southeast);
        _board(12, 14) = opcode(instruction::swap, northwest);

        _mcus.push_back(mcu{10, 12, 0, (~0ull << 1)});


    }

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
    u64* q = nullptr;
    
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
}

void world::tick() {
    
    for (auto& c : _chests) {
        if (_board(c.x - 1, c.y + 1)) {
            c._queue.push_back(_board(c.x - 1, c.y + 1));
            _board(c.x - 1, c.y + 1) = 0;
        }
        if (c._queue.size() && !_board(c.x, c.y))
            _board(c.x, c.y) = c._queue.pop_front();
    }
    
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
    
    /* // With unbounded grid, no need to kill escapees
    vector<mcu> _news;
    for (mcu& x : _mcus) {
        if ((x.x > 0) && (x.y > 0) && (x.x < _board.rows() - 1) && (x.y < _board.columns())) {
            _news.push_back(x);
        }
    }
    swap(_news, _mcus);
     */
    
}

}
