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
#include <sstream>
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

struct _json_value {
    
    [[noreturn]] static void unimplemented() { throw 0; }
    
    virtual ~_json_value() = default;
    virtual size_t size() const { unimplemented(); }
    virtual json const& at(size_t) const { unimplemented(); }
    virtual json const& at(std::string_view) const { unimplemented(); }
    virtual std::string_view as_string() const  { unimplemented(); }
    virtual double as_number() const  { unimplemented(); }
    virtual table3<std::string, json> const& as_object() const { unimplemented(); }
    virtual vector<json> const& as_array() const { unimplemented(); }
    virtual bool is_string() const { unimplemented(); }
    virtual bool is_number() const { unimplemented(); }
    virtual bool is_array() const { unimplemented(); }
    virtual bool is_object() const { unimplemented(); }
    static _json_value* from(std::string_view&);
    virtual std::string debug() const = 0;
    virtual _json_value* clone() const = 0;
    
}; // _json_value

json::json(json const& x)
: _ptr(x._ptr ? x._ptr->clone() : nullptr) {
}

json::~json() { delete _ptr; }

json& json::operator=(json const& x) {
    json tmp(x);
    using std::swap;
    swap(*this, tmp);
    return *this;
}

size_t json::size() const { return _ptr->size(); }

json const& json::operator[](size_t i) const { return _ptr->at(i); }
json const& json::operator[](std::string_view s) const { return _ptr->at(s); }

std::string_view json::as_string() const { return _ptr->as_string(); }
double json::as_number() const { return _ptr->as_number(); }

i64 json::as_i64() const {
    double a = _ptr->as_number();
    i64 b = (i64) a;
    assert(((double) b) == a);
    return b;
}


json json::from(std::string_view& v) {
    
    return json(_json_value::from(v));
    
}

json json::from(std::string_view&& v) {
    std::string_view u{v};
    _json_value* p = _json_value::from(u);
    while (u.size() && isspace(u.front()))
        u.remove_prefix(1);
    assert(u.empty());
    return json(p);
}


std::string _string_from(std::string_view& v) {
    while (isspace(v.front()))
        v.remove_prefix(1);
    assert(v.front() == '"');
    v.remove_prefix(1);
    auto c = v.begin();
    while (*c != '"') {
        assert(c != v.end());
        ++c; // bug: account for escape characters
    }
    std::string s(v.begin(), c);
    v.remove_prefix(c - v.begin() + 1);
    return s;
}

double _number_from(std::string_view& v) {
    char* q;
    double d = std::strtod(v.data(), &q);
    v.remove_prefix(q - v.data());
    return d;
}

struct _json_object : _json_value {
    
    table3<std::string, json> _table;
    
    virtual size_t size() const override {
        return _table.size();
    }
    
    virtual json const& at(std::string_view key) const override {
        return _table.get(key);
    }
    
    static _json_object* from(std::string_view& v) {
        _json_object* p = new _json_object;
        while (isspace(v.front())) v.remove_prefix(1);
        assert(v.front() == '{'); v.remove_prefix(1);
        while (isspace(v.front())) v.remove_prefix(1);
        while (v.front() != '}') {
            auto s = _string_from(v);
            while (isspace(v.front())) v.remove_prefix(1);
            assert(v.front() == ':'); v.remove_prefix(1);
            p->_table.insert(s, json(_json_value::from(v)));
            while (isspace(v.front())) v.remove_prefix(1);
            assert((v.front() == ',') || (v.front() == '}'));
            if (v.front() == ',') {
                v.remove_prefix(1); while (isspace(v.front())) v.remove_prefix(1);
            }
        }
        v.remove_prefix(1);
        return p;
    }
    
    virtual std::string debug() const override {
        std::string s;
        s.append("{ ");
        for (auto const& [k, v] : _table) {
            s.append(k);
            s.append(" : ");
            s.append(v._ptr->debug());
            s.append(", ");
        }
        s.append("}");
        return s;
    }
    
    virtual _json_object* clone() const override {
        auto p = new _json_object;
        p->_table.reserve(_table.size());
        for (auto&& [key, value] : _table) {
            p->_table.insert(key, value);
        }
        return p;
    }
    
};

struct _json_array : _json_value {
    
    vector<json> _array;
    
    virtual json const& at(size_t i) const override {
        return _array[i];
    }
    
    virtual size_t size() const override {
        return _array.size();
    }
    
    static _json_array* from(std::string_view& v) {
        _json_array* p = new _json_array;
        while (isspace(v.front())) v.remove_prefix(1);
        assert(v.front() == '['); v.remove_prefix(1);
        while (isspace(v.front())) v.remove_prefix(1);
        while (v.front() != ']') {
            p->_array.push_back(json(_json_value::from(v)));
            while (isspace(v.front())) v.remove_prefix(1);
            assert((v.front() == ',') || (v.front() == ']'));
            if (v.front() == ',') {
                v.remove_prefix(1); while (isspace(v.front())) v.remove_prefix(1);
            }
        }
        v.remove_prefix(1);
        return p;
    }
    
    virtual std::string debug() const override {
        std::string s;
        s.append("[ ");
        for (auto const& v : _array) {
            s.append(v._ptr->debug());
            s.append(", ");
        }
        s.append("]");
        return s;
    }
    
    virtual _json_array* clone() const override {
        _json_array* p = new _json_array;
        p->_array = _array;
        return p;
    }
    
    
};

struct _json_string : _json_value {
    
    std::string _string;
    
    explicit _json_string(std::string_view v) : _string(v) {}
    explicit _json_string(std::string&& s) : _string(std::move(s)) {}
    
    virtual std::string_view as_string() const override {
        return _string.c_str();
    }
    
    static _json_string* from(std::string_view& v) {
        return new _json_string(_string_from(v));
    }
    
    virtual std::string debug() const override {
        std::string s;
        s.append("\"");
        s.append(_string);
        s.append("\"");
        return s;
    }
    
    _json_string* clone() const override {
        return new _json_string(_string);
    }
    
};

struct _json_number : _json_value {
    
    double _number;
    
    explicit _json_number(double d) : _number(d) {}
    
    virtual double as_number() const override {
        return _number;
    }
    
    static _json_number* from(std::string_view& v) {
        return new _json_number(_number_from(v));
    }
    
    virtual std::string debug() const override {
        std::stringstream s;
        s << _number;
        return s.str();
    }
    
    virtual _json_number* clone() const override {
        return new _json_number(_number);
    }
    
    
};



_json_value* _json_value::from(std::string_view& v) {
    
    while (isspace(v.front())) v.remove_prefix(1);
    
    switch (v.front()) {
        case '{': // object
            return _json_object::from(v);
        case '[': // array
            return _json_array::from(v);
        case '"': // string
            return _json_string::from(v);
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
            return _json_number::from(v);
        case 't': // true
        case 'f': // false
        case 'n': // null
        default:
            assert(false);
    }
    
    return nullptr;
    
}


}
