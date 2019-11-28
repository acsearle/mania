//
//  table4-test.cpp
//  mania-test
//
//  Created by Antony Searle on 28/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//


#include "table4.hpp"
#include "debug.hpp"

#include "tattler.hpp"
#include "catch.hpp"

namespace manic {

TEST_CASE("table4") {
    
    SECTION("regression") {
        
        table4<u64, u64> x;
        REQUIRE_FALSE(x.contains(0));
        
    }
    
    SECTION("default") {
        
        table4<u64, u64> t;
        REQUIRE(t.size() == 0);
        //REQUIRE(t.capacity() >= 0);
        REQUIRE_FALSE(t.contains(0));
        REQUIRE_FALSE(t.try_get(1));
        //REQUIRE(t.begin() == t.end());
        
        SECTION("insert") {
            
            // t.insert(1, 2);
            t.entry(1).or_insert(2);
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
        
        table4<u64, u64> t;
        for (u64 i = 0; i != N; ++i) {
            t.entry(i).or_insert(hash(i));
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
        
        /*
        u64 s = 0;
        for (auto&& [k, v] : t) {
            s += k;
        }
        REQUIRE(s == N * (N - 1) / 2);
        
        auto h = t._histogram();
        
        for (auto& [k, v] : h) {
            std::cout << "(" << k << ", " << v << ")\n";
        }
        */
        for (u64 i = 0; i != N; ++i) {
            t.erase(i);
            REQUIRE(t.size() == N - 1 - i);
            //t.shrink_to_fit();
            //REQUIRE(t.size() == N - 1 - i);
        }
        
        REQUIRE(t.size() == 0);
        
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
    
    SECTION("lifetimes") {
        
        const int N = 1'000'000;
        
        table4<int, tattler> t;
        for (int i = 0; i != N; ++i) {
            t.entry(i).or_insert(tattler());
        }
        REQUIRE(tattler::_live == N);
        for (int i = 0; i != N; ++i) {
            t.erase(i);
        }
        REQUIRE(tattler::_live == 0);
        
    }
    
}

}
