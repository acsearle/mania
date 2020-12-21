//
//  enumerate-test.cpp
//  mania-test
//
//  Created by Antony Searle on 7/8/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "enumerate.hpp"

#include <vector>

#include <catch2/catch.hpp>

namespace manic {

    TEST_CASE("enumerate") {

        SECTION("basic") {

            std::vector<int> v;
            v.resize(10);
            for (auto&& [i, x] : enumerate(v))
                REQUIRE(v[i] == x);

        }

    }
    
} // namespace manic
