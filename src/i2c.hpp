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

#ifndef PERIPHERY_I2C_HPP
#define PERIPHERY_I2C_HPP

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <system_error>




namespace periphery {

class i2c {
public:
    // ... constructor / destructor ...
    i2c(const std::string path);
    ~i2c();
    // ... disable copy-constructor and copy assignment ...
    i2c(const i2c&) = delete;
    i2c& operator=(const i2c&) = delete;
    // ... primary functions ...
    void write()
    //transfer()
private:
    int fd;
};


i2c::i2c(const std::string path) {
    // ... open device ...
    if ((this->fd = open(path, O_RDWR) < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }

    // ... query supported functions ...
    uint32_t supported_funcs;
    if (ioctl(this->fd, I2C_FUNCS, &supported_funcs) < 0) {
        auto e = std::system_error(EFAULT, std::system_category());
        close(this->fd);
        throw e;
    }

    // ... check that this device has I2C function ...
    if (!(supported_funcs & I2C_FUNC_I2C)) {
        close(this->fd);
        throw std::exception("I2C not supported on ???");
    }
}


i2c::~i2c() {
    // ... we can report an error, but should never throw from destructor ...
    if (close(this->fd) < 0) {
        // ... do nothing ...
    }
}



} // ... namespace periphery ...



#endif
