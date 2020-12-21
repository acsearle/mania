//
//  serialization-test.cpp
//  mania-test
//
//  Created by Antony Searle on 23/12/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include <catch2/catch.hpp>

#include "table3.hpp"
#include "serialize.hpp"
#include "string.hpp"
#include "string_view.hpp"

namespace manic {

TEST_CASE("serialization") {

    std::vector<int> x(7, 8);
    auto a = std::pair('a', 7.0);
    string_view v("hello");
    std::FILE* p = fopen("foo", "wb");
    table3<string, int> t;
    t.insert("one", 1);
    t.insert("two", 2);
    
    SECTION("file") {
        serialize(x, p);
        serialize(a, p);
        serialize(v, p);
        serialize(t, p);
        fclose(p);
        p = fopen("foo", "rb");
        auto y = deserialize<std::vector<int>>(p);
        REQUIRE(std::equal(x.begin(), x.end(), y.begin(), y.end()));
        auto b = deserialize<std::pair<char, double>>(p);
        REQUIRE(a == b);
        auto u = deserialize<string>(p);
        REQUIRE(u == v);
        auto w = deserialize<table3<string, int>>(p);
        REQUIRE(t == w);
    }
    fclose(p);
    //remove("foo");

}

}
