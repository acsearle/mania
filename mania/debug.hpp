//
//  debug.hpp
//  mania
//
//  Created by Antony Searle on 8/2/18.
//  Copyright © 2018 Antony Searle. All rights reserved.
//

#ifndef debug_hpp
#define debug_hpp

#include <iostream>
#include <utility>
#include <string_view>

namespace mania {
    
    // address hexdump type expression value
    
    /*
     template<class T>
     constexpr string_view get_name()
     {
     char const* p = __PRETTY_FUNCTION__;
     while (*p++ != '=');
     for (; *p == ' '; ++p);
     char const* p2 = p;
     int count = 1;
     for (;;++p2)
     {
     switch (*p2)
     {
     case '[':
     ++count;
     break;
     case ']':
     --count;
     if (!count)
     return {p, std::size_t(p2 - p)};
     }
     }
     return {};
     }*/
    
    template<typename T>
    constexpr std::string_view type_to_string() {
        const char* a = __PRETTY_FUNCTION__; // type_to_string [T = ...]
        return a;
    }
    
    template<typename T>
    auto debug(T&& value, const char* expr) {
        // address
        std::cout << std::hex << &value << std::dec << std::endl;
        // hexdump
        const unsigned char* a = reinterpret_cast<const unsigned char*>(&value);
        for (int i = 0; i != sizeof(value); ++i)
            printf("%2x", (int) a[i]);
        printf("\n");
        std::cout << type_to_string<T&&>() << std::endl;
        std::cout << value << std::endl;
        return std::forward<T>(value);
    }
    
#define DUMP(X) std::cout << #X " = " << (X) << "\n";
    
} // namespace mania


#endif /* debug_hpp */
