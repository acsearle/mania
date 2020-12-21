// epoch-based garbage collection
//
// in imitation of Crossbeam https://github.com/crossbeam-rs/crossbeam in an
// attempt to grok it

#include <array>
#include <functional>
#include <map>
#include <optional>
#include <thread>
#include <vector>

#include "atomic.hpp"
#include "corrode.hpp"
#include "epoch.hpp"
#include "tagged.hpp"

#include <catch2/catch.hpp>

namespace manic {

template<typename F>
void defer(F&& f);


template<typename T>
struct Queue {
    
    struct Node {
        Atomic<Node const*> next;
        union { T payload; };
    };
    
    // false sharing
    Atomic<Node const*> head;
    Atomic<Node const*> tail;
    
    Queue()
    : Queue(new Node { nullptr }) {
    }
    
    explicit Queue(Node const* ptr)
    : head(ptr)
    , tail(ptr) {
    }
    
    void push(T x) {
        Node const* desired = new Node { nullptr, std::move(x) };
        Node const* tail = this->tail.load(std::memory_order_acquire);
        for (;;) {
            assert(tail);
            Node const* next = tail->next.load(std::memory_order_acquire);
            if (!next && tail->next.compare_exchange_strong(next,
                                                            desired,
                                                            std::memory_order_release,
                                                            std::memory_order_acquire)) {
                return;
            }
            assert(next);
            if (this->tail.compare_exchange_strong(tail,
                                                   next,
                                                   std::memory_order_release,
                                                   std::memory_order_acquire)) {
                tail = next;
            }
        }
    }
    
    std::optional<T> try_pop() {
        Node const* head = this->head.load(std::memory_order_acquire);
        for (;;) {
            assert(head);
            if (Node const* next = head->next.load(std::memory_order_acquire)) {
                if (this->head.compare_exchange_strong(head,
                                                       next,
                                                       std::memory_order_release,
                                                       std::memory_order_acquire)) {
                    defer([=] { delete head; });
                    T tmp{std::move(next->payload)};
                    next->payload.~T();
                    return tmp;
                }
            } else {
                return std::optional<T>{};
            }
        }
    }
    
}; // struct Queue


struct Deferred {
    std::function<void()> _array[32];
    usize _filled;
    u64 _epoch;
};

struct Local;

struct Global {
    
    Atomic<u64> epoch;
    Atomic<TaggedPtr<Local>> head;
    
    static Global const& get() {
        // global is deliberately leaked at shutdown
        static Global* global = new Global {
            0,
            TaggedPtr<Local> { nullptr }
        };
        return *global;
    }
    
};

struct Local {
    
public:
    
    Atomic<u64> epoch; // <-- written frequently
    Atomic<TaggedPtr<Local>> next; // <-- written only when a thread ends
    
private:
    
    mutable u64 _local_epoch;
    mutable Deferred _deferred;
    
    explicit Local(TaggedPtr<Local> ptr)
    : _deferred()
    , epoch(0)
    , _local_epoch(0)
    , next(ptr) {
    }
    
    static Local* make() {
        // insert node at head of list
        Atomic<TaggedPtr<Local>> const& head = Global::get().head;
        Local* desired = new Local(head.load(std::memory_order_relaxed));
        while (!head.compare_exchange_weak(desired->next,
                                           TaggedPtr<Local> { desired },
                                           std::memory_order_release,
                                           std::memory_order_relaxed)) {}
        return desired;
    }
    
    void mark() const {
        // mark node for deletion
        this->next.fetch_or(1, std::memory_order_release);
    }
        
public:
        
    static Local const& get() {
        thread_local struct Guard {
            Local const* local;
            Guard() : local(make()) {}
            ~Guard() { local->mark(); }
        } guard;
        return *guard.local;
    }
    
    void defer(std::function<void()> f) const {
        _deferred.emplace_back(_local_epoch, std::move(f));
    }
    
    // be careful of coroutine heap allocation overhead; but this should be
    // infrequently called
    Generator<u64> epochs() const {
        Atomic<TaggedPtr<Local>> const* pred = &Global::get().head;
        TaggedPtr<Local> curr = pred->load(std::memory_order_acquire);
        while (curr.ptr) {
            TaggedPtr<Local> next = curr->next.load(std::memory_order_relaxed);
            if (!next.tag) {
                // read the epoch
                co_yield curr->epoch.load(std::memory_order_relaxed);
            } else if (!curr.tag) {
                // node is marked for deletion and predecessor is not, so we can
                // attempt to delete it
                if (pred->compare_exchange_strong(curr,
                                                  next & 0,
                                                  std::memory_order_relaxed,
                                                  std::memory_order_relaxed)) {
                    // we won the race to delete the node, now we defer its actual
                    // reclamation
                    this->defer([ptr = curr.ptr] {
                        delete ptr;
                    });
                }
                continue;
            }
            // advance
            pred = &curr->next;
            curr = next;
        }
    }
    
}; // struct Local

TEST_CASE("epoch", "[epoch]") {
    
    return;
    
    std::size_t n = 8;
    
    std::vector<std::thread> t;
    for (int i = 0; i != n; ++i) {
        t.emplace_back([=] {
            
            Global const& global = Global::get();
            Local const& local = Local::get();
            
            for (;;) {

                // see crossbeam repin
                u64 local_epoch = local.epoch.load(std::memory_order_relaxed);
                u64 global_epoch = global.epoch.load(std::memory_order_relaxed);
                if (local_epoch != global_epoch) {
                    // unsafe for old memory accesses to leak into new epoch
                    local.epoch.store(global_epoch, std::memory_order_release);
                    // safe for new memory accesses to leak into old epoch
                }
                
                
                // we should advance "infrequently"
                // one defintion could be 1/nth of the time when n is number of
                // threads, since only one thread every epoch actually advances
                // it, and the others just overwrite the advance
                // see crossbeam try_advance
                bool flag = true;
                for (auto e : local.epochs()) {
                    if (e != local_epoch) {
                        flag = false;
                        break;
                    }
                }
                
                if (flag) {
                    // synchronize with other threads only when advancing
                    std::atomic_thread_fence(std::memory_order_acquire);
                    global.epoch.store(local_epoch + 1, std::memory_order_release);
                    printf("%d %llu\n", i, local_epoch);
                }
                
                // to sleep and wake we will need to implement crossbeam pin and unpin
                
            }
            
        });
    }
    
    
    while (!t.empty()) {
        t.back().join();
        t.pop_back();
    }
    
}

}
