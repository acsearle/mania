//
//  f8-test.cpp
//  mania-test
//
//  Created by Antony Searle on 24/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "f8.hpp"

#include <catch2/catch.hpp>

namespace manic {
    
    TEST_CASE("f8") {
        
        f8 a;
        a = 0;
        f8 b;
        b = 1;
        f8 c;
        c = 0.5;
        
        REQUIRE((a * b) == 0);
        REQUIRE((a * a) == 0);
        REQUIRE((b * b) == 1);
        
        REQUIRE((b + b) == 2);
        REQUIRE((b - b) == 0);
        REQUIRE((b / b) == 1);
        
        REQUIRE(c != 0.5);
        
        REQUIRE((c != 0));
        REQUIRE((c != 1));
        REQUIRE((c != 0.5));
        
        
    }
    
}
