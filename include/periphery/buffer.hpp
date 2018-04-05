/*
 * cpp-periphery
 * https://github.com/mpb27/cpp-periphery
 * License: MIT
 *
 * Notes:
 *     This code is borrowed from experimental networking TS:   https://github.com/chriskohlhoff/networking-ts-impl
 *
 * Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
 * Copyright (c) 2018      Mark Butowski
 *
 */

#ifndef PERIPHERY_BUFFER_HPP
#define PERIPHERY_BUFFER_HPP

#include <array>
#include <cstddef>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

#if __has_include(<string_view>) && __cplusplus >= 201701L
    #include <string_view>
#endif


namespace periphery {

class mutable_buffer {
public:
    /// Construct empty buffer.
    mutable_buffer() noexcept
            : data_(0), size_(0)
    {
        // ... empty ...
    }

    /// Construct a buffer to represent a given memory range.
    mutable_buffer(void* data, std::size_t size) noexcept
            : data_(data), size_(size)
    {
        // ... empty ...
    }

    /// Get a pointer to the beginning of the memory range.
    void* data() const noexcept
    {
        return data_;

    }

    /// Get the size of the memory range.
    std::size_t size() const noexcept
    {
        return size_;
    }

    /// Move the start of the buffer by the specified number of bytes.
    mutable_buffer& operator+=(std::size_t n) noexcept
    {
        std::size_t offset = n<size_ ? n : size_;
        data_ = static_cast<char*>(data_) + offset;
        size_ -= offset;
        return *this;
    }

private:
    void* data_;
    std::size_t size_;
};

class const_buffer
{
public:
    /// Construct an empty buffer.
    const_buffer() noexcept
            :data_(0), size_(0)
    {
        // ... empty ...
    }

    /// Construct a buffer to represent a given memory range.
    const_buffer(const void* data, std::size_t size) noexcept
            : data_(data), size_(size)
    {
        // ... empty ...
    }

    /// Construct a non-modifiable buffer from a modifiable one.
    const_buffer(const mutable_buffer& b) noexcept
            : data_(b.data()), size_(b.size())
    {
        // ... empty ...
    }

    /// Get a pointer to the beginning of the memory range.
    const void* data() const noexcept
    {
        return data_;
    }

    /// Get the size of the memory range.
    std::size_t size() const noexcept
    {
        return size_;
    }

    /// Move the start of the buffer by the specified number of bytes.
    const_buffer& operator+=(std::size_t n) noexcept
    {
        std::size_t offset = n<size_ ? n : size_;
        data_ = static_cast<const char*>(data_) + offset;
        size_ -= offset;
        return *this;
    }

private:
    const void* data_;
    std::size_t size_;

};



/// Create a new modifiable buffer that is offset from the start of another.
/**
 * @relates mutable_buffer
 */
inline mutable_buffer operator+(const mutable_buffer& b, std::size_t n) noexcept
{
    std::size_t offset = n < b.size() ? n : b.size();
    char* new_data = static_cast<char*>(b.data()) + offset;
    std::size_t new_size = b.size() - offset;
    return mutable_buffer(new_data, new_size);
}

/// Create a new modifiable buffer that is offset from the start of another.
/**
 * @relates mutable_buffer
 */
inline mutable_buffer operator+(std::size_t n, const mutable_buffer& b) noexcept
{
    return b + n;
}

/// Create a new non-modifiable buffer that is offset from the start of another.
/**
 * @relates const_buffer
 */
inline const_buffer operator+(const const_buffer& b, std::size_t n) noexcept
{
    std::size_t offset = n < b.size() ? n : b.size();
    const char* new_data = static_cast<const char*>(b.data()) + offset;
    std::size_t new_size = b.size() - offset;
    return const_buffer(new_data, new_size);
}

/// Create a new non-modifiable buffer that is offset from the start of another.
/**
 * @relates const_buffer
 */
inline const_buffer operator+(std::size_t n, const const_buffer& b) noexcept
{
    return b + n;
}



/// Create a new modifiable buffer from an existing buffer.
/**
 * @returns <tt>mutable_buffer(b)</tt>.
 */
inline mutable_buffer buffer(const mutable_buffer& b) noexcept
{
    return mutable_buffer(b);
}


/// Create a new modifiable buffer from an existing buffer.
/**
 * @returns A mutable_buffer value equivalent to:
 * @code mutable_buffer(
 *     b.data(),
 *     min(b.size(), max_size_in_bytes)); @endcode
 */
inline mutable_buffer buffer(const mutable_buffer& b, size_t max_size_in_bytes) noexcept
{
    return mutable_buffer(b.data(),
            b.size()<max_size_in_bytes ? b.size() : max_size_in_bytes);
}


/// Create a new non-modifiable buffer from an existing buffer.
/**
 * @returns <tt>const_buffer(b)</tt>.
 */
inline const_buffer buffer(const const_buffer& b) noexcept
{
    return const_buffer(b);
}



/// Create a new non-modifiable buffer from an existing buffer.
/**
 * @returns A const_buffer value equivalent to:
 * @code const_buffer(
 *     b.data(),
 *     min(b.size(), max_size_in_bytes)); @endcode
 */
inline const_buffer buffer(const const_buffer& b, size_t max_size_in_bytes) noexcept
{
    return const_buffer(b.data(),
            b.size()<max_size_in_bytes ? b.size() : max_size_in_bytes);
}


/// Create a new modifiable buffer that represents the given memory range.
/**
 * @returns <tt>mutable_buffer(data, size_in_bytes)</tt>.
 */
inline mutable_buffer buffer(void* data, size_t size_in_bytes) noexcept
{
    return mutable_buffer(data, size_in_bytes);
}

/// Create a new non-modifiable buffer that represents the given memory range.
/**
 * @returns <tt>const_buffer(data, size_in_bytes)</tt>.
 */
inline const_buffer buffer(const void* data, size_t size_in_bytes) noexcept
{
    return const_buffer(data, size_in_bytes);
}


/// Create a new modifiable buffer that represents the given POD array.
/**
 * @returns A mutable_buffer value equivalent to:
 * @code mutable_buffer(
 *     static_cast<void*>(data),
 *     N * sizeof(PodType)); @endcode
 */
template<typename PodType, std::size_t N>
inline mutable_buffer buffer(PodType (& data)[N]) noexcept
{
    return mutable_buffer(data, N*sizeof(PodType));
}


/// Create a new modifiable buffer that represents the given POD array.
/**
 * @returns A mutable_buffer value equivalent to:
 * @code mutable_buffer(
 *     static_cast<void*>(data),
 *     min(N * sizeof(PodType), max_size_in_bytes)); @endcode
 */
template<typename PodType, std::size_t N>
inline mutable_buffer buffer(PodType (& data)[N],
        std::size_t max_size_in_bytes) noexcept
{
    return mutable_buffer(data,
            N*sizeof(PodType)<max_size_in_bytes ? N*sizeof(PodType) : max_size_in_bytes);
}


/// Create a new non-modifiable buffer that represents the given POD array.
/**
 * @returns A const_buffer value equivalent to:
 * @code const_buffer(
 *     static_cast<const void*>(data),
 *     N * sizeof(PodType)); @endcode
 */
template<typename PodType, std::size_t N>
inline const_buffer buffer(const PodType (& data)[N]) noexcept
{
    return const_buffer(data, N * sizeof(PodType));
}


/// Create a new non-modifiable buffer that represents the given POD array.
/**
 * @returns A const_buffer value equivalent to:
 * @code const_buffer(
 *     static_cast<const void*>(data),
 *     min(N * sizeof(PodType), max_size_in_bytes)); @endcode
 */
template<typename PodType, std::size_t N>
inline const_buffer buffer(const PodType (& data)[N], std::size_t max_size_in_bytes) noexcept
{
    return const_buffer(data,
            N * sizeof(PodType) < max_size_in_bytes ? N * sizeof(PodType) : max_size_in_bytes);
}

/// Create a new modifiable buffer that represents the given POD array.
/**
 * @returns A mutable_buffer value equivalent to:
 * @code mutable_buffer(
 *     data.data(),
 *     data.size() * sizeof(PodType)); @endcode
 */
template <typename PodType, std::size_t N>
inline mutable_buffer buffer(std::array<PodType, N>& data) noexcept
{
    return mutable_buffer(data.data(), data.size() * sizeof(PodType));
}

/// Create a new modifiable buffer that represents the given POD array.
/**
 * @returns A mutable_buffer value equivalent to:
 * @code mutable_buffer(
 *     data.data(),
 *     min(data.size() * sizeof(PodType), max_size_in_bytes)); @endcode
 */
template <typename PodType, std::size_t N>
inline mutable_buffer buffer(std::array<PodType, N>& data, std::size_t max_size_in_bytes) noexcept
{
    return mutable_buffer(data.data(),
            data.size() * sizeof(PodType) < max_size_in_bytes ? data.size() * sizeof(PodType) : max_size_in_bytes);
}

/// Create a new non-modifiable buffer that represents the given POD array.
/**
 * @returns A const_buffer value equivalent to:
 * @code const_buffer(
 *     data.data(),
 *     data.size() * sizeof(PodType)); @endcode
 */
template <typename PodType, std::size_t N>
inline const_buffer buffer(std::array<const PodType, N>& data) noexcept
{
    return const_buffer(data.data(), data.size() * sizeof(PodType));
}

/// Create a new non-modifiable buffer that represents the given POD array.
/**
 * @returns A const_buffer value equivalent to:
 * @code const_buffer(
 *     data.data(),
 *     min(data.size() * sizeof(PodType), max_size_in_bytes)); @endcode
 */
template <typename PodType, std::size_t N>
inline const_buffer buffer(std::array<const PodType, N>& data, std::size_t max_size_in_bytes) noexcept
{
    return const_buffer(data.data(),
            data.size() * sizeof(PodType) < max_size_in_bytes ? data.size() * sizeof(PodType) : max_size_in_bytes);
}

/// Create a new non-modifiable buffer that represents the given POD array.
/**
 * @returns A const_buffer value equivalent to:
 * @code const_buffer(
 *     data.data(),
 *     data.size() * sizeof(PodType)); @endcode
 */
template <typename PodType, std::size_t N>
inline const_buffer buffer(const std::array<PodType, N>& data) noexcept
{
    return const_buffer(data.data(), data.size() * sizeof(PodType));
}

/// Create a new non-modifiable buffer that represents the given POD array.
/**
 * @returns A const_buffer value equivalent to:
 * @code const_buffer(
 *     data.data(),
 *     min(data.size() * sizeof(PodType), max_size_in_bytes)); @endcode
 */
template <typename PodType, std::size_t N>
inline const_buffer buffer(const std::array<PodType, N>& data, std::size_t max_size_in_bytes) noexcept
{
    return const_buffer(data.data(),
            data.size() * sizeof(PodType) < max_size_in_bytes ? data.size() * sizeof(PodType) : max_size_in_bytes);
}

/// Create a new modifiable buffer that represents the given POD vector.
/**
 * @returns A mutable_buffer value equivalent to:
 * @code mutable_buffer(
 *     data.size() ? &data[0] : 0,
 *     data.size() * sizeof(PodType)); @endcode
 *
 * @note The buffer is invalidated by any vector operation that would also
 * invalidate iterators.
 */
template <typename PodType, typename Allocator>
inline mutable_buffer buffer(std::vector<PodType, Allocator>& data) noexcept
{
    return mutable_buffer(data.size() ? &data[0] : 0, data.size() * sizeof(PodType));
}

/// Create a new modifiable buffer that represents the given POD vector.
/**
 * @returns A mutable_buffer value equivalent to:
 * @code mutable_buffer(
 *     data.size() ? &data[0] : 0,
 *     min(data.size() * sizeof(PodType), max_size_in_bytes)); @endcode
 *
 * @note The buffer is invalidated by any vector operation that would also
 * invalidate iterators.
 */
template <typename PodType, typename Allocator>
inline mutable_buffer buffer(std::vector<PodType, Allocator>& data, std::size_t max_size_in_bytes) noexcept
{
    return mutable_buffer(data.size() ? &data[0] : 0,
            data.size() * sizeof(PodType) < max_size_in_bytes ? data.size() * sizeof(PodType) : max_size_in_bytes);
}

/// Create a new non-modifiable buffer that represents the given POD vector.
/**
 * @returns A const_buffer value equivalent to:
 * @code const_buffer(
 *     data.size() ? &data[0] : 0,
 *     data.size() * sizeof(PodType)); @endcode
 *
 * @note The buffer is invalidated by any vector operation that would also
 * invalidate iterators.
 */
template <typename PodType, typename Allocator>
inline const_buffer buffer(const std::vector<PodType, Allocator>& data) noexcept
{
    return const_buffer(data.size() ? &data[0] : 0, data.size() * sizeof(PodType));
}

/// Create a new non-modifiable buffer that represents the given POD vector.
/**
 * @returns A const_buffer value equivalent to:
 * @code const_buffer(
 *     data.size() ? &data[0] : 0,
 *     min(data.size() * sizeof(PodType), max_size_in_bytes)); @endcode
 *
 * @note The buffer is invalidated by any vector operation that would also
 * invalidate iterators.
 */
template <typename PodType, typename Allocator>
inline const_buffer buffer(const std::vector<PodType, Allocator>& data, std::size_t max_size_in_bytes) noexcept
{
    return const_buffer(data.size() ? &data[0] : 0,
            data.size() * sizeof(PodType) < max_size_in_bytes ? data.size() * sizeof(PodType) : max_size_in_bytes);
}

/// Create a new modifiable buffer that represents the given string.
/**
 * @returns <tt>mutable_buffer(data.size() ? &data[0] : 0,
 * data.size() * sizeof(Elem))</tt>.
 *
 * @note The buffer is invalidated by any non-const operation called on the
 * given string object.
 */
template <typename Elem, typename Traits, typename Allocator>
inline mutable_buffer buffer(std::basic_string<Elem, Traits, Allocator>& data) noexcept
{
    return mutable_buffer(data.size() ? &data[0] : 0, data.size() * sizeof(Elem));
}

/// Create a new non-modifiable buffer that represents the given string.
/**
 * @returns A mutable_buffer value equivalent to:
 * @code mutable_buffer(
 *     data.size() ? &data[0] : 0,
 *     min(data.size() * sizeof(Elem), max_size_in_bytes)); @endcode
 *
 * @note The buffer is invalidated by any non-const operation called on the
 * given string object.
 */
template <typename Elem, typename Traits, typename Allocator>
inline mutable_buffer buffer(std::basic_string<Elem, Traits, Allocator>& data, std::size_t max_size_in_bytes) noexcept
{
    return mutable_buffer(data.size() ? &data[0] : 0,
            data.size() * sizeof(Elem) < max_size_in_bytes ? data.size() * sizeof(Elem) : max_size_in_bytes);
}

/// Create a new non-modifiable buffer that represents the given string.
/**
 * @returns <tt>const_buffer(data.data(), data.size() * sizeof(Elem))</tt>.
 *
 * @note The buffer is invalidated by any non-const operation called on the
 * given string object.
 */
template <typename Elem, typename Traits, typename Allocator>
inline const_buffer buffer(const std::basic_string<Elem, Traits, Allocator>& data) noexcept
{
    return const_buffer(data.data(), data.size() * sizeof(Elem));
}

/// Create a new non-modifiable buffer that represents the given string.
/**
 * @returns A const_buffer value equivalent to:
 * @code const_buffer(
 *     data.data(),
 *     min(data.size() * sizeof(Elem), max_size_in_bytes)); @endcode
 *
 * @note The buffer is invalidated by any non-const operation called on the
 * given string object.
 */
template <typename Elem, typename Traits, typename Allocator>
inline const_buffer buffer(
        const std::basic_string<Elem, Traits, Allocator>& data,
        std::size_t max_size_in_bytes) noexcept
{
    return const_buffer(data.data(), data.size() * sizeof(Elem) < max_size_in_bytes ? data.size() * sizeof(Elem) : max_size_in_bytes);
}


#if __cpp_lib_string_view

/// Create a new modifiable buffer that represents the given string_view.
/**
 * @returns <tt>mutable_buffer(data.size() ? &data[0] : 0,
 * data.size() * sizeof(Elem))</tt>.
 */
template <typename Elem, typename Traits>
inline const_buffer buffer(std::basic_string_view<Elem, Traits> data) noexcept
{
    return const_buffer(data.size() ? &data[0] : 0, data.size() * sizeof(Elem));
}

/// Create a new non-modifiable buffer that represents the given string.
/**
 * @returns A mutable_buffer value equivalent to:
 * @code mutable_buffer(
 *     data.size() ? &data[0] : 0,
 *     min(data.size() * sizeof(Elem), max_size_in_bytes)); @endcode
 */
template <typename Elem, typename Traits>
inline const_buffer buffer(std::basic_string_view<Elem, Traits> data, std::size_t max_size_in_bytes) noexcept
{
    return const_buffer(data.size() ? &data[0] : 0,
            data.size() * sizeof(Elem) < max_size_in_bytes ? data.size() * sizeof(Elem) : max_size_in_bytes);
}

#endif


}  // ... namespace periphery ...

#endif // PERIPHERY_BUFFER_HPP