//
//  y.cpp
//  aarc
//
//  Created by Antony Searle on 15/8/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#include <catch2/catch.hpp>

#include "y.hpp"

TEST_CASE("y", "[y]") {
    
    auto a = Y([](auto& self) -> void* { return &self; });
    auto b = std::move(a);
    REQUIRE(b() == &b);
    
}
