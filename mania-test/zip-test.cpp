//
//  zip-test.cpp
//  mania-test
//
//  Created by Antony Searle on 7/8/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "zip.hpp"

#include <list>
#include <numeric>
#include <vector>

#include "catch.hpp"

namespace manic {
    
    TEST_CASE("zip") {
        
        std::list<int> u(10);
        std::vector<double> v(10);
        
        std::iota(u.begin(), u.end(), 0);
        std::iota(v.begin(), v.end(), 0);
        
        SECTION("basic") {
            
            for (auto&& [x, y] : zip(u, v))
                REQUIRE(x == y);
            
        }
        
        SECTION("explicit") {
            auto z = zip(u, v);
            for (auto b = z.begin(); b != z.end(); ++b) {
                auto&& [x, y] = *b;
                REQUIRE(x == y);
            }
                
        }
        
        SECTION("copy") {
            
            auto z = zip(u, v);
            // decltype(z) y(z);
            
        }
        
    }
    
} // namespace manic
