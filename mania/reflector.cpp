//
//  reflector.cpp
//  mania
//
//  Created by Antony Searle on 1/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "reflector.hpp"
#include "parse.hpp"

namespace manic {

// enum name { enumerator = constexpr, ... }

auto parse_cpp_whitespace() {
    return [](string_view& v) {
        // parse whitespace
        while (v) {
            switch (*v) {
                case '/': {
                    ++v;
                    if (!v)
                        return true;
                    switch (*v) {
                        case '/':
                            // C++-style comment
                            ++v;
                            while (v && *v != '\n')
                                ++v;
                            continue;
                        case '*':
                            // C-style comment
                            ++v;
                            while (v && *v != '*')
                                ++v;
                            continue;
                        default:
                            ; // fallthrough
                    }
                } // fallthrough
                default:
                    if (!iswspace(*v))
                        return true;
                    ++v;
            }
        }
        return true;
    };
}

auto parse_name() {
    return parse_plus([](string_view& v) {
        return v && iswalnum(*v) && ((void) ++v, true);
    });
}

auto parse_enum() {
    return parse_all_of(parse_literal("enum"),
                        parse_cpp_whitespace(),
                        parse_name(),
                        parse_cpp_whitespace(),
                        parse_literal('{'),
                        parse_cpp_whitespace(),
                        parse_literal('}')
                        );
}

void parse_cpp(string_view& v) {
    
}

    
} // namspace manic
