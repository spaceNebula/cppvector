#include <cstddef>
#include <utility>
#include <memory>

template<typename T>
class Vector {
private:
    // "Сырая" память вектора с соблюдением RAII
    class _memory {
    public:
        T *_buf = nullptr;
        size_t _capacity = 0;

        _memory() = default;

        explicit _memory(size_t n) {
            _buf = _allocate(n);
            _capacity = n;
        }

        _memory(const _memory &) = delete;

        _memory(const _memory &&other) noexcept {
            Swap(other);
        }

        ~_memory() {
            _deallocate(_buf);
        }

        _memory &operator=(const _memory &) = delete;

        _memory &operator=(_memory &&other) noexcept {
            Swap(other);
            return *this;
        }

        static T *_allocate(size_t n) {
            return static_cast<T *>(operator new(n * sizeof(T)));
        }

        static void _deallocate(T *buf) {
            operator delete(buf);
        }

        T *operator+(size_t i) {
            return _buf + i;
        }

        const T *operator+(size_t i) const {
            return _buf + i;
        }

        T &operator[](size_t pos) {
            return _buf[pos];
        }

        const T &operator[](size_t pos) const {
            return _buf[pos];
        }

        void Swap(_memory &other) noexcept {
            std::swap(_buf, other._buf);
            std::swap(_capacity, other._capacity);
        }
    };

    // Приватные поля класса.
private:
    _memory _data;
    size_t _size = 0;

    // Приватные методы класса, в терминах которых его описывать будет проще.

    static void _construct(void *buf) {
        new(buf) T();
    }

    static void _construct(void *buf, const T &elem) {
        new(buf) T(elem);
    }

    static void _construct(void *buf, T && elem) {
        new(buf) T(std::move(elem));
    }

    static void _destroy(T *buf) {
        buf->~T();
    }

public:
    // _ - префикс для приватного поля/метода класса.

    Vector() = default;

    explicit Vector(size_t n) : _data(n) {
        std::uninitialized_value_construct_n(_data._buf, n);
        _size = n;
    }

    Vector(const Vector &other) : _data(other._size) {
        std::uninitialized_copy_n(other._data._buf, other._size, _data._buf);
        _size = other._size;
    }

    Vector(const Vector &&other) noexcept {
        Swap(other);
    }

    // Базовая гарантия безопасности
    Vector &operator=(const Vector &other) {
        if (other._size > _size) {
            Vector temp(other);
            swap(temp);
        } else {
            for (size_t i = 0; i < other._size && i < _size; ++i) {
                _data[i] = other[i];
            }
            if (_size < other._size) {
                std::uninitialized_copy_n(
                        other._data._buf + _size, other._size - _size, _data._buf + _size);
            } else if (_size > other._size) {
                std::destroy_n(_data._buf + other._size, _size - other._size);
            }
            _size = other._size;
        }
        return *this;
    }

    ~Vector() {
        std::destroy_n(_data._buf, _size);
    }

    //строгая гарантия безопасности
    void reserve(size_t n) {
        if (_data._capacity < n) {
            _memory _data2(n);
            std::uninitialized_move_n(_data._buf, _size, _data2._buf);
            std::destroy_n(_data._buf, _size);
            _data = std::move(_data2);
        }
    }

    void swap(Vector &other) noexcept {
        _data.Swap(other._data);
        std::swap(_size, other._size);
    }

    Vector &operator=(Vector &&other) noexcept {
        swap(other);
        return *this;
    }

    //строгая гарантия безопасности
    void resize(size_t n) {
        reserve(n);
        if (_size < n) {
            std::uninitialized_value_construct_n(_data + _size, n - _size);
        } else if (_size > n) {
            std::destroy_n(_data + n, _size - n);
        }
        _size = n;
    }

    //строгая гарантия безопасности
    void push_back(const T &elem) {
        if (_size == _data._capacity) {
            reserve(_size == 0 ? 1 : 2 * _size);
        }
        new(_data + _size) T(elem);
        ++_size;
    }

    void push_back(const T &&elem) {
        if (_size == _data._capacity) {
            reserve(_size == 0 ? 1 : 2 * _size);
        }
        new(_data + _size) T(std::move(elem));
        ++_size;
    }

    template<typename ... Args>
    T &emplace_back(Args &&... args) {
        if (_size == _data._capacity) {
            reserve(_size == 0 ? 1 : 2 * _size);
        }
        auto elem = new(_data + _size) T(std::forward<Args>(args)...);
        ++_size;
        return *elem;
    }

    void pop_back() {
        std::destroy_at(_data + _size - 1);
        --_size;
    }

    size_t size() const {
        return _size;
    }

    size_t capacity() const {
        return _data._capacity;
    }

    const T &operator[](size_t pos) const {
        return _data[pos];
    }

    T &operator[](size_t pos) {
        return _data[pos];
    }

    void clear() noexcept {
        for (size_t i = 0; i < _size; ++i) {
            _destroy(_data._buf + i);
        }
        _size = 0;
    }

    T *begin() {
        return _data._buf;
    }

    T *end() {
        return (_data._buf + _size);
    }

    const T *begin() const {
        return _data._buf;
    }

    const T *end() const {
        return (_data._buf + _size);
    }
};