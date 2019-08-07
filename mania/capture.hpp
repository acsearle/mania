//
//  capture.hpp
//  mania
//
//  Created by Antony Searle on 7/8/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef capture_hpp
#define capture_hpp

#include <tuple>
#include <utility>

namespace manic {
    
    // Make a tuple that perfect-captures the arguments.
    //
    // If an argument is a (const) reference, the tuple will store a (const)
    // reference.  If the argument is an rvalue, the tuple will hold a move-
    // constructed value.
    
    template<typename... Args>
    std::tuple<Args...> capture(Args&&... args) {
        return std::tuple<Args...>(std::forward<Args>(args)...);
    }
    
}

#endif /* capture_hpp */
