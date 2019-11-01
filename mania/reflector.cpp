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

/*
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
*/

bool parse_cpp_comment(string_view& v) {
    string_view u = v;
    while (u && iswspace(*u))
        ++u;
    if (!u || (*u != '/'))
        return false;
    ++u;
    if (!u || (*u != '/'))
        return false;
    v = u;
    while (v && (*v != '\n'))
        ++v;
    if (v) {
        assert(*v == '\n');
        ++v;
    }
    return true;
}

bool parse_c_comment(string_view& v) {
    string_view u = v;
    while (u && iswspace(*u))
        ++u;
    if (!u || (*u != '/'))
        return false;
    ++u;
    if (!u || (*u != '*'))
        return false;
    do {
        while (u && (*u != '*'))
            ++u;
        if (!u)
            return false;
        ++u;
        if (!u)
            return false;
    } while (*u != '/');
    v = u;
    return true;
}

bool parse_preprocessor_directive(string_view& v) {
    string_view u = v;
    while (u && iswspace(*u))
        ++u;
    if (!u || (*u != '#'))
        return false;
    v = u;
    while (v && (*v != '\n'))
        ++v;
    if (v) {
        assert(*v == '\n');
        ++v;
    }
    return true;
}

auto parse_cpp_whitespace() {
    return [](string_view& v) {
        return (parse_preprocessor_directive(v)
                || parse_c_comment(v)
                || parse_cpp_comment(v));
    };
}

bool iswalnum_(u32 c) {
    return iswalnum(c) || (c == '_');
}

auto parse_name() {
    return parse_plus([](string_view& v) {
        return v && iswalnum_(*v) && ((void) ++v, true);
    });
}

auto parse_enum() {
    return parse_all_of(parse_literal("enum"),
                        parse_cpp_whitespace(),
                        parse_name(),
                        parse_cpp_whitespace(),
                        parse_character('{'),
                        parse_cpp_whitespace(),
                        parse_character('}')
                        );
}

void parse_cpp(string_view& v) {
    
}

    
} // namspace manic
