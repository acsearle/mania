//
//  json.cpp
//  mania
//
//  Created by Antony Searle on 23/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include <cctype>
#include <utility>
#include <string>
#include <cstdlib>
#include "json.hpp"
#include "table3.hpp"
#include "vector.hpp"

namespace manic {

std::string _string_from_file(FILE* f) {
    // Bug: peformance
    std::string s;
    int c;
    while ((c = fgetc(f)) != EOF)
        s.push_back(c);
    return s;
}


json json::from(const char* b, const char* e) {
    
    return json(value::from(b, e));
    
}


std::string _string_from(const char*& b, const char* e) {
    while (isspace(*b))
        ++b;
    assert(*b == '"');
    ++b;
    const char* c = b;
    while (*c != '"') {
        assert(c != e);
        ++c; // bug: account for escape characters
    }
    std::string s(b, c);
    b = c + 1;
    return s;
}

struct json_object : json::value {
    
    table3<std::string, json> _table;
    
    virtual size_t size() const {
        return _table.size();
    }
    
    virtual json const& at(char const* key) const {
        return _table.get(key);
    }
    
    static json_object* from(const char*& b, const char* e) {
        json_object* p = new json_object;
        while (isspace(*b)) ++b;
        assert(*b == '{'); ++b;
        while (isspace(*b)) ++b;
        while (*b != '}') {
            auto s = _string_from(b, e);
            while (isspace(*b)) ++b;
            assert(*b == ':'); ++b;
            p->_table.insert(s, json(json::value::from(b, e)));
            while (isspace(*b)) ++b;
            assert((*b == ',') || (*b == '}'));
            if (*b == ',') {
                ++b; while (isspace(*b)) ++b;
            }
        }
        ++b;
        return p;
    }
    
    virtual void debug() const {
        printf("{ ");
        for (auto const& [k, v] : _table) {
            puts(k.c_str());
            printf(" : ");
            v._ptr->debug();
            printf(", ");
        }
        printf("}");
    }
    
};

struct json_array : json::value {
    
    vector<json> _array;
    
    virtual json const& at(size_t i) const override {
        return _array[i];
    }
    
    virtual size_t size() const override {
        return _array.size();
    }
    
    static json_array* from(const char*& b, const char* e) {
        json_array* p = new json_array;
        while (isspace(*b)) ++b;
        assert(*b == '['); ++b;
        while (isspace(*b)) ++b;
        while (*b != ']') {
            p->_array.push_back(json(json::value::from(b, e)));
            while (isspace(*b)) ++b;
            assert((*b == ',') || (*b == ']'));
            if (*b == ',') {
                ++b; while (isspace(*b)) ++b;
            }
        }
        ++b;
        return p;
    }
    
    virtual void debug() const override {
        printf("[ ");
        for (auto const& v : _array) {
            v._ptr->debug();
            printf(", ");
        }
        printf("]");
    }

    
};

struct json_string : json::value {
    
    std::string _string;
    
    virtual char const* as_string() const {
        return _string.c_str();
    }
    
    static json_string* from(const char*& b, const char* e) {
        json_string* p = new json_string;
        p->_string = _string_from(b, e);
        return p;
    }
    
    virtual void debug() const {
        printf("\"%s\"", _string.c_str());
    }
    
};

struct json_number : json::value {
    
    double _number;
    
    virtual double as_number() const {
        return _number;
    }
    
    static json_number* from(const char*& b, const char* e) {
        json_number* p = new json_number;
        char* q;
        p->_number = std::strtod(b, &q);
        b = q;
        return p;
    }
    
    virtual void debug() const {
        printf("%g", _number);
    }

    
};


json::value* json::value::from(const char*& b, const char* e) {
    
    while (isspace(*b))
        ++b;
    
    switch (*b) {
        case '{': // object
            return json_object::from(b, e);
        case '[': // array
            return json_array::from(b, e);
        case '"': // string
            return json_string::from(b, e);
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
            return json_number::from(b, e);
        case 't': // true
        case 'f': // false
        case 'n': // null
        default:
            assert(false);
    }

    return nullptr;

}

}
