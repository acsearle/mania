//
//  corrode.hpp
//  aarc
//
//  Created by Antony Searle on 8/8/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#ifndef corrode_hpp
#define corrode_hpp

#include <experimental/coroutine>

#include "maybe.hpp"
#include "reactor.hpp"
#include "pool.hpp"

namespace std::experimental {
        
    template<typename... Args>
    struct coroutine_traits<void, Args...> {
        
        struct promise_type {
            void get_return_object() {}
            suspend_never initial_suspend() { return {}; }
            void return_void() {}
            suspend_never final_suspend() { return {}; }
            void unhandled_exception() { terminate(); }
        };
        
    };
    
}


// await transfer to another thread
//
// dispatches its continuation to the thread pool

inline constexpr struct  {
    bool await_ready() const { return false; }
    template<typename Promise>
    void await_suspend(std::experimental::coroutine_handle<Promise> h) const {
        manic::pool_submit_one(std::move(h));
    }
    void await_resume() const {};
} transfer;

template<typename Rep, typename Period>
struct await_duration {
    std::chrono::duration<Rep, Period> _t;
    explicit await_duration(std::chrono::duration<Rep, Period> t) : _t(t) {}
    bool await_ready() { return false; }
    template<typename T>
    void await_suspend(std::experimental::coroutine_handle<T> h) {
        manic::reactor::get().after(std::move(_t), h);
    }
    void await_resume() {}
};

template<typename Rep, typename Period>
await_duration<Rep, Period> operator co_await(std::chrono::duration<Rep, Period> t) {
    return await_duration<Rep, Period>(std::move(t));
}

template<typename Clock, typename Duration>
struct await_time_point {
    std::chrono::time_point<Clock, Duration> _t;
    explicit await_time_point(std::chrono::time_point<Clock, Duration> t) : _t(std::move(t)) {}
    bool await_ready() { return false; }
    template<typename T>
    void await_suspend(std::experimental::coroutine_handle<T> h) {
        manic::reactor::get().when(std::move(_t), h);
    }
    void await_resume() {}
};




struct async_read {
    
    int _fd;
    void* _buf;
    size_t _count;
    ssize_t _return_value;
    
    async_read(int fd, void* buf, size_t count)
    : _fd(fd)
    , _buf(buf)
    , _count(count)
    , _return_value(-1) {
    }

    void _execute() {
        _return_value = read(_fd, _buf, _count);
        assert(_return_value > 0);
    }

    bool await_ready() {
        //return false;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(_fd, &fds);
        timeval t{0, 0};
        return ((select(_fd + 1, &fds, nullptr, nullptr, &t) == 1)
                && ((void) _execute(), true));
    }
    
    void await_suspend(std::experimental::coroutine_handle<> handle) {
        manic::reactor::get().when_readable(_fd, [=]() mutable {
            _execute();
            handle();
        });
    }
    
    ssize_t await_resume() {
        return _return_value;
    }
    
};



struct async_write {
    
    int _fd;
    void const* _buf;
    size_t _count;
    ssize_t _return_value;
    
    async_write(int fd, void const* buf, size_t count)
    : _fd(fd)
    , _buf(buf)
    , _count(count) {
    }

    void _execute() {
        _return_value = write(_fd, _buf, _count);
        assert(_return_value > 0);
    }

    bool await_ready() {
        //return false;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(_fd, &fds);
        timeval t{0, 0};
        return ((select(_fd + 1, nullptr, &fds, nullptr, &t) == 1)
                && ((void) _execute(), true));
    }
    
    void await_suspend(std::experimental::coroutine_handle<> handle) {
        manic::reactor::get().when_writeable(_fd, [=]() mutable {
            _execute();
            handle();
        });
    }
    
    ssize_t await_resume() {
        return _return_value;
    }
    
};


template<typename T = void>
struct future {
    
    struct promise_type {
        
        std::experimental::coroutine_handle<> _continuation;
        manic::maybe<T> _value;
        
        auto get_return_object() {
            return future<T>{
                std::experimental::coroutine_handle<promise_type>::from_promise(*this)
            };
        }
        auto initial_suspend() { return std::experimental::suspend_always{}; } // <-- do we really want to be lazy?
        void return_value(T x) { _value.emplace(std::move(x)); }
        
        struct final_awaitable {
            
            bool await_ready() { return false; }
            auto await_suspend(std::experimental::coroutine_handle<promise_type> handle) {
                return handle.promise()._continuation;
            }
            void await_resume() {};
        };
        
        auto final_suspend() { return final_awaitable{}; }
        void unhandled_exception() { std::terminate(); }
        
    };
    
    std::experimental::coroutine_handle<promise_type> _continuation;
    
    bool await_ready() {
        return _continuation.done();
    }
    
    auto await_suspend(std::experimental::coroutine_handle<> continuation) {
        _continuation.promise()._continuation = continuation;
        return _continuation;
    }
    
    T await_resume() {
        return std::move(_continuation.promise()._value.value);
    }
    
};


// fixme: refactor out common parts
template<>
struct future<void> {
    
    struct promise_type {
        
        std::experimental::coroutine_handle<> _continuation;
        
        auto get_return_object() {
            return future<void>{
                std::experimental::coroutine_handle<promise_type>::from_promise(*this)
            };
        }
        auto initial_suspend() { return std::experimental::suspend_always{}; }
        void return_void() {};
        
        struct final_awaitable {
            
            bool await_ready() { return false; }
            auto await_suspend(std::experimental::coroutine_handle<promise_type> handle) {
                return handle.promise()._continuation;
            }
            void await_resume() {};
        };
        
        auto final_suspend() { return final_awaitable{}; }
        void unhandled_exception() { std::terminate(); }
        
    };
    
    std::experimental::coroutine_handle<promise_type> _continuation;
    
    bool await_ready() {
        return _continuation.done();
    }
    
    auto await_suspend(std::experimental::coroutine_handle<> continuation) {
        _continuation.promise()._continuation = continuation;
        return _continuation;
    }
    
    void await_resume() {}
        
};



template<typename T>
struct Generator {
        
    struct promise_type {
        
        auto get_return_object() {
            return Generator {
                std::experimental::coroutine_handle<promise_type>::from_promise(*this)
            };
        }
        
        auto initial_suspend() {
            return std::experimental::suspend_always{};
        }
        
        auto final_suspend() {
            return std::experimental::suspend_always{};
        }
        
        template<typename U>
        auto yield_value(U&& u) {
            _value = &u;
            return std::experimental::suspend_always{};
        }
        
        void unhandled_exception() {
            abort();
        }
        
        void return_void() {}
        
        T* _value;
        
    };
    
    std::experimental::coroutine_handle<promise_type> _coroutine;
    
    struct sentinel {};
    
    struct iterator {

        std::experimental::coroutine_handle<promise_type> _coroutine;
        
        iterator& operator++() {
            _coroutine.resume();
            return *this;
        }
        
        bool operator!=(sentinel) {
            return !_coroutine.done();
        }
        
        T& operator*() {
            return *_coroutine.promise()._value;
        }

    };
    
    iterator begin() {
        _coroutine.resume();
        return iterator { _coroutine };
    }
    
    sentinel end() {
        return sentinel {};
    }
    
    ~Generator() {
        _coroutine.destroy();
    }
    
    
};

#endif /* corrode_hpp */
