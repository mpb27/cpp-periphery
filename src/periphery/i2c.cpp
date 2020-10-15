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

#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>


#include "periphery/i2c.hpp"

namespace periphery {

//class ReadMessage : public I2C::Message {};
//class WriteMessage : public I2C::Message {};

I2C::I2C(const std::string& path)
    : m_path(path)
{
    // ... open device ...
    m_fd = open(path.c_str(), O_RDWR);
    if (m_fd < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }

    // ... query supported functions ...
    unsigned long supported_funcs;
    if (ioctl(m_fd, I2C_FUNCS, &supported_funcs) < 0) {
        auto e = std::system_error(EFAULT, std::system_category());
        close(m_fd);
        throw e;
    }

    // ... check that this device has I2C function ...
    if (!(supported_funcs & I2C_FUNC_I2C)) {
        close(m_fd);
        throw std::runtime_error("I2C not supported on ???");
    }
}


I2C::~I2C()
{
    // ... we can report an error, but should never throw from destructor ...
    if (close(m_fd) < 0) {
        // ... do nothing ...
    }
}


I2C::Message I2C::Message::write(std::initializer_list<uint8_t> tx_data)
{
    Message msg;
    msg.flags = 0;
    msg.data = std::vector<uint8_t>(tx_data);
    return msg;
}


I2C::Message I2C::Message::write(const std::vector<uint8_t>& tx_data)
{
    Message msg;
    msg.flags = 0;
    msg.data = std::vector<uint8_t>(tx_data);
    return msg;
}


I2C::Message I2C::Message::read(size_t len)
{
    Message msg;
    msg.flags = I2C_M_RD;
    msg.data = std::vector<uint8_t>(len);
    return msg;
}


void I2C::transfer(uint16_t addr, std::initializer_list<std::reference_wrapper<Message>> messages) const
{
    transfer_impl(addr, messages.begin(), messages.end());
}


void I2C::transfer(uint16_t addr, std::vector<std::reference_wrapper<Message>>& messages) const
{
    transfer_impl(addr, messages.begin(), messages.end());
}


template <typename ForwardIt>
void I2C::transfer_impl(uint16_t addr, ForwardIt first, ForwardIt last) const
{
    // ... create i2c_msg structure needed by linux call ...
    unsigned int count = last - first;
    std::unique_ptr<i2c_msg[]> p(new i2c_msg[count]);
    for (size_t i = 0 ; i < count; ++i) {
        auto& m = (Message&) *first++;
        p[i].addr  = addr;
        p[i].flags = m.flags;
        p[i].len   = (uint16_t) m.data.size();
        p[i].buf   = m.data.data();
    }

    // ... create transfer descriptor ...
    i2c_rdwr_ioctl_data i2c_rdwr_data { p.get(), count };

    int error = ioctl(m_fd, I2C_RDWR, &i2c_rdwr_data);
    if (error < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }
}


std::string I2C::toString() const
{
    return "I2C (" + m_path + ")";
}

} // ... namespace periphery ...
