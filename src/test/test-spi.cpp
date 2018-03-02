#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <iostream>

#include "periphery/spi.hpp"

int main()
{
    //unsigned char tmp[256];

    periphery::Spi spi("/dev/spi1.1",
        periphery::Spi::Mode0,
        periphery::Spi::MsbFirst,
        1000000);

    spi.mode(periphery::Spi::Mode3);
    spi.bit_order(periphery::Spi::LsbFirst);
    spi.speed(250000);
    spi.bits_per_word(16);

    std::cout << spi << std::endl;


    uint8_t tx[16];
    uint8_t rx[16];

    spi.transfer(tx, rx, 16);
}
