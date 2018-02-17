//
//  queue.hpp
//  mania
//
//  Created by Antony Searle on 15/2/18.
//  Copyright © 2018 Antony Searle. All rights reserved.
//

#ifndef queue_hpp
#define queue_hpp

#include <iterator>

namespace mania {
    
    template<typename T>
    class queue {
        
        T* _begin;
        T* _end;
        T* _ptr;
        T* _capacity;
        
    public:
        
        using size_type = std::size_t;
        
        queue()
        : _begin(nullptr)
        , _end(nullptr)
        , _ptr(nullptr)
        , _capacity(nullptr) {
        }
        
        queue(size_type n)
        : _begin(new T[n])
        , _end(_begin + n)
        , _ptr(_begin)
        , _capacity(_end) {
        }
        
        ~queue() {
            delete[] _ptr;
        }
        
    }; // class queue<T>
    
} // namespace mania

#endif /* queue_hpp */
