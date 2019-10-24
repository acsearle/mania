//
//  json.hpp
//  mania
//
//  Created by Antony Searle on 23/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef json_hpp
#define json_hpp

#include <utility>
#include <string>
#include <string_view>
#include "table3.hpp"
#include "vector.hpp"

namespace manic {

std::string _string_from_file(FILE* f);


struct json {
    
    struct value {
        
        virtual ~value() = default;
        virtual size_t size() const { throw 0; }
        virtual json const& at(size_t) const { throw 0; }
        virtual json const& at(char const*) const { throw 0; }
        virtual char const* as_string() const  { throw 0; }
        virtual double as_number() const  { throw 0; }
        virtual table3<std::string, json> const& as_object() const { throw 0; }
        virtual vector<json> const& as_array() const { throw 0; }
        virtual bool is_string() const  { throw 0; }
        virtual bool is_number() const  { throw 0; }
        virtual bool is_array() const  { throw 0; }
        virtual bool is_object() const { throw 0; }
        static value* from(const char*& b, const char* e);
        virtual void debug() const = 0;
        
    };
    
    value* _ptr;
    
    json() = delete;
    explicit json(value* p) : _ptr(p) {}
    json(json const&) = delete;
    json(json&& x) : _ptr(std::exchange(x._ptr, nullptr)) {}
    ~json() { delete _ptr; }
    json& operator=(json const&) = delete;
    json& operator=(json&& x) { json tmp(std::move(x)); std::swap(_ptr, tmp._ptr); return *this; }
    
    static json from(char const* b, char const* e);
    static json from_file(FILE*);
    
    size_t size() const { return _ptr->size(); }
    
    json const& operator[](size_t i) const { return _ptr->at(i); }
    json const& operator[](char const* s) const { return _ptr->at(s); }
    
    char const* as_string() const { return _ptr->as_string(); }
    double as_number() const { return _ptr->as_number(); }
    
    i64 as_i64() const {
        double a = _ptr->as_number();
        i64 b = (i64) a;
        assert(((double) b) == a);
        return b;
    }
    
    
};

} // namespace manic

#endif /* json_hpp */
