#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <iostream>

#include "periphery/spi.hpp"

int main()
{
    using namespace periphery;

    Spi spi("/dev/spi1.1",
        Spi::Mode::Zero,
        Spi::BitOrder::MsbFirst,
        1000000);

    spi.mode(Spi::Mode::Three);
    spi.bit_order(Spi::BitOrder::LsbFirst);
    spi.speed(250000);
    spi.bits_per_word(16);

    std::cout << spi << std::endl;


    uint8_t tx[16];
    uint8_t rx[16];

    spi.transfer(tx, rx, 16);
}
