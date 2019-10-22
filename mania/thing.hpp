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
#include "vector.hpp"

namespace manic {

struct mcu {
    
    // MCU has a 2d location in a plane of memory cells, instead of a function
    // pointer into 1d memory
    
    u8 x;
    u8 y;
    
    // MCU has a single register currently.  Last two bits encode direction it
    // moves (NESW).  One register (and 8 bit opcodes) may not be sufficient
    
    u8 d;
    
    // Each tick, the MCU reads its instruction from the cell it is "at".  The
    // instruction may address only the diagonally adjacent cells for read write
    // etc.
    //
    // Code and data are mingled together, but diagonal access means it is easy
    // to keep them locally separate on two complementary grids.
    
    mcu(u8 x_, u8 y_, u8 d_) : x(x_), y(y_), d(d_) {}
    
};

inline std::ostream& operator<<(std::ostream& s, const manic::mcu& x) {
    return (s << "{ " << (int) x.x << ", " << (int)x.y << ", " << (int) x.d << " }");
    }
    
struct world {
    
    // State of world
    
    matrix<u8> _board; // 2d grid of memory cells
    vector<mcu> _mcus; // list of entities
    
    // The world is a 2d grid of 8-bit memory locations.
    
    // Impl. details
    
    vector<size_t> _died; // indices of entities marked for death
    vector<mcu> _born; // entities born this tick
    matrix<u32> _rgba; // crappy visualization texture

        
    world() {
        
        _board.resize(16, 16);
        _rgba.resize(16, 16);
        
        _mcus.push_back(mcu(8, 8, 0x10)); // mcu at centre, heading north, primed for 4 loops
        
        _board(8, 7) = 0xF0; // fork
        _board(8, 6) = 0x80; // conditional ccw
        _board(6, 6) = 0x41; // sub sw, turning ccw
        _board(7, 7) = 0x01; // value to sub
        _board(6, 9) = 0x41; // sub se, turning ccw
        _board(7, 10) = 0x01; // value to sub
        _board(8, 9) = 0x42; // sub sw, turning ccw
        
        _board(8, 1) = 0xD0; // terminate after escaping loop
        _board(9, 1) = 0xD0; // terminate spawns
        
        
        //               dd
        //
        //
        //             - ?
        //              1F
        //               m
        //             - -
        //              1
        //
        
        
        
        
    }
    
    void exec(mcu& x) {
        
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
            case 0xF: { // FORK INTO DIAGONALLY ADJACENT CELL
                mcu y(x); y.x = u; y.y = v; _born.push_back(y);
            } break;

        }
    }
    
    void tick() {
        
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
        
        // make texture
        for (u8 i = 0; i != 16; ++i)
            for (u8 j = 0; j != 16; ++j)
                _rgba(i, j) = _board(i, j);
        for (mcu& x : _mcus) {
            u8 u = x.x & 0xF;
            u8 v = x.y & 0xF;
            _rgba(u, v) |= 0x00FF0000;
        }
        
    }
    
    static u32 to_color(u8 n) {
        u32 c = (n & 0xF0) | ((n & 0x0F) << 12) | 0xFF000000;
        return c;
    }
    
    void print_world() {
        for (ptrdiff_t j = 0; j != 16; ++j) {
            for (ptrdiff_t i = 0; i != 16; ++i) {
                printf("%2X ", _board(i, j));
            }
            printf("\n");
        }
                
                
    }
    
    void simulate() {

        print_world();

        while (_mcus.size()) {
        
            for (auto& x : _mcus)
                std::cout << x;
            std::cout << std::endl;
            // breakpoint here
             tick();
            
        }
    }
    
    
}; // struct world


} // namespace manic



#endif /* thing_hpp */
