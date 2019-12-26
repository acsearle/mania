//
//  entity2.hpp
//  mania
//
//  Created by Antony Searle on 25/12/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef entity2_hpp
#define entity2_hpp

#include "common.hpp"
#include "rleq.hpp"

namespace manic {

struct world;

// Discriminated unions are more serializable and more amenable to stack
// allocation, but also have disadvantages
//
// consider struct B : A and C : A where A has some member data and functions.
// Do we implement this as struct { int discriminant; union  { B b; C c; } } or as
// struct { T m_a; int driscriminant; union { U m_b; V m_c; }}

struct entity2 {
  
    i64 x;
    i64 y;
    u64 t;
    
    enum : u64 {
        TRUCK = 1,
        MINE,
        SILO,
        SMELTER,
    };
        
    u64 discriminant;
    
    entity2() {
        std::memset(this, 0, sizeof(entity2));
    }
    
    entity2(entity2&& x) {
        std::memcpy(this, &x, sizeof(entity2));
        std::memset(&x, 0, sizeof(entity2));
    }
    
    ~entity2() {
        switch (discriminant) {
            case SILO:
                _queue.~rleq<u64>();
                break;
            default:
                break;
        }
    }

    void tick(world&);
    
    void tick_truck(world&);
    void tick_mine(world&);
    void tick_silo(world&);
    void tick_smelter(world&);
    
    
    union {

        struct { // TRUCK
            
            u64 s; // microstate
            u64 a, b, c, d; // registers
            
        };
        
        struct { // MINE
            
            u64 m; // material
            
        };
        
        struct { // SILO
            
            rleq<u64> _queue; // store
            
        };
        
        struct { // SMELTER
            // stateless
        };
        
    };
        
};


template<typename Serializer>
void serialize(entity2 const& x, Serializer& s) {
    serialize(x.x, s);
    serialize(x.y, s);
    serialize(x.t, s);
    serialize(x.discriminant, s);
    switch (x.discriminant) {
        case entity2::TRUCK:
            serialize(x.s, s);
            serialize(x.a, s);
            serialize(x.b, s);
            serialize(x.c, s);
            serialize(x.d, s);
            break;
        case entity2::MINE:
            serialize(x.m, s);
            break;
        case entity2::SILO:
            serialize(x._queue, s);
            break;
        case entity2::SMELTER:
            break;
        default:
            assert(false);
            break;
    };
}

template<typename Deserializer>
auto deserialize(placeholder<entity2>, Deserializer& d) {
    entity2 x;
    x.x = deserialize<i64>(d);
    x.y = deserialize<i64>(d);
    x.t = deserialize<u64>(d);
    x.discriminant = deserialize<u64>(d);
    switch (x.discriminant) {
        case entity2::TRUCK:
            x.s = deserialize<u64>(d);
            x.a = deserialize<u64>(d);
            x.b = deserialize<u64>(d);
            x.c = deserialize<u64>(d);
            x.d = deserialize<u64>(d);
            break;
        case entity2::MINE:
            x.m = deserialize<u64>(d);
            break;
        case entity2::SILO:
            x._queue = deserialize<rleq<u64>>(d);
            break;
        case entity2::SMELTER:
            break;
        default:
            assert(false);
            break;
    };
    return x;
}


} // namespace manic

#endif /* entity2_hpp */
