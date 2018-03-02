#include "periphery/mmio.hpp"

int main()
{
    unsigned char tmp[256];

    periphery::Mmio mmio(0x0, 0x1000);

    mmio.read(0, tmp, 256);
    mmio.write(0x100, tmp, 256);
}
