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
#include <system_error>


#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>

namespace periphery {

class spi {
public:
    enum bit_order { MSB_FIRST, LSB_FIRST };
    enum mode { MODE_0 = 0, MODE_1 = 1, MODE_2 = 2, MODE_3 = 3 };

    // ... constructor / destructor ...
    spi(const std::string path, mode mode, bit_order bit_order, uint32_t speed, uint8_t bits_per_word, uint8_t extra_flags);
    ~spi();
    // ... disable copy-constructor and copy assignment ...
    spi(const spi&) = delete;
    spi& operator=(const spi&) = delete;
    // ... primary functions ...


private:
    int fd;
};


spi::spi(const std::string path, mode mode, bit_order bit_order, uint32_t speed, uint8_t bits_per_word, uint8_t extra_flags) {
    // ... open device ...
    if ((this->fd = open(path.c_str(), O_RDWR)) < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }

    // ... set mode, bit order flags ...
    uint8_t flags = mode | ((bit_order == LSB_FIRST) ? SPI_LSB_FIRST : 0);
    if (ioctl(this->fd, SPI_IOC_WR_MODE, &flags) < 0) {
        auto e = std::system_error(EFAULT, std::system_category());
        close(this->fd);
        throw e;
    }

    // ... set speed ...
    if (ioctl(this->fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        auto e = std::system_error(EFAULT, std::system_category());
        close(this->fd);
        throw e;
    }

    // ... set bits per word ...
    if (ioctl(this->fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0) {
        auto e = std::system_error(EFAULT, std::system_category());
        close(this->fd);
        throw e;
    }
}


spi::~spi() {
    // ... we can report an error, but should never throw from destructor ...
    if (close(this->fd) < 0) {
        // ... do nothing ...
    }
}



} // ... namespace periphery ...

#endif  // ... PERIPHERY_SPI_HPP ...
