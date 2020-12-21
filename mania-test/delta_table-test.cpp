//
//  delta_table-test.cpp
//  mania-test
//
//  Created by Antony Searle on 27/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "delta_table.hpp"
#include "debug.hpp"

#include <catch2/catch.hpp>
#include "tattler.hpp"

namespace manic {
        
    TEST_CASE("delta_table") {
        
        SECTION("regression") {
            
            delta_table<u64, u64> x;
            REQUIRE_FALSE(x.contains(0));
            
        }
        
        SECTION("default") {
            
            delta_table<u64, u64> t;
            REQUIRE(t.size() == 0);
            REQUIRE_FALSE(t.contains(0));
            REQUIRE_FALSE(t.try_get(1));
        
            SECTION("insert") {
                
                t.insert(1, 2);
                REQUIRE(t.size() == 1);
                REQUIRE(t.contains(1));
                
                SECTION("get") {
                    REQUIRE(t.get(1) == 2);
                }
                
                SECTION("erase") {
                    t.erase(1);
                    REQUIRE(t.size() == 0);
                    REQUIRE_FALSE(t.contains(1));
                }
                
            }

        }
        
        
        
        SECTION("stress") {
            
            const u64 N = 1'000'000;
            
            delta_table<u64, u64> t;
            for (u64 i = 0; i != N; ++i) {
                t.insert(i, hash(i));
            }
            REQUIRE(t.size() == N);
            
            for (u64 i = 0; i != N / 2; ++i) {
                REQUIRE(t.contains(i));
                REQUIRE_FALSE(t.contains(i + N));
            }
            
            for (u64 i = 0; i != N; ++i) {
                u64 v = t.get(i);
                REQUIRE(v == hash(i));
            }
                        
            for (u64 i = 0; i != N; ++i) {
                t.erase(i);
                REQUIRE(t.size() == N - 1 - i);
            }
            
            REQUIRE(t.size() == 0);
            
        }
        
        SECTION("lifetimes") {
            
            const int N = 1'000'000;
            
            delta_table<int, tattler> t;
            for (int i = 0; i != N; ++i) {
                t.insert(i, tattler());
            }
            REQUIRE(tattler::_live == N);
            for (int i = 0; i != N; ++i) {
                t.erase(i);
            }
            REQUIRE(tattler::_live == 0);
            
        }
        
    }

}
