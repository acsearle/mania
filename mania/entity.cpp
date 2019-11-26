//
//  thing.cpp
//  mania
//
//  Created by Antony Searle on 22/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "instruction.hpp"
#include "elements.hpp"
#include "entity.hpp"
#include "world.hpp"

namespace manic {

void mcu::tick(world& _world) {
    
    auto& x = *this;
    auto& _board = _world._board;
    
    // instructions are opcode:target
    // target for operation may be the cell NE SE SW NW or the register
    // itself, or ignored
    
    using namespace instruction;
    
    // Check some invariants
    {
        assert(is_occupied(x.a) == is_conserved(x.a));
        assert(!is_occupied(x.d));
        assert(false
               || ((x.a & FLAGS_MASK) == NUMBER_TAG)
               || ((x.a & FLAGS_MASK) == INSTRUCTION_TAG)
               || ((x.a & FLAGS_MASK) == GHOST_TAG)
               || ((x.a & FLAGS_MASK) == MATERIAL_TAG)
               );
        assert(false
               || ((x.d & FLAGS_MASK) == NUMBER_TAG)
               || ((x.d & FLAGS_MASK) == INSTRUCTION_TAG)
               || ((x.d & FLAGS_MASK) == GHOST_TAG)
               );
    }
    
    // define some helpers
    //
    auto try_mutate = [](u64& x, u64 y) {
        if (!is_conserved(x)) {
            x = (x & ~VALUE_MASK) | (y & VALUE_MASK);
        }
    };
    
    if (x.s == entering) {
        // We are entering the cell for the first time; unobstruct our origin cell
        u64* q = nullptr;
        switch (x.d & 3) {
            case 0:
                q = &_board({x.x, x.y + 1});
                break;
            case 1:
                q = &_board({x.x - 1, x.y});
                break;
            case 2:
                q = &_board({x.x, x.y - 1});
                break;
            case 3:
                q = &_board({x.x + 1, x.y});
                break;
        }
        assert(is_occupied(*q)); // we had a lock on our old cell
        assert(!is_conserved(*q)); // we aren't colliding with a physical object
        vacate(*q);
    } else if (x.s == newborn) {
        x.s = entering;
    }
    

    if (x.s != exiting) {
        i64 u = x.x;
        i64 v = x.y;
        u64 instruction_ = _board({u, v});
        u64 opcode_ = instruction_ & OPCODE_MASK;
        u64 target = instruction_ & ADDRESS_MASK;
        u64* p = nullptr;
        u64* q = nullptr;
        
        // Get these addresses on demand from a functor rather than compute everywhere
        
        // resolve what we are operating on - a nearby cell or a register
        switch (target) {
                // a diagonally adjacent cell of the board
            case northeast:
                ++u; --v; p = &_board({u, v}); // NE
                break;
            case southeast:
                ++u; ++v; p = &_board({u, v}); // SE
                break;
            case southwest:
                --u; ++v; p = &_board({u, v}); // SW
                break;
            case northwest:
                --u; --v; p = &_board({u, v}); // NW
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
                q = &_board({x.x, x.y - 1});
                break;
            case 1:
                q = &_board({x.x + 1, x.y});
                break;
            case 2:
                q = &_board({x.x, x.y + 1});
                break;
            case 3:
                q = &_board({x.x - 1, x.y});
                break;
        }
        
        // perform the operation
        switch (opcode_ | (x.s & MICROSTATE_MASK)) {
                
            case noop:
            default:
                // most values are not opcodes
                // when the opcode is changed while an entity is waiting on it,
                // we default to exiting
                x.s = exiting;
                break;

            case load: // LOAD target into accumulator
                if (!is_conserved(x.a))
                    x.a = ghost_of(*p);
                x.s = exiting;
                break;

            case store: // STORE accumulator into target
                if (!is_conserved(*p))
                    *p = (*p & ~OCCUPIED_FLAG) | ghost_of(x.a);
                x.s = exiting;
                break;
                
            case add: // ADD target to accumulator
                try_mutate(x.a, x.a + *p);
                x.s = exiting;
                break;
                
            case sub: // SUB target from accumulator
                try_mutate(x.a, x.a - *p);
                x.s = exiting;
                break;
                
            case bitwise_and: // AND
                try_mutate(x.a, x.a & *p);
                x.s = exiting;
                break;
                
            case bitwise_or: // OR
                try_mutate(x.a, x.a | *p);
                x.s = exiting;
                break;
                
            case bitwise_xor: // XOR
                try_mutate(x.a, x.a ^ *p);
                x.s = exiting;
                break;
                
            case decrement: // DEC
                try_mutate(*p, *p - 1);
                x.s = exiting;
                break;
                
            case decrement_saturate: // SATURATING DECREMENT (TURN CCW IF DIRECTION NONZERO)
                if (!is_conserved(*p) && (*p & VALUE_MASK))
                    *p = (*p & FLAGS_MASK) | ((*p - 1) & VALUE_MASK);
                x.s = exiting;
                break;
                
            case increment: // INC
                try_mutate(*p, *p + 1);
                x.s = exiting;
                break;
                
            case increment_saturate: // SATURATING INCREMENT (TURN CW IF DIRECTION NONZERO)
                if (!is_conserved(*p) && (*p & VALUE_MASK))
                    *p = (*p & FLAGS_MASK) | ((*p - 1) & VALUE_MASK);
                x.s = exiting;
                break;
                
            case and_complement_of:
                try_mutate(x.a, x.a &~ *p);
                x.s = exiting;
                break;
                
            case flip_decrement: // flip
                try_mutate(*p, *p - 1);
                _board({x.x, x.y}) = (_board({x.x, x.y}) & ~OPCODE_MASK) | flip_increment;
                x.s = exiting;
                break;
                
            case flip_increment: // flip
                try_mutate(*p, *p + 1);
                _board({x.x, x.y}) = (_board({x.x, x.y}) & ~OPCODE_MASK) | flip_decrement;
                x.s = exiting;
                break;

            case instruction::swap:
                assert(!(instruction_ & REGISTER_FLAG));
                // assert something about physical and occupied flags being consistent?
                std::swap(x.a, *p);
                x.s = exiting;
                break;
                
            case conservative_or: // "load bits"?
                assert(false);
                x.a |= std::exchange(*p, x.a & *p);
                x.s = exiting;
                break;
                
            case conservative_and: // "store bits"?
                assert(false);
                x.a &= std::exchange(*p, x.a | *p);
                x.s = exiting;
                break;
                
            case less_than:
                assert(!is_conserved(x.d));
                x.d = (x.d & FLAGS_MASK) | ((x.d + (signed_of(x.a) < signed_of(*p))) & VALUE_MASK);
                x.s = exiting;
                break;

            case greater_than:
                assert(!is_conserved(x.d));
                x.d = (x.d & FLAGS_MASK) | ((x.d + (signed_of(x.a) > signed_of(*p))) & VALUE_MASK);
                x.s = exiting;
                break;

            case less_than_or_equal_to:
                assert(!is_conserved(x.d));
                x.d = (x.d & FLAGS_MASK) | ((x.d + (signed_of(x.a) <= signed_of(*p))) & VALUE_MASK);
                x.s = exiting;
                break;
                
            case greater_than_or_equal_to:
                assert(!is_conserved(x.d));
                x.d = (x.d & FLAGS_MASK) | ((x.d + (signed_of(x.a) >= signed_of(*p))) & VALUE_MASK);
                x.s = exiting;
                break;

            case equal_to:
                assert(!is_conserved(x.d));
                x.d = (x.d & FLAGS_MASK) | ((x.d + ((x.a & EQUALITY_MASK) == (*p & EQUALITY_MASK))) & VALUE_MASK);
                x.s = exiting;
                break;

            case not_equal_to:
                assert(!is_conserved(x.d));
                x.d = (x.d & FLAGS_MASK) | ((x.d + ((x.a & EQUALITY_MASK) != (*p & EQUALITY_MASK))) & VALUE_MASK);
                x.s = exiting;
                break;

            case compare:
                assert(false);
                // jesus
                x.d = (*p < x.a) - (x.a < *p); // which way around is best?
                x.s = exiting;
                break;
                
            case complement:
                try_mutate(*p, ~*p);
                x.s = exiting;
                break;
                
            case negate:
                try_mutate(*p, -*p);
                break;

            case clear:
                if (!is_conserved(*p))
                    *p &= OCCUPIED_FLAG; // zero, execpt occupation status
                x.s = exiting;
                break;
            
            case shift_left:
                if (!is_conserved(x.a))
                    x.a = (x.a & FLAGS_MASK) | (((x.a & VALUE_MASK) << signed_of(*p)) & VALUE_MASK);
                x.s = exiting;
                break;

            case shift_right:
                if (!is_conserved(x.a))
                    x.a = (x.a & FLAGS_MASK) | (((x.a & VALUE_MASK) >> signed_of(*p)) & VALUE_MASK);
                x.s = exiting;
                x.s = exiting;
                break;

            case dump | entering:
                // fallthrough
            case dump | waiting:
                // assert if conserved, or if occupied and accumulator is conserved?
                //
                // if (*q) {
                if (is_conserved(*q) || (is_conserved(x.a) && is_occupied(*q))) {
                    x.s = waiting;
                } else {
                    *q = x.a;
                    x.a = 0;
                    assert(!is_conserved(x.d));
                    x.d = (x.d & FLAGS_MASK) | ((x.d + 2) & VALUE_MASK);
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
                // jesus
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
                // jesus
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
                q = &_board({x.x, x.y - 1});
                break;
            case 1:
                q = &_board({x.x + 1, x.y});
                break;
            case 2:
                q = &_board({x.x, x.y + 1});
                break;
            case 3:
                q = &_board({x.x - 1, x.y});
                break;
        }
        // Check if we can claim it
        if (!is_occupied(*q)) {
            // step forward
            x.s = entering; // set travelling state
            occupy(*q);
            _world.did_exit(x.x, x.y, x.d);
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


void mine::tick(world& _world) {
    auto& _board = _world._board;

    using namespace instruction;
    u64* p = &_board({this->x, this->y + 1});
    assert(p);
    if (!(*p & (OCCUPIED_FLAG | CONSERVED_FLAG))) {
        *p = m;
    }
    
}

void smelter::tick(world& _world) {
    
    auto& _board = _world._board;

    using namespace instruction;
    using namespace element;
    
    u64* a = &_board({this->x + 1, this->y - 1});
    u64* b = &_board({this->x + 1, this->y + 1});
    u64* c = &_board({this->x - 1, this->y + 1});
    
    if ((*c == carbon) && (*b == hematite) && !is_occupied(*a)) {
        *c = 0;
        *b = 0;
        *a = iron;
    }

}

void silo::tick(world& _world) {
    using namespace instruction;
    
    auto& _board = _world._board;
    
    u64* a = &_board({this->x + 1, this->y - 1});
    u64* c = &_board({this->x - 1, this->y + 1});
    
    if (_queue.size() && !is_occupied(*a))
        *a = _queue.pop_front();
    if (is_item(*c)) {
        _queue.push_back(*c);
        *c = 0;
    }

}

} // namespace manic
