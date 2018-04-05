#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <iostream>

#include <array>
#include <chrono>

#include "periphery/serial.hpp"

int main()
{
    //unsigned char tmp[256];

    periphery::Serial serial("/dev/ttyS0", 9600);

    std::cout << "input waiting  = " << serial.input_waiting() << "\n";
    std::cout << "output waiting = " << serial.output_waiting() << "\n";

    serial.flush();

    serial.poll(std::chrono::milliseconds(100));

    unsigned char tmp[256] = {};
    serial.write(periphery::buffer(tmp));

    std::array<char, 16> myArray;
    serial.read(periphery::buffer(myArray));
    serial.read_timeout(periphery::buffer(myArray), std::chrono::milliseconds{25} );
}
