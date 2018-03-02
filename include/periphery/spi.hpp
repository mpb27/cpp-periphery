/*
 * cpp-periphery
 * https://github.com/mpb27/cpp-periphery
 * License: MIT
 *
 * Notes:
 *   1) Don't throw from destructor:
 *          https://stackoverflow.com/questions/130117
 *   2) RAII example :
 *          https://www.tomdalling.com/blog/software-design/resource-acquisition-is-initialisation-raii-explained/
 *   3) Deleteing copy constructor in C++11:
 *          https://stackoverflow.com/questions/33776697
 */

#ifndef PERIPHERY_SPI_HPP
#define PERIPHERY_SPI_HPP

#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <string>
#include <ostream>

namespace periphery {

class Spi {
public:
    enum class BitOrder { MsbFirst, LsbFirst };
    enum class Mode     { Zero = 0, One = 1, Two = 2, Three = 3 };

    // ... constructor / destructor ...
    Spi(const std::string& path, Mode mode, BitOrder bit_order, uint32_t speed, uint8_t bits_per_word, uint8_t extra_flags);
    Spi(const std::string& path, Mode mode, BitOrder bit_order, uint32_t speed, uint8_t bits_per_word);
    Spi(const std::string& path, Mode mode, BitOrder bit_order, uint32_t speed);
    ~Spi();

    // ... disable copy-constructor and copy assignment ...
    Spi(const Spi&) = delete;
    Spi& operator=(const Spi&) = delete;

    // ... getters ...
    Mode     mode() const;
    BitOrder bit_order() const;
    uint8_t  bits_per_word() const;
    uint32_t speed() const;

    // ... setters ...
    void mode(Mode mode);
    void bit_order(BitOrder bit_order);
    void bits_per_word(uint8_t bits_per_word);
    void speed(uint32_t speed);

    // ... transfer ...
    void transfer(const uint8_t* txbuf, uint8_t* rxbuf, size_t len) const;

    // ... ostream ...
    friend std::ostream& operator<<(std::ostream& stream, const Spi& spi);

private:
    int  m_fd;
};


} // ... namespace periphery ...

#endif  // ... PERIPHERY_SPI_HPP ...
