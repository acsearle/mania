//
//  memory.hpp
//  mania
//
//  Created by Antony Searle on 27/10/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef memory_hpp
#define memory_hpp

#include <utility>
#include <type_traits>

namespace manic {
    
    template<typename T>
    class unique_ptr {
        
        T* _ptr;
        
    public:
        
        template<typename U, typename = std::enable_if_t<std::has_virtual_destructor<T>::value && std::is_convertible<U*, T*>::value>>
        unique_ptr(unique_ptr<U>&& ptr);
        
    };
    
};

#endif /* memory_hpp */
