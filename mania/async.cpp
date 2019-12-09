//
//  async.cpp
//  mania
//
//  Created by Antony Searle on 2/12/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include <mutex>
#include <condition_variable>
#include <thread>

#include "async.hpp"
#include "vector.hpp"


namespace manic {

struct notification_base {
    
    std::mutex _mutex;
    std::condition_variable _condition_variable;
    
    bool _cancelled;
    
    notification_base()
    : _cancelled(false) {
    }
    
    void cancel() {
        auto lock = std::unique_lock(_mutex);
        _cancelled = true;
        _condition_variable.notify_all();
    }
    
    struct cancelled {};
    
}; // struct notification_base

template<typename T>
struct notification_queue
: notification_base {
    
    vector<T> _vector;
    
    template<typename... Args>
    void push(Args&&... args) {
        {
            auto lock = std::unique_lock(_mutex);
            _vector.emplace_back(std::forward<Args>(args)...);
        }
        _condition_variable.notify_one();
    }
    
    T pop() {
        auto lock = std::unique_lock(_mutex);
        while (!_cancelled && _vector.empty())
            _condition_variable.wait(lock);
        if (_cancelled)
            throw cancelled{};
        return _vector.pop_front();
    }
    
};

struct thread_pool
: notification_queue<std::function<void()>> {
    
    ~thread_pool() = delete;
    
    isize _effective_threads;
    isize _available_threads;
    isize _max_threads;
    
    thread_pool()
    : _effective_threads(0)
    , _available_threads(0)
    , _max_threads(std::thread::hardware_concurrency()) {
    }
    
    std::function<void()> pop() {
        auto lock = std::unique_lock(_mutex);
        while (!_cancelled && _vector.empty())
            _condition_variable.wait(lock);
        if (_cancelled)
            throw cancelled{};
        return _vector.pop_front();
    }

    
    void run() {
        auto lock = std::unique_lock(_mutex);
        for (;;) {
            ++_available_threads;
            while (!_cancelled && _vector.empty())
                _condition_variable.wait(lock);
            --_available_threads;
            if (_cancelled)
                return;
            std::function<void()> f{_vector.pop_front()};
            lock.unlock();
            f();
            lock.lock();
        }
    }
    
    void spawn() {
        std::thread{[=]() {
            run();
        }}.detach();
    }
    
    template<typename... Args>
    void push(Args&&... args) {
        {
            auto lock = std::unique_lock(_mutex);
            _vector.emplace_back(std::forward<Args>(args)...);
            if (!_available_threads && (_effective_threads < _max_threads)) {
                ++_effective_threads;
                spawn();
            }
        }
        _condition_variable.notify_one();
    }
    
    thread_pool& get() {
        static thread_pool* p = new thread_pool; //  leaked
        return *p;
    }
    
    void blocking_hint_push() {
        auto lock = std::unique_lock(_mutex);
        if (_vector.empty()) {
            --_effective_threads;
        } else {
            lock.unlock();
            spawn();
        }
    }
    
    void blocking_hint_pop() {
        auto lock = std::unique_lock(_mutex);
        ++_effective_threads;
    }
        
}; // struct thread_pool

} // namespace manic
