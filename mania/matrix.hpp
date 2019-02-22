//
//  matrix.hpp
//  mania
//
//  Created by Antony Searle on 16/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef matrix_hpp
#define matrix_hpp

#include <numeric>
#include <vector>
#include <iostream>

namespace manic {
    
    //
    // Views into row-major memory.
    //
    // We use a hierarchy matrix<T> : matrix_view<T> : const_matrix_view<T>
    //
    // matrix_view can only be copy-constructed from a mutable matrix_view&, so
    // that a const matrix cannot be used to produce a non-const matrix_view.
    //
    // The views have reference sematics.  They can be copy constructed, but
    // assignment (if mutable) copies the viewed sequences.  There is no way
    // to change the view, so it behaves like a non-const reference or a
    // T* const.
    
    using std::array;
    
    // A type that holds a T and provides access to it via the pointer
    // interface.  Useful for implementing operator->() on iterators that
    // return a value.
    
    template<typename T>
    class indirect {
        T _value;
    public:
        indirect() = delete;
        indirect(const indirect&) = delete;
        indirect(indirect&&) = default;
        explicit indirect(T&& x) : _value(std::move(x)) {}
        ~indirect() = default;
        indirect& operator=(const indirect&) = delete;
        indirect& operator=(indirect&&) = delete;
        const T* operator->() const { return std::addressof(_value); }
        const T& operator*() const { return _value; }
        explicit operator bool() const { return true; }
        bool operator!() const { return false; }
    };
    


    // Views contiguous objects.  Reference semantics, so assignment copies
    // the viewed elements
    
    template<typename T>
    struct const_vector_view {
        
        static_assert(!std::is_const<T>::value, "T must be mutable");
        
        using value_type = T;
        using size_type = ptrdiff_t;
        using difference_type = ptrdiff_t;
        using reference = const T&;
        using const_reference = const T&;
        using iterator = const T*;
        using const_iterator = const T*;

        
        T* _begin;
        ptrdiff_t _size;
        
        const_vector_view() = delete;
        const_vector_view(const const_vector_view& r) = default;
        const_vector_view(const_vector_view&& r) = default;
        
        const_vector_view(std::nullptr_t, ptrdiff_t n)
        : _begin(nullptr)
        , _size(n) {
        }
        
        const_vector_view(const T* ptr, ptrdiff_t n)
        : _begin(const_cast<T*>(ptr))
        , _size(n) {
        }

        const_vector_view(const T* first, const T* last)
        : _begin(const_cast<T*>(first))
        , _size(last - first) {
        }
        
        const_vector_view(const std::vector<T>& v)
        : _begin(const_cast<T*>(v.data()))
        , _size(v.size()) {
        }
        
        ~const_vector_view() = default;
        
        const_vector_view& operator=(const const_vector_view&) = delete;
        const_vector_view& operator=(const_vector_view&&) = delete;
        
        const T* begin() const { return _begin; }
        const T* end() const { return _begin + _size; }
        ptrdiff_t size() const { return _size; }
        ptrdiff_t columns() const { return _size; }
        const T& operator[](ptrdiff_t i) const { return _begin[i]; }
        const T& operator()(ptrdiff_t i) const { return _begin[i]; }
        const T& front() const { return *_begin; }
        const T& back() const { return _begin[_size - 1]; }
        
        const_vector_view sub(ptrdiff_t i, ptrdiff_t n) const {
            return const_vector_view(_begin + i, n);
        }
        
        void print() const {
            for (auto&& a : *this)
                std::cout << a << ", ";
            std::cout << std::endl;
        }
        
    };
    
    template<typename T, typename U>
    auto dot(const_vector_view<T> a, const_vector_view<U> b) {
        return std::accumulate(a.begin(), a.end(), b.begin(), decltype(std::declval<T>() * std::declval<U>())(0));
    }
    
    template<typename T>
    T sum(const_vector_view<T> a, T b = 0.0) {
        for (ptrdiff_t i = 0; i != a.size(); ++i)
            b += a(i);
        return b;
    }
    
    template<typename T>
    struct vector_view
    : const_vector_view<T> {
        
        using reference = T&;
        using iterator = T*;

        
        vector_view() = delete;

        // Unusal copy semantics ensure that we cannot discard the const
        // qualification of a view by copying it
        
        vector_view(const vector_view&) = delete;
        vector_view(const vector_view&&) = delete;
        vector_view(vector_view&) = default;
        vector_view(vector_view&&) = default;
        
        vector_view(std::nullptr_t, ptrdiff_t n)
        : const_vector_view<T>(nullptr, n) {
        }
        
        vector_view(T* ptr, ptrdiff_t n)
        : const_vector_view<T>(ptr, n) {
        }
        
        vector_view(T* first, T* last)
        : const_vector_view<T>(first, last) {
        }
        
        vector_view(std::vector<T>& v)
        : const_vector_view<T>(v.data(), v.size()) {
        }

        ~vector_view() = default;

        vector_view operator=(const vector_view& v) {
            return *this = static_cast<const_vector_view<T>>(v);
        }

        vector_view operator=(vector_view&& v) {
            return *this = static_cast<const_vector_view<T>>(v);
        }
        
        vector_view operator=(const_vector_view<T> v) {
            assert(this->size() == v.size());
            std::copy(v.begin(), v.end(), begin());
            return *this;
        }
        
        vector_view operator=(T x) {
            std::fill(begin(), end(), x);
            return *this;
        }
        
        template<typename It>
        void assign(It first, It last) {
            assert(std::distance(first, last) == this->_size);
            std::copy(first, last, begin());
        }
        
        void swap(vector_view<T> v) {
            assert(this->size() == v.size());
            std::swap_ranges(begin(), end(), v.begin());
        }
        
        using const_vector_view<T>::begin;
        using const_vector_view<T>::end;
        using const_vector_view<T>::operator[];
        using const_vector_view<T>::operator();
        using const_vector_view<T>::front;
        using const_vector_view<T>::back;
        using const_vector_view<T>::sub;

        T* begin() { return this->_begin; }
        T* end() { return this->_begin + this->_size; }
        T& operator[](ptrdiff_t i) { return this->_begin[i]; }
        T& operator()(ptrdiff_t i) { return this->_begin[i]; }
        T& front() { return *this->_begin; }
        T& back() { return this->_begin[this->_size - 1]; }

        vector_view sub(ptrdiff_t i, ptrdiff_t n) {
            return vector_view(this->_begin + i, n);
        }

    }; // struct vector_view<T>
    
    template<typename T>
    vector_view<T> operator/=(vector_view<T> a, T b) {
        for (ptrdiff_t i = 0; i != a.size(); ++i)
            a(i) /= b;
        return a;
    }

    template<typename T>
    vector_view<T> operator*=(vector_view<T> a, T b) {
        for (ptrdiff_t i = 0; i != a.size(); ++i)
            a(i) *= b;
        return a;
    }

    
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
    struct vector : vector_view<T>, raw_vector<T> {
        
        vector() = default;
        
        vector(const vector& v) : vector(static_cast<const_vector_view<T>>(v)) {}
        vector(vector&& v) = default;
        
        explicit vector(ptrdiff_t n)
        : vector_view<T>(nullptr, 0)
        , raw_vector<T>(n) {
            this->_begin = this->_allocation;
            this->_size = n;
            std::uninitialized_default_construct_n(this->_begin, n);
        }
        
        explicit vector(const_vector_view<T> v)
        : vector_view<T>(nullptr, 0)
        , raw_vector<T>(v._size) {
            this->_begin = this->_allocation;
            this->_size = v._size;
            std::uninitialized_copy_n(v._begin, v._size, this->_begin);
        }
        
        ~vector() {
            std::destroy_n(this->_begin, this->_size);
        }
        
        vector& operator=(const vector& v) {
            return *this = static_cast<const_vector_view<T>>(v);
        }
        
        vector& operator=(vector&& v) {
            vector(std::move(v)).swap(*this);
            return *this;
        }
        
        vector& operator=(const_vector_view<T> v) {
            // do better
            vector(v).swap(*this);
            return *this;
        }
        
        void swap(vector& v) {
            vector_view<T>::swap(v);
            raw_vector<T>::swap(v);
        }
        
        void resize(ptrdiff_t n);
        void resize(ptrdiff_t n, const T& x);
        
        
        
    };
    
    
    
    template<typename T>
    struct const_matrix_iterator {
        
        T* _begin;
        ptrdiff_t _columns;
        ptrdiff_t _stride;
        
        using difference_type = ptrdiff_t;
        using value_type = const_vector_view<T>;
        using pointer = indirect<const_vector_view<T>>;
        using reference = const_vector_view<T>;
        using iterator_category = std::random_access_iterator_tag;
        
        const_matrix_iterator() : _begin(nullptr), _columns(0), _stride(0) {}
        
        const_matrix_iterator(const const_matrix_iterator&) = default;
        
        const_matrix_iterator(const T* ptr, ptrdiff_t columns, ptrdiff_t stride)
        : _begin(const_cast<T*>(ptr))
        , _columns(columns)
        , _stride(stride) {
            assert(columns <= stride);
        }
        
        ~const_matrix_iterator() = default;
        
        const_matrix_iterator& operator=(const_matrix_iterator const&) = default;
        
        const_vector_view<T> operator*() const {
            return const_vector_view<T>(_begin, _columns);
        }
        
        const_vector_view<T> operator[](ptrdiff_t i) const {
            return const_vector_view<T>(_begin + i * _stride, _columns);
        }
        
        const_matrix_iterator& operator++() {
            _begin += _stride;
            return *this;
        }
        
        const_matrix_iterator& operator--() {
            _begin -= _stride;
            return *this;
        }
        
        const_matrix_iterator& operator+=(ptrdiff_t i) {
            _begin += _stride * i;
            return *this;
        }

        const_matrix_iterator& operator-=(ptrdiff_t i) {
            _begin -= _stride * i;
            return *this;
        }
        
        indirect<const_vector_view<T>> operator->() const {
            return indirect(**this);
        }

    };

    template<typename T>
    const_matrix_iterator<T> operator+(const_matrix_iterator<T> i, ptrdiff_t n) {
        return i += n;
    }
    
    template<typename T>
    bool operator==(const_matrix_iterator<T> a, const_matrix_iterator<T> b) {
        return a._begin == b._begin;
    }
    
    template<typename T>
    bool operator!=(const_matrix_iterator<T> a, const_matrix_iterator<T> b) {
        return a._begin != b._begin;
    }

    template<typename T>
    ptrdiff_t operator-(const_matrix_iterator<T> a, const_matrix_iterator<T> b) {
        assert(a._stride);
        assert(a._stride == b._stride);
        return (a._begin - b._begin) / a._stride;
    }
    
    
    template<typename T>
    struct matrix_iterator : const_matrix_iterator<T> {
        
        using difference_type = ptrdiff_t;
        using value_type = vector_view<T>;
        using pointer = indirect<vector_view<T>>;
        using reference = vector_view<T>;
        using iterator_category = std::random_access_iterator_tag;
        
        matrix_iterator() = default;
        matrix_iterator(const matrix_iterator&) = default;
        
        matrix_iterator(T* ptr, ptrdiff_t columns, ptrdiff_t stride)
        : const_matrix_iterator<T>(ptr, columns, stride) {
        }
        
        ~matrix_iterator() = default;
        
        matrix_iterator& operator=(const matrix_iterator&) = default;
        
        vector_view<T> operator*() const {
            return vector_view<T>(this->_begin, this->_columns);
        }
        
        vector_view<T> operator[](ptrdiff_t i) const {
            return vector_view<T>(this->_begin + i * this->_stride,
                                  this->_columns);
        }
        
        indirect<vector_view<T>> operator->() const {
            return indirect(**this);
        }
        
        matrix_iterator& operator++() {
            this->_begin += this->_stride;
            return *this;
        }
        
        matrix_iterator& operator--() {
            this->_begin -= this->_stride;
            return *this;
        }
        
        matrix_iterator& operator+=(ptrdiff_t i) {
            this->_begin += this->_stride * i;
            return *this;
        }
        
        matrix_iterator& operator-=(ptrdiff_t i) {
            this->_begin -= this->_stride * i;
            return *this;
        }
        
    };
    
    template<typename T>
    matrix_iterator<T> operator+(matrix_iterator<T> a, ptrdiff_t b) {
        return matrix_iterator<T>(a._begin + b * a._stride,
                                  a._columns,
                                  a._stride);
    }

    template<typename T>
    matrix_iterator<T> operator-(matrix_iterator<T> a, ptrdiff_t b) {
        return matrix_iterator<T>(a._begin - b * a._stride,
                                  a._columns,
                                  a._stride);
    }

    
    template<typename T>
    struct const_matrix_view {
        
        using value_type = const_vector_view<T>;
        using size_type = ptrdiff_t;
        using difference_type = ptrdiff_t;
        using reference = value_type;
        using const_reference = value_type;
        using iterator = const_matrix_iterator<T>;
        using const_iterator = const_matrix_iterator<T>;
        

        T* _begin;
        ptrdiff_t _columns;
        ptrdiff_t _stride;
        ptrdiff_t _rows;
        
        const_matrix_view() = delete;
        const_matrix_view(const const_matrix_view&) = default;
        
        const_matrix_view(const T* ptr,
                          ptrdiff_t columns,
                          ptrdiff_t stride,
                          ptrdiff_t rows)
        : _begin(const_cast<T*>(ptr))
        , _columns(columns)
        , _stride(stride)
        , _rows(rows) {
        }
        
        ~const_matrix_view() = default;
        
        const_matrix_view& operator=(const const_matrix_view&) = delete;
        const_matrix_view& operator=(const_matrix_view&&) = delete;
        
        const T* data() const { return _begin; }
        ptrdiff_t columns() const { return _columns; }
        ptrdiff_t width() const { return _columns; }
        ptrdiff_t size() const { return _rows; }
        ptrdiff_t rows() const { return _rows; }
        ptrdiff_t height() const { return _rows; }
        ptrdiff_t stride() const { return _stride; }

        const_iterator begin() const { return const_iterator(_begin, _columns, _stride); }
        const_iterator end() const { return begin() + _rows; }
        const_iterator cbegin() const { return const_iterator(_begin, _columns, _stride); }
        const_iterator cend() const { return begin() + _rows; }

        
        const_vector_view<T> operator[](ptrdiff_t i) const {
            return const_vector_view<T>(_begin + i * _stride, _columns);
        }
        
        const T& operator()(ptrdiff_t i, ptrdiff_t j) const {
            assert(i >= 0);
            assert(j >= 0);
            assert(i < _rows);
            assert(j < _columns);
            return *(_begin + i * _stride + j);
        }
        
        const_vector_view<T> front() const { return *begin(); }
        const_vector_view<T> back() const { return begin()[_rows - 1]; }

        const_matrix_view<T> sub(ptrdiff_t i,
                                 ptrdiff_t j,
                                 ptrdiff_t r,
                                 ptrdiff_t c) const {
            assert(0 <= i);
            assert(0 <= j);
            assert(0 < r);
            assert(i + r <= _rows);
            assert(0 < c);
            assert(j + c <= _columns);
            return const_matrix_view(_begin + i * _stride + j, c, _stride, r);
        }
        
        void print() const {
            for (auto&& row : *this) {
                for (auto&& value : row)
                    std::cout << value << ' ';
                std::cout << '\n';
            }
        }

        
    };
    
    template<typename T>
    bool operator==(const_matrix_view<T> a, const_matrix_view<T> b) {
        assert(a.rows() == b.rows());
        assert(a.columns() == b.columns());
        return std::equal(a.begin(), a.end(), b.begin());
    }
    
    template<typename T>
    bool operator!=(const_matrix_view<T> a, const_matrix_view<T> b) {
        return !(a == b);
    }
    
    
    template<typename T>
    struct matrix_view : const_matrix_view<T> {
        
        
        using value_type = const_vector_view<T>;
        using reference = value_type;
        using iterator = matrix_iterator<T>;
        
        
        matrix_view() = delete;
        
        matrix_view(matrix_view&) = default;
        matrix_view(matrix_view&&) = default;
        matrix_view(const matrix_view&) = delete;
        matrix_view(const matrix_view&&) = delete;

        matrix_view(T* ptr,
                    ptrdiff_t columns,
                    ptrdiff_t stride,
                    ptrdiff_t rows)
        : const_matrix_view<T>(ptr,
                               columns,
                               stride,
                               rows) {
        }
        
        ~matrix_view() = default;
        
        matrix_view& operator=(const matrix_view& v) {
            return *this = static_cast<const_matrix_view<T>>(v);
        }
        
        matrix_view& operator=(const_matrix_view<T> v) {
            assert(this->_rows == v._rows);
            std::copy(v.begin(), v.end(), begin());
            return *this;
        }
        
        matrix_view& operator=(const T& x) {
            std::fill(begin(), end(), x);
            return *this;
        }
        
        using const_matrix_view<T>::data;
        using const_matrix_view<T>::begin;
        using const_matrix_view<T>::end;
        using const_matrix_view<T>::operator[];
        using const_matrix_view<T>::operator();
        using const_matrix_view<T>::front;
        using const_matrix_view<T>::back;
        using const_matrix_view<T>::sub;

        T* data() { return this->_begin; }
        
        iterator begin() {
            return iterator(this->_begin,
                            this->_columns,
                            this->_stride);
        }
        
        iterator end() { return begin() + this->_rows; }
        
        vector_view<T> operator[](ptrdiff_t i) {
            assert(0 <= i);
            assert(i < this->_rows);
            return vector_view<T>(this->_begin + i * this->_stride, this->_columns);
        }


        vector_view<T> front() { return *begin(); }
        vector_view<T> back() { return begin()[this->_rows - 1]; }

        matrix_view<T> sub(ptrdiff_t i,
                           ptrdiff_t j,
                           ptrdiff_t r,
                           ptrdiff_t c) {
            assert(0 <= i);
            assert(0 <= j);
            assert(0 <= r);
            assert(i + r <= this->_rows);
            assert(0 <= c);
            assert(j + c <= this->_columns);
            return matrix_view<T>(this->_begin + i * this->_stride + j, c, this->_stride, r);
        }

        T& operator()(ptrdiff_t i, ptrdiff_t j) {
            assert(i >= 0);
            assert(j >= 0);
            assert(i < this->_rows);
            assert(j < this->_columns);
            return *(this->_begin + i * this->_stride + j);
        }
        
        void swap(matrix_view<T> v) {
            for (ptrdiff_t i = 0; i != this->_rows; ++i)
                operator[](i).swap(v[i]);
        }
        
        matrix_view& operator/=(const T& x) {
            for (ptrdiff_t i = 0; i != this->_rows; ++i)
                for (ptrdiff_t j = 0; j != this->_columns; ++j)
                    operator()(i, j) /= x;
            return *this;
        }
        
        matrix_view& operator*=(const T& x) {
            for (ptrdiff_t i = 0; i != this->_rows; ++i)
                for (ptrdiff_t j = 0; j != this->_columns; ++j)
                    operator()(i, j) *= x;
            return *this;
        }
        
        matrix_view& operator+=(const_matrix_view<T> x) {
            for (ptrdiff_t i = 0; i != this->_rows; ++i)
                for (ptrdiff_t j = 0; j != this->_columns; ++j)
                    operator()(i, j) += x(i, j);
            return *this;
        }

        matrix_view& operator-=(const_matrix_view<T> x) {
            for (ptrdiff_t i = 0; i != this->_rows; ++i)
                for (ptrdiff_t j = 0; j != this->_columns; ++j)
                    operator()(i, j) -= x(i, j);
            return *this;
        }

        matrix_view& operator*=(const_matrix_view<T> x) {
            for (ptrdiff_t i = 0; i != this->_rows; ++i)
                for (ptrdiff_t j = 0; j != this->_columns; ++j)
                    operator()(i, j) *= x(i, j);
            return *this;
        }


    }; // struct matrix_view<T>
    
    template<typename T>
    void swap(matrix_view<T> a, matrix_view<T> b) {
        a.swap(b);
    }
    
    template<typename T>
    struct matrix : matrix_view<T> {
        
        std::vector<T> _store;
        
        bool _invariant() {
            assert(this->_columns >= 0);
            assert(this->_stride >= 0);
            assert(this->_rows >= 0);
            assert(this->_begin >= _store.data());
            assert(this->_stride >= this->_columns);
            assert(this->_begin + (this->_rows - 1) * this->_stride + this->_columns <= _store.data() + _store.size());
            return true;
        }
        
        
        
        
        matrix() : matrix_view<T>(nullptr, 0, 0, 0), _store() {}
        
        matrix(const matrix& r) : matrix() {
            *this = r;
        }
        
        matrix(matrix&& r) : matrix() {
            r.swap(*this);
        }
        
        matrix(const_matrix_view<T> v)
        : matrix() {
            operator=(v);
        }
        
        matrix(ptrdiff_t rows, ptrdiff_t columns)
        : matrix() {
            discard_and_resize(rows, columns);
        }
        
        matrix(ptrdiff_t rows, ptrdiff_t columns, T x)
        : matrix(rows, columns) {
            *this = x;
        }
        
        ~matrix() = default;
        
        matrix& operator=(const matrix& r) {
            return operator=(static_cast<const_matrix_view<T>>(r));
        }
        
        matrix& operator=(matrix&& r) {
            matrix(std::move(r)).swap(*this);
            return *this;
        }
        
        matrix& operator=(const_matrix_view<T> r) {
            discard_and_resize(r.rows(), r.columns());
            std::copy(r.begin(), r.end(), this->begin());
            return *this;
        }
        
        matrix& operator=(const T& x) {
            std::fill(this->begin(), this->end(), x);
            return *this;
        }
        
        
        using matrix_view<T>::swap;
        void swap(matrix& r) {
            using std::swap;
            swap(this->_begin, r._begin);
            swap(this->_columns, r._columns);
            swap(this->_stride, r._stride);
            swap(this->_rows, r._rows);
            swap(this->_store, r._store);
        }
        
        // Mutators
        
        void crop(ptrdiff_t i, ptrdiff_t j, ptrdiff_t r, ptrdiff_t c) {
            assert(i >= 0);
            assert(j >= 0);
            assert(r >= 0);
            assert(c >= 0);
            assert(i + r <= this->_rows);
            assert(j + c <= this->_columns);
            this->_begin += i * this->_stride + j;
            this->_columns = c;
            this->_rows = r;
        }
        
        void discard_and_resize(ptrdiff_t rows, ptrdiff_t columns) {
            _store.resize(rows * columns, 0);
            this->_begin = _store.data();
            this->_columns = columns;
            this->_stride = columns;
            this->_rows = rows;
        }
        
        void expand(ptrdiff_t i, ptrdiff_t j, ptrdiff_t r, ptrdiff_t c, T x) {
            matrix<T> a(r, c, x);
            a.sub(i, j, this->_rows, this->_columns) = *this;
            a.swap(*this);
        }
        
        void resize(ptrdiff_t r, ptrdiff_t c, T x = T()) {
            matrix<T> a(r, c, x);
            r = std::min(r, this->_rows);
            c = std::min(c, this->_columns);
            a.sub(0, 0, r, c) = this->sub(0, 0, r ,c);
            a.swap(*this);
        }
        
    };
    
    template<typename T>
    void swap(matrix<T>& a, matrix<T>& b) {
        a.swap(b);
    }
    
    template<typename T> matrix<T> operator+(matrix_view<T> a, matrix_view<T> b) {
        assert(a.rows() == b.rows());
        assert(b.columns() == b.columns());
        matrix<T> c(a.rows(), a.columns());
        for (ptrdiff_t i = 0; i != a.rows(); ++i)
            for (ptrdiff_t j = 0; j != a.columns(); ++j)
                c(i, j) = a(i, j) + b(i, j);
        return c;
    }
    

    template<typename T> matrix<T> operator+(matrix_view<T> a, T b) {
        matrix<T> c(a.rows(), a.columns());
        for (ptrdiff_t i = 0; i != a.rows(); ++i)
            for (ptrdiff_t j = 0; j != a.columns(); ++j)
                c(i, j) = a(i, j) + b;
        return c;
    }
 
    template<typename T> matrix<T> transpose(const_matrix_view<T> a) {
        matrix<T> b(a.columns(), a.rows());
        for (ptrdiff_t i = 0; i != a.rows(); ++i)
            for (ptrdiff_t j = 0; j != a.columns(); ++j)
                b(j, i) = a(i, j);
        return b;
    }
    
    template<typename T> matrix<T> outer_product(const_vector_view<T> a,
                                                 const_vector_view<T> b) {
        matrix<T> c(a.size(), b.size());
        for (ptrdiff_t i = 0; i != c.rows(); ++i)
            for (ptrdiff_t j = 0; j != c.columns(); ++j)
                c(i, j) = a[i] * b[j];
        return c;
    }
    
    template<typename T>
    matrix<T> operator-(const_matrix_view<T> a, const_matrix_view<T> b) {
        matrix<T> c(a.rows(), a.columns());
        for (ptrdiff_t i = 0; i != a.rows(); ++i)
            for (ptrdiff_t j = 0; j != a.columns(); ++j)
                c(i, j) = a(i, j) - b(i, j);
        return c;
    }
    
}

#endif /* matrix_hpp */
