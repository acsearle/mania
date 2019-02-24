//
//  raw_vector.hpp
//  mania
//
//  Created by Antony Searle on 22/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef raw_vector_hpp
#define raw_vector_hpp

namespace manic {
    
    template<typename T>
    struct raw_vector {
        
        T* _allocation;
        ptrdiff_t _capacity;
        
        raw_vector() : _allocation(nullptr), _capacity(0) {}
        
        raw_vector(const raw_vector&) = delete;
        raw_vector(raw_vector&& v) : raw_vector() { swap(v); }
        explicit raw_vector(ptrdiff_t capacity) {
            _allocation = (T*) malloc(capacity * sizeof(T));
            _capacity = capacity;
        }
        raw_vector(T* ptr, ptrdiff_t n) : _allocation(ptr), _capacity(n) {}
        
        ~raw_vector() { free(_allocation); }
        
        raw_vector& operator=(const raw_vector&) = delete;
        raw_vector& operator=(raw_vector&& v) {
            raw_vector(std::move(v)).swap(*this);
            return *this;
        }
        
        void swap(raw_vector& v) {
            using std::swap;
            swap(_allocation, v._allocation);
            swap(_capacity, v._capacity);
        }
        
        ptrdiff_t capacity() const { return _capacity; }
        
    };
    
    template<typename T>
    struct raw_vector<const T>;
    
    template<typename T>
    void swap(raw_vector<T>& a, raw_vector<T>& b) {
        a.swap(b);
    }
    
}

#endif /* raw_vector_hpp */
