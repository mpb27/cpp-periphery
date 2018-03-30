/*
 * cpp-periphery
 * https://github.com/mpb27/cpp-periphery
 * License: MIT
 *
 */

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <system_error>
#include <string>
#include <ostream>
#include <type_traits>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/spi/spidev.h>
#include "periphery/spi.hpp"

namespace periphery {


Spi::Spi(const std::string& path, Mode mode, BitOrder bit_order, uint32_t speed, uint8_t bits_per_word, uint8_t extra_flags)
{
    int error;

    // ... open device ...
    m_fd = open(path.c_str(), O_RDWR);
    if (m_fd < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }

    // ... set mode, bit order flags ...
    uint8_t flags = static_cast<uint8_t>(mode)
                    | ((bit_order == BitOrder::LsbFirst) ? SPI_LSB_FIRST : 0)
                    | extra_flags;
    error = ioctl(m_fd, SPI_IOC_WR_MODE, &flags);
    if (error < 0) {
        auto e = std::system_error(EFAULT, std::system_category());
        close(m_fd);
        throw e;
    }

    // ... set speed ...
    error = ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (error < 0) {
        auto e = std::system_error(EFAULT, std::system_category());
        close(m_fd);
        throw e;
    }

    // ... set bits per word ...
    error = ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word);
    if (error < 0) {
        auto e = std::system_error(EFAULT, std::system_category());
        close(m_fd);
        throw e;
    }
}

Spi::Spi(const std::string& path, Mode mode, BitOrder bit_order, uint32_t speed, uint8_t bits_per_word)
    : Spi(path, mode, bit_order, speed, bits_per_word, 0) { }

Spi::Spi(const std::string& path, Mode mode, BitOrder bit_order, uint32_t speed)
    : Spi(path, mode, bit_order, speed, 8, 0) { }


Spi::~Spi()
{
    // ... we can report an error, but should never throw from destructor ...
    int error = close(m_fd);
    if (error < 0) {
        // ... do nothing ...
    }
}


inline void ioctl_with_throw(int fd, unsigned long int request, void* ptr)
{
    int error = ioctl(fd, request, ptr);
    if (error < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }
}

template<typename T>
inline T ioctl_get_wt(int fd, unsigned long int request)
{
    static_assert(
        std::is_same<T, uint32_t>::value || std::is_same<T, uint16_t>::value || std::is_same<T, uint8_t>::value,
        "Must be 8, 16, or 32bit value.");
    T value = 0;
    ioctl_with_throw(fd, request, &value);
    return value;
}

template<typename T>
inline void ioctl_set_wt(int fd, unsigned long int request, T value)
{
    static_assert(
        std::is_same<T, uint32_t>::value || std::is_same<T, uint16_t>::value || std::is_same<T, uint8_t>::value,
        "Must be 8, 16, or 32bit value.");
    ioctl_with_throw(fd, request, &value);
}

void Spi::mode(Spi::Mode mode)
{
    //uint8_t data8;

    // ... read the mode byte + other things ...
    //ioctl_with_throw(m_fd, SPI_IOC_RD_MODE, &data8);
    uint8_t data8 = ioctl_get_wt<uint8_t>(m_fd, SPI_IOC_RD_MODE);

    // ... update the bits ...
    data8 &= ~(SPI_CPOL | SPI_CPHA);
    data8 |= static_cast<uint8_t>(mode);

    // ... write the mode byte + other things ...
    //ioctl_with_throw(m_fd, SPI_IOC_WR_MODE, &data8);
    ioctl_set_wt<uint8_t>(m_fd, SPI_IOC_WR_MODE, data8);
}

void Spi::bit_order(Spi::BitOrder bit_order)
{
    // ... the bits ...
    uint8_t data8 = (bit_order == BitOrder::LsbFirst) ? 1 : 0;
    // ... write the mode byte + other things ...
    ioctl_with_throw(m_fd, SPI_IOC_WR_LSB_FIRST, &data8);
}

void Spi::bits_per_word(uint8_t bits_per_word)
{
    ioctl_with_throw(m_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word);
}

void Spi::speed(uint32_t speed)
{
    ioctl_with_throw(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
}

Spi::Mode Spi::mode() const
{
    // ... read the mode byte + other things ...
    uint8_t data8;
    ioctl_with_throw(m_fd, SPI_IOC_RD_MODE, &data8);
    // ... clear non mode bits and return ...
    data8 &= (SPI_CPOL | SPI_CPHA);
    return static_cast<Mode>(data8);
}

Spi::BitOrder Spi::bit_order() const
{
    // ... read the bit order + other things ...
    uint8_t data8;
    ioctl_with_throw(m_fd, SPI_IOC_RD_LSB_FIRST, &data8);
    // ... clear non mode bits and return ...
    return (data8 & SPI_LSB_FIRST) == 0 ? Spi::BitOrder::MsbFirst : Spi::BitOrder::LsbFirst;
}

uint32_t Spi::speed() const
{
    //return ioctl_get_wt<uint32_t>(m_fd, SPI_IOC_RD_MAX_SPEED_HZ);
    uint32_t speed;
    ioctl_with_throw(m_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    return speed;
}

uint8_t Spi::bits_per_word() const
{
    uint8_t bits_per_word;
    ioctl_with_throw(m_fd, SPI_IOC_RD_BITS_PER_WORD, &bits_per_word);
    return bits_per_word;
}


void Spi::transfer(const uint8_t* txbuf, uint8_t* rxbuf, size_t len) const
{
    struct spi_ioc_transfer spi_xfer;

    /* Prepare SPI transfer structure */
    memset(&spi_xfer, 0, sizeof(struct spi_ioc_transfer));
    spi_xfer.tx_buf = (__u64)txbuf;
    spi_xfer.rx_buf = (__u64)rxbuf;
    spi_xfer.len = len;
    spi_xfer.delay_usecs = 0;
    spi_xfer.speed_hz = 0;
    spi_xfer.bits_per_word = 0;
    spi_xfer.cs_change = 0;

    /* Transfer */
    ioctl_with_throw(m_fd, SPI_IOC_MESSAGE(1), &spi_xfer);
}


std::ostream& operator<<(std::ostream& stream, const Spi& spi)
{
    stream << "SPI (fd=" << spi.m_fd
           << ", mode=" << static_cast<int>(spi.mode())
           << ", speed=" << spi.speed()
           << ", bit_order=" <<  (spi.bit_order() == Spi::BitOrder::MsbFirst ? "MSB first" : "LSB first")
           << ", bits_per_word=" << spi.bits_per_word()
           << ")";
    return stream;
}


} // ... namespace periphery ...
