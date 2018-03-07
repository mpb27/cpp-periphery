#include <iostream>

#include "periphery/i2c.hpp"

int main()
{
    using namespace periphery;

    I2C i2c("/dev/i2c-0");


    auto msgs = i2c.transfer(
        0x55,
        {
            I2C::Message::write({0x01, 0x00}),
            I2C::Message::read(1)
        }
    );

    uint8_t x = msgs[1].data[0];

    std::cout << "EEPROM(100h) = " << x << std::endl;



    auto writeAddr = I2C::Message::write({0x01, 0x00});
    auto readByte = I2C::Message::read(1);

    i2c.transfer( 0x56, { writeAddr, readByte } );

    uint8_t y = readByte.data[0];

    std::cout << "EEPROM(100h) = " << y << std::endl;
}
