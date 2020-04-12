//
//  y_combinator.hpp
//  mania
//
//  Created by Antony Searle on 17/2/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#ifndef y_combinator_hpp
#define y_combinator_hpp

#include <utility>

namespace manic {

// y_combinator assists the construction of recursive lambdas
//
// y_combinator([](auto&& self, int x) {
//     return x ? x + self(x / 2) : 0;
// })();
//
// The lambda may need to be mutable if the lambda is to be moved out if itself
// for capture by an inner lambda

template<typename F>
struct y_combinator {
    
    F _f;
    
    template<typename X>
    explicit y_combinator(X&& f)
    : _f(std::forward<X>(f)) {
    }
    
    template<typename... Args>
    decltype(auto) operator()(Args&&... args) && {
        return std::move(_f)(std::move(*this), std::forward<Args>(args)...);
    }

    template<typename... Args>
    decltype(auto) operator()(Args&&... args) & {
        return _f(*this, std::forward<Args>(args)...);
    }

    template<typename... Args>
    decltype(auto) operator()(Args&&... args) const& {
        return _f(*this, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    decltype(auto) operator()(Args&&... args) const&& {
        return std::move(_f)(std::move(*this), std::forward<Args>(args)...);
    }
    
};

template<typename F>
y_combinator(F&&) -> y_combinator<std::decay_t<F>>;

}


#endif /* y_combinator_hpp */
