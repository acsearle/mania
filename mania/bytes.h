#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <utility>

#define NOMINMAX
#include <Windows.h>

// contiguous byte deque
//
// it is efficient (amortized O(1)) to insert and erase bytes from both ends
//
// partial reads from the front and partial writes to the back are allowed
// with the may/did idiom, as in 
// 
//     n = read(sock, buf.may_write_back(n), n);
//     if (n >= 0)
//         buf.did_write_back(n);
//
// employs platform-specific techniques to get the true size of allocations
//     Windows  HeapSize
//     macOS    malloc_size
//
// employs platform-specific techniques to expand allocations in place
//     Windows  HeapReAlloc
//     macOS    ?

struct bytes {

	std::byte* _begin;
	std::byte* _end;
	std::byte* _alloc;
	std::byte* _capacity;

	bytes() noexcept 
		: _begin(nullptr)
		, _end(nullptr)
		, _alloc(nullptr)
		, _capacity(nullptr) {
	}
	
	bytes(bytes const&) = delete;
	
	bytes(bytes&& other) noexcept
		: _begin(std::exchange(other._begin, nullptr)) 
		, _end(std::exchange(other._end, nullptr))
		, _alloc(std::exchange(other._alloc, nullptr))
		, _capacity(std::exchange(other._capacity, nullptr)) {
	}
	
	~bytes() { 
		if (_alloc) {
			//operator delete(_alloc);
			[[maybe_unused]] auto r = HeapFree(GetProcessHeap(), 0, _alloc);
			assert(r);
		}
	}
	
	void swap(bytes& other) noexcept {
		using std::swap;
		swap(_begin, other._begin);
		swap(_end, other._end);
		swap(_alloc, other._alloc);
		swap(_capacity, other._capacity);
	}

	bytes clone() {
		bytes c;
		std::memcpy(c.write_back(size()), _begin, size());
		return c;
	}

	void shrink_to_fit() {
		clone().swap(*this);
	}
	
	bytes& operator=(bytes const&) = delete;
	
	bytes& operator=(bytes&& other) noexcept {
		bytes(std::move(other)).swap(*this);
		return *this;
	}

	bool is_empty() const { return _end == _begin; }
	std::size_t size() const { return _end - _begin; }
	
	void clear() { _begin = _end; }

	std::byte* begin() { return _begin;  }
	std::byte* end() { return _end; }
	std::byte const* begin() const { return _begin; }
	std::byte const* end() const { return _end; }
	std::byte const* cbegin() const { return _begin; }
	std::byte const* cend() const { return _end; }

	std::byte* write_back(std::size_t n) { 
		reserve_back(n);  
		return std::exchange(_end, _end + n); 
	}

	std::byte const* read_front(std::size_t n) {
		assert(n <= size());
		return std::exchange(_begin, _begin + n);
	}
	
	std::byte* write_front(std::size_t n) { 
		reserve_front(n); 
		return _begin -= n; 
	}

	std::byte const* read_back(std::size_t n) { 
		assert(n <= size()); 
		return _end -= n; 
	}

	std::byte const* peek_front(std::size_t n) const {
		assert(n <= size());
		return _begin;
	}

	std::byte const* peek_back(std::size_t n) const {
		assert(n <= size());
		return _end - n;
	}

	std::size_t can_write_back() const { return capacity_back(); }
	std::byte* may_write_back(std::size_t n) { reserve_back(n); return _end; }
	void did_write_back(std::size_t n) { assert(n <= capacity_back()); _end += n; }

	std::size_t can_write_front() const { return capacity_front(); }

	std::size_t can_read_front() const { return size(); }
	std::byte const* may_read_front(std::size_t n) const { assert(n <= size()); return _begin; }
	void did_read_front(std::size_t n) { assert(n <= size()); _begin += n; }
	
	std::size_t can_read_back() const { return size(); }

	std::size_t can_unread_front() const { return capacity_front(); }
	void unread_front(std::size_t n) {
		assert(n <= capacity_front());
		_begin -= n;
	}

	std::size_t can_unread_back() const { return capacity_back(); }
	void unread_back(std::size_t n) {
		assert(n <= capacity_back());
		_end += n;
	}

	std::size_t can_unwrite_back() const { return size(); }
	void unwrite_back(std::size_t n) {
		assert(n <= size());
		_end -= n;
	}

	std::size_t can_unwrite_front() const { return size(); }
	void unwrite_front(std::size_t n) {
		assert(n <= size());
		_begin += n;
	}

	std::size_t capacity_front() const { return _begin - _alloc; }
	std::size_t capacity_back() const { return _capacity - _end; }

	void push_back(std::byte x) { reserve_back(1); *_end++ = x; }

	void reserve_back(std::size_t n) {
		if (capacity_back() < n) {
			auto heap = GetProcessHeap();
			auto m = size();
			std::size_t c = _capacity - _alloc;
			std::byte* p = nullptr;
			// attempt to relocate within the existing allocation, O(m)
			if ((capacity_front() >= 2 * m) && (c >= n + 2 * m)) {
				std::memcpy(_alloc + m, _begin, m);
				_end = (_begin = _alloc + m) + m;
				// amortization: after performing O(m) copy we have to write
				// at least O(m) bytes before we will need to move again
				assert(capacity_front() >= m);
				assert(capacity_back() >= m);
			}
			// minimally extend in-place, O(1) (but...?)
			else if (_alloc && (p = (std::byte*)HeapReAlloc(heap, HEAP_REALLOC_IN_PLACE_ONLY, _alloc, (_end - _alloc) + n))) {
				assert(p == _alloc);
				c = HeapSize(heap, 0, p); // get true allocation size
				assert(c != (SIZE_T)-1);
				_capacity = _alloc + c;
			}
			// allocate and relocate, O(m)
			else {
				c = std::max({ 2 * c, 2 * m + n, 3 * m, });
				// p = (std::byte*) operator new(c);
				p = (std::byte*)HeapAlloc(heap, 0, c);
				assert(p);
				c = HeapSize(heap, 0, p);
				assert(c != (SIZE_T)-1);
				std::memcpy(p + m, _begin, m);
				//operator delete(_alloc);
				if (_alloc) {
					[[maybe_unused]] auto r = HeapFree(heap, 0, _alloc);
					assert(r);
				}
				_capacity = (_alloc = p) + c;
				_end = (_begin = _alloc + m) + m;
				// amortization: after performing O(m) copy we have to write
				// at least O(m) bytes before we will need to move again
				assert(capacity_front() >= m);
				assert(capacity_back() >= m);
			}
			assert(capacity_back() >= n); // postcondition
		}
	}

	void reserve_front(std::size_t n) {
		if (capacity_front() < n) {
			auto m = size();
			std::size_t c = _capacity - _alloc;
			// We must at least perform an O(m) copy
			if ((capacity_back() >= 2 * m) && (c >= n + 2 * m)) {
				std::memcpy(_capacity - 2 * m, _begin, m);
			} else {
				// We must perform an allocation
				c = std::max({ 2 * c, 2 * m + n, 3 * m, });
				// auto p = (std::byte*) operator new(c);
				auto heap = GetProcessHeap();
				assert(c > 0);
				auto p = (std::byte*)HeapAlloc(heap, 0, c);
				assert(p);
				c = HeapSize(heap, 0, p);
				assert(c != (SIZE_T)-1);
				std::memcpy(p + c - 2 * m, _begin, m);
				//operator delete(_alloc);
				if (_alloc) {
					[[maybe_unused]] auto r = HeapFree(heap, 0, _alloc);
					assert(r);
				}
				_capacity = (_alloc = p) + c;
			}
			_begin = (_end = _capacity - m) - m;
			// There are at least m free bytes at the beginning and the end,
			// ensuring we must perform at least m operations before triggering
			// another resize
			assert(capacity_front() >= m);
			assert(capacity_back() >= m);
			// Postcondition is satisifed
			assert(capacity_front() >= n);
		}
	}

}; // struct bytes