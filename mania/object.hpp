//
//  object.hpp
//  mania
//
//  Created by Antony Searle on 15/2/18.
//  Copyright © 2018 Antony Searle. All rights reserved.
//

#ifndef object_hpp
#define object_hpp

#include <atomic>
#include <cassert>
#include <utility>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <map>

namespace manic {
    
    constexpr std::size_t hash(const char* p) {
        std::size_t x = 0;
        while (*p) {
            x ^= *p++;
            x ^= x >> 21; x ^= x << 37; x ^= x >> 4;
        }
        return x;
    }
    
    

    union object;
    struct instance;

    template<std::size_t H>
    struct attribute {
        instance* _ptr;
        object operator()(object) const;
    };
    
    union object {
        instance* _ptr;
        attribute<hash("__add__")> __add__;
        object operator+(object) const;
        object operator()(object) const;
    };
    
    struct instance {
        std::atomic<std::ptrdiff_t> _strong;
        object _class;
        std::map<std::size_t, object> _attributes;
        instance() : _strong(0), _class() {}
        instance(const instance&) = delete;
        instance(instance&&) = delete;
        object (*__getattr__)(object self, std::size_t);
        object (*__call__)(object self, object x);
    };
    
    template<std::size_t H>
    object attribute<H>::operator()(object o) const {
        return _ptr->__getattr__(reinterpret_cast<object const&>(*this), H)(o);
    }
    
    object object::operator+(object y) const {
        return __add__(y);
    }
    
    object object::operator()(object x) const {
        return _ptr->__call__(*this, x);
    }
    
    struct type : instance {
    };
    
    /*
    struct function_ : instance {
        function_(object (*f)(object, object)) {
            __call__ = f;
        }
    };
     */
    /*
    object function(object (*f)(object, object)) {
        auto p = new instance;
        p->__call__ = f;
        return object(p);
    }
     */
                         
                         
    
    /*
    struct integer : instance {
        integer() {
            _attributes[hash("__add__")] = function([](object self, object other) {
                return object();
            });
        }
    };
     */
    
} // manic

namespace mania {
    
    class base;
    union object;
    class unimplemented {};
    
    template<std::size_t H>
    union href {
        
        base* _ptr;
        
    public:
        
        // lookup the backing object
        
        operator object&();
        operator const object&() const;
        
        // assign through the reference to the backing object, creating it if
        // neeeded.  consider if these should really return true references or
        // a different proxy (or anything at all)
        
        object& operator=(const href& r) { return operator=(r.operator const object&()); }
        object& operator=(const object&);
        
        template<std::size_t J>
        object operator+(const href<J>& b) const;
        
        // if we want to support x.y.z we have to union z in here and mark it as
        // a double lookup href<hash(y), hash(z)>
        
    };
    
    union object {
        
        base* _ptr; // replace all uses with reinterpret_cast so as not to pollute namespace

    public:

        // To have dot access in the absence of overloading, we have to
        // really have members of all names we might want to use.  So
        // we make a union of all member names, with each encoding its name
        // in its type.  At runtime they will resolve to actual objects or
        // throw exceptions on access.
        
        href<0> x;
        href<1> y;
        // ... thousands ...

        object() : _ptr(nullptr) {}
        explicit object(base* p) : _ptr(p) {}
        object(const object&);
        object(object&& r) : _ptr(std::exchange(r._ptr, nullptr)) {}
        ~object();
        
        explicit object(int x);
        object(const object& a, const object& b);
        
        friend void swap(object& a, object& b) { using std::swap; swap(a._ptr, b._ptr); }
        object& operator=(const object& r) { return operator=(object(r)); }
        object& operator=(object&& r) { swap(*this, r); return *this; }
        object operator+(const object& b) const;
        std::ostream& print(std::ostream& s) const;
        object& operator[](const object&);
        
    }; // union object
    
    std::ostream& operator<<(std::ostream& a, const object& b) {
        return b.print(a);
    }
    
    template<std::size_t H> template<std::size_t J>
    object href<H>::operator+(const href<J>& b) const {
        return operator const object&() + b.operator const object&();
    }
    
    class base {
        mutable std::atomic<std::ptrdiff_t> _strong;
    public:
        object self; // will harmlessly dec() just before _strong is destroyed
        base() : _strong(0), self(this) {}
        base(const base&) : _strong(0) {}
        base(const base&&) : _strong(0) {}
        virtual ~base() = default;
        base& operator=(const base&) = delete;
        base& operator=(base&&) = delete;
        void inc() const {
            _strong.fetch_add(1, std::memory_order_relaxed);
        }
        void dec() const {
            if (!_strong.fetch_sub(1, std::memory_order_release)) {
                std::atomic_thread_fence(std::memory_order_acquire);
                delete this;
            }
        }
        virtual object operator+(const object& b) const { throw unimplemented(); }
        virtual object& lookup(std::size_t h) { throw unimplemented(); }
        virtual const object& lookup(std::size_t h) const { throw unimplemented(); }
        virtual std::ostream& print(std::ostream& s) const { return s << '<' << typeid(*this).name() << '@' << this << '>'; };
        virtual object& set(std::size_t h, const object&) { throw unimplemented(); }
        virtual object len() const { throw unimplemented(); }
        virtual object& operator[](const object&) { throw unimplemented(); }
    };
    
    object::object(const object& a)
    : _ptr(a._ptr) {
        if (_ptr)
            _ptr->inc();
    }

    object::~object() {
        if (_ptr)
            _ptr->dec();
    }
    
    std::ostream& object::print(std::ostream& s) const {
        return _ptr->print(s);
    }
    
    object object::operator+(const object& b) const {
        return *_ptr + b;
    }
    
    object& object::operator[](const object& i) {
        return (*_ptr)[i];
    }
    
    object len(const object& a) {
        return reinterpret_cast<base*const&>(a)->len();
    }
    
    template<std::size_t H>
    href<H>::operator object&() {
        return _ptr->lookup(H);
    }
    
    template<std::size_t H>
    href<H>::operator const object&() const {
        return const_cast<const base*>(_ptr)->lookup(H);
    }
    
    template<std::size_t H>
    object& href<H>::operator=(const object& r) {
        return _ptr->set(H, r);
    }
    
    class integer
    : public base {
    public:
        const int _value;
        explicit integer(int x) : _value(x) {}
        virtual object operator+(const object& b) const override {
            return object(new integer(_value + dynamic_cast<integer&>(*b._ptr)._value));
        }
        virtual std::ostream& print(std::ostream& s) const override {
            return s << _value;
        }
    };
    
    object::object(int x) : _ptr(new integer(x)) {}
    
    class structure
    : public base {
        std::unordered_map<std::size_t, object> _members;
    public:
        virtual const object& lookup(std::size_t h) const override {
            auto a = _members.find(h);
            if (a == _members.end())
                throw unimplemented();
            return a->second;
        }
        virtual object& lookup(std::size_t h) override {
            auto a = _members.find(h);
            if (a == _members.end())
                throw unimplemented();
            return a->second;
        }
        virtual object& set(std::size_t h, const object& r) override {
            auto a = _members.insert(std::make_pair(h, r));
            if (!a.second)
                a.first->second = r;
            return a.first->second;
        }
        virtual std::ostream& print(std::ostream& s) const override {
            s << '{';
            for (auto& x : _members)
                s << '.' << x.first << ':' << x.second << ", ";
            return s << '}';
        }
    };
    
    class list : public base {
        std::vector<object> _members;
    public:
        list(std::initializer_list<object> a) : _members(a) {
        }
        virtual std::ostream& print(std::ostream& s) const override {
            s << '[';
            for (auto& x : _members)
                s << x << ", ";
            return s << ']';
        }
        virtual object& operator[](const object& i) override {
            return _members[dynamic_cast<integer&>(*i._ptr)._value];
        }
        virtual object len() const override {
            return object((int) _members.size());
        }
    }; // class list

    class point
    : public structure {
    public:
        point(const object& a, const object& b) {
            self.x = a;
            self.y = b;
        }
        virtual object operator+(const object& b) const override {
            return object(new point(self.x + b.x, self.y + b.y));
        }
        virtual std::ostream& print(std::ostream& s) const override {
            return s << '(' << self.x << ", " << self.y << ')';
        }
    }; // point
    
    object::object(const object& a, const object& b)
    : _ptr(new point(a, b)) {
    }
    
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
    }
    
#define debug(X) std::cout << #X " -> (" << get_name<decltype((X))>() << ") " << (X) << std::endl
    
    template<std::size_t N>
    constexpr std::size_t foo(const char (&x)[N]) {
        return N;
    }
    
    constexpr std::size_t constexprhash(const char* p) {
        std::size_t x = 0;
        while (*p) {
            x ^= *p++;
            x ^= x >> 21; x ^= x << 37; x ^= x >> 4;
        }
        return x;
    }
    
    constexpr std::size_t mangle(const char* p) {
        std::size_t x = 0;
        int s = 0;
        while (*p) {
            x |= ((std::size_t) *p) << (s & 63);
            s += 8;
            ++p;
        }
        return x;
    }
    
    
    struct xxx {
        xxx() {
            {
                href<mangle("foo")> zzz;
                debug(&zzz);
                
                auto s = mangle("foo");
                char ss[8];
                strcpy(ss, (char*) &s);
                debug(ss);
                
                object a(7);
                object b(8);
                object c = a + b;
                debug(c);
                
                object d(a, b);
                debug(d);
                object e = d + object(c, c);
                debug(e);
                
                e.x = object(666);
                
                debug(e);
                
                object f(new structure());
                f.y = object(7); // can create members by assigning to them
                debug(f);
                
                object v(new list({a, b, c, d, e, f}));
                debug(v[object(3)]);
                debug(v);
            }
            
            
            exit(0);
        };
    };
    
} // mania

/*
namespace mania {
    
    // for dot syntax to work on an object it must really have a member value
    // or function of that name
    //
    // so make a union of proxies with all possible names we want to use (!)
    // and then overload them with relevant operators
    
    // constexpr hash string literals
    
    template<std::size_t H>
    struct vref {
    };
    
    struct horror {
        union {
            vref<1> a;
            vref<2> b;
        };
    };
    
    class object;
    class representation;
    
    class object {
        
        friend class representation;
    
    private:
        
        representation* _ptr;
        
    public:
        
        object()
        : _ptr(nullptr) {
        }
        
        object(const object& r);
        
        object(object&& r)
        : _ptr(std::exchange(r._ptr, nullptr)) {
        }
        
        explicit object(representation* p);
        explicit object(const char* s);
        explicit object(int x);
        
        ~object();
        
        object& operator=(const object& r) {
            return operator=(object(r));
        }
        
        object& operator=(object&& r) {
            swap(r);
            return *this;
        }
        
        void swap(object& r) {
            using std::swap;
            swap(_ptr, r._ptr);
        }
        
        object& operator[](const object& o);
        
        template<typename... Args>
        object operator()(Args&&... args) const;
        
        object operator-() const;
        
        bool operator==(const object& r) const;
        
        std::size_t hash() const;
        
        representation* get() const { return _ptr; }
        
    }; // class object
    
    
    
    class representation {
        
    private:
        
        mutable std::atomic<std::ptrdiff_t> _strong;
        
    public:
        
        representation() : _strong(0) {}
        
        representation(const representation& r) : representation() {}
        
        representation(representation&&) = delete;
        
        virtual ~representation() = default;
        
        void inc() const {
            _strong.fetch_add(1, std::memory_order_relaxed);
        }
        
        void dec() const {
            if (!_strong.fetch_sub(1, std::memory_order_release)) {
                std::atomic_thread_fence(std::memory_order_acquire);
                delete this;
            }
        }
        
        virtual representation* copy() const = 0;
        
        virtual object call(object* first, object* last) { return object(); };
        
        virtual object& subscript(const object&) { throw 0; }
        
        virtual std::size_t hash() const = 0;
        
        virtual bool equals(const object& r) const = 0;
        
        virtual void print() const = 0;
        
    };
    
    object::object(const object& r)
    : _ptr(r._ptr) {
        if (_ptr)
            _ptr->inc();
    }
    
    object::object(representation* p)
    : _ptr(p) {
    }
    
    object::~object() {
        if (_ptr)
            _ptr->dec();
    }
    
    std::size_t object::hash() const {
        return _ptr ? _ptr->hash() : 0;
    }
    
    template<typename... Args>
    object object::operator()(Args&&... args) const {
        std::array<object, sizeof...(Args)> a{{std::forward<Args>(args)...}};
        _ptr->call(a.begin(), a.end());
    }
    
    
    bool object::operator==(const object& r) const {
        return (_ptr == r._ptr) || ((hash() == r.hash()) && _ptr->equals(r));
    }
    
    object& object::operator[](const object& o) {
        return _ptr->subscript(o);
    }
    
    std::ostream& operator<<(std::ostream& a, const object& b) {
        if (b.get()) {
            b.get()->print();
        } else {
            a << "nil";
        }
        return a;
    }

    class integer
    : public representation {
        int _value;
    public:
        integer() = delete;
        explicit integer(int x) : _value(x) {}
        integer(const integer& r) = default;
        virtual integer* copy() const override {
            return new integer(*this);
        }
        virtual std::size_t hash() const override {
            return std::hash<int>()(_value);
        }
        virtual bool equals(const object& r) const override {
            auto p = dynamic_cast<integer*>(r.get());
            return p && (_value == p->_value);
        }
        virtual void print() const override {
            std::cout << _value;
        }
    }; // class integer
    
    object::object(int x)
    : object(new integer(x)) {
    }
    
    class string
    : public representation {
        string _value;
    public:
        string() = delete;
        explicit string(const char* zstr) : _value(zstr) {}
        string(const string& r) = default;
        virtual string* copy() const override {
            return new string(*this);
        }
        virtual std::size_t hash() const override {
            return std::hash<string>()(_value);
        }
        virtual bool equals(const object& r) const override {
            auto p = dynamic_cast<string*>(r.get());
            return p && (_value == p->_value);
        }
        virtual void print() const override {
            std::cout << "\"" << _value << "\"";
        }
    }; // class string
    
    object::object(const char* s)
    : object(new string(s)) {
    }
    
    struct hasher {
        std::size_t operator()(const object& r) const {
            return r.hash();
        }
    };
    
    class table
    : public representation {
        std::unordered_map<object, object, hasher> _value;
    public:
        table() = delete;
        table(const table& r) = default;
        virtual table* copy() const override {
            return new table(*this);
        }
        virtual std::size_t hash() const override {
            return 0;
        }
        virtual bool equals(const object& r) const override {
            auto p = dynamic_cast<table*>(r.get());
            return p && (_value == p->_value);
        }
        virtual object& subscript(const object& r) override {
            auto i = _value.find(r);
            if (i == _value.end()) {
                auto j = _value.insert(std::pair<const object, object>(object(r.get()->copy()), object()));
                assert(j.second);
                i = j.first;
            }
            return i->second;
        }
        virtual void print() const override {
            std::cout << "{";
            for (auto& a : _value) {
                std::cout << a.first << ": " << a.second << ", ";
            }
            std::cout << "}";
        }
    };
    
};
*/

#endif // object_hpp
