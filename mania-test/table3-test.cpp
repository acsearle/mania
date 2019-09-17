//
//  table3-test.cpp
//  mania-test
//
//  Created by Antony Searle on 24/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "hash.hpp"
#include "table3.hpp"

#include "catch.hpp"

namespace manic {
    
    TEST_CASE("table3") {
        
        SECTION("default") {
            
            table3<u64, u64> t;
            REQUIRE(t.size() == 0);
            REQUIRE(t.capacity() == 0);
            
        }
        
        SECTION("stress") {
            
            const u64 N = 10'000'000;
            
            table3<u64, u64> t;
            for (u64 i = 0; i != N; ++i) {
                t.insert(i, hash(i));
            }
            REQUIRE(t.size() == N);
            REQUIRE(t.capacity() >= t.size());
            
            for (u64 i = 0; i != N; ++i) {
                u64 v = t.get(i);
                REQUIRE(v == hash(i));
            }
            
            u64 s = 0;
            for (auto&& [k, v] : t) {
                s += k;
            }
            REQUIRE(s == N * (N - 1) / 2);
            
            auto h = t.histogram();
            
            for (auto& [k, v] : h) {
                std::cout << "(" << k << ", " << v << ")\n";
            }
            
            /*
             
            for (auto& k : keys(t)) {
                std::cout << k << std::endl;
            }
            
            for (auto& v : values(t)) {
                std::cout << v << std::endl;
            }
            
            for (auto& [k, v] : t) {
                std::cout << "(" << k << ", " << v << ")\n";
            }
             
             */
            
            
            
        }
        
        SECTION("regression") {
            
            {
                table3<u64, u64> x;
                REQUIRE_FALSE(x.contains(0));
            }
            
        }
        
    }

}
