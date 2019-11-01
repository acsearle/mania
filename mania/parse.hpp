//
//  parse.hpp
//  mania
//
//  Created by Antony Searle on 29/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef parse_hpp
#define parse_hpp

#include "string.hpp"

namespace manic {

// parsers are function objects of the form
//
// (string_view&) -> bool

inline auto parse_character(u32 c) {
    return [=](string_view& v) {
        return v && (*v == c) && ((void) ++v, true);
    };
}

template<typename Predicate>
auto parse_character_predicate(Predicate predicate) {
    return [=](string_view& v) {
        return v && predicate(*v) && ((void) ++v, true);
    };
}

inline auto parse_literal(string_view u) {
    return [=](string_view& v) {
        string_view s{u};
        string_view t{v};
        while (s && t && (*s == *t)) {
            ++s; ++t;
        }
        return (!s) && ((void) (v.a = t.a), true);
    };
}

inline auto parse_whitespace() {
    return [](string_view& v) {
        while (v && iswspace(*v))
            ++v;
        return true;
    };
}

template<typename F>
auto parse_star(F&& f) {
    return [g = std::forward<F>(f)](string_view& v) {
        while (g(v))
            ;
        return true;
    };
}

template<typename F>
auto parse_plus(F&& f) {
    return [g = std::forward<F>(f)](string_view& v) {
        if (!g(v))
            return false;
        while (g(v))
            ;
        return true;
    };
}

template<typename F>
auto parse_optional(F&& f) {
    return [g = std::forward<F>(f)](string_view& v) {
        g(v);
        return true;
    };
}

template<typename... Args>
auto parse_all_of(Args&&... args) {
    return [=](string_view& v) {
        string_view u = v;
        return (... && args(v)) && ((void) (v = u), true);
    };
}

template<typename... Args>
auto parse_one_of(Args&... args) {
    return [&](string_view& v) {
        return (... || args(v));
    };
}

template<typename F, typename G>
auto parse_effect(F&& f, G&& g) {
    return [ff = std::forward<F>(f), gg = std::forward<G>(g)](string_view& v) {
        auto u = v.a;
        return ff(v) && ((void) gg(string_view(u, v.a)), true);
    };
}


} // namespace manic

#endif /* parse_hpp */
