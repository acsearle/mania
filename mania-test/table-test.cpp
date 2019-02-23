//
//  table-test.cpp
//  mania-test
//
//  Created by Antony Searle on 23/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "table.hpp"

#include "catch.hpp"

namespace manic {
    
    TEST_CASE("table") {
        
        table<uint64_t, uint64_t> t;
        
        SECTION("default") {
            REQUIRE(t.size() == 0);
            REQUIRE(t.begin() == t.end());
            REQUIRE(t.get(0) == nullptr);
            t.remove(1);
        }
        
        SECTION("invariant") {

            REQUIRE(t.size() == 0);
            
            for (uint64_t i = 0; i != 100; ++i)
                t.put(i, hash(i));

            REQUIRE(t.size() == 100);

            t.print();
            t.statistics();
            
            for (uint64_t i = 0; i != 100; ++i) {
                uint64_t* p = t.get(i);
                REQUIRE(p);
                REQUIRE(*p == hash(i));
            }

            REQUIRE(t.size() == 100);

            for (uint64_t i = 100; i != 200; ++i) {
                uint64_t* p = t.get(i);
                REQUIRE(!p);
            }

            REQUIRE(t.size() == 100);

            for (uint64_t i = 50; i != 150; ++i) {
                t.remove(i);
            }
            
            REQUIRE(t.size() == 50);
            
            for (uint64_t i = 0; i != 50; ++i) {
                uint64_t* p = t.get(i);
                REQUIRE(p);
                REQUIRE(*p == hash(i));
            }
            
            REQUIRE(t.size() == 50);
            
            for (uint64_t i = 50; i != 100; ++i) {
                t[i];
            }
            
            REQUIRE(t.size() == 100);

            t.print();
            t.statistics();
            
            
        }
        
    }
    
}
