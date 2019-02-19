//
//  matrix.hpp
//  mania
//
//  Created by Antony Searle on 16/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef matrix_hpp
#define matrix_hpp

#include <vector>

namespace manic {
    
    /*
    template<typename T, typename ContiguousContainer = std::vector<T>>
    struct matrix {
        
        using value_type = typename ContiguousContainer::value_type;
        using size_type = typename ContiguousContainer::size_type;
        using difference_type = typename ContiguousContainer::difference_type;
        using pointer = typename ContiguousContainer::pointer;
        
        ContiguousContainer _data;
        size_type  _m, _n;
        
        matrix() : _m(0), _n(0) {}
        matrix(const matrix&) = default;
        matrix(matrix&&) = default;
        ~matrix() = default;
        matrix& operator=(const matrix&) = default;
        matrix& operator=(matrix&&) = default;
        
        matrix(size_t rows, size_t columns)
        : _m(rows)
        , _n(columns)
        , _data(rows * columns) {}
        
        void resize(size_t rows, size_t columns) {
            _data.resize(rows * columns);
        }
        
        size_type rows() const { return _m; }
        size_type columns() const {return _m; }
        
        value_type* data() { return _data.data(0); }
        
        value_type& operator()(size_type i, size_type j) {
            assert(i < _m);
            assert(j < _n);
            return _data[i * _n + j];
        }

        value_type* operator[](size_type i) {
            assert(i < _m);
            return _data.data() + i * _n;
        }

        const value_type* operator[](size_type i) const {
            assert(i < _m);
            return _data.data() + i * _n;
        }

        
    };
     */
    
    
    
    using std::vector;
    using std::array;
    
    // A type that holds a T and provides access to it via the pointer interface.
    // Useful for implementing operator->() on iterators that
    
    template<typename T>
    class indirect {
        const T _value;
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
        operator bool() const { return true; }
        bool operator!() const { return false; }
    };
    
    template<typename T>
    indirect(T&& x) -> indirect<T>;
    
    // Views a sequence of objects in
    
    template<typename T>
    class vector_view {
        
        T* const _begin;
        T* const _end;
        
    public:
        
        vector_view() = delete;
        
        vector_view(vector_view const& r) = default;
        
        vector_view(T* first, T* last)
        : _begin(first)
        , _end(last) {
        }
        
        ~vector_view() = default;
        
        vector_view const& operator=(vector_view const& r) const {
            assert(r.size() == size());
            std::copy(r.begin(), r.end(), _begin);
            return *this;
        }
        
        vector_view const& operator=(T x) const {
            std::fill(begin(), end(), x);
            return *this;
        }
        
        template<typename It>
        void assign(It first, It last) {
            assert(std::distance(first, last) == size());
            std::copy(first, last, begin());
        }
        
        ptrdiff_t size() const {
            return _end - _begin;
        }
        
        T* begin() const {
            return _begin;
        }
        
        T* end() const {
            return _end;
        }
        
        T& operator[](ptrdiff_t i) const {
            return _begin[i];
        }
        
        vector_view sub(ptrdiff_t i, ptrdiff_t n) {
            return vector_view(_begin + i, _begin + i + n);
        }
        
    };
    
    template<typename T>
    class const_matrix_view;
    
    template<typename T>
    class const_matrix;
    
    template<typename T>
    class matrix;
    
    template<typename T>
    class matrix_view {
        
    public:
        
        class iterator {
            
        public:
            
            using difference_type = ptrdiff_t;
            using value_type = vector_view<T>;
            using pointer = void;
            using reference = vector_view<T>;
            using iterator_category = std::random_access_iterator_tag;
            
            
            iterator() : _begin(nullptr), _stride(0), _columns(0) {}
            
            iterator(iterator const&) = default;
            
            iterator(T* b, ptrdiff_t s, ptrdiff_t c)
            : _begin(b)
            , _stride(s)
            , _columns(c) {
            }
            
            ~iterator() = default;
            
            iterator& operator=(iterator const&) = default;
            
            vector_view<T> operator*() const {
                return vector_view<T>(_begin, _begin + _columns);
            }
            
            iterator& operator++() {
                _begin += _stride;
                return *this;
            }
            
            iterator operator+(ptrdiff_t i) const {
                return iterator(_begin + _stride * i, _stride, _columns);
            }
            
            vector_view<T> operator[](ptrdiff_t i) const {
                return *(*this + i);
            }
            
            bool operator==(iterator const& r) {
                return _begin == r._begin;
            }
            
            bool operator!=(iterator const& r) {
                return _begin != r._begin;
            }
            
            ptrdiff_t operator-(const iterator& r) {
                assert(r._stride == _stride);
                return (_begin - r._begin) / _stride;
            }
            
            indirect<vector_view<T>> operator->() const {
                return indirect(**this);
            }
            
        private:
            
            friend class matrix_view;
            friend class matrix<T>;
            
            T* _begin;
            ptrdiff_t _stride;
            ptrdiff_t _columns;
            
        };
        
        matrix_view() = delete;
        
        matrix_view(matrix_view const& r) = default;
        
        matrix_view(iterator b, ptrdiff_t r) : _begin(b), _rows(r) {}
        
        matrix_view const& operator=(matrix_view const& r) const {
            assert(_rows == r._rows);
            std::copy(r.begin(), r.end(), begin());
            return *this;
        }
        
        matrix_view const& operator=(const T& x) const {
            std::fill(begin(), end(), x);
            return *this;
        }
        
        iterator begin() const {
            return _begin;
        }
        
        iterator end() const {
            return _begin + _rows;
        }
        
        vector_view<T> operator[](ptrdiff_t i) const {
            return _begin[i];
        }
        
        matrix_view sub(ptrdiff_t i, ptrdiff_t j, ptrdiff_t r, ptrdiff_t c) const {
            return matrix_view(iterator(_begin._begin + i * _begin._stride + j,
                                  _begin._stride,
                                  c),
                         r);
        }
        
        T& operator()(ptrdiff_t i, ptrdiff_t j) const {
            return *(_begin._begin + i * _begin._stride + j);
        }

        
        ptrdiff_t rows() const { return _rows; }
        ptrdiff_t columns() const { return _begin._columns; }
        
    private:
        
        iterator const _begin;
        ptrdiff_t const _rows;
        
        friend class matrix<T>;
        
    };
    
    template<typename T>
    bool operator==(matrix_view<T> const& a, matrix_view<T> const& b) {
        return std::equal(a.begin(), a.end(), b.begin());
    }
    
    template<typename T>
    bool operator!=(matrix_view<T> const& a, matrix_view<T> const& b) {
        return std::equal(a.begin(), a.end(), b.begin());
    }
    
    // We can't quite implement matrix by inheriting from matrix_view for
    // several reasons.
    
    template<typename T>
    class matrix {
        
    public:
        
        using iterator = typename matrix_view<T>::iterator;
        using const_iterator = typename matrix_view<const T>::iterator;
        
        matrix() : _begin(), _rows(0), _store() {}
        
        matrix(const matrix& r) : matrix() {
            *this = r;
        }
        
        matrix(matrix&& r) : matrix() {
            r.swap(*this);
        }
        
        matrix(const matrix_view<T>& r)
        : matrix() {
            operator=(r);
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
            return operator=(static_cast<matrix_view<T>>(r));
        }
        
        matrix& operator=(matrix&& r) {
            matrix(std::move(r)).swap(*this);
            return *this;
        }
        
        matrix& operator=(const matrix_view<T>& r) {
            discard_and_resize(r._rows, r._begin._columns);
            std::copy(r.begin(), r.end(), begin());
            return *this;
        }
        
        matrix& operator=(T x) {
            std::fill(begin(), end(), x);
            return *this;
        }
        
        
        void swap(matrix& r) {
            using std::swap;
            swap(_begin, r._begin);
            swap(_rows, r._rows);
            swap(_store, r._store);
        }
        
        operator matrix_view<T>() const {
            return matrix_view<T>(_begin, _rows);
        }

        operator matrix_view<const T>() const {
            return matrix_view<const T>(_begin, _rows);
        }

        ptrdiff_t size() const {
            return _rows;
        }
        
        iterator begin() const {
            return _begin;
        }
        
        iterator end() const {
            return _begin + _rows;
        }
        
        T* data() const {
            return _begin._begin;
        }
        
        vector_view<T> operator[](ptrdiff_t i) const {
            return _begin[i];
        }
        
        vector_view<T> front() const {
            return *_begin;
        }
        
        vector_view<T> back() const {
            return _begin[_rows - 1];
        }
        
        T& operator()(ptrdiff_t i, ptrdiff_t j) const {
            return *(_begin._begin + i * _begin._stride + j);
        }
        
        matrix_view<T> sub(ptrdiff_t i, ptrdiff_t j, ptrdiff_t r, ptrdiff_t c) {
            return matrix_view<T>(iterator(_begin._begin + i * _begin._stride + j,
                                           _begin._stride,
                                           c),
                                  r);
        }
        
        void crop(ptrdiff_t i, ptrdiff_t j, ptrdiff_t r, ptrdiff_t c) {
            _begin._begin += i * _begin._stride + j;
            _begin._columns = c;
            _rows = r;
        }
        
        void discard_and_resize(ptrdiff_t rows, ptrdiff_t columns) {
            _store.resize(rows * columns, 0);
            _begin = iterator(_store.data(), columns, columns);
            _rows = rows;
        }
        
        void expand(ptrdiff_t i, ptrdiff_t j, ptrdiff_t r, ptrdiff_t c, T x) {
            matrix<T> a(r, c, x);
            a.sub(i, j, _rows, _begin._columns) = *this;
            a.swap(*this);
        }
        
        void resize(ptrdiff_t r, ptrdiff_t c, T x = T()) {
            matrix<T> a(r, c, x);
            r = std::min(r, rows());
            c = std::min(c, columns());
            a.sub(0, 0, r, c) = sub(0, 0, r ,c);
            a.swap(*this);
        }
        
        ptrdiff_t rows() const { return _rows; }
        ptrdiff_t columns() const { return _begin._columns; }
        ptrdiff_t stride() const { return _begin._stride; }
        
        void print() const {
            for (auto&& row : *this) {
                for (auto&& value : row)
                    std::cout << value << ' ';
                std::cout << '\n';
            }
        }

    private:
        
        iterator _begin;
        ptrdiff_t _rows;
        std::vector<T> _store;
        
    };
    
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
    
    
}

#endif /* matrix_hpp */
