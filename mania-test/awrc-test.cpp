//
//  awrc-test.cpp
//  mania-test
//
//  Created by Antony Searle on 18/4/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#include "awrc.hpp"

#include "catch.hpp"

namespace manic {

TEST_CASE("ctrie") {
    
    ctrie<int, int> a;
    
    REQUIRE(a.lookup(7) == nullptr);
    
    
    
}

}
