//
//  optional.hpp
//  mania
//
//  Created by Antony Searle on 27/10/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef optional_hpp
#define optional_hpp

#include <type_traits>

#include <experimental/optional>

namespace manic {
    
    template<typename T>
    using optional = std::experimental::optional<T>;
    
} // namespace manic

#endif /* optional_hpp */
