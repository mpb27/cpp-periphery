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
#include <memory>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>


#include "periphery/i2c.hpp"

namespace periphery {

//class ReadMessage : public I2C::Message {};
//class WriteMessage : public I2C::Message {};

I2C::I2C(const std::string& path) {
    // ... open device ...
    fd_ = open(path.c_str(), O_RDWR);
    if (fd_ < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }

    // ... query supported functions ...
    unsigned long supported_funcs;
    if (ioctl(fd_, I2C_FUNCS, &supported_funcs) < 0) {
        auto e = std::system_error(EFAULT, std::system_category());
        close(fd_);
        throw e;
    }

    // ... check that this device has I2C function ...
    if (!(supported_funcs & I2C_FUNC_I2C)) {
        close(fd_);
        throw std::runtime_error("I2C not supported on ???");
    }

    path_ = path;
}


I2C::~I2C() {
    // ... we can report an error, but should never throw from destructor ...
    if (close(fd_) < 0) {
        // ... do nothing ...
    }
}


I2C::Message I2C::Message::write(std::initializer_list<uint8_t> tx_data) {
    Message msg;
    msg.flags = 0;
    msg.data = std::vector<uint8_t>(tx_data);
    return msg;
}


I2C::Message I2C::Message::write(const std::vector<uint8_t>& tx_data) {
    Message msg;
    msg.flags = 0;
    msg.data = std::vector<uint8_t>(tx_data);
    return msg;
}


I2C::Message I2C::Message::read(size_t len) {
    Message msg;
    msg.flags = I2C_M_RD;
    msg.data = std::vector<uint8_t>(len);
    return msg;
}

template <typename ForwardIt>
void I2C::transfer(uint16_t addr, ForwardIt begin, ForwardIt end) const
{
    if (begin == end) {
        return;
    }

    // ... create i2c_msg structure needed by linux call ...
    size_t count = end - begin;
    std::unique_ptr<i2c_msg[]> p(new i2c_msg[count]);
    for (size_t i = 0 ; i < count; ++i) {
        auto& m = (Message&) *begin++;
        p[i].addr  = addr;
        p[i].flags = m.flags;
        p[i].len   = m.data.size();
        p[i].buf   = m.data.data();
    }

    // ... create transfer descriptor ...
    i2c_rdwr_ioctl_data i2c_rdwr_data;
    memset(&i2c_rdwr_data, 0, sizeof(i2c_rdwr_ioctl_data));
    i2c_rdwr_data.msgs = p.get();
    i2c_rdwr_data.nmsgs = count;

    int error = ioctl(fd_, I2C_RDWR, &i2c_rdwr_data);
    if (error < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }
}


void I2C::transfer(uint16_t addr, std::initializer_list<std::reference_wrapper<Message>> messages) const
{
    transfer(addr, messages.begin(), messages.end());
}


template <class... Messages>
void I2C::transfer(uint16_t addr, Messages&&... messages) const
{
    transfer(addr, { messages... });
}


void I2C::transfer(uint16_t addr, Message& message) const
{
    transfer(addr, { message } );
}


std::string I2C::toString() const
{
    return "I2C (" + path_ + ")";
}

} // ... namespace periphery ...
