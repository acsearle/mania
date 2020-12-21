//
//  deque.cpp
//  mania
//
//  Created by Antony Searle on 15/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "deque.hpp"

#include <iostream>
#include <vector>

#include <catch2/catch.hpp>

namespace manic {
    TEST_CASE("deque") {
        
        SECTION("default") {
            
            deque<int> a;
            REQUIRE(a.empty());
            REQUIRE(a.size() == 0);
            REQUIRE(a.capacity() >= 0);
            
            
        }
        
        SECTION("push_back") {
            deque<int> a;
            a.push_back(1);
            a.push_front(2);
            a.push_back(3);
            REQUIRE(a.size() == 3);
            REQUIRE(a[0] == 2);
            REQUIRE(a[1] == 1);
            REQUIRE(a[2] == 3);
            REQUIRE(a.front() == 2);
            REQUIRE(a.back() == 3);
            REQUIRE(a.pop_front() == 2);
            REQUIRE(a.pop_back() == 3);
            REQUIRE(a.pop_front() == 1);
            REQUIRE(a.empty());
        }
        
        
        
    }
}
