//
//  string-test.cpp
//  mania-test
//
//  Created by Antony Searle on 25/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "string.hpp"

#include "catch.hpp"

namespace manic {

TEST_CASE("immutable_string") {
    
    SECTION("sanity") {
        auto s = "immutable_string";
        auto t = immutable_string(s);
        REQUIRE(t == s);
    }
    
}

} // namespace manic
