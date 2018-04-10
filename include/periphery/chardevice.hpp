/*
 * cpp-periphery
 * https://github.com/mpb27/cpp-periphery
 * License: MIT
 */

#ifndef PERIPHERY_CHARDEVICE_HPP
#define PERIPHERY_CHARDEVICE_HPP

#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <chrono>
#include <ostream>
#include <string>

#include <periphery/buffer.hpp>

namespace periphery {

class CharacterDevice {
public:
    enum class Access { ReadOnly, WriteOnly, ReadWrite };

    /* Constructor and Destructor. */
    CharacterDevice(const std::string& path, Access access);
    ~CharacterDevice();

    /* Disable copy constructor and copy assignment. */
    CharacterDevice(const CharacterDevice&) = delete;
    CharacterDevice& operator=(const CharacterDevice&) = delete;

    bool poll(std::chrono::milliseconds timeout) const;
    void flush() const;
    unsigned int input_waiting() const;
    unsigned int output_waiting() const;


    void write(const_buffer buf) const;
    void write(const std::string& data) const;

    int  read     (mutable_buffer buf) const;
    void read_all (mutable_buffer buf) const;
    int  read_timeout     (mutable_buffer buf, std::chrono::milliseconds timeout) const;
    int  read_all_timeout (mutable_buffer buf, std::chrono::milliseconds timeout) const;

private:
    int m_fd;
};


} // ... namespace periphery  ...

#endif // ... PERIPHERY_CHARDEVICE_HPP ...
