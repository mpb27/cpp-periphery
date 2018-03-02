/*
 * cpp-periphery
 * https://github.com/mpb27/cpp-periphery
 * License: MIT
 */

#ifndef PERIPHERY_SERIAL_HPP
#define PERIPHERY_SERIAL_HPP

#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <chrono>
#include <ostream>
#include <string>
#include <vector>

namespace periphery {

class Serial {
public:
    enum class DataBits  { Five, Six, Seven, Eight };
    enum class StopBits  { One, Two };
    enum class Parity    { None, Even, Odd };
    enum class Handshake { None, RtsCts, XonXoff, RtsCtsXonXoff };

    /* Constructor and Destructor. */
    Serial(const std::string& path, uint32_t baudrate, DataBits databits, Parity parity, StopBits stopbits, Handshake handshake);
    Serial(const std::string& path, uint32_t baudrate, DataBits databits, Parity parity, StopBits stopbits);
    Serial(const std::string& path, uint32_t baudrate);
    ~Serial();

    /* Disable copy constructor and copy assignment. */
    Serial(const Serial&) = delete;
    Serial& operator=(const Serial&) = delete;

    /* Should we include getters and setters? */
    /* Nope, just instantiate serial again. */
    bool poll(std::chrono::milliseconds timeout) const;
    void flush() const;
    unsigned int input_waiting() const;
    unsigned int output_waiting() const;

    int  write(const uint8_t* buf, size_t len) const;
    void write_all(const uint8_t* buf, size_t len) const;

    int  read(uint8_t* buf, size_t len) const;
    int  read_timeout(uint8_t* buf, size_t len, std::chrono::milliseconds timeout) const;

    //std::vector<uint8_t> read(size_t len) const;
    std::vector<uint8_t> read(size_t len, std::chrono::milliseconds timeout) const;

private:
    int fd_;
};


} // ... namespace periphery  ...

#endif // ... PERIPHERY_SERIAL_HPP ...
